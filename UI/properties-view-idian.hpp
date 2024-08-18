#pragma once

#include <QWidget>
#include "obs.hpp"
#include "idian/obs-widgets.hpp"

class OBSPropertiesViewIdian : public OBSPropertiesList {
	Q_OBJECT
public:
	OBSPropertiesViewIdian(obs_properties_t *props, OBSData settings, QWidget *parent = nullptr);
private:
	OBSData settings;

	void AddProperty(obs_property_t *prop);
	void CreateProperties(obs_properties_t *props);

	OBSActionBaseClass *AddPropertyBool(obs_property_t *prop);
	OBSActionBaseClass *AddPropertyInt(obs_property_t *prop);
	OBSActionBaseClass *AddPropertyNull(obs_property_t *prop);
};
