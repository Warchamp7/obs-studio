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

#include "HotkeySection.hpp"

#include <OBSApp.hpp>
#include <utility/platform.hpp>

#include <qt-wrappers.hpp>

#include "moc_HotkeySection.cpp"

HotkeySection::HotkeySection(std::string section, std::string displayName)
	: AbstractSettingsSection(section, displayName)
{
}

HotkeySection::~HotkeySection() {}

QWidget *HotkeySection::createSettingsPageWidget(QWidget *parent)
{
	HotkeyWidget *pageWidget = new HotkeyWidget(parent, this);
	setPageWidget(pageWidget);

	emit pageCreated();

	return pageWidget;
}

void HotkeySection::load(bool force)
{
	if (pageWidget()) {
	}
}

void HotkeySection::save()
{
	AbstractSettingsSection::save();
}

HotkeyWidget::HotkeyWidget(QWidget *parent, HotkeySection *section_)
	: AbstractSettingsPage(parent, section_),
	  ui(new Ui::HotkeyPage)
{
	ui->setupUi(this);
}

HotkeyWidget::~HotkeyWidget() {}

void HotkeyWidget::initialLoad() {}

void HotkeyWidget::handlePageActivated(bool isActive)
{
	if (isActive) {
		// TODO: Lazy load hotkeys here
	}
}
