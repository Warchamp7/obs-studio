/******************************************************************************
    Copyright (C) 2014 by Hugh Bailey <obs.jim@gmail.com>

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

#include <QMessageBox>
#include <QList>
#include "window-basic-main.hpp"
#include "window-basic-source-select.hpp"
#include "qt-wrappers.hpp"
#include "obs-app.hpp"

struct AddSourceData {
	obs_source_t *source;
	bool visible;
	obs_transform_info *transform = nullptr;
	obs_sceneitem_crop *crop = nullptr;
	obs_blending_method *blend_method = nullptr;
	obs_blending_type *blend_mode = nullptr;
};

bool OBSBasicSourceSelect::EnumSources(void *data, obs_source_t *source)
{
	if (obs_source_is_hidden(source))
		return false;

	OBSBasicSourceSelect *window =
		static_cast<OBSBasicSourceSelect *>(data);
	const char *name = obs_source_get_name(source);
	const char *id = obs_source_get_unversioned_id(source);

	if (strcmp(id, window->id.c_str()) == 0) {
		QPushButton *button = new QPushButton(name);
		button->setStyleSheet("text-align: left;");
		connect(button, &QPushButton::clicked, window,
			&OBSBasicSourceSelect::AddExistingSource);
		window->ui->addSource_ExistingListFrame->layout()->addWidget(
			button);
	}

	return true;
}

bool OBSBasicSourceSelect::EnumGroups(void *data, obs_source_t *source)
{
	OBSBasicSourceSelect *window =
		static_cast<OBSBasicSourceSelect *>(data);
	const char *name = obs_source_get_name(source);
	const char *id = obs_source_get_unversioned_id(source);

	if (strcmp(id, window->id.c_str()) == 0) {
		OBSBasic *main =
			reinterpret_cast<OBSBasic *>(App()->GetMainWindow());
		OBSScene scene = main->GetCurrentScene();

		obs_sceneitem_t *existing = obs_scene_get_group(scene, name);
		if (!existing) {
			QPushButton *button = new QPushButton(name);
			button->setStyleSheet("text-align: left;");
			connect(button, &QPushButton::clicked, window,
				&OBSBasicSourceSelect::AddExistingSource);
			window->ui->addSource_ExistingListFrame->layout()
				->addWidget(button);
		}
	}

	return true;
}

void OBSBasicSourceSelect::OBSSourceAdded(void *data, calldata_t *calldata)
{
	OBSBasicSourceSelect *window =
		static_cast<OBSBasicSourceSelect *>(data);
	obs_source_t *source = (obs_source_t *)calldata_ptr(calldata, "source");

	QMetaObject::invokeMethod(window, "SourceAdded",
				  Q_ARG(OBSSource, source));
}

void OBSBasicSourceSelect::OBSSourceRemoved(void *data, calldata_t *calldata)
{
	OBSBasicSourceSelect *window =
		static_cast<OBSBasicSourceSelect *>(data);
	obs_source_t *source = (obs_source_t *)calldata_ptr(calldata, "source");

	QMetaObject::invokeMethod(window, "SourceRemoved",
				  Q_ARG(OBSSource, source));
}

/* void OBSBasicSourceSelect::SourceAdded(OBSSource source)
{
	const char *name = obs_source_get_name(source);
	const char *sourceId = obs_source_get_unversioned_id(source);

	if (strcmp(sourceId, id) != 0)
		return;

	ui->sourceList->addItem(name);
} */

/* void OBSBasicSourceSelect::SourceRemoved(OBSSource source)
{
	const char *name = obs_source_get_name(source);
	const char *sourceId = obs_source_get_unversioned_id(source);

	if (strcmp(sourceId, id) != 0)
		return;

	QList<QListWidgetItem *> items =
		ui->sourceList->findItems(name, Qt::MatchFixedString);

	if (!items.count())
		return;

	delete items[0];
} */

