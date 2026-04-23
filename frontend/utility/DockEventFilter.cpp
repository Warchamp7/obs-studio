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

#include "DockEventFilter.hpp"

#include <widgets/OBSBasic.hpp>

#include <QDockWidget>
#include <QEvent>
#include <QMouseEvent>

#include <set>

DockController::DockController(OBSBasic *parent)
	: QWidget(parent),
	  isMouseDown(false),
	  isDragging(false),
	  isResize(false),
	  savedMousePosition(QPoint(0, 0))
{
	visualRegion = new DockVisual(this);

	leftDockTarget = new DockTarget(this, OBS::DockRegion::DockLeft);
	rightDockTarget = new DockTarget(this, OBS::DockRegion::DockRight);

	DockTarget *leftSplitTarget = new DockTarget(this, OBS::DockRegion::SplitLeft);
	DockTarget *rightSplitTarget = new DockTarget(this, OBS::DockRegion::SplitRight);
	DockTarget *topSplitTarget = new DockTarget(this, OBS::DockRegion::SplitTop);
	DockTarget *bottomSplitTarget = new DockTarget(this, OBS::DockRegion::SplitBottom);

	DockTarget *stackTarget = new DockTarget(this, OBS::DockRegion::Stack);

	int splitDockOffset = leftDockTarget->width() / 2;
	int sideDockOffset = window()->rect().width() / 2;

	leftDockTarget->setOffset(QPoint(-sideDockOffset + splitDockOffset, -splitDockOffset));
	rightDockTarget->setOffset(QPoint(sideDockOffset - splitDockOffset, -splitDockOffset));

	leftSplitTarget->setOffset(QPoint(leftSplitTarget->width() * -1.1 - splitDockOffset, -splitDockOffset));
	rightSplitTarget->setOffset(QPoint(rightSplitTarget->width() * 1.1 - splitDockOffset, -splitDockOffset));
	topSplitTarget->setOffset(QPoint(-splitDockOffset, topSplitTarget->height() * -1.1 - splitDockOffset));
	bottomSplitTarget->setOffset(QPoint(-splitDockOffset, bottomSplitTarget->height() * 1.1 - splitDockOffset));

	stackTarget->setOffset(QPoint(-splitDockOffset, -splitDockOffset));

	targetList.append(leftDockTarget);
	targetList.append(rightDockTarget);
	targetList.append(leftSplitTarget);
	targetList.append(rightSplitTarget);
	targetList.append(topSplitTarget);
	targetList.append(bottomSplitTarget);
	targetList.append(stackTarget);

	anchorPoint = window()->rect().center();

	referenceDock = nullptr;
	floatingDock = nullptr;

	floatingMoveTimer = new QTimer(this);
	floatingMoveTimer->setSingleShot(true);

	connect(floatingMoveTimer.get(), &QTimer::timeout, this, &DockController::handleFloatingDockTimerFinished);
}

DockController::~DockController()
{
	disconnect(floatingMoveTimer.get(), &QTimer::timeout, this, &DockController::handleFloatingDockTimerFinished);
	floatingMoveTimer->stop();

	referenceDock = nullptr;
	floatingDock = nullptr;
}

