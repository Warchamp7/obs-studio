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

#include "AbstractSettingsSection.hpp"

#include <OBSApp.hpp>
#include <util/base.h>

#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QStandardItemModel>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>

namespace {
auto noValueTransform = [](QVariant val) {
	return val;
};
} // namespace

AbstractSettingsSection::AbstractSettingsSection(std::string section, std::string name)
	: section_(section),
	  displayName(name),
	  manager_(App()->getSettingsManager())
{
	/* Prevent Qt taking ownership */
	// setParent(nullptr);

	connect(this, &AbstractSettingsSection::pageCreated, this, &AbstractSettingsSection::handleCreate);
}
AbstractSettingsSection::~AbstractSettingsSection() {}

void AbstractSettingsSection::intializeSetting(SettingsItem *item)
{
	manager().setItemDefault(item, item->defaultValue());
	QVariant savedValue = manager().getValue(item);
	item->setSavedValue(savedValue);
}

bool AbstractSettingsSection::isChanged()
{
	bool result = false;

	for (const auto &item : settingsItems) {
		if (item->isChanged()) {
			result = true;
			break;
		}
	}

	return result;
}

void AbstractSettingsSection::setPageWidget(AbstractSettingsPage *widget)
{
	pageWidget_ = widget;
}

AbstractSettingsPage *AbstractSettingsSection::pageWidget()
{
	return pageWidget_;
}

void AbstractSettingsSection::setShowPage(bool show)
{
	showPage_ = show;
}

QWidget *AbstractSettingsSection::createSettingsPageWidget(QWidget *parent)
{
	QWidget *pageContainer = new QWidget(parent);
	QVBoxLayout *pageLayout = new QVBoxLayout();
	pageLayout->setContentsMargins(0, 0, 0, 0);

	QGroupBox *groupBox = new QGroupBox();
	groupBox->setTitle(QString::fromStdString(displayName));
	groupBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

	QVBoxLayout *groupLayout = new QVBoxLayout();
	groupBox->setLayout(groupLayout);

	QPushButton *testButton = new QPushButton();
	testButton->setText("Settings Page Button for " + QString::fromStdString(displayName));

	pageContainer->setLayout(pageLayout);
	pageLayout->addWidget(groupBox);

	groupLayout->addWidget(testButton);

	emit pageCreated();

	return pageContainer;
}

void AbstractSettingsSection::registerSection() {}

void AbstractSettingsSection::activate(bool active)
{
	emit pageActivated(active);
}

void AbstractSettingsSection::load(bool force)
{
	return;
}

void AbstractSettingsSection::save()
{
	for (const auto &item : settingsItems) {
		manager().saveItemPending(item.get());
	}

	updatePendingChangesState(false);

	emit pageSaved();
}

void AbstractSettingsSection::discard()
{
	for (const auto &item : settingsItems) {
		item->clearPendingValue();
	}

	updatePendingChangesState(false);
}

void AbstractSettingsSection::clearChangeStates()
{
	//hasPendingChanges = false;
}

void AbstractSettingsSection::updatePendingChangesState(bool pending)
{
	hasPendingChanges = pending;
	emit sectionChanged(pending);
}

const bool AbstractSettingsSection::isPending() const
{
	return hasPendingChanges;
}

const bool AbstractSettingsSection::isValid() const
{
	for (const auto &item : settingsItems) {
		if (!item->isValid()) {
			return false;
		}
	}

	return true;
}

void AbstractSettingsSection::addSettingsItem(std::unique_ptr<SettingsItem> item)
{
	connect(item.get(), &SettingsItem::pendingChanged, this, &AbstractSettingsSection::itemChanged);

	const std::string &key = item->name();

	itemLookup[key] = item.get();
	settingsItems.emplace_back(std::move(item));
}

SettingsItem *AbstractSettingsSection::findSettingsItem(const char *name)
{
	return findSettingsItem(std::string(name));
}

