/******************************************************************************
    Copyright (C) 2025 by FiniteSingularity <finitesingularityttv@gmail.com>

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

#include "PluginManagerWindow.hpp"

#include <OBSApp.hpp>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QString>
#include <QVBoxLayout>

#include "moc_PluginManagerWindow.cpp"

extern bool safe_mode;

namespace OBS {

PluginManagerWindow::PluginManagerWindow(std::vector<ModuleInfo> const &modules, QWidget *parent)
	: QDialog(parent),
	  modules_(modules),
	  ui(new Ui::PluginManagerWindow)
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	ui->setupUi(this);

	ui->modulesListContainer->viewport()->setAutoFillBackground(false);
	ui->modulesListContents->setAutoFillBackground(false);

	// Set up sidebar entries
	ui->sectionList->clear();
	ui->sectionList->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(ui->sectionList, &QListWidget::itemSelectionChanged, this,
		&PluginManagerWindow::sectionSelectionChanged);

	QListWidgetItem *browse = new QListWidgetItem(QTStr("PluginManager.Section.Discover"));
	browse->setFlags(browse->flags() & ~Qt::ItemIsEnabled);
	browse->setFlags(browse->flags() & ~Qt::ItemIsSelectable);
	browse->setToolTip(QTStr("ComingSoon"));
	ui->sectionList->addItem(browse);

	QListWidgetItem *installed = new QListWidgetItem(QTStr("PluginManager.Section.Manage"));
	ui->sectionList->addItem(installed);

	QListWidgetItem *updates = new QListWidgetItem(QTStr("PluginManager.Section.Updates"));
	updates->setFlags(updates->flags() & ~Qt::ItemIsEnabled);
	updates->setFlags(updates->flags() & ~Qt::ItemIsSelectable);
	updates->setToolTip(QTStr("ComingSoon"));
	ui->sectionList->addItem(updates);

	setSection(ui->sectionList->indexFromItem(installed));

	std::sort(modules_.begin(), modules_.end(), [](const ModuleInfo &a, const ModuleInfo &b) {
		std::string aName = !a.display_name.empty() ? a.display_name : a.module_name;
		std::string bName = !b.display_name.empty() ? b.display_name : b.module_name;
		return aName < bName;
	});

	std::sort(modules_.begin(), modules_.end(), [](const ModuleInfo &a, const ModuleInfo &b) {
		bool missingA = !obs_get_module(a.module_name.c_str()) &&
				!obs_get_disabled_module(a.module_name.c_str());
		bool missingB = !obs_get_module(b.module_name.c_str()) &&
				!obs_get_disabled_module(b.module_name.c_str());

		return !missingA && missingB;
	});

	QLabel *currentLibobsLabel = new QLabel(QString("%1.%2.%3")
							.arg((LIBOBS_API_VER >> 24) & 0xFF)
							.arg((LIBOBS_API_VER >> 16) & 0xFF)
							.arg(LIBOBS_API_VER & 0xFF),
						this);

	ui->modulesList->layout()->addWidget(currentLibobsLabel);

	QVBoxLayout *loadedList = new QVBoxLayout();
	QVBoxLayout *missingList = new QVBoxLayout();

	QLabel *missingLabel = new QLabel(ui->modulesList);
	missingLabel->setText(QTStr("PluginManager.MissingPlugin"));
	missingLabel->setProperty("class", "text-warning text-bold");
	missingLabel->setIndent(0);
	missingLabel->setVisible(false);

	ui->modulesListLayout->addLayout(loadedList);
	ui->modulesListLayout->addWidget(missingLabel);
	ui->modulesListLayout->addLayout(missingList);

	int row = 0;
	for (auto &metadata : modules_) {
		std::string id = metadata.module_name;
		// Check if the module is loaded
		obs_module_t *module = obs_get_module(id.c_str());
		if (!module) {
			module = obs_get_disabled_module(id.c_str());
		}
		bool isNotLoaded = !module;

		if (!isNotLoaded) {
			missingLabel->setVisible(true);
		}

		enum obs_module_load_state state = obs_module_get_load_state(module);

		QString name = !metadata.display_name.empty() ? metadata.display_name.c_str()
							      : metadata.module_name.c_str();

		if (state == OBS_MODULE_INCOMPATIBLE_VER) {
			name += QString(" [INCOMPATIBLE]");
		} else {
			name += QString(" --- %1").arg(state);
		}

		auto item = new QCheckBox(name);
		item->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
		item->setChecked(metadata.enabled);

		if (!metadata.enabledAtLaunch || isNotLoaded) {
			item->setProperty("class", "text-muted");
		}

		if (isNotLoaded) {
			missingList->addWidget(item);
		} else {
			loadedList->addWidget(item);
		}

		connect(item, &QCheckBox::toggled, this, [this, row](bool checked) {
			modules_[row].enabled = checked;
			ui->manageRestartLabel->setVisible(isEnabledPluginsChanged());
		});

		++row;
	}

	if (safe_mode) {
		QLabel *safeModeLabel = new QLabel(ui->modulesList);
		safeModeLabel->setText(QTStr("PluginManager.SafeMode"));
		safeModeLabel->setProperty("class", "text-muted text-italic");
		safeModeLabel->setIndent(0);

		ui->modulesListLayout->insertWidget(0, safeModeLabel);
	}

	// TODO: Hookup loadOutdatedCheckbox to plugin manager settings.

	ui->modulesList->adjustSize();
	ui->modulesListContents->adjustSize();

	ui->manageRestartLabel->setVisible(isEnabledPluginsChanged());

	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void PluginManagerWindow::sectionSelectionChanged()
{
	auto selected = ui->sectionList->selectedItems();
	if (selected.count() != 1) {
		setSection(activeSectionIndex);
	} else {
		auto selectionIndex = ui->sectionList->indexFromItem(selected.first());
		setSection(selectionIndex);
	}
}

void PluginManagerWindow::setSection(QPersistentModelIndex index)
{
	if (ui->sectionList->itemFromIndex(index)) {
		activeSectionIndex = index;
		ui->sectionList->setCurrentIndex(index);
	}
}

bool PluginManagerWindow::isEnabledPluginsChanged()
{
	bool result = false;
	for (auto &metadata : modules_) {
		if (metadata.enabledAtLaunch != metadata.enabled) {
			result = true;
			break;
		}
	}

	return result;
}

}; // namespace OBS
