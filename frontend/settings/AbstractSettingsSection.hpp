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

#include <models/SettingsItem.hpp>
#include <settings/SettingsManager.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QPointer>
#include <QSpinBox>
#include <QWidget>

class SettingsManager;
class AbstractSettingsPage;
class AbstractSettingsSection : public QObject {
	Q_OBJECT

	std::string section_;
	std::string displayName;

	QPointer<AbstractSettingsPage> pageWidget_;

	bool showPage_ = true;

	bool hasPendingChanges = false;

	std::unordered_map<std::string, SettingsItem *> itemLookup;
	std::vector<std::unique_ptr<SettingsItem>> settingsItems;
	void intializeSetting(SettingsItem *item);

protected:
	SettingsManager *manager_;

	bool isChanged();

public:
	AbstractSettingsSection(std::string section, std::string displayName);
	AbstractSettingsSection(std::string section, const char *displayName)
		: AbstractSettingsSection(section, std::string(displayName))
	{
	}
	AbstractSettingsSection(std::string section, QString displayName)
		: AbstractSettingsSection(section, displayName.toStdString())
	{
	}
	~AbstractSettingsSection();

	const std::string &section() const { return section_; }
	const std::string &name() const { return displayName; }
	virtual QIcon icon() { return QIcon(); }

	virtual void registerSection();
	SettingsManager &manager() { return *manager_; }

	void activate(bool active);

	void setPageWidget(AbstractSettingsPage *widget);
	AbstractSettingsPage *pageWidget();

	void setShowPage(bool show);
	bool showPage() { return showPage_; }

	virtual QWidget *createSettingsPageWidget(QWidget *parent);
	virtual void load(bool force = false);
	virtual void save();
	virtual void discard();

	void clearChangeStates();
	void updatePendingChangesState(bool pending);

	const bool isPending() const;
	const bool isValid() const;

	void addSettingsItem(std::unique_ptr<SettingsItem> item);

	SettingsItem *findSettingsItem(const char *name);
	SettingsItem *findSettingsItem(const std::string &name);

	void initializeConfigItems(ConfigType config);

	SettingsItem *registerSetting(std::string settingsName, const char *defaultValue,
				      ConfigType config = ConfigType::UserConfig);
	SettingsItem *registerSetting(std::string settingsName, int defaultValue,
				      ConfigType config = ConfigType::UserConfig);
	SettingsItem *registerSetting(std::string settingsName, uint64_t defaultValue,
				      ConfigType config = ConfigType::UserConfig);
	SettingsItem *registerSetting(std::string settingsName, bool defaultValue,
				      ConfigType config = ConfigType::UserConfig);
	SettingsItem *registerSetting(std::string settingsName, double defaultValue,
				      ConfigType config = ConfigType::UserConfig);

	SettingsItem *connectSettingWidget(const std::string &name, QSpinBox *widget);
	SettingsItem *connectSettingWidget(const std::string &name, QDoubleSpinBox *widget);
	SettingsItem *connectSettingWidget(const std::string &name, QComboBox *widget);
	SettingsItem *connectSettingWidget(const std::string &name, QCheckBox *widget);
	SettingsItem *connectSettingWidget(const std::string &name, QSlider *widget);

	void applyItemValue(SettingsItem *item, QSpinBox *widget);
	void applyItemValue(SettingsItem *item, QSpinBox *widget, std::function<QVariant(QVariant)>);
	void applyItemValue(SettingsItem *item, QDoubleSpinBox *widget);
	void applyItemValue(SettingsItem *item, QDoubleSpinBox *widget, std::function<QVariant(QVariant)>);
	void applyItemValue(SettingsItem *item, QComboBox *widget);
	void applyItemValue(SettingsItem *item, QComboBox *widget, std::function<QVariant(QVariant)>);
	void applyItemValue(SettingsItem *item, QCheckBox *widget);
	void applyItemValue(SettingsItem *item, QCheckBox *widget, std::function<QVariant(QVariant)>);
	void applyItemValue(SettingsItem *item, QSlider *widget);
	void applyItemValue(SettingsItem *item, QSlider *widget, std::function<QVariant(QVariant)>);

	void bindPendingValue(SettingsItem *item, QSpinBox *widget);
	void bindPendingValue(SettingsItem *item, QSpinBox *widget, std::function<QVariant(QVariant)>);
	void bindPendingValue(SettingsItem *item, QDoubleSpinBox *widget);
	void bindPendingValue(SettingsItem *item, QDoubleSpinBox *widget, std::function<QVariant(QVariant)>);
	void bindPendingValue(SettingsItem *item, QComboBox *widget);
	void bindPendingValue(SettingsItem *item, QComboBox *widget, std::function<QVariant(QVariant)>);
	void bindPendingValue(SettingsItem *item, QCheckBox *widget);
	void bindPendingValue(SettingsItem *item, QCheckBox *widget, std::function<QVariant(QVariant)>);
	void bindPendingValue(SettingsItem *item, QSlider *widget);
	void bindPendingValue(SettingsItem *item, QSlider *widget, std::function<QVariant(QVariant)>);

	void bindItemUpdates(SettingsItem *item, QSpinBox *widget);
	void bindItemUpdates(SettingsItem *item, QSpinBox *widget, std::function<QVariant(QVariant)>);
	void bindItemUpdates(SettingsItem *item, QDoubleSpinBox *widget);
	void bindItemUpdates(SettingsItem *item, QDoubleSpinBox *widget, std::function<QVariant(QVariant)>);
	void bindItemUpdates(SettingsItem *item, QComboBox *widget);
	void bindItemUpdates(SettingsItem *item, QComboBox *widget, std::function<QVariant(QVariant)>);
	void bindItemUpdates(SettingsItem *item, QCheckBox *widget);
	void bindItemUpdates(SettingsItem *item, QCheckBox *widget, std::function<QVariant(QVariant)>);
	void bindItemUpdates(SettingsItem *item, QSlider *widget);
	void bindItemUpdates(SettingsItem *item, QSlider *widget, std::function<QVariant(QVariant)>);

	void bindChangeCallback(QObject *receiver, std::string section_, std::string name_,
				std::function<void()> callback);

	int itemsRequireRestart();

signals:
	void sectionChanged(bool hasPending);

	void pageActivated(bool isActive);
	void pageCreated();
	void pageSaved();

public slots:
	void handleCreate();

	void itemChanged(QVariant newValue);
};

class AbstractSettingsPage : public QWidget {
	Q_OBJECT

protected:
	AbstractSettingsSection *section_;

public:
	AbstractSettingsPage(QWidget *parent, AbstractSettingsSection *section);
	~AbstractSettingsPage();

	AbstractSettingsSection &section() { return *section_; }
	SettingsManager &manager() { return section().manager(); }

	void bindChangeCallback(std::string section_, std::string name_, std::function<void()> callback);

public slots:
	void initialLoad();
	void handlePageActivated(bool isActive);
};
