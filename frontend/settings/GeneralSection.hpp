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

#pragma once

#include "ui_GeneralPage.h"

#include <OBSApp.hpp>
#include "AbstractSettingsSection.hpp"

#include <QPointer>

class GeneralWidget;
class GeneralSection : public AbstractSettingsSection {

public:
	GeneralSection(std::string section, std::string displayName);
	GeneralSection(std::string section, const char *displayName) : GeneralSection(section, std::string(displayName))
	{
	}
	GeneralSection(std::string section, QString displayName) : GeneralSection(section, displayName.toStdString()) {}
	~GeneralSection();

	QWidget *createSettingsPageWidget(QWidget *parent) override;
	QPointer<GeneralWidget> pageWidget_;

	void registerSection() override;

	void load(bool force = false) override;
	void save() override;
};

class GeneralWidget : public AbstractSettingsPage {
	Q_OBJECT

	std::unique_ptr<Ui::GeneralPage> ui;

	bool forceUpdateCheck = false;
	int prevLangIndex;

	void loadLanguageList();
	void loadBranchesList();
	void loadMultiviewList();
	void hideOBSWindowWarning(bool checked);

public:
	GeneralWidget(QWidget *parent, GeneralSection *section);
	~GeneralWidget();

signals:
	void signalTest(std::string id);

public slots:
	void initialLoad();
	void handlePageActivated(bool isActive);
};
