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

#include "GeneralSection.hpp"

#include <QCheckBox>
#include <QStandardItemModel>

#include <components/Multiview.hpp>
#include <settings/SettingsItemObserver.hpp>
#include <utility/platform.hpp>
#include <widgets/OBSProjector.hpp>

#include <qt-wrappers.hpp>

// TODO: Remove dependency on OBSBasic
#include <widgets/OBSBasic.hpp>

#include "moc_GeneralSection.cpp"

namespace {
#if defined(_WIN32) || defined(ENABLE_SPARKLE_UPDATER)
void translateBranchInfo(const QString &name, QString &displayName, QString &description)
{
	QString translatedName = QTStr("Basic.Settings.General.ChannelName." + name.toUtf8());
	QString translatedDesc = QTStr("Basic.Settings.General.ChannelDescription." + name.toUtf8());

	if (!translatedName.startsWith("Basic.Settings."))
		displayName = translatedName;
	if (!translatedDesc.startsWith("Basic.Settings."))
		description = translatedDesc;
}
#endif
bool isReplayBufferEnabled(SettingsManager &manager)
{
	bool replayBufferEnabled = false;

	QVariant outputMode = manager.currentValue("Output", "Mode");
	bool simpleBufferEnabled = manager.currentValue("SimpleOutput", "RecRB").toBool();
	bool advancedBufferEnabled = manager.currentValue("AdvOut", "RecRB").toBool();

	if (outputMode == "Simple") {
		replayBufferEnabled = simpleBufferEnabled;
	} else if (outputMode == "Advanced") {
		replayBufferEnabled = advancedBufferEnabled;
	}

	return replayBufferEnabled;
}
} // namespace

#define DEFAULT_LANG "en-US"

GeneralSection::GeneralSection(std::string section, std::string displayName)
	: AbstractSettingsSection(section, displayName)
{
}

GeneralSection::~GeneralSection() {}

QWidget *GeneralSection::createSettingsPageWidget(QWidget *parent)
{
	GeneralWidget *pageWidget = new GeneralWidget(parent, this);
	setPageWidget(pageWidget);

	emit pageCreated();

	return pageWidget;
}