SettingsItem *AbstractSettingsSection::findSettingsItem(const std::string &name)
{
	auto result = itemLookup.find(name);
	if (result != itemLookup.end()) {
		blog(LOG_INFO, "  -- Found settings item '%s'", name.c_str());
		return result->second;
	}

	blog(LOG_INFO, " -- Unable to find settings item '%s'", name.c_str());

	return nullptr;
}

void AbstractSettingsSection::initializeConfigItems(ConfigType config)
{
	for (const auto &item : settingsItems) {
		if (item->config() == config) {
			intializeSetting(item.get());
		}
	}
}

/* Overloads to grab section name from the caller */
SettingsItem *AbstractSettingsSection::registerSetting(std::string settingsName, const char *defaultValue,
						       ConfigType config)
{
	return manager().registerSetting(section(), settingsName, defaultValue, config);
}

SettingsItem *AbstractSettingsSection::registerSetting(std::string settingsName, int defaultValue, ConfigType config)
{
	return manager().registerSetting(section(), settingsName, defaultValue, config);
}

SettingsItem *AbstractSettingsSection::registerSetting(std::string settingsName, uint64_t defaultValue,
						       ConfigType config)
{
	return manager().registerSetting(section(), settingsName, defaultValue, config);
}

SettingsItem *AbstractSettingsSection::registerSetting(std::string settingsName, bool defaultValue, ConfigType config)
{
	return manager().registerSetting(section(), settingsName, defaultValue, config);
}

SettingsItem *AbstractSettingsSection::registerSetting(std::string settingsName, double defaultValue, ConfigType config)
{
	return manager().registerSetting(section(), settingsName, defaultValue, config);
}

SettingsItem *AbstractSettingsSection::connectSettingWidget(const std::string &name, QSpinBox *widget)
{
	SettingsItem *item = findSettingsItem(name);
	if (item) {
		applyItemValue(item, widget);
		bindPendingValue(item, widget);
		bindItemUpdates(item, widget);
	} else {
		blog(LOG_WARNING, "-- No setting is registered for %s -> %s. ", section().c_str(), name.c_str());
	}

	return item;
}

SettingsItem *AbstractSettingsSection::connectSettingWidget(const std::string &name, QDoubleSpinBox *widget)
{
	SettingsItem *item = findSettingsItem(name);
	if (item) {
		applyItemValue(item, widget);
		bindPendingValue(item, widget);
		bindItemUpdates(item, widget);
	} else {
		blog(LOG_WARNING, "-- No setting is registered for %s -> %s. ", section().c_str(), name.c_str());
	}

	return item;
}

SettingsItem *AbstractSettingsSection::connectSettingWidget(const std::string &name, QComboBox *widget)
{
	blog(LOG_INFO, "Connecting setting widget: %s...", name.c_str());

	SettingsItem *item = findSettingsItem(name);
	if (item) {
		blog(LOG_INFO, "-- Found setting item: %s", item->name().c_str());

		applyItemValue(item, widget);
		bindPendingValue(item, widget);
		bindItemUpdates(item, widget);
	} else {
		blog(LOG_WARNING, "-- No setting is registered for %s -> %s. ", section().c_str(), name.c_str());
	}

	return item;
}

SettingsItem *AbstractSettingsSection::connectSettingWidget(const std::string &name, QCheckBox *widget)
{
	SettingsItem *item = findSettingsItem(name);
	if (item) {
		applyItemValue(item, widget);
		bindPendingValue(item, widget);
		bindItemUpdates(item, widget);
	} else {
		blog(LOG_WARNING, "-- No setting is registered for %s -> %s. ", section().c_str(), name.c_str());
	}

	return item;
}

SettingsItem *AbstractSettingsSection::connectSettingWidget(const std::string &name, QSlider *widget)
{
	SettingsItem *item = findSettingsItem(name);
	if (item) {
		applyItemValue(item, widget);
		bindPendingValue(item, widget);
		bindItemUpdates(item, widget);
	} else {
		blog(LOG_WARNING, "-- No setting is registered for %s -> %s. ", section().c_str(), name.c_str());
	}

	return item;
}

