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

#include "ui_AppearancePage.h"

#include "AbstractSettingsSection.hpp"
#include <utility/OBSTheme.hpp>

#include <QPointer>

class AppearanceWidget;
class AppearanceSection : public AbstractSettingsSection {

public:
	AppearanceSection(std::string section, std::string displayName);
	AppearanceSection(std::string section, const char *displayName)
		: AppearanceSection(section, std::string(displayName))
	{
	}
	AppearanceSection(std::string section, QString displayName)
		: AppearanceSection(section, displayName.toStdString())
	{
	}
	~AppearanceSection();

	QWidget *createSettingsPageWidget(QWidget *parent) override;
	QPointer<AppearanceWidget> pageWidget_;

	void registerSection() override;

	void load(bool force = false) override;
	void save() override;
};

class AppearanceWidget : public AbstractSettingsPage {
	Q_OBJECT

	std::unique_ptr<Ui::AppearancePage> ui;

	void loadThemeList();
	void loadVariantList();

	QString getSelectedTheme();

	void enableAppearanceFontControls(bool enable);
	void enableAppearanceDensityControls(bool enable);

public:
	AppearanceWidget(QWidget *parent, AppearanceSection *section);
	~AppearanceWidget();

public slots:
	void initialLoad();
	void handlePageActivated(bool isActive);

	void reloadTheme();
	void updateAppearanceControls();
};
