/******************************************************************************
 Copyright (C) 2025 by Warchamp7 <warchamp7@obsproject.com>
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Video Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Video Public License for more details.
 
 You should have received a copy of the GNU Video Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#pragma once

#include "ui_VideoPage.h"

#include "AbstractSettingsSection.hpp"

#include <QPointer>

class VideoWidget;
class VideoSection : public AbstractSettingsSection {

public:
	VideoSection(std::string section, std::string displayName);
	VideoSection(std::string section, const char *displayName) : VideoSection(section, std::string(displayName)) {}
	VideoSection(std::string section, QString displayName) : VideoSection(section, displayName.toStdString()) {}
	~VideoSection();

	QWidget *createSettingsPageWidget(QWidget *parent) override;
	QPointer<VideoWidget> pageWidget_;

	void registerSection() override;

	void load(bool force = false) override;
	void save() override;
};

class VideoWidget : public AbstractSettingsPage {
	Q_OBJECT

	std::unique_ptr<Ui::VideoPage> ui;

	VideoSection &section();

	void loadResolutionLists();
	void loadDownscaleFilters();
	void loadFPSTypeOptions();

	void resetDownscales(uint32_t cx, uint32_t cy, bool ignoreAllSignals);
	void updateBaseAspectRatioText();
	void updateOutputAspectRatioText();

public:
	VideoWidget(QWidget *parent, VideoSection *section);
	~VideoWidget();

private slots:
	void initialLoad();
	void handlePageActivated(bool isActive);
};