void AbstractSettingsSection::applyItemValue(SettingsItem *item, QSpinBox *widget)
{
	applyItemValue(item, widget, noValueTransform);
}

void AbstractSettingsSection::applyItemValue(SettingsItem *item, QSpinBox *widget,
					     std::function<QVariant(QVariant)> transform)
{
	QVariant settingValue = transform(manager().getValue(item));

	if (settingValue.canConvert<int>()) {
		widget->blockSignals(true);
		widget->setValue(settingValue.toInt());
		widget->blockSignals(false);
	}
}

void AbstractSettingsSection::applyItemValue(SettingsItem *item, QDoubleSpinBox *widget)
{
	applyItemValue(item, widget, noValueTransform);
}

void AbstractSettingsSection::applyItemValue(SettingsItem *item, QDoubleSpinBox *widget,
					     std::function<QVariant(QVariant)> transform)
{
	QVariant settingValue = transform(manager().getValue(item));

	if (settingValue.canConvert<double>()) {
		widget->blockSignals(true);
		widget->setValue(settingValue.toDouble());
		widget->blockSignals(false);
	}
}

void AbstractSettingsSection::applyItemValue(SettingsItem *item, QComboBox *widget)
{
	applyItemValue(item, widget, noValueTransform);
}

void AbstractSettingsSection::applyItemValue(SettingsItem *item, QComboBox *widget,
					     std::function<QVariant(QVariant)> transform)
{
	QVariant settingValue = transform(manager().getValue(item));
	int index = widget->findData(settingValue);

	if (index == -1) {
		index = widget->findText(settingValue.toString());
	}

	if (index != -1) {
		widget->blockSignals(true);
		widget->setCurrentIndex(index);
		widget->blockSignals(false);
	} else {
		widget->insertItem(0, settingValue.toString(), settingValue.toString());
		widget->setCurrentIndex(0);

		QStandardItemModel *model = qobject_cast<QStandardItemModel *>(widget->model());
		if (model) {
			QStandardItem *item = model->item(0);
			if (item) {
				item->setFlags(Qt::NoItemFlags);
			}
		}
	}
}

void AbstractSettingsSection::applyItemValue(SettingsItem *item, QCheckBox *widget)
{
	applyItemValue(item, widget, noValueTransform);
}

void AbstractSettingsSection::applyItemValue(SettingsItem *item, QCheckBox *widget,
					     std::function<QVariant(QVariant)> transform)
{
	QVariant settingValue = transform(manager().getValue(item));

	if (settingValue.canConvert<bool>()) {
		widget->blockSignals(true);
		widget->setChecked(settingValue.toBool());
		widget->blockSignals(false);
	}
}

void AbstractSettingsSection::applyItemValue(SettingsItem *item, QSlider *widget)
{
	applyItemValue(item, widget, noValueTransform);
}

void AbstractSettingsSection::applyItemValue(SettingsItem *item, QSlider *widget,
					     std::function<QVariant(QVariant)> transform)
{
	QVariant settingValue = transform(manager().getValue(item));

	if (settingValue.canConvert<int>()) {
		widget->blockSignals(true);
		widget->setValue(settingValue.toInt());
		widget->blockSignals(false);
	}
}

void AbstractSettingsSection::bindPendingValue(SettingsItem *item, QSpinBox *widget)
{
	bindPendingValue(item, widget, noValueTransform);
}

void AbstractSettingsSection::bindPendingValue(SettingsItem *item, QSpinBox *widget,
					       std::function<QVariant(QVariant)> transform)
{
	connect(widget, &QSpinBox::textChanged, item, [item, transform](QString value) {
		bool valid;
		int newValue = transform(value).toInt(&valid);

		if (valid) {
			item->setPending(newValue);
		}
	});
}

void AbstractSettingsSection::bindPendingValue(SettingsItem *item, QDoubleSpinBox *widget)
{
	bindPendingValue(item, widget, noValueTransform);
}

