#pragma once

#include <QWidget>
#include "obs.hpp"
#include "vertical-scroll-area.hpp"
#include "idian/obs-widgets.hpp"

class OBSPropertiesViewIdian : public VScrollArea {
	Q_OBJECT
public:
	OBSPropertiesViewIdian(obs_properties_t *props, OBSData settings,
			       QWidget *parent = nullptr);

private:
	OBSData settings;
	OBSGroupBox *groupBox;

	QWidget *scrollAreaWidgetContents;
	QVBoxLayout *scrollContentsLayout;

	void AddProperties(obs_properties_t *props);

	OBSActionBaseClass *CreateProperty(obs_property_t *prop);
	OBSActionBaseClass *CreatePropertyBool(obs_property_t *prop);
	OBSActionBaseClass *CreatePropertyInt(obs_property_t *prop);
	OBSActionBaseClass *CreatePropertyList(obs_property_t *prop);
	OBSActionBaseClass *CreatePropertyGroup(obs_property_t *prop);
	OBSActionBaseClass *CreatePropertyNull(obs_property_t *prop);
};
