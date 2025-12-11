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

#include <Idian/AbstractPopup.hpp>

#include <QEvent>
#include <QMainWindow>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QDockWidget>
#include <QTimer>

namespace {
QPoint getAnchorCoordinate(QWidget *widget, idian::Anchor::Point anchor)
{
	QPoint anchorPoint(0, 0);

	if (anchor == idian::Anchor::TopLeft) {
		anchorPoint = widget->geometry().topLeft();
	} else if (anchor == idian::Anchor::TopCenter) {
		anchorPoint = QPoint(widget->geometry().center().x(), widget->geometry().top());
	} else if (anchor == idian::Anchor::TopRight) {
		anchorPoint = widget->geometry().topRight();
	} else if (anchor == idian::Anchor::LeftCenter) {
		anchorPoint = QPoint(widget->geometry().left(), widget->geometry().center().y());
	} else if (anchor == idian::Anchor::RightCenter) {
		anchorPoint = QPoint(widget->geometry().right(), widget->geometry().center().y());
	} else if (anchor == idian::Anchor::BottomLeft) {
		anchorPoint = widget->geometry().bottomLeft();
	} else if (anchor == idian::Anchor::BottomCenter) {
		anchorPoint = QPoint(widget->geometry().center().x(), widget->geometry().bottom());
	} else if (anchor == idian::Anchor::BottomRight) {
		anchorPoint = widget->geometry().bottomRight();
	}

	return anchorPoint;
}

bool isAnchorLeft(idian::Anchor::Point anchor)
{
	return anchor & idian::Anchor::Left;
}
bool isAnchorRight(idian::Anchor::Point anchor)
{
	return anchor & idian::Anchor::Right;
}
bool isAnchorTop(idian::Anchor::Point anchor)
{
	return anchor & idian::Anchor::Top;
}
bool isAnchorBottom(idian::Anchor::Point anchor)
{
	return anchor & idian::Anchor::Bottom;
}
} // namespace

namespace idian {

AbstractPopup::AbstractPopup(QWidget *parent) : QFrame(parent)
{
	if (window()) {
		parentWindow = window();
	}

	setWindowFlag(Qt::Tool, true);
	setWindowFlag(Qt::FramelessWindowHint, true);
	setAttribute(Qt::WA_TranslucentBackground, true);

	setMinimumSize(0, 0);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	mainLayout = new QHBoxLayout();
	mainLayout->setContentsMargins(0, arrowSize, arrowSize, 0);
	mainLayout->setSpacing(0);
	setLayout(mainLayout);
	mainLayout->setSizeConstraint(QLayout::SetFixedSize);

	contents = new QFrame();
	contents->setObjectName("contentsFrame");
	contents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	contentsLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	contentsLayout->setContentsMargins(0, 0, 0, 0);
	contentsLayout->setSpacing(0);

	contents->setLayout(contentsLayout);

	mainLayout->addWidget(contents);

	parentWindow->installEventFilter(this);

	/* Works around an issue where switching between tabbed docks
	 * puts the central widget above these widgets
	 */
	QMainWindow *mainWindow = dynamic_cast<QMainWindow *>(parentWindow);
	if (mainWindow) {
		connect(mainWindow, &QMainWindow::tabifiedDockWidgetActivated, this, &AbstractPopup::updatePosition);
	}

	adjustSize();
	show();
}

AbstractPopup::~AbstractPopup() {}

void AbstractPopup::setWidget(QWidget *widget)
{
	if (contentsWidget) {
		contentsLayout->removeWidget(contentsWidget);
		delete contentsWidget;
	}

	contentsWidget = widget;
	contentsLayout->addWidget(contentsWidget);

	adjustSize();
}

void AbstractPopup::setAnchorTarget(QWidget *anchor_)
{
	if (anchorWidget) {
		anchorWidget->removeEventFilter(this);
	}

	if (!anchor_) {
		return;
	}

	anchorWidget = anchor_;
	anchorWidget->installEventFilter(this);
	if (anchorWidget->parentWidget()) {
		anchorWidget->parentWidget()->installEventFilter(this);
	}

	updatePosition();
}

void AbstractPopup::setAnchorFrom(Anchor::Point corner)
{
	if (anchorFrom == corner) {
		return;
	}

	anchorFrom = corner;
	updatePosition();
}

void AbstractPopup::setAnchorTo(Anchor::Point corner)
{
	if (anchorTo == corner) {
		return;
	}

	anchorTo = corner;
	updatePosition();
}

void AbstractPopup::setOrientation(Qt::Orientation orientation_)
{
	if (orientation == orientation_) {
		return;
	}

	orientation = orientation_;

	if (orientation == Qt::Horizontal) {
		contentsLayout->setDirection(QBoxLayout::LeftToRight);
	} else {
		contentsLayout->setDirection(QBoxLayout::TopToBottom);
	}

	updatePosition();
}

void AbstractPopup::setDisableArrow(bool disable)
{
	if (hideArrow != disable) {
		hideArrow = disable;
		update();
	}
}

bool AbstractPopup::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::Resize) {
		updatePosition();
	} else if (obj == parentWindow && event->type() == QEvent::Move) {
		updatePosition();
	} else if (obj == anchorWidget && event->type() == QEvent::Move) {
		updatePosition();
	} else if (event->type() == QEvent::Show) {
		updateVisibility();
		updatePosition();
	} else if (event->type() == QEvent::Hide) {
		updateVisibility();
	} else if (event->type() == QEvent::Destroy) {
		if (anchorWidget && obj == anchorWidget) {
			anchorWidget->removeEventFilter(this);
			anchorWidget = nullptr;
		}
	}

	return false;
}

