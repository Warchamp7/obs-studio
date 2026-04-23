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

#pragma once

#include <components/DockRegion.hpp>

#include <QDockWidget>
#include <QObject>
#include <QPoint>
#include <QTimer>

class OBSBasic;
class QDockWidget;
class QMouseEvent;

class DockController : public QWidget {
	Q_OBJECT

	bool eventFilter(QObject *parent, QEvent *event);

	bool isMouseDown;
	bool isDragging;
	bool isResize;
	QPoint savedMousePosition;
	QSize savedDockSize;

	QPoint anchorPoint;

	DockTarget *leftDockTarget;
	DockTarget *rightDockTarget;
	QList<DockTarget *> targetList;
	DockVisual *visualRegion;

	QDockWidget *referenceDock;
	QDockWidget *floatingDock;

	QPointer<QTimer> floatingMoveTimer;
	void resetFloatingDockTimer();

	void handleMouseMoved();
	QDockWidget *findReferenceDock();
	void updateTargets(QDockWidget *dockUnderCursor);
	void updatePlaceholder(QDockWidget *dockUnderCursor);
	void handleDragFinished(QDockWidget *dock);

	void moveDockWidget(QDockWidget *dock, DockTarget *target);

	QRect calculatePlaceholderRectForTarget(DockTarget *target);
	void showDockRegions(bool show);

public:
	DockController(OBSBasic *parent);
	~DockController();

public slots:
	void toggleRegions(bool show);
	void handleFloatingDockTimerFinished();

signals:
	void dockRearrangement(bool isActive);
};
