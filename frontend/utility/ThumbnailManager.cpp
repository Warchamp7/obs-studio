/******************************************************************************
    Copyright (C) 2025 by Taylor Giampaolo <warchamp7@obsproject.com>
                          Lain Bailey <lain@obsproject.com>

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

#include "ThumbnailManager.hpp"

#include <utility/ThumbnailView.hpp>
#include <widgets/OBSBasic.hpp>

#include "display-helpers.hpp"

#include <QImageWriter>

constexpr int MIN_THUMBNAIL_UPDATE_INTERVAL_MS = 100;
constexpr int MIN_SOURCE_UPDATE_INTERVAL_MS = 5000;

ThumbnailManager::ThumbnailManager(QObject *parent) : QObject(parent)
{
	connect(&updateTimer, &QTimer::timeout, this, &ThumbnailManager::updateTick);
	updateIntervalFromTotal(0);

	signalHandlers.emplace_back(obs_get_signal_handler(), "source_destroy", ThumbnailManager::obsSourceRemoved,
				    this);
}

ThumbnailManager::~ThumbnailManager() {}

ThumbnailView *ThumbnailManager::createView(QWidget *parent, OBSSource &source)
{
	if (!source) {
		return new ThumbnailView(parent, nullptr);
	}

	const char *uuidPointer = obs_source_get_uuid(source);
	if (!uuidPointer) {
		return new ThumbnailView(parent, nullptr);
	}

	std::string uuid{uuidPointer};

	auto item = getThumbnailItem(uuid, source);
	ThumbnailView *view = new ThumbnailView(parent, item);

	item->incrementViewCount();
	if (view->isEnabled()) {
		item->incrementEnabledCount();
	}

	// Connect ThumbnailView signals
	connect(view, &QObject::destroyed, item.get(), [=]() {
		item->decrementViewCount();
		if (view->isEnabled()) {
			item->decrementEnabledCount();
		}
	});
	connect(view, &ThumbnailView::enabledChanged, item.get(), [=](bool enabled) {
		if (enabled) {
			item->incrementEnabledCount();
		} else {
			item->decrementEnabledCount();
		}
	});
	connect(view, &ThumbnailView::updateRequested, this, &ThumbnailManager::addToPriorityQueue);

	return view;
}

void ThumbnailManager::createThumbnailItem(const std::string &uuid, OBSSource &source)
{
	QPointer<ThumbnailItem> item = new ThumbnailItem(uuid, source, this);
	addToPriorityQueue(uuid);
	thumbnailList[uuid] = item;

	connect(item, &ThumbnailItem::closing, item, [=]() { deleteItemById(item->getUuid()); });

	auto cachedPixmap = thumbnailCache.get(uuid);
	if (cachedPixmap.has_value()) {
		item->setPixmap(cachedPixmap.value());
	}
}

QPointer<ThumbnailItem> ThumbnailManager::getThumbnailItem(const std::string &uuid, OBSSource &source)
{
	if (thumbnailList.find(uuid) == thumbnailList.end()) {
		createThumbnailItem(uuid, source);
	}

	return thumbnailList[uuid];
}

void ThumbnailManager::obsSourceRemoved(void *data, calldata_t *params)
{
	obs_source_t *source = (obs_source_t *)calldata_ptr(params, "source");
	auto uuidPointer = obs_source_get_uuid(source);

	if (uuidPointer) {
		QMetaObject::invokeMethod(static_cast<ThumbnailManager *>(data), &ThumbnailManager::deleteItemById,
					  Qt::QueuedConnection, std::string(uuidPointer));
	}
}

bool ThumbnailManager::updateItem(ThumbnailItem *item)
{
	if (!item) {
		return false;
	}

	return item->update();
}

void ThumbnailManager::updateIntervalFromTotal(size_t newCount)
{
	int intervalMS = MIN_THUMBNAIL_UPDATE_INTERVAL_MS;
	if (priorityQueue.size() == 0 && newCount > 0) {
		int count = (int)newCount;
		intervalMS = MIN_SOURCE_UPDATE_INTERVAL_MS / count;
		if (intervalMS < MIN_THUMBNAIL_UPDATE_INTERVAL_MS) {
			intervalMS = MIN_THUMBNAIL_UPDATE_INTERVAL_MS;
		}
	}

	if (updateTimer.interval() != intervalMS) {
		updateTimer.start(intervalMS);
	}
}

void ThumbnailManager::updateNextItem(int cycleDepth)
{
	if (thumbnailList.size() == 0) {
		return;
	}

	QPointer<ThumbnailItem> item;

	if (priorityQueue.size() > 0) {
		std::string uuid = priorityQueue.front();
		priorityQueue.pop_front();

		item = thumbnailList[uuid];
		updateQueue.push_back(uuid);

		if (!updateItem(item) && cycleDepth < thumbnailList.size()) {
			updateNextItem(cycleDepth + 1);
			return;
		}
	} else if (updateQueue.size() > 0) {
		std::string uuid = updateQueue.front();

		updateQueue.pop_front();
		item = thumbnailList[uuid];
		updateQueue.push_back(uuid);

		if (!updateItem(item) && cycleDepth < thumbnailList.size()) {
			updateNextItem(cycleDepth + 1);
			return;
		}
	}

	updateIntervalFromTotal(thumbnailList.size());
}

void ThumbnailManager::updateTick()
{
	updateNextItem();
}

void ThumbnailManager::addToPriorityQueue(const std::string &uuid)
{
	auto it = std::find(updateQueue.begin(), updateQueue.end(), uuid);
	if (it != updateQueue.end()) {
		updateQueue.erase(it);
	}

	it = std::find(priorityQueue.begin(), priorityQueue.end(), uuid);
	if (it != priorityQueue.end()) {
		priorityQueue.erase(it);
	}

	priorityQueue.push_back(std::string(uuid));
}

void ThumbnailManager::addItemToCache(std::string &uuid, QPixmap &pixmap)
{
	if (pixmap.isNull()) {
		return;
	}

	thumbnailCache.put(uuid, pixmap);
}

void ThumbnailManager::deleteItemById(std::string uuid)
{
	auto entry = thumbnailList.find(uuid);
	if (entry != thumbnailList.end()) {
		for (auto it = updateQueue.begin(); it != updateQueue.end();) {
			if (*it == uuid) {
				it = updateQueue.erase(it);
			} else {
				++it;
			}
		}

		for (auto it = priorityQueue.begin(); it != priorityQueue.end();) {
			if (*it == uuid) {
				it = priorityQueue.erase(it);
			} else {
				++it;
			}
		}

		auto item = entry->second;

		if (item) {
			std::string uuid = item->getUuid();
			QPixmap pixmap = item->getPixmap();
			addItemToCache(uuid, pixmap);
		}

		thumbnailList.erase(uuid);
	}
}