bool DockController::eventFilter(QObject *object, QEvent *event)
{
	auto dock = qobject_cast<QDockWidget *>(object);
	if (!dock) {
		return false;
	}

	QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
	if (!mouseEvent) {
		return false;
	}

	if (event->type() == QEvent::Resize) {
		resetFloatingDockTimer();
		isResize = true;
	}

	QSize dockSize = dock->geometry().size();

	if (event->type() == QEvent::Move && dock->isWindow()) {
		Qt::MouseButtons buttons = QGuiApplication::mouseButtons();
		if (buttons & Qt::LeftButton && !isResize) {
			dock->setAllowedAreas(Qt::NoDockWidgetArea);
			resetFloatingDockTimer();

			if (savedDockSize.isEmpty()) {
				savedDockSize = dockSize;
			}

			if (isDragging == false) {
				updatePlaceholder(nullptr);
				updateTargets(nullptr);
			}

			isMouseDown = true;
			isDragging = true;

			QPoint mousePosition = QCursor::pos();

			savedMousePosition = mousePosition;
			floatingDock = dock;

			handleMouseMoved();
		}
	}

	int titleBarHeight = dock->style()->pixelMetric(QStyle::PM_TitleBarHeight, nullptr, dock);
	QRect titleBarRect = QRect(0, 0, dock->width(), titleBarHeight);

	if (event->type() == QEvent::MouseButtonPress) {
		if (!titleBarRect.contains(mouseEvent->pos())) {
			return false;
		}

		savedMousePosition = mouseEvent->globalPosition().toPoint();
		isMouseDown = true;

		if (!dockSize.isEmpty()) {
			savedDockSize = dockSize;
		}

		for (auto target : targetList) {
			target->setHovered(false);
		}
	} else if (event->type() == QEvent::MouseButtonRelease) {
		if (isMouseDown && isDragging) {
			handleDragFinished(dock);
		} else {
			savedDockSize = QSize();
		}

		isMouseDown = false;
		isDragging = false;
	} else if (event->type() == QEvent::MouseMove) {
		QPoint mousePosition = mouseEvent->globalPosition().toPoint();
		QPoint mouseDistance = mousePosition - savedMousePosition;
		if (!isDragging && isMouseDown && (mouseDistance.manhattanLength() > 3)) {
			isDragging = true;
		}

		savedMousePosition = mouseEvent->globalPosition().toPoint();
		dock->setAllowedAreas(Qt::NoDockWidgetArea);
		handleMouseMoved();
	}

	return false;
}

void DockController::resetFloatingDockTimer()
{
	floatingMoveTimer->start(100);
}

void DockController::handleMouseMoved()
{
	if (isDragging) {
		QDockWidget *dockUnderCursor = findReferenceDock();

		if (dockUnderCursor) {
			updatePlaceholder(dockUnderCursor);
			updateTargets(dockUnderCursor);
		}
	}
}

QDockWidget *DockController::findReferenceDock()
{
	QWidget *widget = window()->childAt(window()->mapFromGlobal(savedMousePosition));
	if (!widget) {
		return nullptr;
	}

	while (widget) {
		if (auto dockWidget = qobject_cast<QDockWidget *>(widget)) {
			if (dockWidget->isVisible()) {
				return dockWidget;
			}
		}

		widget = widget->parentWidget();
	}

	return nullptr;
}

void DockController::updateTargets(QDockWidget *dockUnderCursor)
{
	if (dockUnderCursor && dockUnderCursor != floatingDock) {
		Qt::DockWidgetArea dockArea = OBSBasic::Get()->dockWidgetArea(dockUnderCursor);

		leftDockTarget->setEnabled(dockArea != Qt::LeftDockWidgetArea);
		rightDockTarget->setEnabled(dockArea != Qt::RightDockWidgetArea);

		showDockRegions(true);
		anchorPoint = dockUnderCursor->geometry().center();
	} else {
		showDockRegions(false);
	}
}

void DockController::updatePlaceholder(QDockWidget *dockUnderCursor)
{
	bool showPlaceholder = false;
	for (auto target : targetList) {
		if (!target->isEnabled()) {
			target->setHovered(false);
			continue;
		}

		if (!target->geometry().contains(savedMousePosition)) {
			target->setHovered(false);
			continue;
		}

		target->setHovered(true);

		if (target->isRelativeRegion() && dockUnderCursor) {
			referenceDock = dockUnderCursor;
		}

		QRect placeholderRect = calculatePlaceholderRectForTarget(target);
		visualRegion->setRect(placeholderRect);

		showPlaceholder = true;
	}

	if (showPlaceholder) {
		visualRegion->show();
	} else {
		visualRegion->hide();
		referenceDock = nullptr;
	}
}

void DockController::handleDragFinished(QDockWidget *dock)
{
	isMouseDown = false;
	isDragging = false;
	isResize = false;

	floatingMoveTimer->stop();
	showDockRegions(false);

	QPointer<DockTarget> dockTarget;
	for (auto target : targetList) {
		if (target->isEnabled() && target->geometry().contains(savedMousePosition)) {
			dockTarget = target;
			break;
		}
	}

	QPointer<QDockWidget> safeDock = dock;
	if (dockTarget) {
		QTimer::singleShot(0, this, [this, safeDock, dockTarget]() {
			if (!safeDock) {
				return;
			}

			moveDockWidget(safeDock, dockTarget);
		});
	} else {
		savedDockSize = QSize();
	}

	floatingDock = nullptr;
	visualRegion->hide();
}