void GeneralSection::registerSection()
{
	/* Global Config */
	/* -- General */
	registerSetting("MaxLogs", 10, ConfigType::AppConfig);
	registerSetting("InfoIncrement", -1, ConfigType::AppConfig);
	registerSetting("ProcessPriority", "Normal", ConfigType::AppConfig);
	registerSetting("EnableAutoUpdates", true, ConfigType::AppConfig);
	registerSetting("BrowserHWAccel", true, ConfigType::AppConfig);
	registerSetting("LastUpdateCheck", 0, ConfigType::AppConfig);
	registerSetting("InstallGUID", 0, ConfigType::AppConfig);
	registerSetting("SkipUpdateVersion", 0, ConfigType::AppConfig);
	registerSetting("LastVersion", 0, ConfigType::AppConfig);

	/* User Config */
	/* -- BasicWindow */
	auto hide = manager().registerSetting("BasicWindow", "HideOBSWindowsFromCapture", false);
	hide->setRestartWhenChanged(true);

	manager().registerSetting("BasicWindow", "WarnBeforeStartingStream", false);
	manager().registerSetting("BasicWindow", "WarnBeforeStoppingStream", false);
	manager().registerSetting("BasicWindow", "WarnBeforeStoppingRecord", false);
	manager().registerSetting("BasicWindow", "RecordWhenStreaming", false);
	manager().registerSetting("BasicWindow", "KeepRecordingWhenStreamStops", false);
	manager().registerSetting("BasicWindow", "ReplayBufferWhileStreaming", false);
	manager().registerSetting("BasicWindow", "KeepReplayBufferStreamStops", false);

	manager().registerSetting("BasicWindow", "SnappingEnabled", true);
	manager().registerSetting("BasicWindow", "SnapDistance", 10.0);
	manager().registerSetting("BasicWindow", "ScreenSnapping", true);
	manager().registerSetting("BasicWindow", "SourceSnapping", true);
	manager().registerSetting("BasicWindow", "CenterSnapping", true);

	manager().registerSetting("BasicWindow", "HideProjectorCursor", false);
	manager().registerSetting("BasicWindow", "ProjectorAlwaysOnTop", false);
	manager().registerSetting("BasicWindow", "SaveProjectors", false);
	manager().registerSetting("BasicWindow", "CloseExistingProjectors", false);

	manager().registerSetting("BasicWindow", "SysTrayEnabled", true);
	manager().registerSetting("BasicWindow", "SysTrayWhenStarted", false);
	manager().registerSetting("BasicWindow", "SysTrayMinimizeToTray", false);

	manager().registerSetting("BasicWindow", "OverflowHidden", false);
	manager().registerSetting("BasicWindow", "OverflowAlwaysVisible", false);
	manager().registerSetting("BasicWindow", "OverflowSelectionHidden", false);
	manager().registerSetting("BasicWindow", "ShowSafeAreas", false);
	manager().registerSetting("BasicWindow", "SpacingHelpersEnabled", true);

	manager().registerSetting("BasicWindow", "TransitionOnDoubleClick", false);
	manager().registerSetting("BasicWindow", "StudioPortraitLayout", false);
	manager().registerSetting("BasicWindow", "StudioModeLabels", true);

	manager().registerSetting("BasicWindow", "MultiviewMouseSwitch", true);
	manager().registerSetting("BasicWindow", "MultiviewDrawNames", true);
	manager().registerSetting("BasicWindow", "MultiviewDrawAreas", true);

	manager().registerSetting("BasicWindow", "PreviewEnabled", true);
	manager().registerSetting("BasicWindow", "PreviewProgramMode", false);
	manager().registerSetting("BasicWindow", "SceneDuplicationMode", true);
	manager().registerSetting("BasicWindow", "SwapScenesMode", true);
	manager().registerSetting("BasicWindow", "AlwaysOnTop", false);
	manager().registerSetting("BasicWindow", "ShowTransitions", true);
	manager().registerSetting("BasicWindow", "ShowListboxToolbars", true);
	manager().registerSetting("BasicWindow", "ShowStatusBar", true);
	manager().registerSetting("BasicWindow", "ShowSourceIcons", true);
	manager().registerSetting("BasicWindow", "ShowContextToolbars", true);
	manager().registerSetting("BasicWindow", "StudioModeLabels", true);
	manager().registerSetting("BasicWindow", "VerticalVolControl", true);
	manager().registerSetting("BasicWindow", "DocksLocked", false);
	manager().registerSetting("BasicWindow", "SideDocks", true);
	manager().registerSetting("BasicWindow", "MediaControlsCountdownTimer", true);

	manager().registerSetting("BasicWindow", "geometry", "");
	manager().registerSetting("BasicWindow", "DockState", "");
	manager().registerSetting("BasicWindow", "ExtraBrowserDocks", "[]");
	manager().registerSetting("BasicWindow", "EditPropertiesMode", false);

	manager().registerSetting("BasicWindow", "AlwaysOnTop", true);

	auto lang = registerSetting("Language", DEFAULT_LANG);
	lang->setRestartWhenChanged(true);

	registerSetting("UpdateBranch", "stable");
	registerSetting("EnableAutoUpdates", true);

	registerSetting("AutomaticCollectionSearch", true);

	/* Profile Config */
	registerSetting("Name", "Untitled", ConfigType::ProfileConfig);
	registerSetting("OpenStatsOnStartup", false, ConfigType::ProfileConfig);
}

void GeneralSection::load(bool force)
{
	if (pageWidget()) {
		// pageWidget()->loadLegacy();
	}
}

void GeneralSection::save()
{
	AbstractSettingsSection::save();
}

GeneralWidget::GeneralWidget(QWidget *parent, GeneralSection *section_)
	: AbstractSettingsPage(parent, section_),
	  ui(new Ui::GeneralPage)
{
	ui->setupUi(this);

#if !defined(_WIN32) && !defined(ENABLE_SPARKLE_UPDATER)
	delete ui->updateSettingsGroupBox;
	ui->updateSettingsGroupBox = nullptr;
	ui->updateChannelLabel = nullptr;
	ui->updateChannelBox = nullptr;
	ui->enableAutoUpdates = nullptr;
#else
	// Hide update section if disabled
	if (App()->IsUpdaterDisabled()) {
		ui->updateSettingsGroupBox->hide();
	}
#endif

#ifdef _WIN32
	if (!SetDisplayAffinitySupported()) {
		delete ui->hideOBSFromCapture;
		ui->hideOBSFromCapture = nullptr;
	}
#else
	delete ui->hideOBSFromCapture;
	ui->hideOBSFromCapture = nullptr;
#endif

	connect(&section(), &AbstractSettingsSection::pageActivated, this, &GeneralWidget::handlePageActivated);
	connect(&section(), &AbstractSettingsSection::pageCreated, this, &GeneralWidget::initialLoad);
}

