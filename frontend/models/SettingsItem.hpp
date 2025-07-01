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

#include <QObject>
#include <QPointer>
#include <QVariant>

#include <settings/SettingsManager.hpp>
#include <util/util.hpp>

// Watcher Includes
#include <OBSApp.hpp>
class AbstractSettingsSection;
class SettingsItemObserver;

enum class ConfigType;
enum class ConfigValueType;

class SettingsItem : public QObject {
	Q_OBJECT

	QVariant defaultValue_;
	QVariant pendingValue_;
	QVariant savedValue;

	void setPendingValue(QVariant value);
	void updatePendingValue(QVariant value);

	bool restartWhenChanged = false;
	bool restartRequired = false;

	bool enabled = true;
	bool valid = true;
	std::optional<QString> warningText;

protected:
	ConfigValueType type_;
	ConfigType config_;
	std::string section_;
	std::string name_;

public:
	SettingsItem(ConfigValueType type, ConfigType config, std::string section, std::string name);
	~SettingsItem() {}

	ConfigValueType type() { return type_; }
	ConfigType config() { return config_; }
	const std::string &name() const { return name_; }
	const std::string &section() const { return section_; }

	void setDefaultValue(QVariant value);
	QVariant defaultValue();

	void setSavedValue(QVariant value);
	void updateSavedValue(QVariant value);

	QVariant pendingValue();
	void clearPendingValue();

	bool isChanged();

	void setEnabled(bool enable);
	bool isEnabled();

	void setValid(bool value);
	bool isValid();

	void setWarning(QString text);
	void clearWarning();
	std::optional<QString> warning();

	void setRestartWhenChanged(bool required);
	bool requiresRestart();

signals:
	void pendingChanged(QVariant value);
	void valueChanged(QVariant value);
	void enableChanged(bool enabled);

public slots:
	void setPending(int value);
	void setPending(int64_t value);
	void setPending(uint64_t value);
	void setPending(bool value);
	void setPending(double value);
	void setPending(std::string value);
	void setPending(QString value);
	void setPending(QVariant value);
};
