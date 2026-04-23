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

#include "DockRegion.hpp"

#include <OBSApp.hpp>
#include <Idian/Utils.hpp>

#include <QTimer>
#include <QPushButton>
#include <QDockWidget>
#include <QStyle>
#include <QSvgRenderer>
#include <QPainter>
#include <QVBoxLayout>

namespace {
static QPixmap svgToPixmap(const QSize &imageSize, const QString &svgFile)
{
	QSvgRenderer svgRenderer(svgFile);
	QPixmap image(imageSize);
	image.fill(Qt::transparent);
	QPainter painter(&image);
	svgRenderer.render(&painter);
	painter.end();
	return image;
}
} // namespace

void DockTarget::showEvent(QShowEvent *event)
{
	if (!firstShow) {
		return;
	}

	firstShow = false;
	idian::Utils::applyColorToIcon(regionIndicator);
}

DockTarget::DockTarget(QWidget *parent, OBS::DockRegion region)
	: QWidget(parent),
	  region(region),
	  parent_(parent),
	  globalAnchorPoint(0, 0),
	  anchorOffset(0, 0)
{
	setWindowFlags(Qt::Window);
	setWindowFlag(Qt::Tool, true);
	setWindowFlag(Qt::FramelessWindowHint, true);
	setWindowFlag(Qt::WindowStaysOnTopHint, true);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	setFixedSize(48, 48);

	QVBoxLayout *layout = new QVBoxLayout(this);
	setLayout(layout);
	layout->setContentsMargins(0, 0, 0, 0);

	regionIndicator = new QPushButton(this);
	regionIndicator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	regionIndicator->setCheckable(true);

	auto idianUtils = new idian::Utils(this);
	idianUtils->applyStateStylingEventFilter(regionIndicator);

	QIcon icon;

	switch (region) {
	case OBS::DockRegion::DockLeft:
		icon = QIcon(":res/images/dock-left.svg");
		break;
	case OBS::DockRegion::SplitLeft:
		icon = QIcon(":res/images/split-dock-left.svg");
		break;
	case OBS::DockRegion::DockTop:
		break;
	case OBS::DockRegion::SplitTop:
		icon = QIcon(":res/images/split-dock-up.svg");
		break;
	case OBS::DockRegion::DockRight:
		icon = QIcon(":res/images/dock-right.svg");
		break;
	case OBS::DockRegion::SplitRight:
		icon = QIcon(":res/images/split-dock-right.svg");
		break;
	case OBS::DockRegion::DockBottom:
		break;
	case OBS::DockRegion::SplitBottom:
		icon = QIcon(":res/images/split-dock-down.svg");
		break;
	case OBS::DockRegion::Stack:
		icon = QIcon(":res/images/dock-stack.svg");
		break;
	default:
		break;
	}

	regionIndicator->setIcon(icon);
	layout->addWidget(regionIndicator);

	QWidget *blankTitle = new QWidget(this);
}

void DockTarget::updatePosition()
{
	move(QPoint(globalAnchorPoint + anchorOffset));
}

void DockTarget::setHovered(bool hover)
{
	regionIndicator->setChecked(hover);
}

void DockTarget::setAnchorPoint(QPoint anchor)
{
	if (globalAnchorPoint == parent_->mapToGlobal(anchor)) {
		return;
	}

	globalAnchorPoint = parent_->mapToGlobal(anchor);
	updatePosition();
}

void DockTarget::setOffset(QPoint offset)
{
	if (anchorOffset == offset) {
		return;
	}

	anchorOffset = offset;
	updatePosition();
}

bool DockTarget::isRelativeRegion()
{
	switch (region) {
	case OBS::DockRegion::SplitLeft:
	case OBS::DockRegion::SplitTop:
	case OBS::DockRegion::SplitRight:
	case OBS::DockRegion::SplitBottom:
	case OBS::DockRegion::Stack:
		return true;
	default:
		return false;
	}
}

// ---------------------------------------------------

DockVisual::DockVisual(QWidget *parent) : QWidget(parent)
{
	setWindowFlags(Qt::Window);
	setWindowFlag(Qt::Tool, true);
	setWindowFlag(Qt::FramelessWindowHint, true);
	setWindowFlag(Qt::WindowStaysOnTopHint, true);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_TransparentForMouseEvents, true);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	indicatorFrame = new QFrame(this);
	indicatorFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	setLayout(layout);
	layout->addWidget(indicatorFrame);

	hide();
}

void DockVisual::setRect(QRect rect)
{
	setFixedSize(rect.width(), rect.height());
	move(rect.topLeft());
}
