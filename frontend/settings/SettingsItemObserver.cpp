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

#include "SettingsItemObserver.hpp"

#include <settings/AbstractSettingsSection.hpp>

SettingsItemObserver::SettingsItemObserver(AbstractSettingsPage *parent) : QObject(parent), manager(&parent->manager())
{
	connect(&updateTimer, &QTimer::timeout, this, &SettingsItemObserver::queuedUpdate);
	updateTimer.setSingleShot(true);
}

void SettingsItemObserver::setCallback(std::function<bool()> callback_)
{
	callback = std::move(callback_);
}

void SettingsItemObserver::setCallback(SettingsItemObserver *self,
				       std::function<bool(SettingsItemObserver *)> callback_)
{
	QPointer<SettingsItemObserver> watcher = self;
	callback = [watcher, callback_ = std::move(callback_)]() {
		if (watcher) {
			return callback_(watcher);
		}

		return false;
	};
}

void SettingsItemObserver::watch(std::string section_, std::string name_)
{
	AbstractSettingsSection *section = manager->findSection(section_);
	if (section) {
		SettingsItem *item = section->findSettingsItem(name_);

		if (item) {
			// Connect signal
			connect(item, &SettingsItem::pendingChanged, this, &SettingsItemObserver::watchChanged);
		}
	}
}

void SettingsItemObserver::watch(SettingsItemObserver *observer)
{
	connect(observer, &SettingsItemObserver::updated, this, &SettingsItemObserver::watchChanged);
}

void SettingsItemObserver::update()
{
	if (callback) {
		bool result = callback();
		setEnabled(result);
		emit updated(result);
	}
}

void SettingsItemObserver::watchChanged()
{
	if (!updateTimer.isActive()) {
		updateTimer.start(10);
	}
	emit changed();
}

void SettingsItemObserver::setEnabled(bool value)
{
	enabled_ = value;
}

bool SettingsItemObserver::enabled()
{
	return enabled_;
}

void SettingsItemObserver::queuedUpdate()
{
	update();
}
