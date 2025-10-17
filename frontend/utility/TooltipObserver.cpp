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

#include <OBSApp.hpp>

#include <utility/TooltipObserver.hpp>

#include <QTimer>

TooltipObserver::TooltipObserver(QWidget *target) : targetWidget(target)
{
	if (!target) {
		return;
	}

	if (targetWidget->toolTip().isEmpty()) {
		deleteLater();
		return;
	}

	setParent(target->window());

	targetWidget->installEventFilter(this);

	setText(targetWidget->toolTip());
}

TooltipObserver::~TooltipObserver() {}

void TooltipObserver::setText(QString text)
{
	tooltipText = text;
}

void TooltipObserver::showTooltip()
{
	idian::Tooltip *tooltipWidget = App()->getTooltipWidget();

	tooltipWidget->setText(tooltipText);
	tooltipWidget->setAnchorTarget(targetWidget);
	tooltipWidget->show();
	QTimer::singleShot(0, this, [tooltipWidget]() { tooltipWidget->updatePosition(); });
}

void TooltipObserver::hideTooltip()
{
	idian::Tooltip *tooltipWidget = App()->getTooltipWidget();
	tooltipWidget->hide();
}

bool TooltipObserver::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::Enter) {
		if (!obj->isWidgetType()) {
			return false;
		}
		QWidget *widget = qobject_cast<QWidget *>(obj);

		if (!widget->isEnabled()) {
			return false;
		}

		showTooltip();
	} else if (event->type() == QEvent::Leave) {
		hideTooltip();
	} else if (event->type() == QEvent::ToolTip) {
		return true;
	}

	return false;
}
