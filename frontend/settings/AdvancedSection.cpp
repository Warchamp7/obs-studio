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

#include "AdvancedSection.hpp"

#include <OBSApp.hpp>
#include <utility/platform.hpp>

#include <qt-wrappers.hpp>

#include "moc_AdvancedSection.cpp"

AdvancedSection::AdvancedSection(std::string section, std::string displayName)
	: AbstractSettingsSection(section, displayName)
{
}

AdvancedSection::~AdvancedSection() {}

QWidget *AdvancedSection::createSettingsPageWidget(QWidget *parent)
{
	AdvancedWidget *pageWidget = new AdvancedWidget(parent, this);
	setPageWidget(pageWidget);

	emit pageCreated();

	return pageWidget;
}

void AdvancedSection::load(bool force)
{
	if (pageWidget()) {
	}
}

void AdvancedSection::save()
{
	AbstractSettingsSection::save();
}

AdvancedWidget::AdvancedWidget(QWidget *parent, AdvancedSection *section_)
	: AbstractSettingsPage(parent, section_),
	  ui(new Ui::AdvancedPage)
{
	ui->setupUi(this);
}

AdvancedWidget::~AdvancedWidget() {}

void AdvancedWidget::initialLoad() {}

void AdvancedWidget::handlePageActivated(bool isActive)
{
	if (isActive) {
	}
}
