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

#include <QPointer>

class OBSBasic;
class SettingsWindow : public QDialog {
	Q_OBJECT

private:
	OBSBasic *main;
	SettingsManager *settingsManager;

	std::unique_ptr<Ui::SettingsWindow> ui;

	inline void EnableApplyButton(bool enable)
	{
		// ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(enable);
	}

private slots:
	

private:
	void clearPage();

private slots:
	void switchToPage(QListWidgetItem *current, QListWidgetItem *previous);

protected:
	

public:
	SettingsWindow(QWidget *parent);
	~SettingsWindow();
};
