#include "properties-view-idian.hpp"

#include <util/dstr.hpp>

OBSPropertiesViewIdian::OBSPropertiesViewIdian(obs_properties_t *props,
					       OBSData settings,
					       QWidget *parent)
	: VScrollArea(parent),
	  settings(settings),
	  groupBox(new OBSGroupBox(this))
{
	groupBox->setTitle("Properties");
	setWidgetResizable(true);
	setFrameShape(Shape::NoFrame);

	scrollAreaWidgetContents = new QWidget();
	scrollAreaWidgetContents->setObjectName("scrollAreaWidgetContents");

	scrollContentsLayout = new QVBoxLayout(scrollAreaWidgetContents);
	scrollContentsLayout->setObjectName("scrollContentsLayout");
	scrollContentsLayout->setContentsMargins(0, 0, 0, 0);

	setWidget(scrollAreaWidgetContents);
	scrollContentsLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

	scrollContentsLayout->addWidget(groupBox);
	AddProperties(props);
}

OBSActionBaseClass *
OBSPropertiesViewIdian::CreatePropertyBool(obs_property_t *prop)
{
	const char *name = obs_property_name(prop);
	OBSActionRow *row = new OBSActionRow();
	row->setSuffix(new OBSToggleSwitch(obs_data_get_bool(settings, name)));
	return row;
}

OBSActionBaseClass *
OBSPropertiesViewIdian::CreatePropertyInt(obs_property_t *prop)
{
	const char *name = obs_property_name(prop);
	const int min = obs_property_int_min(prop);
	const int max = obs_property_int_max(prop);
	const int step = obs_property_int_step(prop);
	const enum obs_number_type type = obs_property_int_type(prop);
	const int value = obs_data_get_int(settings, name);

	OBSActionRow *row = new OBSActionRow();
	// TODO Slider vs Spinbox
	OBSSpinBox *obsSpinBox = new OBSSpinBox(row);
	QSpinBox *spinBox = obsSpinBox->spinBox();
	spinBox->setMinimum(min);
	spinBox->setMaximum(max);
	spinBox->setSingleStep(step);
	spinBox->setValue(value);
	row->setSuffix(obsSpinBox);
	return row;
}

OBSActionBaseClass *
OBSPropertiesViewIdian::CreatePropertyList(obs_property_t *prop)
{
	const char *name = obs_property_name(prop);
	enum obs_combo_type type = obs_property_list_type(prop);
	enum obs_combo_format format = obs_property_list_format(prop);

	OBSActionRow *row = new OBSActionRow();
	// TODO: Honor type
	OBSComboBox *comboBox = new OBSComboBox(row);
	if (type == OBS_COMBO_TYPE_EDITABLE) {
		comboBox->setEditable(true);
	}
	size_t count = obs_property_list_item_count(prop);
	for (size_t i = 0; i < count; ++i) {
		const char *name = obs_property_list_item_name(prop, i);
		comboBox->addItem(name); //TODO: This. Obv.
	}
	row->setSuffix(comboBox);
	return row;
}

OBSActionBaseClass *
OBSPropertiesViewIdian::CreatePropertyGroup(obs_property_t *prop)
{
	OBSCollapsibleContainer *container = new OBSCollapsibleContainer(
		obs_property_description(prop), nullptr);
	enum obs_group_type type = obs_property_group_type(prop);
	if (type == OBS_GROUP_CHECKABLE) {
		container->setCheckable(true);
		//TODO Set checked or not
	}

	obs_properties_t *subprops = obs_property_group_content(prop);
	obs_property_t *subprop = obs_properties_first(subprops);
	do {
		OBSActionBaseClass *row = CreateProperty(subprop);
		if (row) {
			container->addRow(row);
		}
	} while (obs_property_next(&subprop));

	return container;
}

OBSActionBaseClass *
OBSPropertiesViewIdian::CreatePropertyNull(obs_property_t *prop)
{
	OBSActionRow *row = new OBSActionRow();
	DStr str;
	dstr_printf(str, "Unsupported property: %s", obs_property_name(prop));
	row->setDescription(str->array);
	return row;
}

OBSActionBaseClass *OBSPropertiesViewIdian::CreateProperty(obs_property_t *prop)
{
	if (!obs_property_visible(prop)) {
		return nullptr;
	}

	OBSActionBaseClass *row = nullptr;
	switch (obs_property_get_type(prop)) {
	case OBS_PROPERTY_INVALID:
		row = CreatePropertyNull(prop);
		break;
	case OBS_PROPERTY_BOOL:
		row = CreatePropertyBool(prop);
		break;
	case OBS_PROPERTY_INT:
		row = CreatePropertyInt(prop);
		break;
	//case OBS_PROPERTY_FLOAT:
	//case OBS_PROPERTY_TEXT:
	//case OBS_PROPERTY_PATH:
	case OBS_PROPERTY_LIST:
		row = CreatePropertyList(prop);
		break;
	//case OBS_PROPERTY_COLOR:
	//case OBS_PROPERTY_BUTTON:
	//case OBS_PROPERTY_FONT:
	//case OBS_PROPERTY_EDITABLE_LIST:
	//case OBS_PROPERTY_FRAME_RATE:
	case OBS_PROPERTY_GROUP:
		row = CreatePropertyGroup(prop); // TODO: Incredibly broken
	//case OBS_PROPERTY_COLOR_ALPHA:
	default:
		row = CreatePropertyNull(prop);
		break;
	}
	if (OBSActionRow *actionRow = dynamic_cast<OBSActionRow *>(row)) {
		actionRow->setTitle(obs_property_description(prop));
	}

	// TODO set disabled, etc.

	return row;
}

void OBSPropertiesViewIdian::AddProperties(obs_properties_t *props)
{
	groupBox->properties()->clear();

	obs_property_t *prop = obs_properties_first(props);

	if (!prop) {
		blog(LOG_INFO, "no properties");
		return;
	}

	do {
		OBSActionBaseClass *row = CreateProperty(prop);
		if (row) {
			groupBox->addRow(row);
		}
	} while (obs_property_next(&prop));
}
