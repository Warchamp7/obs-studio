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

#include <QFrame>
#include <QPointer>

class QPushButton;

namespace OBS {
enum class DockRegion {
	DockLeft,
	DockRight,
	DockTop,
	DockBottom,
	SplitLeft,
	SplitRight,
	SplitTop,
	SplitBottom,
	Stack,
};
}

class DockTarget : public QWidget {
	Q_OBJECT

	OBS::DockRegion region;
	QPoint globalAnchorPoint;
	QPoint anchorOffset;

	QPointer<QPushButton> regionIndicator;
	QPointer<QWidget> parent_;

	void updatePosition();

	bool firstShow = true;

protected:
	void showEvent(QShowEvent *event) override;

public:
	DockTarget(QWidget *parent, OBS::DockRegion region);

	OBS::DockRegion getRegion() const { return region; }

	void setHovered(bool hover);
	void setAnchorPoint(QPoint anchor);
	void setOffset(QPoint offset);
	bool isRelativeRegion();
};

// Dock Visual placeholder TODO: Move to new file
class DockVisual : public QWidget {
	Q_OBJECT

	QPointer<QFrame> indicatorFrame;

public:
	DockVisual(QWidget *parent);

	void setRect(QRect rect);
};