GeneralWidget::~GeneralWidget()
{
	OBSBasic *main = OBSBasic::Get();

	if (forceUpdateCheck) {
		main->CheckForUpdates(false);
		forceUpdateCheck = false;
	}

	disconnect(&section(), &AbstractSettingsSection::pageActivated, this, &GeneralWidget::handlePageActivated);
	disconnect(&section(), &AbstractSettingsSection::pageCreated, this, &GeneralWidget::initialLoad);
}

void GeneralWidget::initialLoad()
{
	loadLanguageList();
	loadMultiviewList();

	// Connect Settings Widgets
	SettingsItem *language = section().connectSettingWidget("Language", ui->language);

#if defined(_WIN32) || defined(ENABLE_SPARKLE_UPDATER)
	loadBranchesList();
	section().connectSettingWidget("EnableAutoUpdates", ui->enableAutoUpdates);
#endif

	section().connectSettingWidget("OpenStatsOnStartup", ui->openStatsOnStartup);

	section().connectSettingWidget("UpdateBranch", ui->updateChannelBox);

	section().connectSettingWidget("AutomaticCollectionSearch", ui->automaticSearch);

	// BasicWindow
	AbstractSettingsSection *basicWindow = manager().findSection("BasicWindow");
	if (basicWindow) {
		if (ui->hideOBSFromCapture) {
			basicWindow->connectSettingWidget("HideOBSWindowsFromCapture", ui->hideOBSFromCapture);
		}

		// -----
		basicWindow->connectSettingWidget("WarnBeforeStartingStream", ui->warnBeforeStreamStart);
		basicWindow->connectSettingWidget("WarnBeforeStoppingStream", ui->warnBeforeStreamStop);
		basicWindow->connectSettingWidget("WarnBeforeStoppingRecord", ui->warnBeforeRecordStop);
		basicWindow->connectSettingWidget("RecordWhenStreaming", ui->recordWhenStreaming);
		basicWindow->connectSettingWidget("KeepRecordingWhenStreamStops", ui->keepRecordStreamStops);

		auto toggleRecordStreaming = [this]() {
			bool checked = manager().currentValue("BasicWindow", "RecordWhenStreaming").toBool();
			ui->keepRecordStreamStops->setEnabled(checked);
			return checked;
		};
		bindChangeCallback("BasicWindow", "RecordWhenStreaming", toggleRecordStreaming);
		toggleRecordStreaming();

		auto toggleBufferAutoStart = [this]() {
			bool replayBufferEnabled = isReplayBufferEnabled(manager());
			ui->replayWhileStreaming->setEnabled(replayBufferEnabled);
		};
		bindChangeCallback("Output", "Mode", toggleBufferAutoStart);
		bindChangeCallback("SimpleOutput", "RecRB", toggleBufferAutoStart);
		bindChangeCallback("AdvOut", "RecRB", toggleBufferAutoStart);
		toggleBufferAutoStart();

		auto toggleKeepBufferActive = [this]() {
			bool replayBufferEnabled = isReplayBufferEnabled(manager());
			bool replayWhileStreaming =
				manager().currentValue("BasicWindow", "ReplayBufferWhileStreaming").toBool();
			bool enable = replayBufferEnabled && replayWhileStreaming;

			ui->keepReplayStreamStops->setEnabled(enable);
			return enable;
		};
		bindChangeCallback("Output", "Mode", toggleKeepBufferActive);
		bindChangeCallback("SimpleOutput", "RecRB", toggleKeepBufferActive);
		bindChangeCallback("AdvOut", "RecRB", toggleKeepBufferActive);
		bindChangeCallback("BasicWindow", "ReplayBufferWhileStreaming", toggleKeepBufferActive);
		toggleKeepBufferActive();

		basicWindow->connectSettingWidget("ReplayBufferWhileStreaming", ui->replayWhileStreaming);
		basicWindow->connectSettingWidget("KeepReplayBufferStreamStops", ui->keepReplayStreamStops);

		// -----
		basicWindow->connectSettingWidget("SnappingEnabled", ui->snappingEnabled);
		auto toggleSnapSettings = [this]() {
			bool snappingEnabled = manager().currentValue("BasicWindow", "SnappingEnabled").toBool();
			ui->snapDistance->setEnabled(snappingEnabled);
			ui->screenSnapping->setEnabled(snappingEnabled);
			ui->sourceSnapping->setEnabled(snappingEnabled);
			ui->centerSnapping->setEnabled(snappingEnabled);
		};
		bindChangeCallback("BasicWindow", "SnappingEnabled", toggleSnapSettings);
		toggleSnapSettings();
		basicWindow->connectSettingWidget("SnapDistance", ui->snapDistance);
		basicWindow->connectSettingWidget("ScreenSnapping", ui->screenSnapping);
		basicWindow->connectSettingWidget("SourceSnapping", ui->sourceSnapping);
		basicWindow->connectSettingWidget("CenterSnapping", ui->centerSnapping);

		// -----
		basicWindow->connectSettingWidget("HideProjectorCursor", ui->hideProjectorCursor);
		basicWindow->connectSettingWidget("ProjectorAlwaysOnTop", ui->projectorAlwaysOnTop);
		basicWindow->connectSettingWidget("SaveProjectors", ui->saveProjectors);
		basicWindow->connectSettingWidget("CloseExistingProjectors", ui->closeProjectors);

		// -----
		basicWindow->connectSettingWidget("SysTrayEnabled", ui->systemTrayEnabled);
		basicWindow->connectSettingWidget("SysTrayWhenStarted", ui->systemTrayWhenStarted);
		basicWindow->connectSettingWidget("SysTrayMinimizeToTray", ui->systemTrayAlways);
		auto toggleSysTraySettings = [this]() {
			bool trayEnabled = manager().currentValue("BasicWindow", "SysTrayEnabled").toBool();
			ui->systemTrayWhenStarted->setEnabled(trayEnabled);
			ui->systemTrayAlways->setEnabled(trayEnabled);
		};
		bindChangeCallback("BasicWindow", "SysTrayEnabled", toggleSysTraySettings);
		toggleSysTraySettings();

		// -----
		basicWindow->connectSettingWidget("OverflowHidden", ui->overflowHide);
		basicWindow->connectSettingWidget("OverflowAlwaysVisible", ui->overflowAlwaysVisible);
		basicWindow->connectSettingWidget("OverflowSelectionHidden", ui->overflowSelectionHide);
		basicWindow->connectSettingWidget("ShowSafeAreas", ui->previewSafeAreas);
		basicWindow->connectSettingWidget("SpacingHelpersEnabled", ui->previewSpacingHelpers);

		// -----
		basicWindow->connectSettingWidget("TransitionOnDoubleClick", ui->doubleClickSwitch);
		basicWindow->connectSettingWidget("StudioPortraitLayout", ui->studioPortraitLayout);
		basicWindow->connectSettingWidget("StudioModeLabels", ui->prevProgLabelToggle);

		// -----
		basicWindow->connectSettingWidget("MultiviewMouseSwitch", ui->multiviewMouseSwitch);
		basicWindow->connectSettingWidget("MultiviewDrawNames", ui->multiviewDrawNames);
		basicWindow->connectSettingWidget("MultiviewDrawAreas", ui->multiviewDrawAreas);
		basicWindow->connectSettingWidget("MultiviewLayout", ui->multiviewLayout);

		/* The commented entries here are settings that do not have controls in the settings window
		 * TODO: Create widgets for these settings
		 */
		// basicWindow->connectSettingWidget("PreviewEnabled", );
		// basicWindow->connectSettingWidget("PreviewProgramMode", );
		// basicWindow->connectSettingWidget("SceneDuplicationMode", );
		// basicWindow->connectSettingWidget("SwapScenesMode", );
		// basicWindow->connectSettingWidget("ShowTransitions", );
		// basicWindow->connectSettingWidget("ShowListboxToolbars", );
		// basicWindow->connectSettingWidget("ShowStatusBar", );
		// basicWindow->connectSettingWidget("ShowSourceIcons", );
		// basicWindow->connectSettingWidget("ShowContextToolbars", );
		// basicWindow->connectSettingWidget("VerticalVolControl", );
		// basicWindow->connectSettingWidget("MediaControlsCountdownTimer", );
		// basicWindow->connectSettingWidget("geometry", );
		// basicWindow->connectSettingWidget("DockState", );
		// basicWindow->connectSettingWidget("ExtraBrowserDocks", );
		// basicWindow->connectSettingWidget("EditPropertiesMode", );
		// basicWindow->connectSettingWidget("DocksLocked", );
		// basicWindow->connectSettingWidget("SideDocks", );
	}
}

