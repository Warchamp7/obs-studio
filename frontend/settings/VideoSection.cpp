/******************************************************************************
 Copyright (C) 2025 by Warchamp7 <warchamp7@obsproject.com>
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include "VideoSection.hpp"

#include <QLineEdit>

#include <OBSApp.hpp>
#include <qt-wrappers.hpp>
#include <utility/BaseLexer.hpp>
#include <utility/platform.hpp>

#include <sstream>

// TODO: Remove dependency on OBSBasic
#include <widgets/OBSBasic.hpp>

#include "moc_VideoSection.cpp"

namespace {
std::tuple<int, int> aspect_ratio(int width, int height)
{
	int common = std::gcd(width, height);

	int newWidth = width / common;
	int newHeight = height / common;

	if (newWidth == 8 && newHeight == 5) {
		newWidth = 16;
		newHeight = 10;
	}

	return std::make_tuple(newWidth, newHeight);
}
bool resolutionTooHigh(uint32_t width, uint32_t height)
{
	return width > 16384 || height > 16384;
}

bool resolutionTooLow(uint32_t width, uint32_t height)
{
	return width < 32 || height < 32;
}
/* parses "[width]x[height]", string, i.e. 1024x768 */
bool validateResolutionString(std::string resolution, uint32_t &width, uint32_t &height)
{
	BaseLexer lex;
	base_token token;

	const char *characters = resolution.c_str();

	lexer_start(lex, characters);

	/* parse width */
	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (token.type != BASETOKEN_DIGIT)
		return false;

	width = std::stoul(token.text.array);

	/* parse 'x' */
	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (strref_cmpi(&token.text, "x") != 0)
		return false;

	/* parse height */
	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (token.type != BASETOKEN_DIGIT)
		return false;

	height = std::stoul(token.text.array);

	/* shouldn't be any more tokens after this */
	if (lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;

	if (resolutionTooHigh(width, height) || resolutionTooLow(width, height)) {
		width = height = 0;
		return false;
	}

	return true;
}
std::string resolutionString(int cx, int cy)
{
	std::stringstream resolution{};
	resolution << cx << "x" << cy;

	return resolution.str();
}
} // namespace

VideoSection::VideoSection(std::string section, std::string displayName) : AbstractSettingsSection(section, displayName)
{
}

VideoSection::~VideoSection() {}

QWidget *VideoSection::createSettingsPageWidget(QWidget *parent)
{
	VideoWidget *pageWidget = new VideoWidget(parent, this);
	setPageWidget(pageWidget);

	emit pageCreated();

	return pageWidget;
}

void VideoSection::registerSection()
{
	/* Profile Config */
	registerSetting("BaseCX", 1920, ConfigType::ProfileConfig);
	registerSetting("BaseCY", 1080, ConfigType::ProfileConfig);
	registerSetting("OutputCX", 1080, ConfigType::ProfileConfig);
	registerSetting("OutputCY", 1080, ConfigType::ProfileConfig);
	registerSetting("FPSCommon", "30", ConfigType::ProfileConfig);
	registerSetting("FPSType", 0, ConfigType::ProfileConfig);
	registerSetting("FPSInt", 30, ConfigType::ProfileConfig);
	registerSetting("FPSNum", 30, ConfigType::ProfileConfig);
	registerSetting("FPSDen", 1, ConfigType::ProfileConfig);
	registerSetting("ScaleType", "bicubic", ConfigType::ProfileConfig);

	/* Move to AdvancedSection??? */
	registerSetting("ColorFormat", "NV12", ConfigType::ProfileConfig);
	registerSetting("ColorSpace", "709", ConfigType::ProfileConfig);
	registerSetting("ColorRange", "Partial", ConfigType::ProfileConfig);
	registerSetting("SdrWhiteLevel", 300, ConfigType::ProfileConfig);
	registerSetting("HdrNominalPeakLevel", 1000, ConfigType::ProfileConfig);
}

void VideoSection::load(bool force)
{
	if (pageWidget()) {
	}
}

void VideoSection::save()
{
	AbstractSettingsSection::save();
}

