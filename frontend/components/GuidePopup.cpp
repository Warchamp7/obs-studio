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

	float arrowSize = 10;

	QPoint arrowPos = mapFromGlobal(arrowPosition);
	int arrowX = std::max(0, arrowPos.x());
	int arrowY = std::max(0, arrowPos.y());

	int arrowOffset = (int)(title->height() - arrowSize) / 2;

	QRectF rect = QRectF(arrowX, arrowY, arrowSize, arrowSize);
	QPointF arrowPoint1;
	QPointF arrowPoint2;
	QPointF arrowTip;

	if (orientation == Qt::Horizontal) {
		arrowY = std::min((int)arrowPos.y() + arrowOffset, (int)(height() - arrowOffset - arrowSize * 2));
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
		arrowX = std::min((int)arrowPos.x() + arrowOffset, (int)(width() - arrowOffset - arrowSize * 2));
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

	QPoint anchorPoint;
	QRect rect = geometry();

	QPoint debug = getAnchorCoordinate(anchorWidget, anchorTo);
	anchorPoint = anchorWidget->parentWidget()->mapToGlobal(getAnchorCoordinate(anchorWidget, anchorTo));

	int offsetX;
	int offsetY;
	if (isAnchorLeft(anchorFrom)) {
		offsetX = 0;
	} else if (isAnchorRight(anchorFrom)) {
		offsetX = width() * -1;
	} else {
		offsetX = width() / 2 * -1;
	}

	if (isAnchorTop(anchorFrom)) {
		offsetY = 0;
	} else if (isAnchorBottom(anchorFrom)) {
		offsetY = height() * -1;
	} else {
		offsetY = height() / 2 * -1;
	}

	bool switchHorizontal = false;
	if (isAnchorRight(anchorTo)) {
		switchHorizontal = anchorPoint.x() + offsetX + width() - 2 >
				   anchorWidget->window()->frameGeometry().right();
	} else if (isAnchorLeft(anchorTo)) {
		switchHorizontal = anchorPoint.x() + offsetX + 2 < anchorWidget->window()->frameGeometry().left();
	}

	bool switchVertical = false;
	if (isAnchorTop(anchorTo)) {
		switchVertical = anchorPoint.y() + offsetY + 2 < anchorWidget->window()->frameGeometry().top();
	} else if (isAnchorBottom(anchorTo)) {
		switchVertical = anchorPoint.y() + offsetY + height() - 2 >
				 anchorWidget->window()->frameGeometry().bottom();
	}

	int arrowWidth = 10;
	int arrowHeight = arrowWidth * 2;

	int anchorX = anchorPoint.x() + offsetX;
	int anchorY = anchorPoint.y() + offsetY;

	showOnLeft = isAnchorLeft(anchorTo) || isAnchorRight(anchorFrom) || switchHorizontal;
	showOnBottom = isAnchorBottom(anchorTo) || isAnchorTop(anchorFrom) || switchVertical;

	if (switchHorizontal) {
		if (isAnchorRight(anchorTo)) {
			anchorX = anchorX - anchorWidget->geometry().width() - width() + offsetX;
		} else if (isAnchorLeft(anchorTo)) {
			anchorX = anchorX + anchorWidget->geometry().width() - offsetX;
		}
	}

	if (switchVertical) {
		anchorY = anchorPoint.y() + (offsetY * -1 - height());
	}

	arrowPosition.setX(anchorX);
	arrowPosition.setY(anchorY);

	if (orientation == Qt::Horizontal) {
		anchorX += 2;

		if (showOnLeft) {
			arrowStart->hide();
			arrowEnd->show();
		} else {
			arrowStart->show();
			arrowEnd->hide();
		}

	} else {
		anchorY += 2;

		if (showOnBottom) {
			arrowStart->show();
			arrowEnd->hide();
		} else {
			arrowStart->hide();
			arrowEnd->show();
		}
	}

	// anchorX = std::max(0, std::min(window()->width() - width(), anchorX));
	// anchorY = std::max(0, std::min(window()->height() - height() - 40, anchorY));

	// anchorX = std::max(0, anchorX);
	// anchorY = std::max(0, anchorY);

	move(anchorX, anchorY);
	update();
	adjustSize();

	/*style()->unpolish(this);
	style()->polish(this);*/

	/* Ensures this widget stays above all others */
	QTimer::singleShot(1, [=] {
		//
		raise();
	});

	QTimer::singleShot(16, [=] { isUpdating = false; });
}
