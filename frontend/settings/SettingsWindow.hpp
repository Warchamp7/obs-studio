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

#pragma once

#include "ui_SettingsWindow.h"

#include <settings/SettingsManager.hpp>

#include <QLabel>
#include <QPointer>
#include <QPushButton>

class OBSBasic;
class SettingsWindow : public QDialog {
	Q_OBJECT

	OBSBasic *main;
	SettingsManager *settingsManager;

	std::unique_ptr<Ui::SettingsWindow> ui;
	QPointer<QLabel> restartNotice;

	enum DataRole { SettingsSectionRole = Qt::UserRole + 1 };

	AbstractSettingsSection *currentPage = nullptr;
	void setCurrentPage(AbstractSettingsSection *page);

	AbstractSettingsSection *pageFromItem(QListWidgetItem *item);

	inline void enableApplyButton(bool enable);

	bool discardChanges = false;

	bool promptSave();

	bool isSettingsValid();
	bool isAllowedToClose();

	void saveAll();
	void discardAndClose();

protected:
	virtual void closeEvent(QCloseEvent *event) override;

public:
	SettingsWindow(QWidget *parent);
	~SettingsWindow();

public slots:
	void reject();
	void showEvent(QShowEvent *event);

	void apply();
	void saveAndClose();

	void switchToPage(QListWidgetItem *current, QListWidgetItem *previous);

	void sectionUpdate(bool pending);
	void updateRestartMessage(int total);
};