void GeneralWidget::handlePageActivated(bool isActive)
{
	if (isActive) {
	}
}

void GeneralWidget::loadLanguageList()
{
	const char *currentLang = App()->GetLocale();

	ui->language->clear();

	for (const auto &locale : GetLocaleNames()) {
		int idx = ui->language->count();

		ui->language->addItem(QT_UTF8(locale.second.c_str()), QT_UTF8(locale.first.c_str()));

		if (locale.first == currentLang)
			ui->language->setCurrentIndex(idx);
	}

	ui->language->model()->sort(0);
}

void GeneralWidget::loadBranchesList()
{
#if defined(_WIN32) || defined(ENABLE_SPARKLE_UPDATER)
	bool configBranchRemoved = true;
	QString configBranch = config_get_string(App()->GetAppConfig(), "General", "UpdateBranch");

	for (const UpdateBranch &branch : App()->GetBranches()) {
		if (branch.name == configBranch)
			configBranchRemoved = false;
		if (!branch.is_visible && branch.name != configBranch)
			continue;

		QString displayName = branch.display_name;
		QString description = branch.description;

		translateBranchInfo(branch.name, displayName, description);
		QString itemDesc = displayName + " - " + description;

		if (!branch.is_enabled) {
			itemDesc.prepend(" ");
			itemDesc.prepend(QTStr("Basic.Settings.General.UpdateChannelDisabled"));
		} else if (branch.name == "stable") {
			itemDesc.append(" ");
			itemDesc.append(QTStr("Basic.Settings.General.UpdateChannelDefault"));
		}

		ui->updateChannelBox->addItem(itemDesc, branch.name);

		// Disable item if branch is disabled
		if (!branch.is_enabled) {
			QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->updateChannelBox->model());
			QStandardItem *item = model->item(ui->updateChannelBox->count() - 1);
			item->setFlags(Qt::NoItemFlags);
		}
	}

	// Fall back to default if not yet set or user-selected branch has been removed
	if (configBranch.isEmpty() || configBranchRemoved)
		configBranch = "stable";

	int idx = ui->updateChannelBox->findData(configBranch);
	ui->updateChannelBox->setCurrentIndex(idx);
