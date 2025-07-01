/******************************************************************************
 Copyright (C) 2025 by Warchamp7 <warchamp7@obsproject.com>
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Advanced Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Advanced Public License for more details.
 
 You should have received a copy of the GNU Advanced Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#pragma once

#include "ui_AdvancedPage.h"

#include "AbstractSettingsSection.hpp"

#include <QPointer>

class AdvancedWidget;
class AdvancedSection : public AbstractSettingsSection {

public:
	AdvancedSection(std::string section, std::string displayName);
	AdvancedSection(std::string section, const char *displayName)
		: AdvancedSection(section, std::string(displayName))
	{
	}
	AdvancedSection(std::string section, QString displayName) : AdvancedSection(section, displayName.toStdString())
	{
	}
	~AdvancedSection();

	QWidget *createSettingsPageWidget(QWidget *parent) override;
	QPointer<AdvancedWidget> pageWidget_;

	void load(bool force = false) override;
	void save() override;
};

class AdvancedWidget : public AbstractSettingsPage {
	Q_OBJECT

	std::unique_ptr<Ui::AdvancedPage> ui;

	AdvancedSection *section_;
	AdvancedSection &section() { return *section_; }

public:
	AdvancedWidget(QWidget *parent, AdvancedSection *section);
	~AdvancedWidget();

private slots:
	void initialLoad();
	void handlePageActivated(bool isActive);
};