static void AddSource(void *_data, obs_scene_t *scene)
{
	AddSourceData *data = (AddSourceData *)_data;
	obs_sceneitem_t *sceneitem;

	sceneitem = obs_scene_add(scene, data->source);

	if (data->transform != nullptr)
		obs_sceneitem_set_info(sceneitem, data->transform);
	if (data->crop != nullptr)
		obs_sceneitem_set_crop(sceneitem, data->crop);
	if (data->blend_method != nullptr)
		obs_sceneitem_set_blending_method(sceneitem,
						  *data->blend_method);
	if (data->blend_mode != nullptr)
		obs_sceneitem_set_blending_mode(sceneitem, *data->blend_mode);

	obs_sceneitem_set_visible(sceneitem, data->visible);
}

char *get_new_source_name(const char *name, const char *format)
{
	struct dstr new_name = {0};
	int inc = 0;

	dstr_copy(&new_name, name);

	for (;;) {
		OBSSourceAutoRelease existing_source =
			obs_get_source_by_name(new_name.array);
		if (!existing_source)
			break;

		dstr_printf(&new_name, format, name, ++inc + 1);
	}

	return new_name.array;
}

static void AddExisting(OBSSource source, bool visible, bool duplicate,
			obs_transform_info *transform, obs_sceneitem_crop *crop,
			obs_blending_method *blend_method,
			obs_blending_type *blend_mode)
{
	OBSBasic *main = reinterpret_cast<OBSBasic *>(App()->GetMainWindow());
	OBSScene scene = main->GetCurrentScene();
	if (!scene)
		return;

	if (duplicate) {
		OBSSource from = source;
		char *new_name = get_new_source_name(
			obs_source_get_name(source), "%s %d");
		source = obs_source_duplicate(from, new_name, false);
		obs_source_release(source);
		bfree(new_name);

		if (!source)
			return;
	}

	AddSourceData data;
	data.source = source;
	data.visible = visible;
	data.transform = transform;
	data.crop = crop;
	data.blend_method = blend_method;
	data.blend_mode = blend_mode;

	obs_enter_graphics();
	obs_scene_atomic_update(scene, AddSource, &data);
	obs_leave_graphics();
}

static void AddExisting(const char *name, bool visible, bool duplicate,
			obs_transform_info *transform, obs_sceneitem_crop *crop,
			obs_blending_method *blend_method,
			obs_blending_type *blend_mode)
{
	OBSSourceAutoRelease source = obs_get_source_by_name(name);
	if (source) {
		AddExisting(source.Get(), visible, duplicate, transform, crop,
			    blend_method, blend_mode);
	}
}

bool AddNew(QWidget *parent, const char *id, const char *name,
	    const bool visible, OBSSource &newSource)
{
	OBSBasic *main = reinterpret_cast<OBSBasic *>(App()->GetMainWindow());
	OBSScene scene = main->GetCurrentScene();
	bool success = false;
	if (!scene)
		return false;

	OBSSourceAutoRelease source = obs_get_source_by_name(name);
	if (source && parent) {
		OBSMessageBox::information(parent, QTStr("NameExists.Title"),
					   QTStr("NameExists.Text"));

	} else {
		const char *v_id = obs_get_latest_input_type_id(id);
		source = obs_source_create(v_id, name, NULL, nullptr);

		if (source) {
			AddSourceData data;
			data.source = source;
			data.visible = visible;

			obs_enter_graphics();
			obs_scene_atomic_update(scene, AddSource, &data);
			obs_leave_graphics();

			newSource = source;

			/* set monitoring if source monitors by default */
			uint32_t flags = obs_source_get_output_flags(source);
			if ((flags & OBS_SOURCE_MONITOR_BY_DEFAULT) != 0) {
				obs_source_set_monitoring_type(
					source,
					OBS_MONITORING_TYPE_MONITOR_ONLY);
			}

			success = true;
		}
	}

	return success;
}

