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

#include <OBSApp.hpp>
#include <widgets/OBSBasic.hpp>
#include <models/SettingsItem.hpp>
#include <settings/AbstractSettingsSection.hpp>
#include <util/base.h>

SettingsManager::SettingsManager() {}
SettingsManager::~SettingsManager() {}

config_t *SettingsManager::getConfig(ConfigType type)
{
	if (type == ConfigType::AppConfig) {
		return App()->GetAppConfig();
	}

	if (type == ConfigType::UserConfig) {
		return App()->GetUserConfig();
	}

	if (type == ConfigType::ProfileConfig) {
		OBSBasic *main = OBSBasic::Get();
		return main->getActiveConfig();
	}

	return nullptr;
}

SettingsItem *SettingsManager::getItem(std::string section, std::string name)
{
	auto *section_ = findSection(section);
	if (!section_) {
		return nullptr;
	}

	auto item = section_->findSettingsItem(name);
	if (!item) {
		return nullptr;
	}

	return item;
}

void SettingsManager::addSimple(std::string simpleName)
{
	auto simple = std::make_unique<AbstractSettingsSection>(simpleName, "");
	simple->setShowPage(false);

	addSection(std::move(simple));
}
void SettingsManager::addSection(std::unique_ptr<AbstractSettingsSection> entry)
{
	blog(LOG_INFO, "Creating Settings Section for: %s", entry->section().c_str());

	connect(entry.get(), &AbstractSettingsSection::sectionChanged, this, &SettingsManager::updateRestartTotal);

	const std::string &key = entry->section();
	AbstractSettingsSection *section = entry.get();

	sectionLookup[key] = section;
	sectionList.emplace_back(std::move(entry));

	section->registerSection();
}

void SettingsManager::createSection(AbstractSettingsSection *entry_)
{
	std::unique_ptr<AbstractSettingsSection> entry(entry_);

	blog(LOG_INFO, "Creating Settings Section for: %s", entry->section().c_str());

	connect(entry.get(), &AbstractSettingsSection::sectionChanged, this, &SettingsManager::updateRestartTotal);

	const std::string &key = entry->section();
	AbstractSettingsSection *section = entry.get();

	sectionLookup[key] = section;
	sectionList.emplace_back(std::move(entry));

	section->registerSection();
}

AbstractSettingsSection *SettingsManager::findSection(std::string section)
{
	auto result = sectionLookup.find(section);
	if (result != sectionLookup.end()) {
		return result->second;
	}

	blog(LOG_INFO, " -- Unable to find section '%s'", section.c_str());

	return nullptr;
}

void SettingsManager::setItemDefault(SettingsItem *item, QVariant value)
{
	if (item->type() == ConfigValueType::Int) {
		if (value.canConvert<int64_t>()) {
			setDefaultInt(item->config(), item->section(), item->name(), value.toInt());
		}
	} else if (item->type() == ConfigValueType::Uint) {
		if (value.canConvert<uint64_t>()) {
			setDefaultUint(item->config(), item->section(), item->name(), value.toInt());
		}
	} else if (item->type() == ConfigValueType::Bool) {
		if (value.canConvert<bool>()) {
			setDefaultBool(item->config(), item->section(), item->name(), value.toBool());
		}
	} else if (item->type() == ConfigValueType::Double) {
		if (value.canConvert<double>()) {
			setDefaultDouble(item->config(), item->section(), item->name(), value.toDouble());
		}
	} else if (item->type() == ConfigValueType::String) {
		if (value.canConvert<std::string>()) {
			setDefaultString(item->config(), item->section(), item->name(), value.toString().toStdString());
		}
	}
}

void SettingsManager::initializeConfigDefaults(ConfigType config)
{
	for (const auto &section : getSections()) {
		section->initializeConfigItems(config);
	}
}

bool SettingsManager::addSetting(std::unique_ptr<SettingsItem> item, QVariant defaultValue)
{
	item->setDefaultValue(defaultValue);

	AbstractSettingsSection *section = findSection(item->section());

	if (section) {
		blog(LOG_INFO, "Added setting: %s %s", item->section().c_str(), item->name().c_str());
		section->addSettingsItem(std::move(item));

		return true;
	}

	return false;
}

SettingsItem *SettingsManager::registerSetting(std::string section, std::string settingsName, const char *defaultValue,
					       ConfigType config)
{
	auto newItem = std::make_unique<SettingsItem>(ConfigValueType::String, config, section, settingsName);
	SettingsItem *itemPtr = newItem.get();

	if (addSetting(std::move(newItem), QVariant(defaultValue))) {
		return itemPtr;
	}

	return nullptr;
}