VideoWidget::VideoWidget(QWidget *parent, VideoSection *section_)
	: AbstractSettingsPage(parent, section_),
	  ui(new Ui::VideoPage)
{
	ui->setupUi(this);

	QRegularExpression rx("\\d{1,5}x\\d{1,5}");
	QValidator *validator = new QRegularExpressionValidator(rx, this);
	ui->baseResolution->lineEdit()->setValidator(validator);
	ui->outputResolution->lineEdit()->setValidator(validator);

	connect(&section(), &AbstractSettingsSection::pageActivated, this, &VideoWidget::handlePageActivated);
	connect(&section(), &AbstractSettingsSection::pageCreated, this, &VideoWidget::initialLoad);
}

VideoWidget::~VideoWidget() {}

void VideoWidget::initialLoad()
{
	loadFPSTypeOptions();
	loadResolutionLists();
	loadDownscaleFilters();

	section().connectSettingWidget("FPSType", ui->fpsType);
	connect(ui->fpsType, &QComboBox::currentIndexChanged, ui->fpsTypes, &QStackedWidget::setCurrentIndex);

	section().connectSettingWidget("FPSCommon", ui->fpsCommon);
	section().connectSettingWidget("FPSInt", ui->fpsInteger);
	section().connectSettingWidget("FPSNum", ui->fpsNumerator);
	section().connectSettingWidget("FPSDen", ui->fpsDenominator);
}

void VideoWidget::handlePageActivated(bool isActive)
{
	if (isActive) {
	}
}

VideoSection &VideoWidget::section()
{
	return *static_cast<VideoSection *>(section_);
}

static const std::vector<double> resolutionScaleValues = {1.0, 1.25, (1.0 / 0.75), 1.5,  (1.0 / 0.6), 1.75,
							  2.0, 2.25, 2.5,          2.75, 3.0};

void VideoWidget::loadResolutionLists()
{
	/* Base Resolution */
	ui->baseResolution->clear();

	int baseWidth = manager().currentValue("Video", "BaseCX").toInt();
	int baseHeight = manager().currentValue("Video", "BaseCY").toInt();

	auto addResolutionOption = [this](int cx, int cy) {
		QString res = QString::fromStdString(resolutionString(cx, cy));
		if (ui->baseResolution->findText(res) == -1) {
			ui->baseResolution->addItem(res);
		}
	};

	for (QScreen *screen : QGuiApplication::screens()) {
		uint32_t width = screen->size().width();
		uint32_t height = screen->size().height();

		// Calculate physical screen resolution based on the virtual screen resolution
		// They might differ if scaling is enabled, e.g. for HiDPI screens
		width = round(width * screen->devicePixelRatio());
		height = round(height * screen->devicePixelRatio());

		addResolutionOption(width, height);
	}

	addResolutionOption(1920, 1080);
	addResolutionOption(1280, 720);

	ui->baseResolution->lineEdit()->setText(resolutionString(baseWidth, baseHeight).c_str());

	/* Output Resolution */
	ui->outputResolution->clear();

	uint32_t outputWidth = manager().currentValue("Video", "OutputCX").toInt();
	uint32_t outputHeight = manager().currentValue("Video", "OutputCY").toInt();

	std::string outputResolution = resolutionString(outputWidth, outputHeight);

	std::string bestScale;
	int bestPixelDiff = 0x7FFFFFFF;
	for (int idx = 0; idx < resolutionScaleValues.size(); idx++) {
		double scale = resolutionScaleValues[idx];
		uint32_t downscaleWidth = uint32_t(double(baseWidth) / scale);
		uint32_t downscaleHeight = uint32_t(double(baseHeight) / scale);

		// Round to multiple of 2
		downscaleWidth &= 0xFFFFFFFC;
		downscaleHeight &= 0xFFFFFFFE;

		std::string downscaleString = resolutionString(downscaleWidth, downscaleHeight);
		ui->outputResolution->addItem(QString::fromStdString(downscaleString));

		/* always try to find the closest output resolution to the
		 * previously set output resolution */
		int newPixelCount = int(downscaleWidth * downscaleHeight);
		int oldPixelCount = int(outputWidth * outputHeight);
		int diff = abs(newPixelCount - oldPixelCount);

		if (diff < bestPixelDiff) {
			bestScale = downscaleString;
			bestPixelDiff = diff;
		}
	}

	/* Select best output */
	float baseAspect = float(baseWidth) / float(baseHeight);
	float outputAspect = float(outputWidth) / float(outputHeight);
	bool closeAspect = close_float(baseAspect, outputAspect, 0.01f);

	if (closeAspect) {
		ui->outputResolution->lineEdit()->setText(QString::fromStdString(outputResolution));
	} else {
		ui->outputResolution->lineEdit()->setText(QString::fromStdString(bestScale));
	}

	/* Update aspect ratio labels */
	std::tuple<int, int> baseRatio = aspect_ratio(baseWidth, baseHeight);
	int ratioWidth = std::get<0>(baseRatio);
	int ratioHeight = std::get<1>(baseRatio);

	ui->baseAspect->setText(QTStr("AspectRatio").arg(QString::number(ratioWidth), QString::number(ratioHeight)));

	validateResolutionString(outputResolution, outputWidth, outputHeight);
	if (outputWidth && outputHeight) {
		std::tuple<int, int> aspect = aspect_ratio(outputWidth, outputHeight);
		ratioWidth = std::get<0>(aspect);
		ratioHeight = std::get<1>(aspect);

		ui->scaledAspect->setText(
			QTStr("AspectRatio").arg(QString::number(ratioWidth), QString::number(ratioHeight)));
	}
}

