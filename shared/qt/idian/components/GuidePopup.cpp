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

#include <Idian/GuidePopup.hpp>

namespace idian {

GuidePopup::GuidePopup(QWidget *parent) : idian::AbstractPopup(parent), idian::Utils(this)
{
	guideWidget = new QFrame(this);
	guideLayout = new QVBoxLayout();
	guideWidget->setLayout(guideLayout);
	guideLayout->setContentsMargins(0, 0, 0, 0);
	guideLayout->setSpacing(0);

	header = new QWidget();
	addClass(header, "guide-header");
	headerLayout = new QHBoxLayout();
	header->setLayout(headerLayout);
	headerLayout->setContentsMargins(0, 0, 0, 0);
	headerLayout->setSpacing(0);

	title = new QLabel();
	title->setText("Info");
	title->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

	dismissButton = new QPushButton();
	addClass(dismissButton, "icon-close");
	addClass(dismissButton, "tooltip-close");

	headerLayout->addWidget(title);
	headerLayout->addWidget(dismissButton);

	info = new QLabel();
	addClass(info, "guide-text");
	info->setWordWrap(true);
	info->setTextFormat(Qt::MarkdownText);
	info->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

	footer = new QFrame();
	footerLayout = new QHBoxLayout();
	footer->setLayout(footerLayout);
	addClass(footer, "guide-footer");
	footerLayout->setContentsMargins(0, 0, 0, 0);
	footerLayout->setSpacing(0);

	footerNext = new QPushButton("Next", footer);
	addClass(footerNext, "button-primary");
	footerLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Preferred));
	footerLayout->addWidget(footerNext);

	guideLayout->addWidget(header);
	guideLayout->addWidget(info);
	guideLayout->addWidget(footer);
	guideLayout->addSpacerItem(new QSpacerItem(10, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Preferred));

	setMultipleSteps(false);

	setWidget(guideWidget);

	connect(dismissButton, &QAbstractButton::clicked, this, &GuidePopup::dismiss);
	connect(footerNext, &QAbstractButton::clicked, this, &GuidePopup::next);
}

GuidePopup::~GuidePopup() {}

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

void GuidePopup::dismiss()
{
	emit rejected();
	deleteLater();
}

void GuidePopup::next()
{
	emit accepted();
}

} // namespace idian