SettingsItem *SettingsManager::registerSetting(std::string section, std::string settingsName, int defaultValue,
					       ConfigType config)
{
	auto newItem = std::make_unique<SettingsItem>(ConfigValueType::Int, config, section, settingsName);
	SettingsItem *itemPtr = newItem.get();

	if (addSetting(std::move(newItem), QVariant(defaultValue))) {
		return itemPtr;
	}

	return nullptr;
}

SettingsItem *SettingsManager::registerSetting(std::string section, std::string settingsName, uint64_t defaultValue,
					       ConfigType config)
{
	auto newItem = std::make_unique<SettingsItem>(ConfigValueType::Uint, config, section, settingsName);
	SettingsItem *itemPtr = newItem.get();

	if (addSetting(std::move(newItem), QVariant(defaultValue))) {
		return itemPtr;
	}

	return nullptr;
}

SettingsItem *SettingsManager::registerSetting(std::string section, std::string settingsName, bool defaultValue,
					       ConfigType config)
{
	auto newItem = std::make_unique<SettingsItem>(ConfigValueType::Bool, config, section, settingsName);
	SettingsItem *itemPtr = newItem.get();

	if (addSetting(std::move(newItem), QVariant(defaultValue))) {
		return itemPtr;
	}

	return nullptr;
}

SettingsItem *SettingsManager::registerSetting(std::string section, std::string settingsName, double defaultValue,
					       ConfigType config)
{
	auto newItem = std::make_unique<SettingsItem>(ConfigValueType::Double, config, section, settingsName);
	SettingsItem *itemPtr = newItem.get();

	if (addSetting(std::move(newItem), QVariant(defaultValue))) {
		return itemPtr;
	}

	return nullptr;
}

QVariant SettingsManager::getValue(std::string section, std::string name)
{
	auto item = getItem(section, name);
	if (!item) {
		return QVariant();
	}

	return getValue(item);
}

QVariant SettingsManager::getValue(SettingsItem *item)
{
	if (item->type() == ConfigValueType::Int) {
		return QVariant(getInt(item->config(), item->section(), item->name()));
	} else if (item->type() == ConfigValueType::Uint) {
		return QVariant(getUint(item->config(), item->section(), item->name()));
	} else if (item->type() == ConfigValueType::Bool) {
		return QVariant(getBool(item->config(), item->section(), item->name()));
	} else if (item->type() == ConfigValueType::Double) {
		return QVariant(getDouble(item->config(), item->section(), item->name()));
	} else if (item->type() == ConfigValueType::String) {
		std::string value = getString(item->config(), item->section(), item->name());
		QString str = QString::fromStdString(value);
		return QVariant(str);
	}

	return QVariant();
}

QVariant SettingsManager::currentValue(std::string section, std::string name)
{
	auto item = getItem(section, name);
	if (!item) {
		return QVariant();
	}

	return currentValue(item);
}

QVariant SettingsManager::currentValue(SettingsItem *item)
{
	bool check = item->name() == "ReplayBufferWhileStreaming";
	if (item->isChanged()) {
		return item->pendingValue();
	} else {
		return getValue(item);
	}
}

QVariant SettingsManager::savedValue(SettingsItem *item)
{
	return getValue(item);
}

QVariant SettingsManager::dirtyValue(SettingsItem *item)
{
	return item->pendingValue();
}

void SettingsManager::saveAll()
{
	restartNeeded = 0;

	// Save all entries
	for (const auto &section : getSections()) {
		section->save();
	}

	config_save_safe(getConfig(ConfigType::AppConfig), "tmp", nullptr);
	config_save_safe(getConfig(ConfigType::UserConfig), "tmp", nullptr);
	config_save_safe(getConfig(ConfigType::ProfileConfig), "tmp", nullptr);

	emit saved();
}

void SettingsManager::discardAll()
{
	for (const auto &section : getSections()) {
		section->discard();
	}
}

bool SettingsManager::everySectionValid()
{
	bool allValid = true;

	// Save all entries
	for (const auto &section : getSections()) {
		allValid = allValid && section->isValid();
	}

	return allValid;
}

bool SettingsManager::anySectionPending()
{
	// Save all entries
	for (const auto &section : getSections()) {
		if (section->isPending()) {
			return true;
		}
	}

	return false;
}

void SettingsManager::saveItemPending(SettingsItem *item)
{
	QVariant value = item->pendingValue();
	if (!value.isValid() || value.isNull()) {
		return;
	}

	saveItemValue(item, value);
}

