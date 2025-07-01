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

#include <QMessageBox>

#include <widgets/OBSBasic.hpp>
#include <settings/AbstractSettingsSection.hpp>
#include <qt-wrappers.hpp>

#include "moc_SettingsWindow.cpp"

SettingsWindow::SettingsWindow(QWidget *parent_)
	: QDialog(parent_),
	  main(qobject_cast<OBSBasic *>(parent_)),
	  ui(new Ui::SettingsWindow)
{
	setAttribute(Qt::WA_DeleteOnClose, true);

	ui->setupUi(this);

	parent();

	enableApplyButton(false);

	ui->settingsPageContainer->setContentsMargins(0, 0, 0, 0);

	settingsManager = App()->getSettingsManager();

	// Loop over all registered pages
	for (const auto &entry : settingsManager->getSections()) {
		connect(entry.get(), &AbstractSettingsSection::sectionChanged, this, &SettingsWindow::sectionUpdate);

		if (!entry->showPage()) {
			continue;
		}

		// Add sidebar entries
		blog(LOG_INFO, "Setup Window Entry %s", entry->name().c_str());

		QListWidgetItem *newItem = new QListWidgetItem();
		newItem->setData(Qt::DisplayRole, QString::fromStdString(entry->name()));
		newItem->setData(DataRole::SettingsSectionRole, QString::fromStdString(entry->section()));
		ui->listWidget->addItem(newItem);

		// Add pages to stacked widget
		QWidget *pageWrapper = new QWidget();
		QVBoxLayout *pageLayout = new QVBoxLayout();
		pageWrapper->setLayout(pageLayout);

		QWidget *entryWidget = entry->createSettingsPageWidget(this);

		pageLayout->addWidget(entryWidget);
		pageLayout->setContentsMargins(9, 0, 0, 0);
		//pageLayout->addSpacerItem(new QSpacerItem(10, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

		ui->settingsPageContainer->addWidget(pageWrapper);
	}

	connect(ui->listWidget, &QListWidget::currentItemChanged, this, &SettingsWindow::switchToPage);

	// Warnings and Errors
	restartNotice = new QLabel(this);
	restartNotice->setText(QTStr("Basic.Settings.ProgramRestart"));
	restartNotice->setProperty("class", "text-danger");
	ui->settingsNotices->layout()->addWidget(restartNotice);
	restartNotice->hide();

	connect(settingsManager, &SettingsManager::restartNeededChanged, this, &SettingsWindow::updateRestartMessage);

	connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this,
		&SettingsWindow::saveAndClose);
	connect(ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &SettingsWindow::reject);
	connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &SettingsWindow::apply);

	ui->listWidget->setCurrentRow(0);
}

SettingsWindow::~SettingsWindow() {}

void SettingsWindow::setCurrentPage(AbstractSettingsSection *page)
{
	if (page == currentPage) {
		return;
	}

	if (currentPage) {
		currentPage->activate(false);
	}

	if (page) {
		currentPage = page;
		currentPage->activate(true);
	}
}

AbstractSettingsSection *SettingsWindow::pageFromItem(QListWidgetItem *item)
{
	QVariant itemSection = item->data(DataRole::SettingsSectionRole);
	std::string section = itemSection.value<QByteArray>().constData();

	blog(LOG_INFO, "SettingsWindow::pageFromItem Title: %s Section: %s", item->text().toStdString().c_str(),
	     section.c_str());

	AbstractSettingsSection *page = settingsManager->findSection(section);

	return page;
}

inline void SettingsWindow::enableApplyButton(bool enable)
{
	ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(enable);
}

bool SettingsWindow::promptSave()
{
	QMessageBox::StandardButton button;

	button = OBSMessageBox::question(this, QTStr("Basic.Settings.ConfirmTitle"), QTStr("Basic.Settings.Confirm"),
					 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

	if (button == QMessageBox::Cancel) {
		return false;
	} else if (button == QMessageBox::Yes) {
		if (!isSettingsValid())
			return false;

		saveAndClose();
	} else if (button == QMessageBox::No) {
		discardAndClose();
	}

	return true;
}

bool SettingsWindow::isSettingsValid()
{
	return true;
}

bool SettingsWindow::isAllowedToClose()
{
	if (discardChanges) {
		return true;
	}

	return !settingsManager->anySectionPending() || promptSave();
}

void SettingsWindow::discardAndClose()
{
	discardChanges = true;
	close();
}

void SettingsWindow::closeEvent(QCloseEvent *event)
{
	if (!isAllowedToClose()) {
		event->ignore();
	}

	if (discardChanges) {
		settingsManager->discardAll();
	}
}

void SettingsWindow::sectionUpdate(bool pending)
{
	enableApplyButton(pending || settingsManager->anySectionPending());
}

void SettingsWindow::updateRestartMessage(int total)
{
	restartNotice->setText(QTStr("Basic.Settings.ProgramRestartTotal").arg(total));

	if (total > 0) {
		restartNotice->show();
	} else {
		restartNotice->hide();
	}
}

void SettingsWindow::reject()
{
	discardChanges = true;
	close();
}
void SettingsWindow::showEvent(QShowEvent *event)
{
	QDialog::showEvent(event);

	/* Reduce the height of the widget area if too tall compared to the screen
	 * size (e.g., 720p) with potential window decoration (e.g., titlebar). */
	const int titleBarHeight = QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight);
	const int maxHeight = round(screen()->availableGeometry().height() - titleBarHeight);
	if (size().height() >= maxHeight) {
		resize(size().width(), maxHeight);
	}
}

void SettingsWindow::saveAll()
{
	settingsManager->saveAll();
}

void SettingsWindow::apply()
{
	saveAll();

	//// UpdateYouTubeAppDockSettings();
	//// ClearChanged();
}

void SettingsWindow::saveAndClose()
{
	saveAll();
	close();
}

void SettingsWindow::switchToPage(QListWidgetItem *current, QListWidgetItem *previous)
{
	// Switch StackedWidget index
	QModelIndex index = ui->listWidget->indexFromItem(current);
	int pageIndex = index.row();

	ui->settingsPageContainer->setCurrentIndex(pageIndex);

	// Update current SettingsPage
	AbstractSettingsSection *page = pageFromItem(current);
	if (page) {
		setCurrentPage(page);
	}
}