void VideoWidget::loadDownscaleFilters()
{
	ui->downscaleFilter->clear();

	QString downscaleFilter = manager().currentValue("Video", "ScaleType").toString();

	ui->downscaleFilter->addItem(QTStr("Basic.Settings.Video.DownscaleFilter.Bilinear"), QT_UTF8("bilinear"));
	ui->downscaleFilter->addItem(QTStr("Basic.Settings.Video.DownscaleFilter.Area"), QT_UTF8("area"));
	ui->downscaleFilter->addItem(QTStr("Basic.Settings.Video.DownscaleFilter.Bicubic"), QT_UTF8("bicubic"));
	ui->downscaleFilter->addItem(QTStr("Basic.Settings.Video.DownscaleFilter.Lanczos"), QT_UTF8("lanczos"));

	if (downscaleFilter == "bilinear") {
		ui->downscaleFilter->setCurrentIndex(0);
	} else if (downscaleFilter == "lanczos") {
		ui->downscaleFilter->setCurrentIndex(3);
	} else if (downscaleFilter == "area") {
		ui->downscaleFilter->setCurrentIndex(1);
	} else {
		ui->downscaleFilter->setCurrentIndex(2);
	}
}

void VideoWidget::loadFPSTypeOptions()
{
	ui->fpsType->clear();
	ui->fpsType->addItem(QTStr("Basic.Settings.Video.FPSCommon"), 0);
	ui->fpsType->addItem(QTStr("Basic.Settings.Video.FPSInteger"), 1);
	ui->fpsType->addItem(QTStr("Basic.Settings.Video.FPSFraction"), 2);
}