void OBSBasicSourceSelect::on_addSourceButton_CreateNew_clicked(bool checked)
{
	bool visible = ui->addSource_SourceVisible->isChecked();

	if (ui->lineEdit->text().isEmpty())
		return;

	if (!AddNew(this, id.c_str(), QT_TO_UTF8(ui->lineEdit->text()), visible,
		    newSource))
		return;

	if (newSource &&
	    strcmp(obs_source_get_id(newSource.Get()), "group") != 0) {
		OBSBasic *main =
			reinterpret_cast<OBSBasic *>(App()->GetMainWindow());
		std::string scene_name =
			obs_source_get_name(main->GetCurrentSceneSource());
		auto undo = [scene_name, main](const std::string &data) {
			OBSSourceAutoRelease source =
				obs_get_source_by_name(data.c_str());
			obs_source_remove(source);

			OBSSourceAutoRelease scene_source =
				obs_get_source_by_name(scene_name.c_str());
			main->SetCurrentScene(scene_source.Get(), true);
		};
		OBSDataAutoRelease wrapper = obs_data_create();
		obs_data_set_string(wrapper, "id", id.c_str());
		OBSSceneItemAutoRelease item = obs_scene_sceneitem_from_source(
			main->GetCurrentScene(), newSource);
		obs_data_set_int(wrapper, "item_id",
				 obs_sceneitem_get_id(item));
		obs_data_set_string(wrapper, "name",
				    ui->lineEdit->text().toUtf8().constData());
		obs_data_set_bool(wrapper, "visible", visible);

		auto redo = [scene_name, main](const std::string &data) {
			OBSSourceAutoRelease scene_source =
				obs_get_source_by_name(scene_name.c_str());
			main->SetCurrentScene(scene_source.Get(), true);

			OBSDataAutoRelease dat =
				obs_data_create_from_json(data.c_str());
			OBSSource source;
			AddNew(NULL, obs_data_get_string(dat, "id"),
			       obs_data_get_string(dat, "name"),
			       obs_data_get_bool(dat, "visible"), source);
			OBSSceneItemAutoRelease item =
				obs_scene_sceneitem_from_source(
					main->GetCurrentScene(), source);
			obs_sceneitem_set_id(item, (int64_t)obs_data_get_int(
							   dat, "item_id"));
		};
		undo_s.add_action(QTStr("Undo.Add").arg(ui->lineEdit->text()),
				  undo, redo,
				  std::string(obs_source_get_name(newSource)),
				  std::string(obs_data_get_json(wrapper)));

		main->CreatePropertiesWindow(newSource);
	}
	close();
	UNUSED_PARAMETER(checked);
}

static inline const char *GetSourceDisplayName(const char *id)
{
	if (strcmp(id, "scene") == 0)
		return Str("Basic.Scene");
	const char *v_id = obs_get_latest_input_type_id(id);
	return obs_source_get_display_name(v_id);
}

Q_DECLARE_METATYPE(OBSScene);

template<typename T> static inline T GetOBSRef(QListWidgetItem *item)
{
	return item->data(static_cast<int>(QtDataRole::OBSRef)).value<T>();
}

