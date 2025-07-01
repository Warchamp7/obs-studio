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

#include "AppearanceSection.hpp"

#include <OBSApp.hpp>
#include <utility/platform.hpp>

#include <qt-wrappers.hpp>

#include "moc_AppearanceSection.cpp"

AppearanceSection::AppearanceSection(std::string section, std::string displayName)
	: AbstractSettingsSection(section, displayName)
{
}

AppearanceSection::~AppearanceSection() {}

void AppearanceSection::registerSection()
{
	registerSetting("Theme", "com.obsproject.Yami.Default");
	registerSetting("FontScale", 10);
	registerSetting("Density", -4);
	registerSetting("AutoReload", false);
}

QWidget *AppearanceSection::createSettingsPageWidget(QWidget *parent)
{
	AppearanceWidget *pageWidget = new AppearanceWidget(parent, this);
	setPageWidget(pageWidget);

	emit pageCreated();

	return pageWidget;
}

void AppearanceSection::load(bool force)
{
	if (pageWidget()) {
	}
}

void AppearanceSection::save()
{
	AbstractSettingsSection::save();
}

AppearanceWidget::AppearanceWidget(QWidget *parent, AppearanceSection *section_)
	: AbstractSettingsPage(parent, section_),
	  ui(new Ui::AppearancePage)
{
	ui->setupUi(this);

	ui->themeVariant->setPlaceholderText(QTStr("Basic.Settings.Appearance.General.NoVariant"));

	ui->appearanceFontScale->setDisplayTicks(true);

	connect(ui->appearanceFontScale, &QSlider::valueChanged, ui->appearanceFontScaleText,
		[this](int value) { ui->appearanceFontScaleText->setText(QString::number(value)); });
	ui->appearanceFontScaleText->setText(QString::number(ui->appearanceFontScale->value()));

	connect(&section(), &AbstractSettingsSection::pageActivated, this, &AppearanceWidget::handlePageActivated);
	connect(&section(), &AbstractSettingsSection::pageCreated, this, &AppearanceWidget::initialLoad);
	connect(&section(), &AbstractSettingsSection::pageSaved, this, &AppearanceWidget::reloadTheme);
}

AppearanceWidget::~AppearanceWidget()
{
	SettingsItem *theme = section().findSettingsItem("Theme");
	if (theme) {
		QVariant savedTheme = manager().getValue(theme);

		// If theme was changed but not saved, revert theme
		if (theme->pendingValue() != savedTheme) {
			App()->SetTheme(savedTheme.toString());
		}
	}
}

void AppearanceWidget::initialLoad()
{
	loadThemeList();
	loadVariantList();

	connect(ui->theme, &QComboBox::currentIndexChanged, this, [this](int index) { loadVariantList(); });
	SettingsItem *theme = section().findSettingsItem("Theme");
	if (theme) {
		connect(ui->themeVariant, &QComboBox::activated, this, [this, theme](int index) {
			theme->setPending(getSelectedTheme());

			reloadTheme();
		});
	}

	SettingsItem *fontScale = section().connectSettingWidget("FontScale", ui->appearanceFontScale);

	SettingsItem *density = section().findSettingsItem("Density");
	if (density) {
		QVariant densityId = manager().getValue(density);

		QAbstractButton *densityButton = ui->appearanceDensityButtonGroup->button(densityId.toInt());
		if (densityButton) {
			densityButton->setChecked(true);
		}

		connect(ui->appearanceDensityButtonGroup, &QButtonGroup::buttonClicked, this,
			[this, density]() { density->setPending(ui->appearanceDensityButtonGroup->checkedId()); });
	}

	connect(App(), &OBSApp::StyleChanged, this, &AppearanceWidget::updateAppearanceControls);
	updateAppearanceControls();
}

void AppearanceWidget::handlePageActivated(bool isActive)
{
	if (isActive) {
	}
}

void AppearanceWidget::loadThemeList()
{
	ProfileScope("OBSBasicSettings::LoadThemeList");

	const OBSTheme *currentTheme = App()->GetTheme();
	const QString currentBaseTheme = currentTheme->isBaseTheme ? currentTheme->id : currentTheme->parent;

	for (const OBSTheme &theme : App()->GetThemes()) {
		if (theme.isBaseTheme && (HighContrastEnabled() || theme.isVisible || theme.id == currentBaseTheme)) {
			ui->theme->addItem(theme.name, theme.id);
		}
	}

	int idx = ui->theme->findData(currentBaseTheme);
	if (idx != -1) {
		ui->theme->setCurrentIndex(idx);
	}
}

void AppearanceWidget::loadVariantList()
{
	const OBSTheme *currentTheme = App()->GetTheme();
	const QString baseThemeId = ui->theme->currentData().toString();

	ui->themeVariant->blockSignals(true);
	ui->themeVariant->clear();

	auto themes = App()->GetThemes();
	std::sort(themes.begin(), themes.end(), [](const OBSTheme &a, const OBSTheme &b) -> bool {
		return QString::compare(a.name, b.name, Qt::CaseInsensitive) < 0;
	});

	QString defaultVariant;
	const OBSTheme *baseTheme = App()->GetTheme(baseThemeId);

	for (const OBSTheme &theme : themes) {
		/* Skip non-visible themes */
		if (!theme.isVisible || theme.isHighContrast)
			continue;
		/* Skip non-child themes */
		if (theme.isBaseTheme || theme.parent != baseThemeId)
			continue;

		ui->themeVariant->addItem(theme.name, theme.id);
		if (baseTheme && theme.filename == baseTheme->filename)
			defaultVariant = theme.id;
	}

	int idx = ui->themeVariant->findData(currentTheme->id);
	if (idx != -1) {
		ui->themeVariant->setCurrentIndex(idx);
	}

	ui->themeVariant->setEnabled(ui->themeVariant->count() > 0);
	ui->themeVariant->blockSignals(false);

	/* If no variant is selected but variants are available set the first one. */
	if (idx == -1 && ui->themeVariant->count() > 0) {
		idx = ui->themeVariant->findData(defaultVariant);
		ui->themeVariant->setCurrentIndex(idx != -1 ? idx : 0);
	}
}

QString AppearanceWidget::getSelectedTheme()
{
	QString themeId = ui->theme->currentData().toString();

	if (ui->themeVariant->currentIndex() != -1) {
		themeId = ui->themeVariant->currentData().toString();
	}

	return themeId;
}

void AppearanceWidget::reloadTheme()
{
	QString themeId = getSelectedTheme();
	App()->SetTheme(themeId);
}

void AppearanceWidget::updateAppearanceControls()
{
	OBSTheme *theme = App()->GetTheme();
	enableAppearanceFontControls(theme->usesFontScale);
	enableAppearanceDensityControls(theme->usesDensity);
	if (!theme->usesFontScale || !theme->usesDensity) {
		ui->appearanceOptionsWarning->setVisible(true);
	} else {
		ui->appearanceOptionsWarning->setVisible(false);
	}
	style()->polish(ui->appearanceOptionsWarningLabel);
}

void AppearanceWidget::enableAppearanceFontControls(bool enable)
{
	ui->appearanceFontScale->setEnabled(enable);
	ui->appearanceFontScaleText->setEnabled(enable);
}

void AppearanceWidget::enableAppearanceDensityControls(bool enable)
{
	const QList<QAbstractButton *> buttons = ui->appearanceDensityButtonGroup->buttons();
	for (QAbstractButton *button : buttons) {
		button->setEnabled(enable);
	}
}
