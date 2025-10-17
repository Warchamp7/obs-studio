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

#include <Idian/AbstractPopup.hpp>
#include <Idian/Utils.hpp>

#include <QPushButton>
#include <QLabel>

namespace idian {

class Tooltip : public idian::AbstractPopup, public idian::Utils {
	Q_OBJECT

	QLabel *label = nullptr;
	QWidget *target = nullptr;

public:
	Tooltip(QWidget *parent = nullptr);
	~Tooltip();

	void setText(QString text);
	void setEnabled(bool enabled);

public slots:
	void show();
};

} // namespace idian
