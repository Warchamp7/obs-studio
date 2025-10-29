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

#include <widgets/VolumeName.hpp>

#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionButton>

#include "moc_VolumeName.cpp"

VolumeName::VolumeName(obs_source_t *source, QWidget *parent) : QAbstractButton(parent)
{
	renamedSignal = OBSSignal(obs_source_get_signal_handler(source), "rename", &VolumeName::obsSourceRenamed, this);
	removedSignal = OBSSignal(obs_source_get_signal_handler(source), "remove", &VolumeName::obsSourceRemoved, this);
	destroyedSignal =
		OBSSignal(obs_source_get_signal_handler(source), "destroy", &VolumeName::obsSourceDestroyed, this);

	setText(obs_source_get_name(source));
}

VolumeName::~VolumeName() {}

void VolumeName::setAlignment(Qt::Alignment alignment_)
{
	if (textAlignment != alignment_) {
		textAlignment = alignment_;
		update();
	}
}

QSize VolumeName::sizeHint() const
{
	QStyleOptionButton opt;
	opt.initFrom(this);

	const QFontMetrics metrics(font());
	QSize textSize = metrics.size(Qt::TextSingleLine, text());

	int width = textSize.width();
	int height = textSize.height();

	int iconWidth = style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &opt, this);
	int iconHeight = iconWidth;

	if (!opt.icon.isNull()) {
		height = std::max(height, iconHeight);
	}

	const int spacing = style()->pixelMetric(QStyle::PM_ButtonMargin, &opt, this) / 2;
	width += iconWidth + spacing;

	QSize contentsSize = style()->sizeFromContents(QStyle::CT_PushButton, &opt, QSize(width, height), this);
	return contentsSize;
}

void VolumeName::obsSourceRenamed(void *data, calldata_t *params)
{
	VolumeName *widget = static_cast<VolumeName *>(data);
	const char *name = calldata_string(params, "new_name");

	QMetaObject::invokeMethod(widget, "onRenamed", Qt::QueuedConnection, Q_ARG(QString, name));
}

void VolumeName::obsSourceRemoved(void *data, calldata_t *)
{
	VolumeName *widget = static_cast<VolumeName *>(data);

	QMetaObject::invokeMethod(widget, "onRemoved", Qt::QueuedConnection);
}

void VolumeName::obsSourceDestroyed(void *data, calldata_t *)
{
	VolumeName *widget = static_cast<VolumeName *>(data);

	QMetaObject::invokeMethod(widget, "onDestroyed", Qt::QueuedConnection);
}

void VolumeName::paintEvent(QPaintEvent *)
{
	QStylePainter painter(this);
	QStyleOptionButton opt;
	opt.initFrom(this);

	painter.drawControl(QStyle::CE_PushButtonBevel, opt);

	QRect contentRect = style()->subElementRect(QStyle::SE_PushButtonContents, &opt, this);
	int paddingRight = opt.rect.right() - contentRect.right();

	int indicatorWidth = style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &opt, this);
	QRect textRect = contentRect.adjusted(0, 0, -indicatorWidth - paddingRight / 2, 0);

	painter.setFont(font());
	painter.setPen(opt.palette.color(QPalette::ButtonText));
	Qt::Alignment align = (textAlignment & (Qt::AlignLeft | Qt::AlignRight | Qt::AlignHCenter)) ? textAlignment
												    : Qt::AlignHCenter;
	align |= Qt::AlignVCenter;

	QFontMetrics metrics(font());
	QString elidedText = metrics.elidedText(text(), Qt::ElideMiddle, textRect.width());
	QRect metricsRect = metrics.boundingRect(text());

	int textWidth = metricsRect.width();
	int maxTextWidth = contentRect.width() - indicatorWidth - paddingRight / 2;

	painter.drawText(textRect, align, textWidth > maxTextWidth ? elidedText : text());

	QStyleOption arrowOpt = opt;
	QRect arrowRect(opt.rect.right() - indicatorWidth - paddingRight / 2,
			opt.rect.center().y() - indicatorWidth / 2, indicatorWidth, indicatorWidth);
	arrowOpt.rect = arrowRect;

	painter.drawPrimitive(QStyle::PE_IndicatorArrowDown, arrowOpt);
}

void VolumeName::onRenamed(QString name)
{
	setText(name);
	updateGeometry();

	std::string nameStr = name.toStdString();

	emit renamed(nameStr.c_str());
}

void VolumeName::onRemoved()
{
	emit removed();
}

void VolumeName::onDestroyed()
{
	emit destroyed();
}
