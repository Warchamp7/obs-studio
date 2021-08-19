/******************************************************************************
    Copyright (C) 2023 by Lain Bailey <lain@obsproject.com>

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

#include "OBSBasicSourceSelect.hpp"

#include <QMessageBox>
#include <QList>

#include "qt-wrappers.hpp"
#include "OBSApp.hpp"

#include "moc_OBSBasicSourceSelect.cpp"

struct AddSourceData {
	/* Input data */
	obs_source_t *source;
	bool visible;
	obs_transform_info *transform = nullptr;
	obs_sceneitem_crop *crop = nullptr;
	obs_blending_method *blend_method = nullptr;
	obs_blending_type *blend_mode = nullptr;

	/* Return data */
	obs_sceneitem_t *scene_item = nullptr;
};

static inline const char *getSourceDisplayName(const char *id)
{
	if (strcmp(id, "scene") == 0) {
		return Str("Basic.Scene");
	}
	const char *v_id = obs_get_latest_input_type_id(id);
	return obs_source_get_display_name(v_id);
}

char *getNewSourceName(const char *name, const char *format)
{
	struct dstr new_name = {0};
	int inc = 0;

	dstr_copy(&new_name, name);

	for (;;) {
		OBSSourceAutoRelease existing_source = obs_get_source_by_name(new_name.array);
		if (!existing_source)
			break;

		dstr_printf(&new_name, format, name, ++inc + 1);
	}

	return new_name.array;
}

static void AddSource(void *_data, obs_scene_t *scene)
{
	AddSourceData *data = (AddSourceData *)_data;
	obs_sceneitem_t *sceneitem;

	sceneitem = obs_scene_add(scene, data->source);

	if (data->transform != nullptr)
		obs_sceneitem_set_info2(sceneitem, data->transform);
	if (data->crop != nullptr)
		obs_sceneitem_set_crop(sceneitem, data->crop);
	if (data->blend_method != nullptr)
		obs_sceneitem_set_blending_method(sceneitem, *data->blend_method);
	if (data->blend_mode != nullptr)
		obs_sceneitem_set_blending_mode(sceneitem, *data->blend_mode);

	obs_sceneitem_set_visible(sceneitem, data->visible);

	data->scene_item = sceneitem;
}

static void AddExisting(OBSSource source, bool visible, bool duplicate, obs_transform_info *transform,
			obs_sceneitem_crop *crop, obs_blending_method *blend_method, obs_blending_type *blend_mode)
{
	OBSBasic *main = OBSBasic::Get();
	OBSScene scene = main->GetCurrentScene();
	if (!scene)
		return;

	if (duplicate) {
		OBSSource from = source;
		char *new_name = getNewSourceName(obs_source_get_name(source), "%s %d");
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

static void AddExisting(const char *name, bool visible, bool duplicate, obs_transform_info *transform,
			obs_sceneitem_crop *crop, obs_blending_method *blend_method, obs_blending_type *blend_mode)
{
	OBSSourceAutoRelease source = obs_get_source_by_name(name);
	if (source) {
		AddExisting(source.Get(), visible, duplicate, transform, crop, blend_method, blend_mode);
	}
}

bool AddNew(QWidget *parent, const char *id, const char *name, const bool visible, OBSSource &newSource,
	    OBSSceneItem &newSceneItem)
{
	OBSBasic *main = OBSBasic::Get();
	OBSScene scene = main->GetCurrentScene();
	bool success = false;
	if (!scene)
		return false;

	OBSSourceAutoRelease source = obs_get_source_by_name(name);
	if (source && parent) {
		OBSMessageBox::information(parent, QTStr("NameExists.Title"), QTStr("NameExists.Text"));

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
			newSceneItem = data.scene_item;

			/* set monitoring if source monitors by default */
			uint32_t flags = obs_source_get_output_flags(source);
			if ((flags & OBS_SOURCE_MONITOR_BY_DEFAULT) != 0) {
				obs_source_set_monitoring_type(source, OBS_MONITORING_TYPE_MONITOR_ONLY);
			}

			success = true;
		}
	}

	return success;
}

OBSBasicSourceSelect::OBSBasicSourceSelect(OBSBasic *parent, undo_stack &undo_s)
	: QDialog(parent),
	  ui(new Ui::OBSBasicSourceSelect),
	  undo_s(undo_s)
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	ui->setupUi(this);

	/* The scroll viewport is not accessible via Designer, so we have to disable autoFillBackground here.
	 * 
	 * Additionally when Qt calls setWidget on a scrollArea to set the contents widget, it force sets
	 * autoFillBackground to true overriding whatever is set in Designer so we have to do that here too.
	 */
	ui->sourceTypeScrollArea->viewport()->setAutoFillBackground(false);
	ui->scrollAreaWidgetContents->setAutoFillBackground(false);

	ui->recentScrollArea->viewport()->setAutoFillBackground(false);
	ui->recentScrollContents->setAutoFillBackground(false);

	ui->existingScrollArea->viewport()->setAutoFillBackground(false);
	ui->existingScrollContents->setAutoFillBackground(false);

	ui->selectRecentFrame->setVisible(true);
	ui->selectExistingFrame->setVisible(false);
	ui->createNewFrame->setVisible(false);

	getSourceTypes();
	getSources();
	updateRecentSources();

	connect(ui->lineEdit, &QLineEdit::returnPressed, this, &OBSBasicSourceSelect::createNewSource);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	App()->DisableHotkeys();
}