void SettingsManager::saveItemValue(SettingsItem *item, QVariant value)
{

	bool success = false;

	if (item->type() == ConfigValueType::Int) {
		if (value.canConvert<int64_t>()) {
			blog(LOG_INFO, "-- Saving int for %s to %s", item->name().c_str(),
			     value.toString().toStdString().c_str());
			setInt(item->config(), item->section(), item->name(), value.toInt());
			success = true;
		}
	} else if (item->type() == ConfigValueType::Uint) {
		if (value.canConvert<uint64_t>()) {
			setUint(item->config(), item->section(), item->name(), value.toInt());
			success = true;
		}
	} else if (item->type() == ConfigValueType::Bool) {
		if (value.canConvert<bool>()) {
			blog(LOG_INFO, "-- Saving boolean for %s to %s", item->name().c_str(),
			     value.toString().toStdString().c_str());
			setBool(item->config(), item->section(), item->name(), value.toBool());
			success = true;
		}
	} else if (item->type() == ConfigValueType::Double) {
		if (value.canConvert<double>()) {
			setDouble(item->config(), item->section(), item->name(), value.toDouble());
			success = true;
		}
	} else if (item->type() == ConfigValueType::String) {
		if (value.canConvert<QString>()) {
			setString(item->config(), item->section(), item->name(), value.toString().toStdString());
			success = true;
		}
	}

	if (success) {
		item->clearPendingValue();
		item->updateSavedValue(value);
	}
}

int64_t SettingsManager::getInt(ConfigType config, std::string section, std::string name)
{
	return config_get_int(getConfig(config), section.c_str(), name.c_str());
}

uint64_t SettingsManager::getUint(ConfigType config, std::string section, std::string name)
{
	return config_get_uint(getConfig(config), section.c_str(), name.c_str());
}

bool SettingsManager::getBool(ConfigType config, std::string section, std::string name)
{
	return config_get_bool(getConfig(config), section.c_str(), name.c_str());
}

double SettingsManager::getDouble(ConfigType config, std::string section, std::string name)
{
	return config_get_double(getConfig(config), section.c_str(), name.c_str());
}

std::string SettingsManager::getString(ConfigType config, std::string section, std::string name)
{
	const char *str = config_get_string(getConfig(config), section.c_str(), name.c_str());
	if (str != NULL) {
		return std::string(str);
	}

	return std::string();
}

void SettingsManager::setInt(ConfigType config, std::string section, std::string name, int64_t value)
{
	config_set_int(getConfig(config), section.c_str(), name.c_str(), value);
}

void SettingsManager::setUint(ConfigType config, std::string section, std::string name, uint64_t value)
{
	config_set_uint(getConfig(config), section.c_str(), name.c_str(), value);
}

void SettingsManager::setBool(ConfigType config, std::string section, std::string name, bool value)
{
	config_set_bool(getConfig(config), section.c_str(), name.c_str(), value);
}

void SettingsManager::setDouble(ConfigType config, std::string section, std::string name, double value)
{
	config_set_double(getConfig(config), section.c_str(), name.c_str(), value);
}

void SettingsManager::setString(ConfigType config, std::string section, std::string name, std::string value)
{
	config_set_string(getConfig(config), section.c_str(), name.c_str(), value.c_str());
}

void SettingsManager::setDefaultInt(ConfigType config, std::string section, std::string name, int64_t value)
{
	config_set_default_int(getConfig(config), section.c_str(), name.c_str(), value);
}

void SettingsManager::setDefaultUint(ConfigType config, std::string section, std::string name, uint64_t value)
{
	config_set_default_uint(getConfig(config), section.c_str(), name.c_str(), value);
}

void SettingsManager::setDefaultBool(ConfigType config, std::string section, std::string name, bool value)
{
	config_set_default_bool(getConfig(config), section.c_str(), name.c_str(), value);
}

void SettingsManager::setDefaultDouble(ConfigType config, std::string section, std::string name, double value)
{
	config_set_default_double(getConfig(config), section.c_str(), name.c_str(), value);
}

void SettingsManager::setDefaultString(ConfigType config, std::string section, std::string name, std::string value)
{
	config_set_default_string(getConfig(config), section.c_str(), name.c_str(), value.c_str());
}

void SettingsManager::updateRestartTotal()
{
	int total = 0;

	for (const auto &section : getSections()) {
		total += section->itemsRequireRestart();
	}

	if (restartNeeded == total) {
		return;
	}

	restartNeeded = total;
	emit restartNeededChanged(total);

}
