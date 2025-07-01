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

#include "AudioSection.hpp"

#include <OBSApp.hpp>
#include <properties-view.hpp>
#include <qt-wrappers.hpp>
#include <components/OBSSourceLabel.hpp>
#include <components/SilentUpdateCheckBox.hpp>
#include <components/SilentUpdateSpinBox.hpp>
#include <utility/platform.hpp>

// TODO: Remove dependency on OBSBasic
#include <widgets/OBSBasic.hpp>

#include "moc_AudioSection.cpp"

#define VOLUME_METER_DECAY_FAST 23.53
#define VOLUME_METER_DECAY_MEDIUM 11.76
#define VOLUME_METER_DECAY_SLOW 8.57

namespace {
QVariant sampleRateToString(QVariant value)
{
	if (value == 48000) {
		return QVariant("48 kHz");
	} else if (value == 44100) {
		return QVariant("44.1 kHz");
	}

	return QVariant();
}

QVariant sampleRateToInt(QVariant value)
{
	if (value == "48 kHz") {
		return QVariant(48000);
	} else if (value == "44.1 kHz") {
		return QVariant(44100);
	}

	return QVariant();
}
} // namespace

AudioSection::AudioSection(std::string section, std::string displayName) : AbstractSettingsSection(section, displayName)
{
}

AudioSection::~AudioSection() {}

void AudioSection::registerSection()
{
	/* Global Config */
	registerSetting("DisableAudioDucking", true, ConfigType::AppConfig);

	/* User Config */
	auto lowLatency = registerSetting("LowLatencyAudioBuffering", false);
	lowLatency->setRestartWhenChanged(true);

	/* Profile Config */
	auto sample = registerSetting("SampleRate", (uint64_t)48000, ConfigType::ProfileConfig);
	sample->setRestartWhenChanged(true);
	auto channels = registerSetting("ChannelSetup", "Stereo", ConfigType::ProfileConfig);
	channels->setRestartWhenChanged(true);

	registerSetting("MeterDecayRate", VOLUME_METER_DECAY_FAST, ConfigType::ProfileConfig);
	registerSetting("PeakMeterType", (uint64_t)0, ConfigType::ProfileConfig);

	registerSetting("MonitoringDeviceName", "", ConfigType::ProfileConfig);
	registerSetting("MonitoringDeviceId", "", ConfigType::ProfileConfig);
}

QWidget *AudioSection::createSettingsPageWidget(QWidget *parent)
{
	AudioWidget *pageWidget = new AudioWidget(parent, this);
	setPageWidget(pageWidget);

	emit pageCreated();

	return pageWidget;
}

void AudioSection::load(bool force)
{
	if (pageWidget()) {
	}
}

void AudioSection::save()
{
	AbstractSettingsSection::save();
}

AudioWidget::AudioWidget(QWidget *parent, AudioSection *section_)
	: AbstractSettingsPage(parent, section_),
	  ui(new Ui::AudioPage)
{
	ui->setupUi(this);

	connect(&section(), &AbstractSettingsSection::pageActivated, this, &AudioWidget::handlePageActivated);
	connect(&section(), &AbstractSettingsSection::pageCreated, this, &AudioWidget::initialLoad);
	connect(&section(), &AbstractSettingsSection::pageSaved, this, &AudioWidget::save);
}

AudioWidget::~AudioWidget() {}