void AbstractPopup::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);

	QColor background = palette().color(QWidget::backgroundRole());

	QStyleOptionFrame frameOpt;
	frameOpt.initFrom(this);
	frameOpt.rect = rect();

	QPoint arrowPos = mapFromGlobal(arrowPosition);
	QPoint arrowPoint1, arrowPoint2, arrowTip;

	if (orientation == Qt::Horizontal) {
		int minPoint = std::min(rect().top() + arrowSize * 2, rect().center().y());
		int maxPoint = std::max(rect().bottom() - arrowSize * 2, rect().center().y());

		arrowPos.setY(std::clamp(arrowPos.y(), minPoint, maxPoint));

		frameOpt.rect.setWidth(frameOpt.rect.width() - arrowSize);

		if (showOnLeft) {
			// Right-facing arrow
			arrowPos -= QPoint(1, 0);
			arrowPoint1 = QPoint(arrowPos.x() - arrowSize, arrowPos.y() - arrowSize);
			arrowPoint2 = QPoint(arrowPos.x() - arrowSize, arrowPos.y() + arrowSize);
		} else {
			// Left-facing arrow
			arrowPos += QPoint(1, 0);
			arrowPoint1 = QPoint(arrowPos.x() + arrowSize, arrowPos.y() - arrowSize);
			arrowPoint2 = QPoint(arrowPos.x() + arrowSize, arrowPos.y() + arrowSize);

			if (!hideArrow) {
				frameOpt.rect.moveLeft(frameOpt.rect.left() + arrowSize);
			}
		}
	} else {
		int minPoint = std::min(rect().left() + arrowSize * 2, rect().center().x());
		int maxPoint = std::max(rect().right() - arrowSize * 2, rect().center().x());

		arrowPos.setX(std::clamp(arrowPos.x(), minPoint, maxPoint));

		frameOpt.rect.setHeight(frameOpt.rect.height() - arrowSize);

		if (showOnBottom) {
			// Up-facing arrow
			arrowPos += QPoint(0, 1);
			arrowPoint1 = QPoint(arrowPos.x() - arrowSize, arrowPos.y() + arrowSize);
			arrowPoint2 = QPoint(arrowPos.x() + arrowSize, arrowPos.y() + arrowSize);

			if (!hideArrow) {
				frameOpt.rect.moveTop(frameOpt.rect.top() + arrowSize);
			}
		} else {
			// Down-facing arrow
			arrowPos -= QPoint(0, 1);
			arrowPoint1 = QPoint(arrowPos.x() - arrowSize, arrowPos.y() - arrowSize);
			arrowPoint2 = QPoint(arrowPos.x() + arrowSize, arrowPos.y() - arrowSize);
		}
	}

	style()->drawPrimitive(QStyle::PE_PanelTipLabel, &frameOpt, &painter, this);

	if (!hideArrow) {
		arrowTip = QPoint(arrowPos.x(), arrowPos.y());

		QPainterPath path;
		path.moveTo(arrowTip);
		path.lineTo(arrowPoint1);
		path.lineTo(arrowPoint2);
		path.lineTo(arrowTip);

		painter.fillPath(path, QBrush(background));
	}
}

