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

#include "display-helpers.hpp"
#include "ThumbnailManager.hpp"

#include <QImageWriter>

ThumbnailManager::ThumbnailManager()
{
	texrender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);

	signal_handler_t *sh = obs_get_signal_handler();
	sigs.emplace_back(sh, "source_create", obsSourceAdded, this);
	sigs.emplace_back(sh, "source_remove", obsSourceRemoved, this);

	sigs.emplace_back(sh, "source_activate", obsSourceActivated, this);
	sigs.emplace_back(sh, "source_deactivate", obsSourceDeactivated, this);

	updateTimer = new QTimer();
	connect(updateTimer, &QTimer::timeout, this, &ThumbnailManager::updateTick);
	updateTimer->start(200);
}

ThumbnailManager::~ThumbnailManager() {}

QPixmap ThumbnailManager::getThumbnail(OBSSource source)
{
	const char *uuid = obs_source_get_uuid(source);

	if (thumbnails.find(uuid) != thumbnails.end()) {
		return thumbnails[uuid].pixmap;
	}

	return QPixmap();
}

void ThumbnailManager::obsSourceAdded(void *param, calldata_t *calldata)
{
	OBSSource source((obs_source_t *)calldata_ptr(calldata, "source"));

	QMetaObject::invokeMethod(static_cast<ThumbnailManager *>(param), "sourceAdded", Q_ARG(OBSSource, source));
}

void ThumbnailManager::obsSourceRemoved(void *param, calldata_t *calldata)
{
	OBSSource source((obs_source_t *)calldata_ptr(calldata, "source"));

	QMetaObject::invokeMethod(static_cast<ThumbnailManager *>(param), "sourceRemoved", Q_ARG(OBSSource, source));
}

void ThumbnailManager::obsSourceActivated(void *param, calldata_t *calldata)
{
	OBSSource source((obs_source_t *)calldata_ptr(calldata, "source"));

	QMetaObject::invokeMethod(static_cast<ThumbnailManager *>(param), "sourceActivated", Q_ARG(OBSSource, source));
}

void ThumbnailManager::obsSourceDeactivated(void *param, calldata_t *calldata)
{
	OBSSource source((obs_source_t *)calldata_ptr(calldata, "source"));

	QMetaObject::invokeMethod(static_cast<ThumbnailManager *>(param), "sourceDeactivated",
				  Q_ARG(OBSSource, source));
}

void ThumbnailManager::sourceAdded(OBSSource source)
{
	const char *name = obs_source_get_name(source);
	blog(LOG_INFO, "[ThumbnailManager] Add source '%s' to thumbnails list", name);

	obs_weak_source_t *weak = obs_source_get_weak_source(source);

	// Add new entry to list
	ThumbnailItem newEntry = {weak, 0, false, false, QPixmap()};

	thumbnails.insert({obs_source_get_uuid(source), newEntry});
}

void ThumbnailManager::sourceRemoved(OBSSource source)
{
	const char *name = obs_source_get_name(source);
	blog(LOG_INFO, "[ThumbnailManager] Remove source '%s' from thumbnails list", name);

	const char *uuid = obs_source_get_uuid(source);
	if (thumbnails.find(uuid) != thumbnails.end()) {
		thumbnails[uuid].removed = true;
	}
}

void ThumbnailManager::sourceActivated(OBSSource source)
{
	const char *name = obs_source_get_name(source);
	blog(LOG_INFO, "[ThumbnailManager] Set '%s' as active", name);

	const char *uuid = obs_source_get_uuid(source);
	if (thumbnails.find(uuid) != thumbnails.end()) {
		thumbnails[uuid].active = true;
	}
}

void ThumbnailManager::sourceDeactivated(OBSSource source)
{
	const char *name = obs_source_get_name(source);
	blog(LOG_INFO, "[ThumbnailManager] Set '%s' as inactive", name);

	const char *uuid = obs_source_get_uuid(source);
	if (thumbnails.find(uuid) != thumbnails.end()) {
		thumbnails[uuid].active = false;
	}
}