void AudioWidget::initialLoad()
{
	loadAudioDeviceOptions();
	loadMonitoringOptions();
	loadDecayRateOptions();
	loadPeakMeterOptions();

	auto sampleRate = section().findSettingsItem("SampleRate");
	if (sampleRate) {
		section().applyItemValue(sampleRate, ui->sampleRate, sampleRateToString);
		section().bindPendingValue(sampleRate, ui->sampleRate, sampleRateToInt);
		section().bindItemUpdates(sampleRate, ui->sampleRate, sampleRateToString);
	}

	section().connectSettingWidget("ChannelSetup", ui->channelSetup);
	section().connectSettingWidget("MeterDecayRate", ui->meterDecayRate);
	section().connectSettingWidget("PeakMeterType", ui->peakMeterType);

	/* The helper methods will use the itemData if it exists and this setting expects the actual text.
	 * Connect the pending value update for MonitoringDeviceName manually.
	 *
	 * Use the applyItemValue helper with MonitoringDeviceName so that an invalid selection on load
	 * will show a disabled entry with the device name, rather than the device id.
	 */
	auto monitoringDeviceName = section().findSettingsItem("MonitoringDeviceName");
	if (monitoringDeviceName) {
		section().applyItemValue(monitoringDeviceName, ui->monitoringDevice);
		connect(ui->monitoringDevice, &QComboBox::currentIndexChanged, monitoringDeviceName,
			[this, monitoringDeviceName](int index) {
				QVariant data = ui->monitoringDevice->itemText(index);

				if (data.isValid()) {
					monitoringDeviceName->setPending(data);
				}
			});
	}
	auto monitoringDeviceId = section().findSettingsItem("MonitoringDeviceId");
	if (monitoringDeviceId) {
		section().bindPendingValue(monitoringDeviceId, ui->monitoringDevice);
		section().bindItemUpdates(monitoringDeviceId, ui->monitoringDevice);
	}

	section().connectSettingWidget("DisableAudioDucking", ui->disableAudioDucking);
	section().connectSettingWidget("LowLatencyAudioBuffering", ui->lowLatencyBuffering);
}

void AudioWidget::handlePageActivated(bool isActive)
{
	if (isActive) {
	}
}

void AudioWidget::save()
{
	OBSBasic *main = OBSBasic::Get();

	/* TODO: Re-implement Push to Talk and Push to Mute for audio sources */

	/* TODO: Update OBSBasic to respond to SettingsManager signals instead */
	auto UpdateAudioDevice = [this, main](const char *input, QComboBox *combo, const char *name, int index) {
		QString comboValue;

		int idx = combo->currentIndex();
		if (idx != -1) {
			QVariant itemData = combo->itemData(idx);
			comboValue = combo->itemData(idx).toString();
		}

		main->ResetAudioDevice(input, comboValue.toUtf8().constData(), Str(name), index);
	};

	UpdateAudioDevice(App()->OutputAudioSource(), ui->desktopAudioDevice1, "Basic.DesktopDevice1", 1);
	UpdateAudioDevice(App()->OutputAudioSource(), ui->desktopAudioDevice2, "Basic.DesktopDevice2", 2);
	UpdateAudioDevice(App()->InputAudioSource(), ui->auxAudioDevice1, "Basic.AuxDevice1", 3);
	UpdateAudioDevice(App()->InputAudioSource(), ui->auxAudioDevice2, "Basic.AuxDevice2", 4);
	UpdateAudioDevice(App()->InputAudioSource(), ui->auxAudioDevice3, "Basic.AuxDevice3", 5);
	UpdateAudioDevice(App()->InputAudioSource(), ui->auxAudioDevice4, "Basic.AuxDevice4", 6);
}

AudioSection &AudioWidget::section()
{
	return *static_cast<AudioSection *>(section_);
}

void AudioWidget::loadListValues(QComboBox *widget, obs_property_t *prop, int index)
{
	size_t count = obs_property_list_item_count(prop);

	OBSSourceAutoRelease source = obs_get_output_source(index);
	const char *deviceId = nullptr;
	OBSDataAutoRelease settings = nullptr;

	if (source) {
		settings = obs_source_get_settings(source);
		if (settings)
			deviceId = obs_data_get_string(settings, "device_id");
	}

	widget->clear();
	widget->addItem(QTStr("Basic.Settings.Audio.Disabled"), "disabled");

	for (size_t i = 0; i < count; i++) {
		const char *name = obs_property_list_item_name(prop, i);
		const char *val = obs_property_list_item_string(prop, i);
		widget->addItem(QT_UTF8(name), QT_UTF8(val));
	}

	if (deviceId) {
		QVariant var(QT_UTF8(deviceId));
		int idx = widget->findData(var);
		if (idx != -1) {
			widget->setCurrentIndex(idx);
		} else {
			widget->insertItem(0,
					   QTStr("Basic.Settings.Audio."
						 "UnknownAudioDevice"),
					   var);
			widget->setCurrentIndex(0);
		}
	}
}