QPoint AbstractPopup::calculateOffset(Anchor::Point anchorFrom)
{
	int offsetX = 0;
	int offsetY = 0;

	if (anchorFrom & Anchor::HCenter) {
		offsetX = -width() / 2;
	} else if (anchorFrom & Anchor::Right) {
		offsetX = -width();
	}

	if (anchorFrom & Anchor::VCenter) {
		offsetY = -height() / 2;
	} else if (anchorFrom & Anchor::Bottom) {
		offsetY = -height();
	}

	return QPoint(offsetX, offsetY);
}

bool AbstractPopup::needsHorizontalFlip(const QPoint &pos, const QRect &bounds)
{
	if (anchorTo & Anchor::Right) {
		if (pos.x() + width() > bounds.right()) {
			return true;
		}
	}
	if (anchorTo & Anchor::Left) {
		if (pos.x() < bounds.left()) {
			return true;
		}
	}

	return false;
}

bool AbstractPopup::needsVerticalFlip(const QPoint &pos, const QRect &bounds)
{
	if (anchorTo & Anchor::Top) {
		if (pos.y() < bounds.top()) {
			return true;
		}
	}
	if (anchorTo & Anchor::Bottom) {
		if (pos.y() + height() > bounds.bottom()) {
			return true;
		}
	}

	return false;
}

void AbstractPopup::updateVisibility()
{
	if (!anchorWidget) {
		if (isVisible()) {
			hide();
		}
		return;
	}
}

void AbstractPopup::updatePosition()
{
	if (isUpdating) {
		return;
	}

	isUpdating = true;

	if (!anchorWidget) {
		isUpdating = false;
		return;
	}

	QPoint globalAnchor = anchorWidget->parentWidget()->mapToGlobal(getAnchorCoordinate(anchorWidget, anchorTo));

	QPoint offset = calculateOffset(anchorFrom);
	QPoint popupPos = globalAnchor + offset;

	QRect screenRect = anchorWidget->window()->frameGeometry();
	bool flipH = needsHorizontalFlip(popupPos, screenRect);
	bool flipV = needsVerticalFlip(popupPos, screenRect);

	showOnLeft = anchorTo & Anchor::Left || anchorFrom & Anchor::Right || flipH;
	showOnBottom = anchorTo & Anchor::Bottom || anchorFrom & Anchor::Top || flipV;

	if (flipH) {
		int adjustment = anchorWidget->geometry().width();
		if (anchorTo & Anchor::Right) {
			popupPos.setX(popupPos.x() - adjustment - width() + offset.x());
		} else if (anchorTo & Anchor::Left) {
			popupPos.setX(popupPos.x() + adjustment - offset.x());
		}
	}

	if (flipV) {
		popupPos.setY(globalAnchor.y() - offset.y() - height());
	}

	arrowPosition = globalAnchor;

	if (orientation == Qt::Horizontal) {
		if (showOnLeft) {
			// Arrow on right edge
			mainLayout->setContentsMargins(0, 0, arrowSize, 0);
			arrowPosition.rx() = popupPos.x() + width();
		} else {
			// Arrow on left edge
			mainLayout->setContentsMargins(arrowSize, 0, 0, 0);
			arrowPosition.rx() = popupPos.x();

			if (hideArrow) {
				popupPos.rx() -= arrowSize;
			}
		}
	} else {
		if (showOnBottom) {
			// Arrow on top edge
			mainLayout->setContentsMargins(0, arrowSize, 0, 0);
			arrowPosition.ry() = popupPos.y();
		} else {
			// Arrow on bottom edge
			mainLayout->setContentsMargins(0, 0, 0, arrowSize);
			arrowPosition.ry() = popupPos.y() + height();

			if (hideArrow) {
				popupPos.ry() += arrowSize;
			}
		}
	}

	move(popupPos);
	update();
	// adjustSize();

	QTimer::singleShot(1, [=] { raise(); });

	isUpdating = false;
}

void AbstractPopup::show()
{
	updatePosition();
	QFrame::show();
}

} // namespace idian
