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

#include "StreamSection.hpp"

#include <OBSApp.hpp>
#include <utility/platform.hpp>

#include <qt-wrappers.hpp>

#include "moc_StreamSection.cpp"

StreamSection::StreamSection(std::string section, std::string displayName)
	: AbstractSettingsSection(section, displayName)
{
}

StreamSection::~StreamSection() {}

QWidget *StreamSection::createSettingsPageWidget(QWidget *parent)
{
	StreamWidget *pageWidget = new StreamWidget(parent, this);
	setPageWidget(pageWidget);

	emit pageCreated();

	return pageWidget;
}

void StreamSection::load(bool force)
{
	if (pageWidget()) {
	}
}

void StreamSection::save()
{
	AbstractSettingsSection::save();
}

StreamWidget::StreamWidget(QWidget *parent, StreamSection *section_)
	: AbstractSettingsPage(parent, section_),
	  ui(new Ui::StreamPage)
{
	ui->setupUi(this);
}

StreamWidget::~StreamWidget() {}

void StreamWidget::initialLoad()
{
}

void StreamWidget::handlePageActivated(bool isActive)
{
	if (isActive) {
	}
}
