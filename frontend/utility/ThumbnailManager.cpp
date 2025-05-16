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

#define UPDATE_INTERVAL 100

ThumbnailManager::ThumbnailManager()
{
	texrender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);

	signal_handler_t *sh = obs_get_signal_handler();
	sigs.emplace_back(sh, "source_create", obsSourceAdded, this);
	sigs.emplace_back(sh, "source_remove", obsSourceRemoved, this);

	updateTimer = new QTimer();
	connect(updateTimer, &QTimer::timeout, this, &ThumbnailManager::updateTick);
	updateTimer->start(UPDATE_INTERVAL);
}

ThumbnailManager::~ThumbnailManager() {}

QPixmap ThumbnailManager::getThumbnail(OBSSource source)
{
	std::string uuid = obs_source_get_uuid(source);

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

void ThumbnailManager::sourceAdded(OBSSource source)
{
	const char *name = obs_source_get_name(source);
	blog(LOG_INFO, "[ThumbnailManager] Add source '%s' to thumbnails list", name);

	// Add new entry to list
	std::string uuid = obs_source_get_uuid(source);
	ThumbnailItem newEntry = {uuid, {}, QPixmap()};

	thumbnails.insert({uuid, newEntry});
}

void ThumbnailManager::sourceRemoved(OBSSource source)
{
	const char *name = obs_source_get_name(source);
	blog(LOG_INFO, "[ThumbnailManager] Remove source '%s' from thumbnails list", name);

	std::string uuid = obs_source_get_uuid(source);
	if (thumbnails.find(uuid) != thumbnails.end()) {
		thumbnails.erase(uuid);
	}
}

void ThumbnailManager::updateTick()
{
	if (thumbnails.size() == 0) {
		return;
	}

	std::string uuid;
	ThumbnailItem item;

	bool doUpdate = false;

	auto steadyNow = steady_clock::now();

	for (auto &entry : thumbnails) {
		uuid = entry.first;
		item = entry.second;

		OBSSourceAutoRelease source = obs_get_source_by_uuid(item.uuid.c_str());
		if (!source) {
			thumbnails.erase(uuid);
			continue;
		}

		const char *name = obs_source_get_name(source);

		bool isShowing = obs_source_showing(source);
		uint32_t flags = obs_source_get_output_flags(source);

		bool updatedRecently = false;
		long long lastUpdateDuration = 0;

		long long previewOutOfDate = std::max((long long)(5000 + (UPDATE_INTERVAL * thumbnails.size())), 30LL * 1000);

		if (item.lastUpdate) {
			lastUpdateDuration = duration_cast<milliseconds>(steadyNow - *item.lastUpdate).count();
			if (lastUpdateDuration < previewOutOfDate) {
				updatedRecently = true;
			}
		}

		if ((flags & OBS_SOURCE_VIDEO) == 0) {
			continue;
		}

		if (!item.lastUpdate.has_value()) {
			// Force update previews for newly added entries
			blog(LOG_INFO, "[ThumbnailManager] Update source with no update yet '%s'", name);

			doUpdate = true;
			break;
		} else if (!updatedRecently) {
			const char *id = obs_source_get_unversioned_id(source);

			if (isShowing) {
				blog(LOG_INFO,
				     "[ThumbnailManager] Source '%s' last updated %.2f seconds ago. Limit: %.2fs", name,
				     (float)lastUpdateDuration / 1000, (float)previewOutOfDate / 1000);

				doUpdate = true;
				break;
			} else if (strcmp(id, "scene") == 0) {
				// Always update previews for scenes even when not active
				blog(LOG_INFO,
				     "[ThumbnailManager] Scene '%s' last updated %.2f seconds ago. Limit: %.2fs", name,
				     (float)lastUpdateDuration / 1000, (float)previewOutOfDate / 1000);

				doUpdate = true;
				break;
			}
		}
	}

	if (!doUpdate) {
		return;
	}

	QPixmap pixmap = generateThumbnail(texrender, item.uuid, !item.lastUpdate.has_value());
	if (!pixmap.isNull()) {
		thumbnails[uuid].pixmap = pixmap;
		thumbnails[uuid].lastUpdate = steadyNow;
	}
}

QPixmap ThumbnailManager::generateThumbnail(gs_texrender_t *texrender, std::string uuid, bool forceShow)
{
	QPixmap pixmap;

	OBSSourceAutoRelease source = obs_get_source_by_uuid(uuid.c_str());
	if (!source) {
		return pixmap;
	}

	obs_enter_graphics();

	int width = 320;
	int height = 180;

	gs_texrender_reset(texrender);
	if (gs_texrender_begin(texrender, width, height)) {
		vec4 clear_color = {0.0f, 0.0f, 0.0f, 0.0f};
		if (!obs_source_showing(source)) {
			clear_color = {0.0f, 0.0f, 0.0f, 0.5f};
		}
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

		if (forceShow) {
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

	return pixmap;
}