void DockController::moveDockWidget(QDockWidget *draggedDock, DockTarget *target)
{
	if (QCoreApplication::closingDown()) {
		return;
	}

	draggedDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

	QDockWidget *referenceDock = this->referenceDock;
	if (target->isRelativeRegion() && !referenceDock) {
		return;
	}

	if (draggedDock == referenceDock) {
		return;
	}

	auto main = OBSBasic::Get();

	Qt::DockWidgetArea newDockWidgetArea = Qt::BottomDockWidgetArea;

	int constrainedDockWidth = 0;
	if (referenceDock) {
		newDockWidgetArea = main->dockWidgetArea(referenceDock);
		constrainedDockWidth = std::min(savedDockSize.width(), referenceDock->width());
	}

	OBS::DockRegion region = target->getRegion();
	QRect rect{};

	draggedDock->setFloating(false);

	// We cannot split dock widgets when one is in a tabbed group. We have to break up the tabbed grouping and
	// manually restore it after placing the dragged dock next to it.
	QList<QDockWidget *> tabbedDocks = main->tabifiedDockWidgets(referenceDock);
	if (!tabbedDocks.empty() && target->isRelativeRegion() && region != OBS::DockRegion::Stack) {
		// There's no way in Qt to actually figure out the order of tabified docks. We can get a list of
		// all -other- docks in the tabified group, so combining two different lists in a set allows us
		// to deduce the order.
		std::set<QDockWidget *> orderedDocks;
		QList<QDockWidget *> copyDocks = tabbedDocks.first() != referenceDock
							 ? main->tabifiedDockWidgets(tabbedDocks.first())
							 : main->tabifiedDockWidgets(tabbedDocks.last());
		orderedDocks.insert(tabbedDocks.begin(), tabbedDocks.end());
		orderedDocks.insert(copyDocks.begin(), copyDocks.end());

		tabbedDocks = QList<QDockWidget *>(orderedDocks.begin(), orderedDocks.end());

		// Remove the dragged dock from the list
		tabbedDocks.removeOne(draggedDock);

		Qt::DockWidgetArea tempArea = main->dockWidgetArea(referenceDock);

		for (auto entry : tabbedDocks) {
			if (entry != referenceDock) {
				main->addDockWidget(tempArea, entry);
			}
		}
	}

	auto restoreTabbedDocks = [main, tabbedDocks, referenceDock]() {
		if (tabbedDocks.empty()) {
			return;
		}

		QDockWidget *prevDock = referenceDock;
		for (auto dock : tabbedDocks) {
			if (dock == prevDock) {
				continue;
			}

			QString movedName = dock->objectName();
			main->tabifyDockWidget(prevDock, dock);
			prevDock = dock;
		}

		referenceDock->setVisible(true);
	};

	switch (region) {
	case OBS::DockRegion::DockLeft:
		main->addDockWidget(Qt::LeftDockWidgetArea, draggedDock);
		break;
	case OBS::DockRegion::DockRight:
		main->addDockWidget(Qt::RightDockWidgetArea, draggedDock);
		break;
	case OBS::DockRegion::DockTop:
		// Region not allowed
		break;
	case OBS::DockRegion::DockBottom:
		main->addDockWidget(Qt::BottomDockWidgetArea, draggedDock);
		break;
	case OBS::DockRegion::SplitLeft:
		draggedDock->setMinimumWidth(constrainedDockWidth);
		draggedDock->setMaximumWidth(constrainedDockWidth);

		main->addDockWidget(newDockWidgetArea, draggedDock);
		main->splitDockWidget(referenceDock, draggedDock, Qt::Horizontal);

		referenceDock->setMinimumWidth(referenceDock->width());
		main->splitDockWidget(draggedDock, referenceDock, Qt::Horizontal);

		restoreTabbedDocks();

		break;
	case OBS::DockRegion::SplitRight:
		draggedDock->setMinimumWidth(constrainedDockWidth);
		draggedDock->setMaximumWidth(constrainedDockWidth);
		main->addDockWidget(newDockWidgetArea, draggedDock);
		main->splitDockWidget(referenceDock, draggedDock, Qt::Horizontal);
		restoreTabbedDocks();

		break;
	case OBS::DockRegion::SplitTop:
		main->addDockWidget(newDockWidgetArea, draggedDock);
		main->splitDockWidget(referenceDock, draggedDock, Qt::Vertical);
		main->splitDockWidget(draggedDock, referenceDock, Qt::Vertical);
		restoreTabbedDocks();

		break;
	case OBS::DockRegion::SplitBottom:
		main->addDockWidget(newDockWidgetArea, draggedDock);
		main->splitDockWidget(referenceDock, draggedDock, Qt::Vertical);
		restoreTabbedDocks();

		break;
	case OBS::DockRegion::Stack:
		main->addDockWidget(newDockWidgetArea, draggedDock);
		main->tabifyDockWidget(referenceDock, draggedDock);
		break;
	default:
		break;
	}

	draggedDock->setVisible(true);

	main->updateGeometry();
	main->layout()->activate();

	draggedDock->setMinimumSize(0, 0);
	draggedDock->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

	if (referenceDock) {
		referenceDock->setMinimumWidth(0);
	}

	savedDockSize = QSize(0, 0);

	return;
}