void AbstractSettingsSection::bindPendingValue(SettingsItem *item, QDoubleSpinBox *widget,
					       std::function<QVariant(QVariant)> transform)
{
	connect(widget, &QDoubleSpinBox::textChanged, item, [item, transform](QString value) {
		bool valid;
		double newValue = transform(value).toDouble(&valid);

		if (valid) {
			item->setPending(newValue);
		}
	});
}

void AbstractSettingsSection::bindPendingValue(SettingsItem *item, QComboBox *widget)
{
	bindPendingValue(item, widget, noValueTransform);
}

void AbstractSettingsSection::bindPendingValue(SettingsItem *item, QComboBox *widget,
					       std::function<QVariant(QVariant)> transform)
{
	connect(widget, &QComboBox::currentIndexChanged, item, [item, widget, transform](int index) {
		QVariant data = widget->itemData(index);
		if (!data.isValid()) {
			data = widget->itemText(index);
		}

		QVariant value = transform(data);

		if (value.isValid()) {
			item->setPending(value);
		}
	});
}

void AbstractSettingsSection::bindPendingValue(SettingsItem *item, QCheckBox *widget)
{
	bindPendingValue(item, widget, noValueTransform);
}

void AbstractSettingsSection::bindPendingValue(SettingsItem *item, QCheckBox *widget,
					       std::function<QVariant(QVariant)> transform)
{
	connect(widget, &QAbstractButton::toggled, item, [item, transform](bool checked) {
		/* */
		item->setPending(transform(checked).toBool());
	});
}

void AbstractSettingsSection::bindPendingValue(SettingsItem *item, QSlider *widget)
{
	bindPendingValue(item, widget, noValueTransform);
}

void AbstractSettingsSection::bindPendingValue(SettingsItem *item, QSlider *widget,
					       std::function<QVariant(QVariant)> transform)
{
	connect(widget, &QSlider::valueChanged, item, [item, transform](int value) {
		bool valid;
		int newValue = transform(value).toInt(&valid);

		if (valid) {
			item->setPending(newValue);
		}
	});
}

void AbstractSettingsSection::bindItemUpdates(SettingsItem *item, QSpinBox *widget)
{
	bindItemUpdates(item, widget, noValueTransform);
}

void AbstractSettingsSection::bindItemUpdates(SettingsItem *item, QSpinBox *widget,
					      std::function<QVariant(QVariant)> transform)
{
	connect(item, &SettingsItem::valueChanged, widget, [this, widget, transform](QVariant newValue) {
		QVariant itemValue = transform(newValue);

		if (itemValue.canConvert<int>()) {
			widget->blockSignals(true);
			widget->setValue(itemValue.toInt());
			widget->blockSignals(false);
		}
	});
}

void AbstractSettingsSection::bindItemUpdates(SettingsItem *item, QDoubleSpinBox *widget)
{
	bindItemUpdates(item, widget, noValueTransform);
}

void AbstractSettingsSection::bindItemUpdates(SettingsItem *item, QDoubleSpinBox *widget,
					      std::function<QVariant(QVariant)> transform)
{
	connect(item, &SettingsItem::valueChanged, widget, [this, widget, transform](QVariant newValue) {
		QVariant itemValue = transform(newValue);

		if (itemValue.canConvert<double>()) {
			widget->blockSignals(true);
			widget->setValue(itemValue.toDouble());
			widget->blockSignals(false);
		}
	});
}

void AbstractSettingsSection::bindItemUpdates(SettingsItem *item, QComboBox *widget)
{
	bindItemUpdates(item, widget, noValueTransform);
}

void AbstractSettingsSection::bindItemUpdates(SettingsItem *item, QComboBox *widget,
					      std::function<QVariant(QVariant)> transform)
{
	connect(item, &SettingsItem::valueChanged, widget, [this, widget, transform](QVariant newValue) {
		QVariant itemValue = transform(newValue);

		int index = widget->findData(itemValue);

		if (index == -1) {
			index = widget->findText(itemValue.toString());
		}

		if (index) {
			widget->blockSignals(true);
			widget->setCurrentIndex(index);
			widget->blockSignals(false);
		}
	});
}