OBSBasicSourceSelect::OBSBasicSourceSelect(OBSBasic *parent, undo_stack &undo_s)
	: QDialog(parent), ui(new Ui::OBSBasicSourceSelect), undo_s(undo_s)
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	ui->setupUi(this);

	ui->addSource_SelectSourceFrame->setVisible(false);

	const char *unversioned_type;
	const char *type;
	bool foundValues = false;
	bool foundDeprecated = false;
	size_t idx = 0;

	sourceButtons = new QButtonGroup(this);

	QList<QPushButton *> *list = new QList<QPushButton *>();

	auto getButtonAfter = [this, &list](const QString &name) {
		for (int i = 0; i < list->size(); ++i) {
			QPushButton *btn = list->at(i);
			if (btn && btn->text().compare(name) >= 0)
				return i;
		}
		return list->size();
	};

	auto addSource = [this, parent, list,
			  getButtonAfter](bool deprecated, const char *type,
					  const char *name) {
		QString qname = QT_UTF8(name);
		QPushButton *sourceBtn = new QPushButton(qname, this);
		sourceBtn->setCheckable(true);
		sourceBtn->setStyleSheet("text-align: left;");
		sourceBtn->setProperty("obs_type", QT_UTF8(type));

		QIcon icon;

		if (strcmp(type, "scene") == 0)
			icon = parent->GetSceneIcon();
		else
			icon = parent->GetSourceIcon(type);

		sourceBtn->setIcon(icon);
		int after = getButtonAfter(qname);
		list->insert(after, sourceBtn);
		sourceButtons->addButton(sourceBtn);
	};

	while (obs_enum_input_types2(idx++, &type, &unversioned_type)) {
		const char *name = obs_source_get_display_name(type);
		uint32_t caps = obs_get_source_output_flags(type);

		if ((caps & OBS_SOURCE_CAP_DISABLED) != 0)
			continue;

		if ((caps & OBS_SOURCE_DEPRECATED) == 0) {
			addSource(false, unversioned_type, name);
		} else {
			addSource(true, unversioned_type, name);
			foundDeprecated = true;
		}
		foundValues = true;
	}

	addSource(false, "scene", Str("Basic.Scene"));

	QPushButton *groupBtn = new QPushButton(QTStr("Group"), this);
	groupBtn->setProperty("obs_type", QT_UTF8("group"));
	groupBtn->setIcon(parent->GetGroupIcon());
	groupBtn->setStyleSheet("text-align: left;");
	groupBtn->setCheckable(true);
	list->append(groupBtn);
	sourceButtons->addButton(groupBtn);

	QLayout *layout = ui->addSource_ButtonGrid->layout();
	QGridLayout *grid = (QGridLayout *)layout;
	int row = 0;
	int column = 0;
	for (int i = 0; i < list->size(); ++i) {
		if (column == 2) {
			row += 1;
			column = 0;
		}
		QPushButton *btn = list->at(i);
		// TODO Re-enable once spacer behaves correctly
		/*if (btn->property("obs_type").toString().compare("group") ==
			 0) {
			QFrame *line = line = new QFrame(this);
			line->setFrameShape(QFrame::HLine);
			line->setFrameShadow(QFrame::Sunken);
			row += 1;
			grid->addWidget(line, row, 0, 1, 2);
			row += 1;
			column = 0;
		}*/
		grid->addWidget(btn, row, column);
		column += 1;
	}

	row += 1;
	QSpacerItem *spacer = new QSpacerItem(10, 20, QSizePolicy::Fixed,
					      QSizePolicy::Expanding);
	//layout->addItem(spacer);
	grid->addItem(spacer, row, 1, 1, Qt::AlignHCenter);
	connect(sourceButtons, SIGNAL(buttonClicked(int)), this,
		SLOT(SelectSourceType()));

	App()->DisableHotkeys();
}

OBSBasicSourceSelect::~OBSBasicSourceSelect()
{
	App()->UpdateHotkeyFocusSetting();
}