void ThumbnailManager::updateTick()
{
	if (thumbnails.size() == 0) {
		return;
	}

	const char *uuid;
	ThumbnailItem item;

	while (updateIterator != thumbnails.end()) {
		uuid = updateIterator->first;
		item = updateIterator->second;

		if (item.removed) {
			blog(LOG_INFO, "[ThumbnailManager] Advancing past removed source");

			std::advance(updateIterator, 1);
			thumbnails.erase(uuid);
			continue;
		}

		if (!item.source) {
			break;
		}

		OBSSourceAutoRelease source = obs_weak_source_get_source(item.source);

		if (!source) {
			std::advance(updateIterator, 1);
			continue;
		}

		uint32_t flags = obs_source_get_output_flags(source);

		time_t now = std::time(0);
		bool updatedRecently = std::time(0) - item.lastUpdate < 5;

		if ((flags & OBS_SOURCE_VIDEO) == 0 || updatedRecently) {
			std::advance(updateIterator, 1);
			continue;
		} else if (!item.active) {
			const char *id = obs_source_get_unversioned_id(source);

			// Always update previews for scenes even when not active
			if (strcmp(id, "scene") == 0) {
				break;
			} else if (item.lastUpdate == 0) {
				break;
			} else {
				std::advance(updateIterator, 1);
				continue;
			}
		} else {
			break;
		}
	}

	if (updateIterator == thumbnails.end()) {
		updateIterator = thumbnails.begin();
		return;
	}

	if (!item.source) {
		return;
	}

	OBSSourceAutoRelease source = obs_weak_source_get_source(item.source);

	const char *name = obs_source_get_name(source);
	blog(LOG_INFO, "[ThumbnailManager] Update thumbnail for %s", name);

	obs_enter_graphics();

	int width = 320;
	int height = 180;

	gs_texrender_reset(texrender);
	if (gs_texrender_begin(texrender, width, height)) {
		vec4 clear_color = {0.0f, 0.0f, 0.0f, 1.0f}; // RGBA
		gs_clear(GS_CLEAR_COLOR, &clear_color, 1.0f, 0);

		int x, y;
		int newCX, newCY;
		float scale;

		uint32_t sourceCX = std::max(obs_source_get_width(source), 1u);
		uint32_t sourceCY = std::max(obs_source_get_height(source), 1u);

		GetScaleAndCenterPos(sourceCX, sourceCY, width, height, x, y, scale);

		newCX = int(scale * float(sourceCX));
		newCY = int(scale * float(sourceCY));

		gs_viewport_push();
		gs_projection_push();

		gs_ortho(0.0f, float(sourceCX), 0.0f, float(sourceCY), -100.0f, 100.0f);
		gs_set_viewport(x, y, newCX, newCY);

		if (item.lastUpdate == 0) {
			// Force render for sources with no preview yet
			obs_source_inc_showing(source);
			obs_source_video_render(source);
			obs_source_dec_showing(source);
		} else {
			obs_source_video_render(source);
		}

		gs_projection_pop();
		gs_viewport_pop();

		gs_texrender_end(texrender);
	};

	QPixmap pixmap;
	gs_texture_t *tex = gs_texrender_get_texture(texrender);
	if (tex) {
		gs_stagesurf_t *stage = gs_stagesurface_create(width, height, GS_RGBA);
		gs_stage_texture(stage, tex);

		uint8_t *data;
		uint32_t linesize;
		if (gs_stagesurface_map(stage, &data, &linesize)) {
			// Copy to QImage
			QImage image(data, width, height, linesize, QImage::Format_RGBA8888);
			pixmap = QPixmap::fromImage(image);

			// QImageWriter writer("F:/Debug.png");
			// writer.write(image);

			// Use pixmap
			gs_stagesurface_unmap(stage);
		}
		gs_stagesurface_destroy(stage);
	}
	obs_leave_graphics();

	if (!pixmap.isNull()) {
		blog(LOG_INFO, "[ThumbnailManager] Update tick success");
		thumbnails[uuid].pixmap = pixmap;
		thumbnails[uuid].lastUpdate = std::time(0);
	}
}