#endif
}

void GeneralWidget::loadMultiviewList()
{
	ui->multiviewLayout->addItem(QTStr("Basic.Settings.General.MultiviewLayout.Horizontal.Top"),
				     static_cast<int>(MultiviewLayout::HORIZONTAL_TOP_8_SCENES));
	ui->multiviewLayout->addItem(QTStr("Basic.Settings.General.MultiviewLayout.Horizontal.Bottom"),
				     static_cast<int>(MultiviewLayout::HORIZONTAL_BOTTOM_8_SCENES));
	ui->multiviewLayout->addItem(QTStr("Basic.Settings.General.MultiviewLayout.Vertical.Left"),
				     static_cast<int>(MultiviewLayout::VERTICAL_LEFT_8_SCENES));
	ui->multiviewLayout->addItem(QTStr("Basic.Settings.General.MultiviewLayout.Vertical.Right"),
				     static_cast<int>(MultiviewLayout::VERTICAL_RIGHT_8_SCENES));
	ui->multiviewLayout->addItem(QTStr("Basic.Settings.General.MultiviewLayout.Horizontal.18Scene.Top"),
				     static_cast<int>(MultiviewLayout::HORIZONTAL_TOP_18_SCENES));
	ui->multiviewLayout->addItem(QTStr("Basic.Settings.General.MultiviewLayout.Horizontal.Extended.Top"),
				     static_cast<int>(MultiviewLayout::HORIZONTAL_TOP_24_SCENES));
	ui->multiviewLayout->addItem(QTStr("Basic.Settings.General.MultiviewLayout.4Scene"),
				     static_cast<int>(MultiviewLayout::SCENES_ONLY_4_SCENES));
	ui->multiviewLayout->addItem(QTStr("Basic.Settings.General.MultiviewLayout.9Scene"),
				     static_cast<int>(MultiviewLayout::SCENES_ONLY_9_SCENES));
	ui->multiviewLayout->addItem(QTStr("Basic.Settings.General.MultiviewLayout.16Scene"),
				     static_cast<int>(MultiviewLayout::SCENES_ONLY_16_SCENES));
	ui->multiviewLayout->addItem(QTStr("Basic.Settings.General.MultiviewLayout.25Scene"),
				     static_cast<int>(MultiviewLayout::SCENES_ONLY_25_SCENES));
}

void GeneralWidget::hideOBSWindowWarning(bool checked)
{
	if (!checked)
		return;

	if (config_get_bool(App()->GetUserConfig(), "General", "WarnedAboutHideOBSFromCapture"))
		return;

	OBSMessageBox::information(this, QTStr("Basic.Settings.General.HideOBSWindowsFromCapture"),
				   QTStr("Basic.Settings.General.HideOBSWindowsFromCapture.Message"));

	config_set_bool(App()->GetUserConfig(), "General", "WarnedAboutHideOBSFromCapture", true);
	config_save_safe(App()->GetUserConfig(), "tmp", nullptr);
}