OBSBasicSourceSelect::~OBSBasicSourceSelect()
{
	App()->UpdateHotkeyFocusSetting();
}

void OBSBasicSourceSelect::getSources()
{
	sources.clear();

	obs_enum_sources(enumSourcesCallback, this);
	emit sourcesUpdated();
}

void OBSBasicSourceSelect::updateRecentSources()
{
	int count = 0;
	for (const obs_source_t *source : sources) {
		if (count >= 6) {
			return;
		}

		const char *name = obs_source_get_name(source);

		QPushButton *button = new QPushButton(name);
		button->setProperty("class", "add-item-button");
		connect(button, &QPushButton::clicked, this, &OBSBasicSourceSelect::addExistingSource);
		ui->recentListLayout->insertWidget(0, button);

		count++;
	}
}

void OBSBasicSourceSelect::updateExistingSources()
{
	for (const obs_source_t *source : sources) {
		const char *id = obs_source_get_unversioned_id(source);

		if (strcmp(id, sourceTypeId.c_str()) == 0) {
			const char *name = obs_source_get_name(source);

			QPushButton *button = new QPushButton(name);
			button->setProperty("class", "add-item-button");
			connect(button, &QPushButton::clicked, this, &OBSBasicSourceSelect::addExistingSource);
			ui->existingListLayout->insertWidget(0, button);
		}
	}
}

bool OBSBasicSourceSelect::enumSourcesCallback(void *data, obs_source_t *source)
{
	if (obs_source_is_hidden(source))
		return true;

	OBSBasicSourceSelect *window = static_cast<OBSBasicSourceSelect *>(data);

	/*
	const char *name = obs_source_get_name(source);
	const char *id = obs_source_get_unversioned_id(source);

	if (strcmp(id, window->sourceTypeId.c_str()) == 0) {
		QPushButton *button = new QPushButton(name);
		connect(button, &QPushButton::clicked, window, &OBSBasicSourceSelect::addExistingSource);
		window->ui->existingListLayout->insertWidget(0, button);
	}
	*/

	window->sources.push_back(source);

	return true;
}

bool OBSBasicSourceSelect::enumGroupsCallback(void *data, obs_source_t *source)
{
	OBSBasicSourceSelect *window = static_cast<OBSBasicSourceSelect *>(data);
	const char *name = obs_source_get_name(source);
	const char *id = obs_source_get_unversioned_id(source);

	if (strcmp(id, window->sourceTypeId.c_str()) == 0) {
		OBSBasic *main = OBSBasic::Get();
		OBSScene scene = main->GetCurrentScene();

		obs_sceneitem_t *existing = obs_scene_get_group(scene, name);
		if (!existing) {
			QPushButton *button = new QPushButton(name);
			connect(button, &QPushButton::clicked, window, &OBSBasicSourceSelect::addExistingSource);
			window->ui->existingListFrame->layout()->addWidget(button);
		}
	}

	return true;
}

void OBSBasicSourceSelect::OBSSourceAdded(void *data, calldata_t *calldata)
{
	OBSBasicSourceSelect *window = static_cast<OBSBasicSourceSelect *>(data);
	obs_source_t *source = (obs_source_t *)calldata_ptr(calldata, "source");

	QMetaObject::invokeMethod(window, "SourceAdded", Q_ARG(OBSSource, source));
}

int OBSBasicSourceSelect::getSortedButtonPosition(const QList<QPushButton *> *list, const char *name)
{
	QString qname = QT_UTF8(name);

	const int total = list->size();
	for (int i = 0; i < total; ++i) {
		QPushButton *btn = list->at(i);
		if (btn && btn->text().compare(name) >= 0)
			return i;
	}
	return total;
}

