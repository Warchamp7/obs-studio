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

#include "AccessibilitySection.hpp"

#include <OBSApp.hpp>
#include <utility/platform.hpp>

#include <qt-wrappers.hpp>

#include "moc_AccessibilitySection.cpp"

AccessibilitySection::AccessibilitySection(std::string section, std::string displayName)
	: AbstractSettingsSection(section, displayName)
{
}

AccessibilitySection::~AccessibilitySection() {}

QWidget *AccessibilitySection::createSettingsPageWidget(QWidget *parent)
{
	AccessibilityWidget *pageWidget = new AccessibilityWidget(parent, this);
	setPageWidget(pageWidget);

	emit pageCreated();

	return pageWidget;
}

void AccessibilitySection::load(bool force)
{
	if (pageWidget()) {
	}
}

void AccessibilitySection::save()
{
	AbstractSettingsSection::save();
}

AccessibilityWidget::AccessibilityWidget(QWidget *parent, AccessibilitySection *section_)
	: AbstractSettingsPage(parent, section_),
	  ui(new Ui::AccessibilityPage)
{
	ui->setupUi(this);
}

AccessibilityWidget::~AccessibilityWidget() {}

void AccessibilityWidget::initialLoad() {}

void AccessibilityWidget::handlePageActivated(bool isActive)
{
	if (isActive) {
	}
}
