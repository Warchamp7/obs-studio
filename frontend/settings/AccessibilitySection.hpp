/******************************************************************************
 Copyright (C) 2025 by Warchamp7 <warchamp7@obsproject.com>
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Accessibility Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Accessibility Public License for more details.
 
 You should have received a copy of the GNU Accessibility Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#pragma once

#include "ui_AccessibilityPage.h"

#include "AbstractSettingsSection.hpp"

#include <QPointer>

class AccessibilityWidget;
class AccessibilitySection : public AbstractSettingsSection {

public:
	AccessibilitySection(std::string section, std::string displayName);
	AccessibilitySection(std::string section, const char *displayName)
		: AccessibilitySection(section, std::string(displayName))
	{
	}
	AccessibilitySection(std::string section, QString displayName)
		: AccessibilitySection(section, displayName.toStdString())
	{
	}
	~AccessibilitySection();

	QWidget *createSettingsPageWidget(QWidget *parent) override;
	QPointer<AccessibilityWidget> pageWidget_;

	void load(bool force = false) override;
	void save() override;
};

class AccessibilityWidget : public AbstractSettingsPage {
	Q_OBJECT

	std::unique_ptr<Ui::AccessibilityPage> ui;

	AccessibilitySection *section_;
	AccessibilitySection &section() { return *section_; }

public:
	AccessibilityWidget(QWidget *parent, AccessibilitySection *page);
	~AccessibilityWidget();

private slots:
	void initialLoad();
	void handlePageActivated(bool isActive);
};
