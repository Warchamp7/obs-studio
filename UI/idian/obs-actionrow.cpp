/******************************************************************************
    Copyright (C) 2023 by Dennis Sädtler <dennis@obsproject.com>

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

#include <QComboBox>
#include <QApplication>
#include <QSizePolicy>

#include "obs-actionrow.hpp"
#include <util/base.h>
#include <QSvgRenderer>

OBSActionRow::OBSActionRow(const QString &name, QWidget *parent)
	: OBSActionBaseClass(parent)
{
	layout = new QGridLayout(this);
	layout->setVerticalSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);

	QSizePolicy policy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	setFocusPolicy(Qt::StrongFocus);
	setSizePolicy(policy);
	setLayout(layout);
	setAccessibleName(name);

	layout->setColumnMinimumWidth(0, 0);
	layout->setColumnStretch(0, 0);
	layout->setColumnStretch(1, 40);
	layout->setColumnStretch(2, 55);

	nameLbl = new QLabel();
	nameLbl->setText(name);
	OBSWidgetUtils::addClass(nameLbl, "title");

	layout->addWidget(nameLbl, 0, 1, Qt::AlignLeft);
}

OBSActionRow::OBSActionRow(const QString &name, const QString &desc,
			   QWidget *parent)
	: OBSActionRow(name, parent)
{
	descLbl = new QLabel();
	descLbl->setText(desc);
	OBSWidgetUtils::addClass(descLbl, "subtitle");
	setAccessibleDescription(desc);

	layout->addWidget(descLbl, 1, 1, Qt::AlignLeft);
}

void OBSActionRow::setPrefix(QWidget *w, bool auto_connect)
{
	setSuffixEnabled(false);

	int rowspan = !!descLbl ? 2 : 1;
	_prefix = w;

	if (auto_connect)
		this->autoConnectWidget(w);

	_prefix->setParent(this);
	layout->addWidget(_prefix, 0, 0, rowspan, 1, Qt::AlignLeft);
	layout->setColumnStretch(0, 3);
}

void OBSActionRow::setSuffix(QWidget *w, bool auto_connect)
{
	setPrefixEnabled(false);

	int rowspan = !!descLbl ? 2 : 1;
	_suffix = w;

	if (auto_connect)
		this->autoConnectWidget(w);

	_suffix->setParent(this);
	layout->addWidget(_suffix, 0, 2, rowspan, 1,
			  Qt::AlignRight | Qt::AlignVCenter);
}

void OBSActionRow::setPrefixEnabled(bool enabled)
{
	if (!_prefix)
		return;
	if (enabled)
		setSuffixEnabled(false);
	if (enabled == _prefix->isEnabled() && enabled == _prefix->isVisible())
		return;

	layout->setColumnStretch(0, enabled ? 3 : 0);
	_prefix->setEnabled(enabled);
	_prefix->setVisible(enabled);
}

void OBSActionRow::setSuffixEnabled(bool enabled)
{
	if (!_suffix)
		return;
	if (enabled)
		setPrefixEnabled(false);
	if (enabled == _suffix->isEnabled() && enabled == _suffix->isVisible())
		return;

	_suffix->setEnabled(enabled);
	_suffix->setVisible(enabled);
}

void OBSActionRow::setChangeCursor(bool change)
{
	changeCursor = change;
	OBSWidgetUtils::toggleClass("cursorPointer", change);

	style()->unpolish(this);
	style()->polish(this);
}

void OBSActionRow::enterEvent(QEnterEvent *e)
{
	if (changeCursor) {
		setCursor(Qt::PointingHandCursor);
	}

	OBSWidgetUtils::addClass(QString("hover"));
	style()->unpolish(this);
	style()->polish(this);

	if (hasPrefix() || hasSuffix()) {
		OBSWidgetUtils::polishChildren();
	}

	OBSActionBaseClass::enterEvent(e);
}

void OBSActionRow::leaveEvent(QEvent *e)
{
	OBSWidgetUtils::removeClass("hover");
	style()->unpolish(this);
	style()->polish(this);

	if (hasPrefix() || hasSuffix()) {
		OBSWidgetUtils::polishChildren();
	}

	OBSActionBaseClass::leaveEvent(e);
}

void OBSActionRow::mouseReleaseEvent(QMouseEvent *e)
{
	if (e->button() & Qt::LeftButton) {
		emit clicked();
	}
	QFrame::mouseReleaseEvent(e);
}

void OBSActionRow::keyReleaseEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Space || e->key() == Qt::Key_Enter) {
		emit clicked();
	}
	QFrame::keyReleaseEvent(e);
}

void OBSActionRow::autoConnectWidget(QWidget *w)
{
	setAccessibleName(nameLbl->text());

	/* If element is a QAbstractButton subclass, and checkable,
	 * forward clicks on the widget. */
	QAbstractButton *abtn = dynamic_cast<QAbstractButton *>(w);
	if (abtn && abtn->isCheckable()) {
		setChangeCursor(true);
		setFocusProxy(abtn);

		// Pass click to button
		connect(this, &OBSActionRow::clicked, abtn,
			&QAbstractButton::click);
		return;
	}

	// If it's a QComboBox, popup the menu
	OBSComboBox *cbx = dynamic_cast<OBSComboBox *>(w);
	if (cbx) {
		setChangeCursor(false);
		setFocusProxy(cbx);

		return;
	}

	// Disable row focus for DoubleSpinBox
	OBSDoubleSpinBox *sbx = dynamic_cast<OBSDoubleSpinBox *>(w);
	if (sbx) {
		setFocusProxy(sbx);

		return;
	}
}