QPointer<QPushButton> OBSBasicSourceSelect::createTypeButton(const char *type, const char *name)
{
	OBSBasic *main = reinterpret_cast<OBSBasic *>(App()->GetMainWindow());

	QString qname = QT_UTF8(name);
	QPushButton *sourceBtn = new QPushButton(qname, this);
	sourceBtn->setCheckable(true);
	sourceBtn->setProperty("obs_type", QT_UTF8(type));

	QIcon icon;
	if (strcmp(type, "scene") == 0)
		icon = main->GetSceneIcon();
	else
		icon = main->GetSourceIcon(type);

	sourceBtn->setIcon(icon);

	return sourceBtn;
}

void OBSBasicSourceSelect::getSourceTypes()
{
	const char *unversioned_type;
	const char *type;

	size_t idx = 0;

	sourceButtons = new QButtonGroup(this);
	QList<QPushButton *> *list = new QList<QPushButton *>();

	while (obs_enum_input_types2(idx++, &type, &unversioned_type)) {
		const char *name = obs_source_get_display_name(type);
		uint32_t caps = obs_get_source_output_flags(type);

		if ((caps & OBS_SOURCE_CAP_DISABLED) != 0)
			continue;

		QPushButton *typeBtn = createTypeButton(unversioned_type, name);
		int after = getSortedButtonPosition(list, name);
		list->insert(after, typeBtn);
		sourceButtons->addButton(typeBtn);
	}

	QPushButton *typeBtn = createTypeButton("scene", Str("Basic.Scene"));
	int after = getSortedButtonPosition(list, Str("Basic.Scene"));
	list->insert(after, typeBtn);
	sourceButtons->addButton(typeBtn);

	QLayout *layout = ui->typeButtonsGrid->layout();
	QGridLayout *grid = (QGridLayout *)layout;
	int row = 1;
	int column = 0;
	for (int i = 0; i < list->size(); ++i) {
		if (column == 2) {
			row += 1;
			column = 0;
		}
		QPushButton *btn = list->at(i);

		grid->addWidget(btn, row, column);
		column += 1;

		if (i == 0) {
			setTabOrder(ui->sourceTypeScrollArea, btn);
		}
	}

	ui->sourceTypeScrollArea->setFocus();

	row += 1;
	QSpacerItem *spacer = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Expanding);
	//layout->addItem(spacer);
	grid->addItem(spacer, row, 1, 1, Qt::AlignHCenter);
	connect(sourceButtons, &QButtonGroup::buttonClicked, this, &OBSBasicSourceSelect::sourceTypeClicked);
}

void OBSBasicSourceSelect::OBSSourceRemoved(void *data, calldata_t *calldata)
{
	OBSBasicSourceSelect *window = static_cast<OBSBasicSourceSelect *>(data);
	obs_source_t *source = (obs_source_t *)calldata_ptr(calldata, "source");

	QMetaObject::invokeMethod(window, "SourceRemoved", Q_ARG(OBSSource, source));
}

void OBSBasicSourceSelect::createNewSource()
{
	bool visible = ui->sourceVisible->isChecked();

	if (ui->lineEdit->text().isEmpty())
		return;

	OBSSceneItem item;
	if (!AddNew(this, sourceTypeId.c_str(), QT_TO_UTF8(ui->lineEdit->text()), visible, newSource, item))
		return;

	if (newSource && strcmp(obs_source_get_id(newSource.Get()), "group") != 0) {
		OBSBasic *main = reinterpret_cast<OBSBasic *>(App()->GetMainWindow());
		std::string scene_name = obs_source_get_name(main->GetCurrentSceneSource());
		auto undo = [scene_name, main](const std::string &data) {
			OBSSourceAutoRelease source = obs_get_source_by_name(data.c_str());
			obs_source_remove(source);

			OBSSourceAutoRelease scene_source = obs_get_source_by_name(scene_name.c_str());
			main->SetCurrentScene(scene_source.Get(), true);
		};
		OBSDataAutoRelease wrapper = obs_data_create();
		obs_data_set_string(wrapper, "id", sourceTypeId.c_str());
		obs_data_set_int(wrapper, "item_id", obs_sceneitem_get_id(item));
		obs_data_set_string(wrapper, "name", ui->lineEdit->text().toUtf8().constData());
		obs_data_set_bool(wrapper, "visible", visible);

		auto redo = [scene_name, main](const std::string &data) {
			OBSSourceAutoRelease scene_source = obs_get_source_by_name(scene_name.c_str());
			main->SetCurrentScene(scene_source.Get(), true);

			OBSDataAutoRelease dat = obs_data_create_from_json(data.c_str());
			OBSSource source;
			OBSSceneItem item;
			AddNew(NULL, obs_data_get_string(dat, "id"), obs_data_get_string(dat, "name"),
			       obs_data_get_bool(dat, "visible"), source, item);
			obs_sceneitem_set_id(item, (int64_t)obs_data_get_int(dat, "item_id"));
		};
		undo_s.add_action(QTStr("Undo.Add").arg(ui->lineEdit->text()), undo, redo,
				  std::string(obs_source_get_name(newSource)), std::string(obs_data_get_json(wrapper)));

		main->CreatePropertiesWindow(newSource);
	}
	close();
}