void VideoWidget::resetDownscales(uint32_t cx, uint32_t cy, bool ignoreAllSignals)
{
	OBSBasic *main = OBSBasic::Get();
	/*
	
	QString oldOutputRes;
	std::string bestScale;
	int bestPixelDiff = 0x7FFFFFFF;
	uint32_t out_cx = outputCX;
	uint32_t out_cy = outputCY;

	advRescale = ui->advOutRescale->lineEdit()->text();
	advRecRescale = ui->advOutRecRescale->lineEdit()->text();
	advFFRescale = ui->advOutFFRescale->lineEdit()->text();

	bool lockedOutputRes = !ui->outputResolution->isEditable();

	if (!lockedOutputRes) {
		ui->outputResolution->blockSignals(true);
		ui->outputResolution->clear();
	}
	if (ignoreAllSignals) {
		ui->advOutRescale->blockSignals(true);
		ui->advOutRecRescale->blockSignals(true);
		ui->advOutFFRescale->blockSignals(true);
	}
	ui->advOutRescale->clear();
	ui->advOutRecRescale->clear();
	ui->advOutFFRescale->clear();

	if (!out_cx || !out_cy) {
		out_cx = cx;
		out_cy = cy;
		oldOutputRes = ui->baseResolution->lineEdit()->text();
	} else {
		oldOutputRes = QString::number(out_cx) + "x" + QString::number(out_cy);
	}

	for (size_t idx = 0; idx < numVals; idx++) {
		uint32_t downscaleCX = uint32_t(double(cx) / vals[idx]);
		uint32_t downscaleCY = uint32_t(double(cy) / vals[idx]);
		uint32_t outDownscaleCX = uint32_t(double(out_cx) / vals[idx]);
		uint32_t outDownscaleCY = uint32_t(double(out_cy) / vals[idx]);

		downscaleCX &= 0xFFFFFFFC;
		downscaleCY &= 0xFFFFFFFE;
		outDownscaleCX &= 0xFFFFFFFE;
		outDownscaleCY &= 0xFFFFFFFE;

		string res = ResString(downscaleCX, downscaleCY);
		string outRes = ResString(outDownscaleCX, outDownscaleCY);
		if (!lockedOutputRes)
			ui->outputResolution->addItem(res.c_str());
		ui->advOutRescale->addItem(outRes.c_str());
		ui->advOutRecRescale->addItem(outRes.c_str());
		ui->advOutFFRescale->addItem(outRes.c_str());
*/
	/* always try to find the closest output resolution to the
		 * previously set output resolution */
	/*
		int newPixelCount = int(downscaleCX * downscaleCY);
		int oldPixelCount = int(out_cx * out_cy);
		int diff = abs(newPixelCount - oldPixelCount);

		if (diff < bestPixelDiff) {
			bestScale = res;
			bestPixelDiff = diff;
		}
	}

	string res = ResString(cx, cy);

	if (!lockedOutputRes) {
		float baseAspect = float(cx) / float(cy);
		float outputAspect = float(out_cx) / float(out_cy);
		bool closeAspect = close_float(baseAspect, outputAspect, 0.01f);

		if (closeAspect) {
			ui->outputResolution->lineEdit()->setText(oldOutputRes);
			on_outputResolution_editTextChanged(oldOutputRes);
		} else {
			ui->outputResolution->lineEdit()->setText(bestScale.c_str());
			on_outputResolution_editTextChanged(bestScale.c_str());
		}

		ui->outputResolution->blockSignals(false);

		if (!closeAspect) {
			ui->outputResolution->setProperty("changed", QVariant(true));
			videoChanged = true;
		}
	}

	if (advRescale.isEmpty())
		advRescale = res.c_str();
	if (advRecRescale.isEmpty())
		advRecRescale = res.c_str();
	if (advFFRescale.isEmpty())
		advFFRescale = res.c_str();

	ui->advOutRescale->lineEdit()->setText(advRescale);
	ui->advOutRecRescale->lineEdit()->setText(advRecRescale);
	ui->advOutFFRescale->lineEdit()->setText(advFFRescale);

	if (ignoreAllSignals) {
		ui->advOutRescale->blockSignals(false);
		ui->advOutRecRescale->blockSignals(false);
		ui->advOutFFRescale->blockSignals(false);
	}
*/
}

void VideoWidget::updateOutputAspectRatioText()
{
	/*
	uint32_t newCX;
	uint32_t newCY;

	if (ConvertResText(resText, newCX, newCY) && newCX && newCY) {
		outputCX = newCX;
		outputCY = newCY;

		std::tuple<int, int> aspect = aspect_ratio(outputCX, outputCY);

		ui->scaledAspect->setText(
			QTStr("AspectRatio")
				.arg(QString::number(std::get<0>(aspect)), QString::number(std::get<1>(aspect))));
	}
*/
}
