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

#include <Idian/Tooltip.hpp>

namespace idian {

Tooltip::Tooltip(QWidget *parent_) : idian::AbstractPopup(parent_), idian::Utils(this)
{
	target = parent_;
	label = new QLabel(this);

	setAttribute(Qt::WA_TransparentForMouseEvents, true);

	setAnchorTarget(parent_);
	setAnchorTo(idian::Anchor::TopCenter);
	setAnchorFrom(idian::Anchor::BottomCenter);
	setOrientation(Qt::Vertical);
	setDisableArrow(true);

	setWidget(label);

	hide();
}

Tooltip::~Tooltip() {}

void Tooltip::setText(QString text)
{
	label->setText(text);
}

void Tooltip::setEnabled(bool enabled)
{
	QWidget::setEnabled(enabled);
	show();
}

void Tooltip::show()
{
	if (isEnabled()) {
		updatePosition();
		AbstractPopup::show();
	} else {
		AbstractPopup::hide();
	}
}

} // namespace idian