void OBSBasicSourceSelect::on_createNewSource_clicked(bool checked)
{
	createNewSource();
	UNUSED_PARAMETER(checked);
}

void OBSBasicSourceSelect::addExistingSource(bool checked)
{
	QPushButton *btn = (QPushButton *)sender();
	bool visible = ui->sourceVisible->isChecked();

	QString source_name = btn->text();
	OBSSourceAutoRelease source = obs_get_source_by_name(source_name.toStdString().c_str());
	if (source) {
		AddExisting(source.Get(), visible, false, nullptr, nullptr, nullptr, nullptr);

		OBSBasic *main = OBSBasic::Get();
		const char *scene_name = obs_source_get_name(main->GetCurrentSceneSource());

		auto undo = [scene_name, main](const std::string &) {
			obs_source_t *scene_source = obs_get_source_by_name(scene_name);
			main->SetCurrentScene(scene_source, true);
			obs_source_release(scene_source);

			obs_scene_t *scene = obs_get_scene_by_name(scene_name);
			OBSSceneItem item;
			auto cb = [](obs_scene_t *, obs_sceneitem_t *sceneitem, void *data) {
				OBSSceneItem &last = *reinterpret_cast<OBSSceneItem *>(data);
				last = sceneitem;
				return true;
			};
			obs_scene_enum_items(scene, cb, &item);

			obs_sceneitem_remove(item);
			obs_scene_release(scene);
		};

		auto redo = [scene_name, main, source_name, visible](const std::string &) {
			obs_source_t *scene_source = obs_get_source_by_name(scene_name);
			main->SetCurrentScene(scene_source, true);
			obs_source_release(scene_source);
			AddExisting(QT_TO_UTF8(source_name), visible, false, nullptr, nullptr, nullptr, nullptr);
		};

		undo_s.add_action(QTStr("Undo.Add").arg(source_name), undo, redo, "", "");
	}
	close();
	UNUSED_PARAMETER(checked);
}

void OBSBasicSourceSelect::sourceTypeClicked(QAbstractButton *button)
{
	QString type = button->property("obs_type").toString();
	if (type.toStdString().compare(sourceTypeId) == 0)
		return;

	sourceTypeId = type.toUtf8();

	QString placeHolderText{QT_UTF8(getSourceDisplayName(sourceTypeId.c_str()))};

	QString text{placeHolderText};
	int i = 2;
	OBSSourceAutoRelease source = nullptr;
	while ((source = obs_get_source_by_name(QT_TO_UTF8(text)))) {
		text = QString("%1 %2").arg(placeHolderText).arg(i++);
	}

	ui->lineEdit->setText(text);
	ui->lineEdit->setFocus(); //Fixes deselect of text.
	ui->lineEdit->selectAll();

	QLayout *layout = ui->existingListFrame->layout();

	// Clear existing buttons when switching types
	QLayoutItem *child = nullptr;
	while ((child = layout->takeAt(0)) != nullptr) {
		delete child->widget();
		delete child;
	}

	ui->selectRecentFrame->setVisible(false);
	ui->selectExistingFrame->setVisible(true);
	ui->createNewFrame->setVisible(true);

	if (strcmp(sourceTypeId.c_str(), "scene") == 0) {
		OBSBasic *main = reinterpret_cast<OBSBasic *>(App()->GetMainWindow());
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
			button->setProperty("class", "add-item-button");
			button->setAccessibleDescription(QTStr("Basic.SourceSelect.Accessible.Existing"));
			connect(button, &QPushButton::clicked, this, &OBSBasicSourceSelect::addExistingSource);
			layout->addWidget(button);

			if (i == 0) {
				setTabOrder(ui->existingScrollArea, button);
			}
		}
	} else if (strcmp(sourceTypeId.c_str(), "group") == 0) {
		obs_enum_sources(enumGroupsCallback, this);
	} else {
		updateExistingSources();
	}

	if (layout->count() == 0) {
		QLabel *noExisting = new QLabel();
		noExisting->setText(
			QTStr("Basic.SourceSelect.NoExisting").arg(getSourceDisplayName(sourceTypeId.c_str())));
		noExisting->setProperty("class", "text-muted");
		layout->addWidget(noExisting);
	}
}

void OBSBasicSourceSelect::SourcePaste(SourceCopyInfo &info, bool dup)
{
	OBSSource source = OBSGetStrongRef(info.weak_source);
	if (!source)
		return;

	AddExisting(source, info.visible, dup, &info.transform, &info.crop, &info.blend_method, &info.blend_mode);
}