void AbstractSettingsSection::bindItemUpdates(SettingsItem *item, QCheckBox *widget)
{
	bindItemUpdates(item, widget, noValueTransform);
}

void AbstractSettingsSection::bindItemUpdates(SettingsItem *item, QCheckBox *widget,
					      std::function<QVariant(QVariant)> transform)
{
	connect(item, &SettingsItem::valueChanged, widget, [this, widget, transform](QVariant newValue) {
		QVariant itemValue = transform(newValue);

		if (itemValue.canConvert<bool>()) {
			widget->blockSignals(true);
			widget->setChecked(itemValue.toBool());
			widget->blockSignals(false);
		}
	});
}

void AbstractSettingsSection::bindItemUpdates(SettingsItem *item, QSlider *widget)
{
	bindItemUpdates(item, widget, noValueTransform);
}

void AbstractSettingsSection::bindItemUpdates(SettingsItem *item, QSlider *widget,
					      std::function<QVariant(QVariant)> transform)
{
	connect(item, &SettingsItem::valueChanged, widget, [this, widget, transform](QVariant newValue) {
		bool valid;
		int itemValue = transform(newValue).toInt(&valid);

		if (valid) {
			widget->blockSignals(true);
			widget->setValue(itemValue);
			widget->blockSignals(false);
		}
	});
}

void AbstractSettingsSection::bindChangeCallback(QObject *receiver, std::string section_, std::string name_,
						 std::function<void()> callback)
{
	auto *section = manager().findSection(section_);
	if (section) {
		SettingsItem *item = section->findSettingsItem(name_);

		if (item) {
			// Connect signal
			connect(item, &SettingsItem::pendingChanged, receiver, callback);
		}
	}
}

int AbstractSettingsSection::itemsRequireRestart()
{
	int itemsRequireRestart = 0;

	for (const auto &item : settingsItems) {
		if (item->requiresRestart()) {
			itemsRequireRestart++;
		}
	}

	return itemsRequireRestart;
}

/*
void AbstractSettingsSection::connectSettingWidget(const std::string &name, QWidget *widget)
{
	AbstractSettingsItem *item = findSettingsItem(name);

	QComboBox *combobox = dynamic_cast<QComboBox *>(widget);
	QLineEdit *lineEdit = dynamic_cast<QLineEdit *>(widget);
	QCheckBox *checkbox = dynamic_cast<QCheckBox *>(widget);
	QSpinBox *spinbox = dynamic_cast<QSpinBox *>(widget);
	QDoubleSpinBox *doubleSpinbox = dynamic_cast<QDoubleSpinBox *>(widget);

	if (combobox) {

	}

	if (checkbox) {
		checkbox->isChecked();
	}

	connect(ui->testSpinbox, &QSpinBox::textChanged, item, qOverload<QString>(&AbstractSettingsItem::setPending));

	/* Handle page load /
	connect(page(), &AbstractSettingsSection::pageCreated, this, [this, spinTest]() {
		QVariant settingValue = spinTest->getValue();
		blog(LOG_INFO, "Page created, set testSpinbox value to: %d", settingValue.toInt());
		ui->testSpinbox->blockSignals(true);
		ui->testSpinbox->setValue(settingValue.toInt());
		ui->testSpinbox->blockSignals(false);
	});
}
*/

void AbstractSettingsSection::handleCreate()
{
	hasPendingChanges = false;
}

void AbstractSettingsSection::itemChanged(QVariant newValue)
{
	updatePendingChangesState(isChanged());
}

AbstractSettingsPage::AbstractSettingsPage(QWidget *parent, AbstractSettingsSection *section)
	: QWidget(parent),
	  section_(section)
{
}

AbstractSettingsPage::~AbstractSettingsPage()
{
	section().setPageWidget(nullptr);
}

void AbstractSettingsPage::initialLoad() {}
void AbstractSettingsPage::bindChangeCallback(std::string section_, std::string name_, std::function<void()> callback)
{
	section().bindChangeCallback(this, std::move(section_), std::move(name_), std::move(callback));
}
void AbstractSettingsPage::handlePageActivated(bool isActive) {}