void AudioWidget::loadAudioDeviceOptions()
{
	const char *input_id = App()->InputAudioSource();
	const char *output_id = App()->OutputAudioSource();

	obs_properties_t *input_props = obs_get_source_properties(input_id);
	obs_properties_t *output_props = obs_get_source_properties(output_id);

	if (input_props) {
		obs_property_t *inputs = obs_properties_get(input_props, "device_id");
		loadListValues(ui->auxAudioDevice1, inputs, 3);
		loadListValues(ui->auxAudioDevice2, inputs, 4);
		loadListValues(ui->auxAudioDevice3, inputs, 5);
		loadListValues(ui->auxAudioDevice4, inputs, 6);
		obs_properties_destroy(input_props);
	}

	if (output_props) {
		obs_property_t *outputs = obs_properties_get(output_props, "device_id");
		loadListValues(ui->desktopAudioDevice1, outputs, 1);
		loadListValues(ui->desktopAudioDevice2, outputs, 2);
		obs_properties_destroy(output_props);
	}

	if (obs_video_active()) {
		ui->sampleRate->setEnabled(false);
		ui->channelSetup->setEnabled(false);
	}
}

void AudioWidget::loadMonitoringOptions()
{
	QComboBox *combobox = ui->monitoringDevice;

	auto enum_devices = [](void *param, const char *name, const char *id) {
		QComboBox *cb = (QComboBox *)param;
		cb->addItem(name, id);
		return true;
	};

	combobox->clear();
	combobox->addItem(QTStr("Basic.Settings.Advanced.Audio.MonitoringDevice.Default"), "default");

	obs_enum_audio_monitoring_devices(enum_devices, combobox);
}

void AudioWidget::loadDecayRateOptions()
{
	ui->meterDecayRate->clear();
	ui->meterDecayRate->addItem(QTStr("Basic.Settings.Audio.MeterDecayRate.Fast"), VOLUME_METER_DECAY_FAST);
	ui->meterDecayRate->addItem(QTStr("Basic.Settings.Audio.MeterDecayRate.Medium"), VOLUME_METER_DECAY_MEDIUM);
	ui->meterDecayRate->addItem(QTStr("Basic.Settings.Audio.MeterDecayRate.Slow"), VOLUME_METER_DECAY_SLOW);
}

void AudioWidget::loadPeakMeterOptions()
{
	ui->peakMeterType->clear();
	ui->peakMeterType->addItem(QTStr("Basic.Settings.Audio.PeakMeterType.SamplePeak"), 0);
	ui->peakMeterType->addItem(QTStr("Basic.Settings.Audio.PeakMeterType.TruePeak"), 1);
}

