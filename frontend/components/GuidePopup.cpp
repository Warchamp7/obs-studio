#pragma once

#include "GuidePopup.hpp"

#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QPushButton>
#include <QDockWidget>
#include <QTimer>
#include <util/base.h>
#include <OBSApp.hpp>

namespace {
QPoint getAnchorCoordinate(QWidget *widget, Anchor::Point anchor)
{
	QPoint anchorPoint(0, 0);

	if (anchor == Anchor::TopLeft) {
		anchorPoint = widget->geometry().topLeft();
	} else if (anchor == Anchor::TopCenter) {
		anchorPoint = QPoint(widget->geometry().center().x(), widget->geometry().top());
	} else if (anchor == Anchor::TopRight) {
		anchorPoint = widget->geometry().topRight();
	} else if (anchor == Anchor::LeftCenter) {
		anchorPoint = QPoint(widget->geometry().left(), widget->geometry().center().y());
	} else if (anchor == Anchor::RightCenter) {
		anchorPoint = QPoint(widget->geometry().right(), widget->geometry().center().y());
	} else if (anchor == Anchor::BottomLeft) {
		anchorPoint = widget->geometry().bottomLeft();
	} else if (anchor == Anchor::BottomCenter) {
		anchorPoint = QPoint(widget->geometry().center().x(), widget->geometry().bottom());
	} else if (anchor == Anchor::BottomRight) {
		anchorPoint = widget->geometry().bottomRight();
	}

	return anchorPoint;
}

bool isAnchorLeft(Anchor::Point anchor)
{
	return anchor & Anchor::Left;
}
bool isAnchorRight(Anchor::Point anchor)
{
	return anchor & Anchor::Right;
}
bool isAnchorTop(Anchor::Point anchor)
{
	return anchor & Anchor::Top;
}
bool isAnchorBottom(Anchor::Point anchor)
{
	return anchor & Anchor::Bottom;
}
} // namespace

GuidePopup::GuidePopup(QWidget *parent) : QFrame(parent)
{
	if (window()) {
		parentWindow = window();
	}

	setWindowFlag(Qt::Tool, true);
	setWindowFlag(Qt::FramelessWindowHint, true);
	setAttribute(Qt::WA_TranslucentBackground, true);

	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

	mainLayout = new QHBoxLayout();
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);
	setLayout(mainLayout);

	arrowStart = new QWidget();
	arrowStart->setMinimumSize(10, 10);
	arrowStart->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

	arrowEnd = new QWidget();
	arrowEnd->setMinimumSize(10, 10);
	arrowEnd->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

	contents = new QFrame(this);
	contents->setObjectName("contentsFrame");
	contents->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	contentsLayout = new QHBoxLayout();
	contentsLayout->setContentsMargins(0, 0, 0, 0);
	contentsLayout->setSpacing(0);

	contents->setLayout(contentsLayout);

	textLayout = new QVBoxLayout();
	textLayout->setContentsMargins(0, 0, 0, 0);
	textLayout->setSpacing(0);

	header = new QWidget();
	headerLayout = new QHBoxLayout();
	headerLayout->setContentsMargins(0, 0, 0, 0);
	headerLayout->setSpacing(0);
	header->setLayout(headerLayout);

	title = new QLabel();
	title->setObjectName("infoTitle");
	title->setText("Info");
	title->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

	dismissButton = new QPushButton();
	dismissButton->setProperty("class", "icon-close");
	dismissButton->setObjectName("dismissBtn");

	headerLayout->addWidget(title);
	headerLayout->addWidget(dismissButton);

	info = new QLabel();
	info->setObjectName("infoText");
	info->setWordWrap(true);
	info->setTextFormat(Qt::MarkdownText);
	info->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

	footer = new QFrame();
	footer->setObjectName("infoFooter");
	footerLayout = new QHBoxLayout();
	footerLayout->setContentsMargins(0, 0, 0, 0);
	footerLayout->setSpacing(0);
	footer->setLayout(footerLayout);

	footerNext = new QPushButton("Next", footer);
	footerLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Preferred));
	footerLayout->addWidget(footerNext);

	textLayout->addWidget(header);
	textLayout->addWidget(info);
	textLayout->addWidget(footer);
	textLayout->addSpacerItem(new QSpacerItem(10, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Preferred));

	contentsLayout->addLayout(textLayout);

	mainLayout->addWidget(arrowStart);
	mainLayout->setAlignment(arrowStart, Qt::AlignTop);
	mainLayout->addWidget(contents);
	mainLayout->addWidget(arrowEnd);

	parentWindow->installEventFilter(this);

	setMultipleSteps(false);

	connect(dismissButton, &QAbstractButton::clicked, this, &GuidePopup::dismiss);
	connect(footerNext, &QAbstractButton::clicked, this, &GuidePopup::next);

	/* Works around an issue where switching between tabbed docks
	 * puts the central widget above these widgets
	 */
	QMainWindow *mainWindow = dynamic_cast<QMainWindow *>(parentWindow);
	if (mainWindow) {
		connect(mainWindow, &QMainWindow::tabifiedDockWidgetActivated, this, &GuidePopup::updatePosition);
	}

	show();
}

