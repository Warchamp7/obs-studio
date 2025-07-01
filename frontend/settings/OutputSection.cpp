/******************************************************************************
 Copyright (C) 2025 by Warchamp7 <warchamp7@obsproject.com>
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Output Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Output Public License for more details.
 
 You should have received a copy of the GNU Output Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include "OutputSection.hpp"

#include <OBSApp.hpp>
#include <utility/platform.hpp>

#include <qt-wrappers.hpp>

#include "moc_OutputSection.cpp"

OutputSection::OutputSection(std::string section, std::string displayName)
	: AbstractSettingsSection(section, displayName)
{
}

OutputSection::~OutputSection() {}

QWidget *OutputSection::createSettingsPageWidget(QWidget *parent)
{
	OutputWidget *pageWidget = new OutputWidget(parent, this);
	setPageWidget(pageWidget);

	emit pageCreated();

	return pageWidget;
}

void OutputSection::registerSection()
{
	registerSetting("Mode", "Simple", ConfigType::ProfileConfig);

	manager().registerSetting("SimpleOutput", "RecQuality", "Stream", ConfigType::ProfileConfig);
	manager().registerSetting("SimpleOutput", "RecRB", false, ConfigType::ProfileConfig);

	manager().registerSetting("AdvOut", "RecType", "Standard", ConfigType::ProfileConfig);
	manager().registerSetting("AdvOut", "RecRB", false, ConfigType::ProfileConfig);
}

void OutputSection::load(bool force)
{
	if (pageWidget()) {
	}
}

void OutputSection::save()
{
	AbstractSettingsSection::save();
}

OutputWidget::OutputWidget(QWidget *parent, OutputSection *section_)
	: AbstractSettingsPage(parent, section_),
	  ui(new Ui::OutputPage)
{
	ui->setupUi(this);
}

OutputWidget::~OutputWidget() {}

void OutputWidget::initialLoad() {}

void OutputWidget::handlePageActivated(bool isActive)
{
	if (isActive) {
	}
}
