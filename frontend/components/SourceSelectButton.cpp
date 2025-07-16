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

#include "SourceSelectButton.hpp"

#include <QDrag>
#include <QFrame>
#include <QMimeData>
#include <QPainter>
#include <QStyleOptionButton>

#include <widgets/OBSBasic.hpp>

SourceSelectButton::SourceSelectButton(obs_source_t *source_, QWidget *parent) : QFrame(parent)
{
	OBSBasic *main = reinterpret_cast<OBSBasic *>(App()->GetMainWindow());

	source = source_;
	const char *sourceName = obs_source_get_name(source);
	const char *id = obs_source_get_id(source);

	uint32_t flags = obs_source_get_output_flags(source);
	bool hasVideo = (flags & OBS_SOURCE_VIDEO) == OBS_SOURCE_VIDEO;

	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	button = new QPushButton(this);
	button->setCheckable(true);
	button->setAttribute(Qt::WA_Moved);
	button->setAccessibleName(sourceName);
	button->show();

	layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	setLayout(layout);

	label = new QLabel(sourceName);
	label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
	label->setAttribute(Qt::WA_TransparentForMouseEvents);
	label->setObjectName("name");

	/*
	QFrame *thumbnail = new QFrame(this);
	// OBSSourceWidget *thumbnail = new OBSSourceWidget(this);
	// thumbnail->setSource(source);
	thumbnail->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	thumbnail->setAttribute(Qt::WA_TransparentForMouseEvents);
	thumbnail->setObjectName("thumbnail");
	thumbnail->setMinimumSize(160, 90);
	thumbnail->setMaximumSize(160, 90);
	*/

	QLabel *image = new QLabel(this);
	image->setObjectName("thumbnail");
	image->setAttribute(Qt::WA_TransparentForMouseEvents);
	image->setMinimumSize(160, 90);
	image->setMaximumSize(160, 90);
	image->setAlignment(Qt::AlignCenter);

	QPixmap pixmap = main->thumbnailManager->getThumbnail(source);
	if (!pixmap.isNull()) {
		image->setPixmap(pixmap.scaled(160, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	} else {
		QIcon icon;
		icon = main->GetSourceIcon(id);

		image->setPixmap(icon.pixmap(45, 45));
	}

	layout->addWidget(image);
	layout->addWidget(label);

	button->setFixedSize(width(), height());
	button->move(0, 0);

	setFocusProxy(button);
}

SourceSelectButton::~SourceSelectButton() {}

QPointer<QPushButton> SourceSelectButton::getButton()
{
	return button;
}

QString SourceSelectButton::text()
{
	return label->text();
}

void SourceSelectButton::resizeEvent(QResizeEvent *event)
{
	UNUSED_PARAMETER(event);

	button->setFixedSize(width(), height());
	button->move(0, 0);
}

void SourceSelectButton::moveEvent(QMoveEvent *event)
{
	UNUSED_PARAMETER(event);

	button->setFixedSize(width(), height());
	button->move(0, 0);
}

void SourceSelectButton::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		dragStartPosition = event->pos();
	}

	QFrame::mousePressEvent(event);
}

void SourceSelectButton::mouseMoveEvent(QMouseEvent *event)
{
	if (!(event->buttons() & Qt::LeftButton)) {
		return;
	}

	if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
		return;
	}

	// Prepare the data
	QMimeData *mimeData = new QMimeData;
	std::string uuid = obs_source_get_uuid(source);
	mimeData->setData("application/x-obs-source-uuid", uuid.c_str());

	// Create the drag
	QDrag *drag = new QDrag(this);
	drag->setMimeData(mimeData);
	drag->setPixmap(this->grab());
	drag->exec(Qt::CopyAction);
}
