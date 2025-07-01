/******************************************************************************
 Copyright (C) 2025 by Warchamp7 <warchamp7@obsproject.com>
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Audio Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Audio Public License for more details.
 
 You should have received a copy of the GNU Audio Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#pragma once

#include "ui_AudioPage.h"

#include <obs.hpp>
#include "AbstractSettingsSection.hpp"

#include <QPointer>
#include <QSpinBox>

#define VOLUME_METER_DECAY_FAST 23.53
#define VOLUME_METER_DECAY_MEDIUM 11.76
#define VOLUME_METER_DECAY_SLOW 8.57

class AudioWidget;
class AudioSection : public AbstractSettingsSection {

public:
	AudioSection(std::string section, std::string displayName);
	AudioSection(std::string section, const char *displayName) : AudioSection(section, std::string(displayName)) {}
	AudioSection(std::string section, QString displayName) : AudioSection(section, displayName.toStdString()) {}
	~AudioSection();

	QWidget *createSettingsPageWidget(QWidget *parent) override;
	QPointer<AudioWidget> pageWidget_;

	void registerSection() override;

	using AudioSource_t = std::tuple<OBSWeakSource, QPointer<QCheckBox>, QPointer<QSpinBox>, QPointer<QCheckBox>,
					 QPointer<QSpinBox>>;
	std::vector<AudioSource_t> audioSources;
	std::vector<OBSSignal> audioSourceSignals;

	void load(bool force = false) override;
	void save() override;
};

class AudioWidget : public AbstractSettingsPage {
	Q_OBJECT

	std::unique_ptr<Ui::AudioPage> ui;

	AudioSection &section();

	void loadListValues(QComboBox *widget, obs_property_t *prop, int index);
	void loadAudioDeviceOptions();
	void loadMonitoringOptions();
	void loadDecayRateOptions();
	void loadPeakMeterOptions();
	void createAudioHotkeys();

public:
	AudioWidget(QWidget *parent, AudioSection *section);
	~AudioWidget();

private slots:
	void initialLoad();
	void handlePageActivated(bool isActive);
	void save();
};