GuidePopup::~GuidePopup() {}

void GuidePopup::setAnchorTarget(QWidget *anchor_)
{
	if (anchorWidget) {
		anchorWidget->removeEventFilter(this);
	}

	anchorWidget = anchor_;
	anchorWidget->installEventFilter(this);
	anchorWidget->parentWidget()->installEventFilter(this);

	updatePosition();
}

void GuidePopup::setTitle(QString text)
{
	title->setText(text);
}

void GuidePopup::setInfo(QString text)
{
	info->setText(text);
}

void GuidePopup::setMultipleSteps(bool enable)
{
	multipleSteps = enable;

	if (multipleSteps) {
		dismissButton->hide();
		footer->show();
	} else {
		dismissButton->show();
		footer->hide();
	}
}

void GuidePopup::setAnchorFrom(Anchor::Point corner)
{
	if (anchorFrom == corner) {
		return;
	}

	anchorFrom = corner;
	updatePosition();
}

void GuidePopup::setAnchorTo(Anchor::Point corner)
{
	if (anchorTo == corner) {
		return;
	}

	anchorTo = corner;
	updatePosition();
}

void GuidePopup::setOrientation(Qt::Orientation orientation_)
{
	if (orientation == orientation_) {
		return;
	}

	orientation = orientation_;

	if (orientation == Qt::Horizontal) {
		QHBoxLayout *newLayout = new QHBoxLayout();
		newLayout->setContentsMargins(0, 0, 0, 0);
		newLayout->setSpacing(0);
		while (QLayoutItem *item = layout()->takeAt(0)) {
			if (item->widget()) {
				newLayout->addWidget(item->widget());
			}
		}

		delete mainLayout;

		setLayout(newLayout);
		mainLayout = newLayout;
	} else {
		QVBoxLayout *newLayout = new QVBoxLayout();
		newLayout->setContentsMargins(0, 0, 0, 0);
		newLayout->setSpacing(0);
		while (QLayoutItem *item = layout()->takeAt(0)) {
			if (item->widget()) {
				newLayout->addWidget(item->widget());
			}
		}

		delete mainLayout;

		setLayout(newLayout);
		mainLayout = newLayout;
	}
}

void GuidePopup::dismiss()
{
	emit rejected();
	deleteLater();
}

void GuidePopup::next()
{
	emit accepted();
}

bool GuidePopup::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::Resize) {
		updatePosition();
	} else if (obj == parentWindow && event->type() == QEvent::Move) {
		updatePosition();
	} else if (obj == anchorWidget && event->type() == QEvent::Move) {
		updatePosition();
	} else if (event->type() == QEvent::Show) {
		updatePosition();
	} else if (event->type() == QEvent::Hide) {
		hide();
	}

	return false;
}

