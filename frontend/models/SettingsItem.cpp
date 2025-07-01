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

#include "SettingsItem.hpp"

SettingsItem::SettingsItem(ConfigValueType type, ConfigType config, std::string section, std::string name)
	: type_(type),
	  config_(config),
	  section_(section),
	  name_(name)
{
	warningText.reset();
}

void SettingsItem::setDefaultValue(QVariant value)
{
	defaultValue_ = value;
}

QVariant SettingsItem::defaultValue()
{
	return defaultValue_;
}

void SettingsItem::setSavedValue(QVariant value)
{
	savedValue = value;
}

void SettingsItem::updateSavedValue(QVariant value)
{
	setSavedValue(value);

	emit valueChanged(value);
}

QVariant SettingsItem::pendingValue()
{
	return pendingValue_;
}

void SettingsItem::clearPendingValue()
{
	pendingValue_ = QVariant();
}

void SettingsItem::setPendingValue(QVariant value)
{
	pendingValue_ = value;
}

void SettingsItem::updatePendingValue(QVariant value)
{
	if (pendingValue_ == value) {
		return;
	}

	setPendingValue(value);
	emit pendingChanged(value);
}

bool SettingsItem::isChanged()
{
	return !pendingValue_.isNull() && pendingValue_ != savedValue;
}

void SettingsItem::setEnabled(bool enable)
{
	if (enabled == enable) {
		return;
	}

	enabled = enable;
	emit enableChanged(enable);
}

bool SettingsItem::isEnabled()
{
	return enabled;
}

void SettingsItem::setValid(bool value)
{
	valid = value;
}

bool SettingsItem::isValid()
{
	return valid;
}

void SettingsItem::setWarning(QString text)
{
	if (text.isEmpty()) {
		clearWarning();
		return;
	}

	warningText = text;
}

void SettingsItem::clearWarning()
{
	warningText.reset();
}

std::optional<QString> SettingsItem::warning()
{
	return warningText;
}

void SettingsItem::setRestartWhenChanged(bool restartRequired)
{
	restartWhenChanged = restartRequired;
}

bool SettingsItem::requiresRestart()
{
	return restartWhenChanged && isChanged();
}

void SettingsItem::setPending(int64_t value)
{
	blog(LOG_INFO, "Set pending int64 value: %s", std::to_string(value).c_str());
	updatePendingValue(QVariant(value));
}
void SettingsItem::setPending(uint64_t value)
{
	blog(LOG_INFO, "Set pending uint64 value: %s", std::to_string(value).c_str());
	updatePendingValue(QVariant(value));
}
void SettingsItem::setPending(int value)
{
	blog(LOG_INFO, "Set pending int value: %s", std::to_string(value).c_str());
	updatePendingValue(QVariant(value));
}
void SettingsItem::setPending(double value)
{
	updatePendingValue(QVariant(value));
}
void SettingsItem::setPending(std::string value)
{
	QString string = QString::fromStdString(value);
	updatePendingValue(QVariant(string));
}

void SettingsItem::setPending(QString value)
{
	blog(LOG_INFO, "Set pending QString value: %s", value.toStdString().c_str());
	updatePendingValue(QVariant(value));
}

void SettingsItem::setPending(bool value)
{
	blog(LOG_INFO, "Set pending bool value: %s", value ? "true" : "false");
	updatePendingValue(QVariant(value));
}

void SettingsItem::setPending(QVariant value)
{
	blog(LOG_INFO, "Set pending QVariant value: %s", value.toString().toStdString().c_str());
	updatePendingValue(value);
}
