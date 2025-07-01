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

#include <util/util.hpp>

enum class ConfigType { AppConfig, UserConfig, ProfileConfig };
enum class ConfigValueType { Int, Uint, Bool, Double, String };

class AbstractSettingsSection;
class SettingsItem;

class SettingsManager : public QObject {
	Q_OBJECT

	std::unordered_map<std::string, AbstractSettingsSection *> sectionLookup;
	std::vector<std::unique_ptr<AbstractSettingsSection>> sectionList;

	config_t *getConfig(ConfigType type);

	SettingsItem *getItem(std::string section, std::string name);

	int64_t getInt(ConfigType config, std::string section, std::string name);
	uint64_t getUint(ConfigType config, std::string section, std::string name);
	bool getBool(ConfigType config, std::string section, std::string name);
	double getDouble(ConfigType config, std::string section, std::string name);
	std::string getString(ConfigType config, std::string section, std::string name);

	void setInt(ConfigType config, std::string section, std::string name, int64_t value);
	void setUint(ConfigType config, std::string section, std::string name, uint64_t value);
	void setBool(ConfigType config, std::string section, std::string name, bool value);
	void setDouble(ConfigType config, std::string section, std::string name, double value);
	void setString(ConfigType config, std::string section, std::string name, std::string value);

	void setDefaultInt(ConfigType config, std::string section, std::string name, int64_t value);
	void setDefaultUint(ConfigType config, std::string section, std::string name, uint64_t value);
	void setDefaultBool(ConfigType config, std::string section, std::string name, bool value);
	void setDefaultDouble(ConfigType config, std::string section, std::string name, double value);
	void setDefaultString(ConfigType config, std::string section, std::string name, std::string value);

	int restartNeeded = 0;

signals:
	void saved();
	void restartNeededChanged(int total);

public:
	SettingsManager();
	~SettingsManager();

	void addSimple(std::string simpleName);
	void addSection(std::unique_ptr<AbstractSettingsSection> entry);
	void createSection(AbstractSettingsSection *entry);
	const auto &getSections() const { return sectionList; }

	AbstractSettingsSection *findSection(std::string section);

	void setItemDefault(SettingsItem *item, QVariant value);
	void initializeConfigDefaults(ConfigType config);

	SettingsItem *registerSetting(std::string section, std::string settingsName, const char *defaultValue,
				      ConfigType config = ConfigType::UserConfig);
	SettingsItem *registerSetting(std::string section, std::string settingsName, int defaultValue,
				      ConfigType config = ConfigType::UserConfig);
	SettingsItem *registerSetting(std::string section, std::string settingsName, uint64_t defaultValue,
				      ConfigType config = ConfigType::UserConfig);
	SettingsItem *registerSetting(std::string section, std::string settingsName, bool defaultValue,
				      ConfigType config = ConfigType::UserConfig);
	SettingsItem *registerSetting(std::string section, std::string settingsName, double defaultValue,
				      ConfigType config = ConfigType::UserConfig);

	bool addSetting(std::unique_ptr<SettingsItem> item, QVariant defaultValue);

	QVariant getValue(std::string section, std::string name);
	QVariant getValue(SettingsItem *item);
	QVariant currentValue(std::string section, std::string name);
	QVariant currentValue(SettingsItem *item);
	QVariant savedValue(SettingsItem *item);
	QVariant dirtyValue(SettingsItem *item);

	void saveAll();
	void discardAll();

	bool everySectionValid();
	bool anySectionPending();

	void saveItemPending(SettingsItem *item);
	void saveItemValue(SettingsItem *item, QVariant value);

public slots:
	void updateRestartTotal();
};
