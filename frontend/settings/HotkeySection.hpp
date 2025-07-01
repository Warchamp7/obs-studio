/******************************************************************************
 Copyright (C) 2025 by Warchamp7 <warchamp7@obsproject.com>
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Hotkey Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Hotkey Public License for more details.
 
 You should have received a copy of the GNU Hotkey Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#pragma once

#include "ui_HotkeyPage.h"

#include "AbstractSettingsSection.hpp"

#include <QPointer>

class HotkeyWidget;
class HotkeySection : public AbstractSettingsSection {

public:
	HotkeySection(std::string section, std::string displayName);
	HotkeySection(std::string section, const char *displayName) : HotkeySection(section, std::string(displayName))
	{
	}
	HotkeySection(std::string section, QString displayName) : HotkeySection(section, displayName.toStdString()) {}
	~HotkeySection();

	QWidget *createSettingsPageWidget(QWidget *parent) override;
	QPointer<HotkeyWidget> pageWidget_;

	void load(bool force = false) override;
	void save() override;
};

class HotkeyWidget : public AbstractSettingsPage {
	Q_OBJECT

	std::unique_ptr<Ui::HotkeyPage> ui;

	HotkeySection *section_;
	HotkeySection &section() { return *section_; }

public:
	HotkeyWidget(QWidget *parent, HotkeySection *page);
	~HotkeyWidget();

private slots:
	void initialLoad();
	void handlePageActivated(bool isActive);
};
