/******************************************************************************
    Copyright (C) 2023 by Dennis Sädtler <dennis@obsproject.com>

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
#include <QWidget>
#include <QLayout>
#include <QSpinBox>
#include <QPushButton>

class OBSSpinBox : public QFrame {
	Q_OBJECT;

public:
	OBSSpinBox(QWidget *parent = nullptr);

	QSpinBox *spinBox() const { return sbox; }

private:
	QHBoxLayout *layout;
	QPushButton *decr;
	QPushButton *incr;
	QSpinBox *sbox;
};

class OBSDoubleSpinBox : public QFrame {
	Q_OBJECT;

public:
	OBSDoubleSpinBox(QWidget *parent = nullptr);

	QDoubleSpinBox *spinBox() const { return sbox; }

private:
	QHBoxLayout *layout;
	QPushButton *decr;
	QPushButton *incr;
	QDoubleSpinBox *sbox;
};
