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

#include <QBoxLayout>
#include <QFrame>
#include <QWidget>

namespace idian {

struct Anchor {
	enum Side : uint8_t {
		Left = 1 << 0,
		HCenter = 1 << 1,
		Right = 1 << 2,
		Top = 1 << 3,
		VCenter = 1 << 4,
		Bottom = 1 << 5
	};

	using Point = uint8_t;

	static constexpr Point TopLeft = Top | Left;
	static constexpr Point TopCenter = Top | HCenter;
	static constexpr Point TopRight = Top | Right;
	static constexpr Point LeftCenter = Left | VCenter;
	static constexpr Point Center = HCenter | VCenter;
	static constexpr Point RightCenter = Right | VCenter;
	static constexpr Point BottomLeft = Bottom | Left;
	static constexpr Point BottomCenter = Bottom | HCenter;
	static constexpr Point BottomRight = Bottom | Right;
};

class AbstractPopup : public QFrame {
	Q_OBJECT

	bool multipleSteps = false;

	QBoxLayout *mainLayout = nullptr;

	QPoint arrowPosition;
	int arrowSize = 8;
	bool hideArrow = true;

	QWidget *parentWindow = nullptr;

	QFrame *contents = nullptr;
	QBoxLayout *contentsLayout = nullptr;
	QWidget *contentsWidget = nullptr;

	QWidget *anchorWidget = nullptr;

	bool showOnLeft = false;
	bool showOnBottom = false;

	Qt::Orientation orientation = Qt::Horizontal;

	Anchor::Point anchorFrom = Anchor::TopLeft;
	Anchor::Point anchorTo = Anchor::TopRight;

	QPoint calculateOffset(Anchor::Point anchorFrom);
	bool needsHorizontalFlip(const QPoint &pos, const QRect &screen);
	bool needsVerticalFlip(const QPoint &pos, const QRect &screen);

	bool isUpdating = false;
	QTimer *queuedUpdate;

	void updateVisibility();

protected:
	bool eventFilter(QObject *obj, QEvent *ev) override;
	void paintEvent(QPaintEvent *event) override;

public:
	AbstractPopup(QWidget *parent = nullptr);
	~AbstractPopup();

	void updatePosition();

	void setAnchorTarget(QWidget *anchorWidget);
	void setWidget(QWidget *widget);

	void setAnchorFrom(Anchor::Point corner);
	void setAnchorTo(Anchor::Point corner);
	void setOrientation(Qt::Orientation orientation_);
	void setDisableArrow(bool disable = true);

public slots:
	void show();
};
} // namespace idian