void AudioWidget::createAudioHotkeys()
{
	if (ui->audioSourceLayout->rowCount() > 0) {
		QLayoutItem *forDeletion = ui->audioSourceLayout->takeAt(0);
		forDeletion->widget()->deleteLater();
		delete forDeletion;
	}
	auto layout = new QFormLayout();
	layout->setVerticalSpacing(15);
	layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

	section().audioSourceSignals.clear();
	section().audioSources.clear();

	auto widget = new QWidget();
	widget->setLayout(layout);
	ui->audioSourceLayout->addRow(widget);

	const char *enablePtm = Str("Basic.Settings.Audio.EnablePushToMute");
	const char *ptmDelay = Str("Basic.Settings.Audio.PushToMuteDelay");
	const char *enablePtt = Str("Basic.Settings.Audio.EnablePushToTalk");
	const char *pttDelay = Str("Basic.Settings.Audio.PushToTalkDelay");
	auto AddSource = [&](obs_source_t *source) {
		if (!(obs_source_get_output_flags(source) & OBS_SOURCE_AUDIO))
			return true;

		auto form = new QFormLayout();
		form->setVerticalSpacing(0);
		form->setHorizontalSpacing(5);
		form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

		auto ptmCB = new SilentUpdateCheckBox();
		ptmCB->setText(enablePtm);
		ptmCB->setChecked(obs_source_push_to_mute_enabled(source));
		form->addRow(ptmCB);

		auto ptmSB = new SilentUpdateSpinBox();
		ptmSB->setSuffix(" ms");
		ptmSB->setRange(0, INT_MAX);
		ptmSB->setValue(obs_source_get_push_to_mute_delay(source));
		form->addRow(ptmDelay, ptmSB);

		auto pttCB = new SilentUpdateCheckBox();
		pttCB->setText(enablePtt);
		pttCB->setChecked(obs_source_push_to_talk_enabled(source));
		form->addRow(pttCB);

		auto pttSB = new SilentUpdateSpinBox();
		pttSB->setSuffix(" ms");
		pttSB->setRange(0, INT_MAX);
		pttSB->setValue(obs_source_get_push_to_talk_delay(source));
		form->addRow(pttDelay, pttSB);

		/* TODO: Fixme
		connect(ptmCB, &QCheckBox::toggled, section(), &AbstractSettingsSection::simpleChange);
		connect(ptmSB, &QSpinBox::valueChanged, section(), &AbstractSettingsSection::simpleChange);

		connect(pttCB, &QCheckBox::toggled, section(), &AbstractSettingsSection::simpleChange);
		connect(pttSB, &QSpinBox::valueChanged, section(), &AbstractSettingsSection::simpleChange);
		*/

		section().audioSourceSignals.reserve(section().audioSourceSignals.size() + 4);

		auto handler = obs_source_get_signal_handler(source);
		section().audioSourceSignals.emplace_back(
			handler, "push_to_mute_changed",
			[](void *data, calldata_t *param) {
				QMetaObject::invokeMethod(static_cast<QObject *>(data), "setCheckedSilently",
							  Q_ARG(bool, calldata_bool(param, "enabled")));
			},
			ptmCB);
		section().audioSourceSignals.emplace_back(
			handler, "push_to_mute_delay",
			[](void *data, calldata_t *param) {
				QMetaObject::invokeMethod(static_cast<QObject *>(data), "setValueSilently",
							  Q_ARG(int, calldata_int(param, "delay")));
			},
			ptmSB);
		section().audioSourceSignals.emplace_back(
			handler, "push_to_talk_changed",
			[](void *data, calldata_t *param) {
				QMetaObject::invokeMethod(static_cast<QObject *>(data), "setCheckedSilently",
							  Q_ARG(bool, calldata_bool(param, "enabled")));
			},
			pttCB);
		section().audioSourceSignals.emplace_back(
			handler, "push_to_talk_delay",
			[](void *data, calldata_t *param) {
				QMetaObject::invokeMethod(static_cast<QObject *>(data), "setValueSilently",
							  Q_ARG(int, calldata_int(param, "delay")));
			},
			pttSB);

		section().audioSources.emplace_back(OBSGetWeakRef(source), ptmCB, ptmSB, pttCB, pttSB);

		auto label = new OBSSourceLabel(source);
		TruncateLabel(label, label->text());
		label->setMinimumSize(QSize(170, 0));
		label->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
		connect(label, &OBSSourceLabel::Removed,
			[=]() { QMetaObject::invokeMethod(this, "ReloadAudioSources"); });
		connect(label, &OBSSourceLabel::Destroyed,
			[=]() { QMetaObject::invokeMethod(this, "ReloadAudioSources"); });

		layout->addRow(label, form);
		return true;
	};

	using AddSource_t = decltype(AddSource);
	obs_enum_sources(
		[](void *data, obs_source_t *source) {
			auto &AddSource = *static_cast<AddSource_t *>(data);
			if (!obs_source_removed(source))
				AddSource(source);
			return true;
		},
		static_cast<void *>(&AddSource));

	if (layout->rowCount() == 0)
		ui->audioHotkeysGroupBox->hide();
	else
		ui->audioHotkeysGroupBox->show();
}