/*
* Button for expanding a collapsible ActionRow
*/
OBSActionCollapseButton::OBSActionCollapseButton(QWidget *parent)
{
	setCheckable(true);
}

void OBSActionCollapseButton::paintEvent(QPaintEvent *e)
{
	UNUSED_PARAMETER(e);

	QStyleOptionButton opt;
	opt.initFrom(this);
	QPainter p(this);

	bool checked = isChecked();

	opt.state.setFlag(QStyle::State_On, checked);
	opt.state.setFlag(QStyle::State_Off, !checked);

	opt.state.setFlag(QStyle::State_Sunken, checked);

	p.setRenderHint(QPainter::Antialiasing, true);
	p.setRenderHint(QPainter::SmoothPixmapTransform, true);

	style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &opt, &p, this);
	style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, &p, this);
}

/*
* ActionRow variant that can be expanded to show another properties list
*/
OBSCollapsibleActionRow::OBSCollapsibleActionRow(const QString &name,
						 const QString &desc,
						 bool toggleable,
						 QWidget *parent)
	: OBSActionBaseClass(parent)
{
	layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	setLayout(layout);

	if (desc != nullptr)
		ar = new OBSActionRow(name, desc, this);
	else
		ar = new OBSActionRow(name, this);

	layout->addWidget(ar);

	plist = new OBSPropertiesList(this);
	plist->setVisible(false);
	layout->addWidget(plist);

	collapseBtn = new OBSActionCollapseButton(this);
	collapseBtn->setFocusProxy(this);

	ar->setChangeCursor(true);

	if (toggleable) {
		plist->setEnabled(false);
		sw = new OBSToggleSwitch(false);
		QWidget *multiSuffix = new QWidget(ar);
		QHBoxLayout *multiLayout = new QHBoxLayout;

		sw->setFocusPolicy(Qt::StrongFocus);

		// ToDo: make switch buddy of toggleswitch and a11y for extend button
		multiLayout->setContentsMargins(0, 0, 0, 0);
		multiLayout->addWidget(sw);
		multiLayout->addWidget(collapseBtn);

		multiSuffix->setLayout(multiLayout);

		ar->setSuffix(multiSuffix, false);
		connect(sw, &OBSToggleSwitch::toggled, plist,
			&OBSPropertiesList::setEnabled);
	} else {
		ar->setSuffix(collapseBtn);
	}

	connect(collapseBtn, &QAbstractButton::clicked, this,
		&OBSCollapsibleActionRow::toggleVisibility);

	connect(ar, &OBSActionRow::clicked, this,
		&OBSCollapsibleActionRow::toggleVisibility);
}

void OBSCollapsibleActionRow::toggleVisibility()
{
	bool visible = !plist->isVisible();

	plist->setVisible(visible);
	collapseBtn->setChecked(visible);
}

void OBSCollapsibleActionRow::addRow(OBSActionBaseClass *ar)
{
	plist->addRow(ar);
}
