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

#include <QWidget>

class SettingsPage {
private:
	QString settingsKey;
	QString displayName;

public:
	SettingsPage(QString settingsKey, QString displayName);
	~SettingsPage();

	QString name() { return displayName; }
	QIcon icon() { return QIcon(); }
	virtual QWidget *createWidget(QWidget *parent);
};

class SettingsManager {
private:
	std::vector<SettingsPage *> settingsList;

public:
	SettingsManager();
	~SettingsManager();

	void registerPage(SettingsPage *newPage);
	std::vector<SettingsPage *> getPages() { return settingsList; }
};