QRect DockController::calculatePlaceholderRectForTarget(DockTarget *target)
{
	OBS::DockRegion region = target->getRegion();

	QRect rect{};

	if (target->isRelativeRegion() && !referenceDock) {
		return rect;
	}

	QRect windowRect = parentWidget()->rect();

	switch (region) {
	case OBS::DockRegion::DockLeft:
		rect = QRect(QPoint(parentWidget()->mapToGlobal(windowRect.topLeft())), QSize(96, windowRect.height()));
		break;
	case OBS::DockRegion::DockRight:
		rect = QRect(QPoint(parentWidget()->mapToGlobal(windowRect.topRight() - QPoint(96, 0))),
			     QSize(96, windowRect.height()));
		break;
	case OBS::DockRegion::DockTop:
		break;
	case OBS::DockRegion::DockBottom:
		break;
	case OBS::DockRegion::SplitLeft:
		rect = QRect(QPoint(parentWidget()->mapToGlobal(referenceDock->geometry().topLeft())),
			     QSize(referenceDock->geometry().width() / 2, referenceDock->geometry().height()));
		break;
	case OBS::DockRegion::SplitRight:
		rect = QRect(QPoint(parentWidget()->mapToGlobal(
				     QPoint(referenceDock->geometry().center().x(), referenceDock->geometry().top()))),
			     QSize(referenceDock->geometry().width() / 2, referenceDock->geometry().height()));
		break;
	case OBS::DockRegion::SplitTop:
		rect = QRect(QPoint(parentWidget()->mapToGlobal(referenceDock->geometry().topLeft())),
			     QSize(referenceDock->geometry().width(), referenceDock->geometry().height() / 2));
		break;
	case OBS::DockRegion::SplitBottom:
		rect = QRect(QPoint(parentWidget()->mapToGlobal(
				     QPoint(referenceDock->geometry().left(), referenceDock->geometry().center().y()))),
			     QSize(referenceDock->geometry().width(), referenceDock->geometry().height() / 2));
		break;
	case OBS::DockRegion::Stack:
		rect = QRect(QPoint(parentWidget()->mapToGlobal(referenceDock->geometry().topLeft())),
			     QSize(referenceDock->geometry().width(), referenceDock->geometry().height()));
		break;
	}

	return rect;
}

void DockController::showDockRegions(bool show)
{
	int sideDockOffset = window()->rect().width() / 2;

	for (auto target : targetList) {
		if (show && target->isEnabled()) {

			target->show();
			if (target->isRelativeRegion()) {
				target->setAnchorPoint(anchorPoint);
				continue;
			}

			target->setAnchorPoint(window()->rect().center());
			if (target->getRegion() == OBS::DockRegion::DockLeft) {
				target->setOffset(QPoint(-sideDockOffset + target->width() / 2, -target->height() / 2));
			} else if (target->getRegion() == OBS::DockRegion::DockRight) {
				target->setOffset(
					QPoint(sideDockOffset - target->width() * 1.5, -target->height() / 2));
			}
		} else {
			target->hide();
		}
	}
}

void DockController::toggleRegions(bool show)
{
	showDockRegions(show);
}

void DockController::handleFloatingDockTimerFinished()
{
	isResize = false;

	if (!floatingDock) {
		return;
	}

	Qt::MouseButtons buttons = QGuiApplication::mouseButtons();
	if (buttons & Qt::LeftButton) {
		// If mouse button is still held down when the timer ends, reset it again
		resetFloatingDockTimer();
		return;
	}

	savedMousePosition = QCursor::pos();

	if (isDragging) {
		handleDragFinished(floatingDock);
	}

	isMouseDown = false;
	isDragging = false;
}
