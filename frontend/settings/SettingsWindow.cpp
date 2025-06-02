#include "SettingsWindow.hpp"
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

#include "SettingsWindow.hpp"

#include <widgets/OBSBasic.hpp>

#include "moc_SettingsWindow.cpp"

SettingsWindow::SettingsWindow(QWidget *parent)
	: QDialog(parent),
	  main(qobject_cast<OBSBasic *>(parent)),
	  ui(new Ui::SettingsWindow)
{
	ui->setupUi(this);

	settingsManager = main->settingsManager();

	std::vector<SettingsPage *> settingsList = settingsManager->getPages();

	// Loop over all registered pages
	for (SettingsPage *entry : settingsList) {
		// Add sidebar entries
		blog(LOG_INFO, "Setup Entry %s", entry->name());

		QListWidgetItem *newItem = new QListWidgetItem();
		newItem->setData(Qt::DisplayRole, entry->name());
		ui->listWidget->addItem(newItem);

		// Add pages to stacked widget
		QWidget *pageWrapper = new QWidget();
		QVBoxLayout *pageLayout = new QVBoxLayout();
		pageWrapper->setLayout(pageLayout);

		pageLayout->addWidget(entry->createWidget(this));
		pageLayout->addSpacerItem(new QSpacerItem(10, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

		ui->settingsPageContainer->addWidget(pageWrapper);
	}

	connect(ui->listWidget, &QListWidget::currentItemChanged, this, &SettingsWindow::switchToPage);

	ui->listWidget->setCurrentRow(0);
}

SettingsWindow::~SettingsWindow() {}

void SettingsWindow::switchToPage(QListWidgetItem *current, QListWidgetItem *previous)
{
	QModelIndex index = ui->listWidget->indexFromItem(current);
	int pageIndex = index.row();

	ui->settingsPageContainer->setCurrentIndex(pageIndex);
}
