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

#include "SettingsManager.hpp"

#include <QGroupBox>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>

#include <util/base.h>

QWidget *SettingsPage::createWidget(QWidget *parent)
{
	QWidget *pageContainer = new QWidget();
	QVBoxLayout *pageLayout = new QVBoxLayout();

	QGroupBox *groupBox = new QGroupBox();
	groupBox->setTitle(displayName);
	groupBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

	QVBoxLayout *groupLayout = new QVBoxLayout();
	groupBox->setLayout(groupLayout);

	QPushButton *testButton = new QPushButton();
	testButton->setText("Settings Page Button for " + displayName);

	pageContainer->setLayout(pageLayout);
	pageLayout->addWidget(groupBox);

	groupLayout->addWidget(testButton);

	return pageContainer;
}

SettingsPage::SettingsPage(QString key, QString name) : settingsKey(key), displayName(name) {}

SettingsPage::~SettingsPage() {}

SettingsManager::SettingsManager()
{
	
}

SettingsManager::~SettingsManager() {}

void SettingsManager::registerPage(SettingsPage *newPage)
{
	settingsList.emplace_back(newPage);
}
