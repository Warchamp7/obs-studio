/******************************************************************************
    Copyright (C) 2025 by Taylor Giampaolo <warchamp7@obsproject.com>

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

#include <obs.hpp>

#include <QObject>
#include <QPixmap>
#include <QTimer>

struct ThumbnailItem {
	OBSWeakSource source;
	time_t lastUpdate;
	bool active;
	bool removed;
	QPixmap pixmap;
};

class ThumbnailManager : public QObject {
	Q_OBJECT

public:
	ThumbnailManager();
	~ThumbnailManager();

	QPixmap getThumbnail(OBSSource source);

private:
	std::unordered_map<const char *, ThumbnailItem> thumbnails;
	std::unordered_map<const char *, ThumbnailItem>::iterator updateIterator = thumbnails.begin();

	QTimer *updateTimer;

	std::vector<OBSSignal> sigs;

	static void obsSourceAdded(void *param, calldata_t *calldata);
	static void obsSourceRemoved(void *param, calldata_t *calldata);
	static void obsSourceActivated(void *param, calldata_t *calldata);
	static void obsSourceDeactivated(void *param, calldata_t *calldata);

	void updateTick();

	gs_texrender_t *texrender;

public slots:
	void sourceAdded(OBSSource source);
	void sourceRemoved(OBSSource source);

	void sourceActivated(OBSSource source);
	void sourceDeactivated(OBSSource source);
};
