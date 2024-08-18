#include "properties-view-idian.hpp"

#include <util/dstr.hpp>

OBSPropertiesViewIdian::OBSPropertiesViewIdian(obs_properties_t *props, OBSData settings, QWidget *parent) : OBSPropertiesList(parent), settings(settings) {
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	
	CreateProperties(props);
}


OBSActionBaseClass *OBSPropertiesViewIdian::AddPropertyBool(obs_property_t *prop) {
	const char *name = obs_property_name(prop);
	OBSActionRow *row = new OBSActionRow(this);
	row->setSuffix(new OBSToggleSwitch(obs_data_get_bool(settings, name)));
	return row;
}

OBSActionBaseClass *OBSPropertiesViewIdian::AddPropertyInt(obs_property_t *prop) {
	const char *name = obs_property_name(prop);
	const int min = obs_property_int_min(prop);
	const int max = obs_property_int_max(prop);
	const int step = obs_property_int_step(prop);
	const enum obs_number_type type = obs_property_int_type(prop);
	const int value = obs_data_get_int(settings, name);



	OBSActionRow *row = new OBSActionRow(this);
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

OBSActionBaseClass *OBSPropertiesViewIdian::AddPropertyNull(obs_property_t *prop) {
	OBSActionRow *row = new OBSActionRow(this);
	DStr str;
	dstr_printf(str, "Unsupported property: %s", obs_property_name(prop));
	row->setDescription(str->array);
	return row;
}

void OBSPropertiesViewIdian::AddProperty(obs_property_t *prop) {
	OBSActionBaseClass *row = nullptr;
	switch(obs_property_get_type(prop)) {
		case OBS_PROPERTY_INVALID:
			row = AddPropertyNull(prop);
			break;
		case OBS_PROPERTY_BOOL:
			row = AddPropertyBool(prop);
			break;
		case OBS_PROPERTY_INT:
			row = AddPropertyInt(prop);
			break;
		default:
			row = AddPropertyNull(prop);
			break;
	}
	if (OBSActionRow *actionRow = dynamic_cast<OBSActionRow *>(row)) {
		actionRow->setTitle(obs_property_description(prop));
	}

	// TODO set disabled, etc.
	addRow(row);
}

void OBSPropertiesViewIdian::CreateProperties(obs_properties_t *props) {
	clear();


	obs_property_t *prop = obs_properties_first(props);
	
	if(!prop) {
		blog(LOG_INFO, "no properties");
		return;
	}
	
	do {
		AddProperty(prop);
	} while(obs_property_next(&prop));
}

