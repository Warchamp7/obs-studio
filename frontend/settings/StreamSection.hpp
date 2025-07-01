/******************************************************************************
 Copyright (C) 2025 by Warchamp7 <warchamp7@obsproject.com>
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Stream Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Stream Public License for more details.
 
 You should have received a copy of the GNU Stream Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#pragma once

#include "ui_StreamPage.h"

#include "AbstractSettingsSection.hpp"

#include <QPointer>

class StreamWidget;
class StreamSection : public AbstractSettingsSection {

public:
	StreamSection(std::string section, std::string displayName);
	StreamSection(std::string section, const char *displayName) : StreamSection(section, std::string(displayName))
	{
	}
	StreamSection(std::string section, QString displayName) : StreamSection(section, displayName.toStdString()) {}
	~StreamSection();

	QWidget *createSettingsPageWidget(QWidget *parent) override;
	QPointer<StreamWidget> pageWidget_;

	void load(bool force = false) override;
	void save() override;
};

class StreamWidget : public AbstractSettingsPage {
	Q_OBJECT

	std::unique_ptr<Ui::StreamPage> ui;

	StreamSection *section_;
	StreamSection &section() { return *section_; }

public:
	StreamWidget(QWidget *parent, StreamSection *section);
	~StreamWidget();

private slots:
	void initialLoad();
	void handlePageActivated(bool isActive);
};