void OBSBasicSourceSelect::AddExistingSource(bool checked)
{
	QPushButton *btn = (QPushButton *)sender();
	bool visible = ui->addSource_SourceVisible->isChecked();

	QString source_name = btn->text();
	OBSSourceAutoRelease source =
		obs_get_source_by_name(source_name.toStdString().c_str());
	if (source) {
		AddExisting(source.Get(), visible, false, nullptr, nullptr,
			    nullptr, nullptr);

		OBSBasic *main =
			reinterpret_cast<OBSBasic *>(App()->GetMainWindow());
		const char *scene_name =
			obs_source_get_name(main->GetCurrentSceneSource());

		auto undo = [scene_name, main](const std::string &data) {
			UNUSED_PARAMETER(data);
			obs_source_t *scene_source =
				obs_get_source_by_name(scene_name);
			main->SetCurrentScene(scene_source, true);
			obs_source_release(scene_source);

			obs_scene_t *scene = obs_get_scene_by_name(scene_name);
			OBSSceneItem item;
			auto cb = [](obs_scene_t *scene,
				     obs_sceneitem_t *sceneitem, void *data) {
				UNUSED_PARAMETER(scene);
				OBSSceneItem &last =
					*reinterpret_cast<OBSSceneItem *>(data);
				last = sceneitem;
				return true;
			};
			obs_scene_enum_items(scene, cb, &item);

			obs_sceneitem_remove(item);
			obs_scene_release(scene);
		};

		auto redo = [scene_name, main, source_name,
			     visible](const std::string &data) {
			UNUSED_PARAMETER(data);
			obs_source_t *scene_source =
				obs_get_source_by_name(scene_name);
			main->SetCurrentScene(scene_source, true);
			obs_source_release(scene_source);
			AddExisting(QT_TO_UTF8(source_name), visible, false,
				    nullptr, nullptr, nullptr, nullptr);
		};

		undo_s.add_action(QTStr("Undo.Add").arg(source_name), undo,
				  redo, "", "");
	}
	close();
	UNUSED_PARAMETER(checked);
}

void OBSBasicSourceSelect::SelectSourceType()
{
	QPushButton *btn = (QPushButton *)sourceButtons->checkedButton();
	QString type = btn->property("obs_type").toString();
	if (type.toStdString().compare(id) == 0)
		return;

	id = type.toStdString();

	QString placeHolderText{QT_UTF8(GetSourceDisplayName(id.c_str()))};

	QString text{placeHolderText};
	int i = 2;
	OBSSourceAutoRelease source = nullptr;
	while ((source = obs_get_source_by_name(QT_TO_UTF8(text)))) {
		text = QString("%1 %2").arg(placeHolderText).arg(i++);
	}

	ui->lineEdit->setText(text);
	ui->lineEdit->setFocus(); //Fixes deselect of text.
	ui->lineEdit->selectAll();

	QLayout *layout = ui->addSource_ExistingListFrame->layout();

	// Clear existing buttons when switching types
	QLayoutItem *child = nullptr;
	while ((child = layout->takeAt(1)) != nullptr) {
		delete child->widget();
		delete child;
	}

	ui->addSource_SelectSourceFrame->setVisible(true);

	if (strcmp(id.c_str(), "scene") == 0) {
		OBSBasic *main =
			reinterpret_cast<OBSBasic *>(App()->GetMainWindow());
		OBSSource curSceneSource = main->GetCurrentSceneSource();

		int count = main->ui->scenes->count();
		for (int i = 0; i < count; i++) {
			QListWidgetItem *item = main->ui->scenes->item(i);
			OBSScene scene = GetOBSRef<OBSScene>(item);
			OBSSource sceneSource = obs_scene_get_source(scene);

			if (curSceneSource == sceneSource)
				continue;

			const char *name = obs_source_get_name(sceneSource);
			QPushButton *button = new QPushButton(name);
			button->setStyleSheet("text-align: left;");
			connect(button, &QPushButton::clicked, this,
				&OBSBasicSourceSelect::AddExistingSource);
			layout->addWidget(button);
		}
	} else if (strcmp(id.c_str(), "group") == 0) {
		obs_enum_sources(EnumGroups, this);
	} else {
		obs_enum_sources(EnumSources, this);
	}
}

void OBSBasicSourceSelect::SourcePaste(SourceCopyInfo &info, bool dup)
{
	OBSSource source = OBSGetStrongRef(info.weak_source);
	if (!source)
		return;

	AddExisting(source, &info.visible, dup, &info.transform, &info.crop,
		    &info.blend_method, &info.blend_mode);
}