void GuidePopup::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);

	QStyleOptionFrame opt;
	initStyleOption(&opt);

	QColor contentsBg = contents->palette().color(QWidget::backgroundRole());

	int arrowSize = 10;

	QPoint arrowPos = mapFromGlobal(arrowPosition);

	int arrowX = std::clamp(arrowPos.x(), 0, width() - arrowSize);
	int arrowY = std::clamp(arrowPos.y(), 0, height() - arrowSize);

	int arrowOffset = (int)(title->height() - arrowSize) / 2;

	QRectF rect = QRectF(arrowX, arrowY, arrowSize, arrowSize);
	QPointF arrowPoint1, arrowPoint2, arrowTip;

	if (orientation == Qt::Horizontal) {
		arrowY = std::min(arrowPos.y() + arrowOffset, height() - arrowOffset - arrowSize * 2);
		if (anchorFrom & Anchor::Center) {
			arrowY -= arrowOffset + arrowSize;
		}

		rect.setY(arrowY);

		if (showOnLeft) {
			rect.setHeight(arrowSize * 2);

			// Right-facing arrow
			arrowPoint1 = rect.topLeft();
			arrowPoint2 = rect.bottomLeft();
			arrowTip = QPointF(rect.right(), rect.top() + arrowSize);
		} else {
			rect.setHeight(arrowSize * 2);

			// Left-facing arrow
			arrowPoint1 = rect.topRight();
			arrowPoint2 = rect.bottomRight();
			arrowTip = QPointF(rect.left(), rect.top() + arrowSize);
		}
	} else {
		arrowX = std::min(arrowPos.x() + arrowOffset, width() - arrowOffset - arrowSize * 2);
		if (anchorFrom & Anchor::Center) {
			arrowX -= arrowOffset + arrowSize;
		}

		rect.setX(arrowX);

		if (showOnBottom) {
			rect.setWidth(arrowSize * 2);

			// Up-facing arrow
			arrowPoint1 = rect.bottomLeft();
			arrowPoint2 = rect.bottomRight();
			arrowTip = QPointF(rect.left() + arrowSize, rect.top());
		} else {
			rect.setWidth(arrowSize * 2);

			// Down-facing arrow
			arrowPoint1 = rect.topLeft();
			arrowPoint2 = rect.topRight();
			arrowTip = QPointF(rect.left() + arrowSize, rect.bottom());
		}
	}

	QPainterPath path;
	path.moveTo(arrowTip);
	path.lineTo(arrowPoint1);
	path.lineTo(arrowPoint2);
	path.lineTo(arrowTip);

	painter.fillPath(path, QBrush(contentsBg));
}

QPoint GuidePopup::calculateOffset(Anchor::Point anchorFrom)
{
	int offsetX = 0;
	int offsetY = 0;

	if (anchorFrom & Anchor::HCenter)
		offsetX = -width() / 2;
	else if (anchorFrom & Anchor::Right)
		offsetX = -width();

	if (anchorFrom & Anchor::VCenter)
		offsetY = -height() / 2;
	else if (anchorFrom & Anchor::Bottom)
		offsetY = -height();

	return QPoint(offsetX, offsetY);
}

bool GuidePopup::needsHorizontalFlip(const QPoint &pos, const QRect &bounds)
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

bool GuidePopup::needsVerticalFlip(const QPoint &pos, const QRect &bounds)
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

void GuidePopup::updateVisibility()
{
	if (!anchorWidget) {
		if (isVisible()) {
			hide();
		}
		return;
	}

	if (anchorWidget == anchorWidget->window()) {
		if (isVisible()) {
			hide();
		}
		return;
	}

	if (!isVisible()) {
		show();
	}
}

void GuidePopup::updatePosition()
{
	if (isUpdating) {
		return;
	}

	isUpdating = true;

	updateVisibility();

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
			arrowPosition.rx() = popupPos.x() + width();
		} else {
			// Arrow on left edge
			arrowPosition.rx() = popupPos.x();
		}

		arrowStart->setVisible(!showOnLeft);
		arrowEnd->setVisible(showOnLeft);
	} else {
		if (showOnBottom) {
			// Arrow on top edge
			arrowPosition.ry() = popupPos.y();
		} else {
			// Arrow on bottom edge
			arrowPosition.ry() = popupPos.y() + height();
		}

		arrowStart->setVisible(showOnBottom);
		arrowEnd->setVisible(!showOnBottom);
	}

	move(popupPos);
	update();
	adjustSize();

	QTimer::singleShot(1, [=] { raise(); });

	isUpdating = false;
}
