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

GuidePopup::GuidePopup(QWidget *parent) : QFrame(parent)
{
	if (window()) {
		setParent(window());
	}

	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

	mainLayout = new QHBoxLayout();
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);
	setLayout(mainLayout);

	arrowStart = new QWidget();
	arrowStart->setMinimumSize(8, 8);

	arrowEnd = new QWidget();
	arrowEnd->setMinimumSize(8, 8);

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
	arrowEnd->hide();

	window()->installEventFilter(this);

	setMultipleSteps(false);

	connect(dismissButton, &QAbstractButton::clicked, this, &GuidePopup::dismiss);
	connect(footerNext, &QAbstractButton::clicked, this, &GuidePopup::next);

	/* Works around an issue where switching between tabbed docks
	 * puts the central widget above these widgets
	 */
	QMainWindow *mainWindow = dynamic_cast<QMainWindow *>(window());
	if (mainWindow) {
		connect(mainWindow, &QMainWindow::tabifiedDockWidgetActivated, this, &GuidePopup::updatePosition);
	}

	show();
}

GuidePopup::~GuidePopup() {}

void GuidePopup::setAnchor(QWidget *anchor_)
{
	if (anchor) {
		anchor->removeEventFilter(this);
	}

	anchor = anchor_;
	anchor->installEventFilter(this);

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

void GuidePopup::setAnchorCorner(Qt::Corner corner)
{
	if (anchorCorner == corner) {
		return;
	}

	anchorCorner = corner;
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
	} else if (obj == anchor && event->type() == QEvent::Move) {
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

	int arrowWidth = 10;
	float arrowHeight = title->height() / 8 * 6;
	int arrowOffset = title->height() / 8;

	QRectF rect = QRectF(0, arrowOffset, arrowWidth, arrowHeight);
	QPointF arrowPoint1;
	QPointF arrowPoint2;
	QPointF arrowTip;

	if (orientation == Qt::Horizontal) {
		if (showOnLeft) {
			rect = arrowEnd->geometry();
			rect.setY(rect.y() + arrowOffset);
			rect.setHeight(arrowHeight);

			// Right-facing arrow
			arrowPoint1 = rect.topLeft();
			arrowPoint2 = rect.bottomLeft();
			arrowTip = QPointF(rect.right(), rect.top() + arrowHeight / 2);
		} else {
			rect = arrowStart->geometry();
			rect.setY(rect.y() + arrowOffset);
			rect.setHeight(arrowHeight);

			// Left-facing arrow
			arrowPoint1 = rect.topRight();
			arrowPoint2 = rect.bottomRight();
			arrowTip = QPointF(rect.left(), rect.top() + arrowHeight / 2);
		}
	} else {
		if (showOnBottom) {
			rect = arrowStart->geometry();
			rect.setX(rect.x() + arrowOffset);
			rect.setWidth(arrowHeight);

			// Up-facing arrow
			arrowPoint1 = rect.bottomLeft();
			arrowPoint2 = rect.bottomRight();
			arrowTip = QPointF(rect.left() + arrowHeight / 2, rect.top());
		} else {
			rect = arrowEnd->geometry();
			rect.setX(rect.x() + arrowOffset);
			rect.setWidth(arrowHeight);

			// Down-facing arrow
			arrowPoint1 = rect.topLeft();
			arrowPoint2 = rect.topRight();
			arrowTip = QPointF(rect.left() + arrowHeight / 2, rect.bottom());
		}
	}

	QPainterPath path;
	path.moveTo(arrowTip);
	path.lineTo(arrowPoint1);
	path.lineTo(arrowPoint2);
	path.lineTo(arrowTip);

	painter.fillPath(path, QBrush(contentsBg));
}

void GuidePopup::updatePosition()
{
	if (!anchor) {
		if (isVisible()) {
			hide();
		}
		return;
	}

	if (anchor == anchor->window()) {
		if (isVisible()) {
			hide();
		}
		return;
	}

	if (!isVisible()) {
		show();
	}

	QRect rect = geometry();
	QPoint anchorPoint = anchor->geometry().topRight();

	if (anchor->geometry().x() < 0 || anchor->geometry().y() < 0) {
		hide();
		return;
	}

	if (anchorCorner == Qt::TopLeftCorner) {
		anchorPoint = anchor->geometry().topLeft();
	} else if (anchorCorner == Qt::BottomLeftCorner) {
		anchorPoint = anchor->geometry().bottomLeft();
	} else if (anchorCorner == Qt::BottomRightCorner) {
		anchorPoint = anchor->geometry().bottomRight();
	}

	int arrowWidth = 10;
	int arrowHeight = title->height();

	int anchorX = 0;
	int anchorY = 0;

	bool anchoredToLeft = anchorCorner == Qt::TopLeftCorner || anchorCorner == Qt::BottomLeftCorner;
	bool anchoredToBottom = anchorCorner == Qt::BottomLeftCorner || anchorCorner == Qt::BottomRightCorner;

	showOnLeft = anchoredToLeft;
	showOnBottom = anchoredToBottom;

	if (orientation == Qt::Horizontal) {
		arrowStart->resize(arrowWidth, arrowHeight);
		arrowEnd->resize(arrowWidth, arrowHeight);

		anchorX = anchorPoint.x() + 2;
		anchorY = anchorPoint.y() + 2;

		if (!showOnLeft && anchorPoint.x() + width() > window()->width()) {
			showOnLeft = true;
		} else if (showOnLeft && anchorPoint.x() - width() < 0) {
			showOnLeft = false;
		}

		if (showOnLeft) {
			anchorX = anchor->geometry().left() - width();

			arrowStart->hide();
			arrowEnd->show();
		} else {
			arrowStart->show();
			arrowEnd->hide();
		}

	} else {
		arrowStart->resize(arrowHeight, arrowWidth);
		arrowEnd->resize(arrowHeight, arrowWidth);

		anchorX = anchorPoint.x() + 2;
		anchorY = anchorPoint.y() + 2;

		if (!showOnBottom && anchorPoint.y() - height() < 0) {
			showOnBottom = true;
		} else if (showOnBottom && anchorPoint.y() + height() + 20 > window()->height()) {
			showOnBottom = false;
		}

		if (showOnBottom) {
			arrowStart->show();
			arrowEnd->hide();

			if (!anchoredToBottom) {
				anchorY += 20;
			}
		} else {
			anchorY = anchor->geometry().top() - height();

			arrowStart->hide();
			arrowEnd->show();
		}
	}

	int arrowXOffset = std::max(0, anchorX + width() - window()->width());
	int arrowYOffset = std::max(0, anchorY + height() - window()->height() + 40);

	anchorX = std::max(0, std::min(window()->width() - width(), anchorX));
	anchorY = std::max(0, std::min(window()->height() - height() - 40, anchorY));

	if (orientation == Qt::Horizontal) {
		arrowStart->move(0, arrowYOffset);
		contents->move(arrowWidth, 0);
		arrowEnd->move(contents->geometry().right(), arrowYOffset);
	} else if (orientation == Qt::Vertical) {
		arrowStart->move(arrowXOffset, 0);
		contents->move(0, arrowWidth);
		arrowEnd->move(arrowXOffset, contents->geometry().bottom());
	}

	move(anchorX, anchorY);
	adjustSize();

	style()->unpolish(this);
	style()->polish(this);

	QTimer::singleShot(1, [=] { raise(); });
}
