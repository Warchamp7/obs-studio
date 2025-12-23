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

#include "TestBuildDialog.hpp"

#include <widgets/OBSBasic.hpp>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

TestBuildDialog::TestBuildDialog(QWidget *parent) : QDialog(parent)
{
	setWindowTitle("Preview Build");
	setMinimumSize(600, 340);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSizeConstraint(QLayout::SetFixedSize);
	setLayout(layout);

	QFrame *textFrame = new QFrame(this);
	QVBoxLayout *textLayout = new QVBoxLayout(textFrame);
	textFrame->setLayout(textLayout);
	textFrame->setProperty("class", "dialog-container dialog-frame");

	QLabel *bannerLabel = new QLabel(this);
	QPixmap bannerPixmap(":/res/images/test-build-banner.png");
	bannerLabel->setPixmap(bannerPixmap.scaledToWidth(580, Qt::SmoothTransformation));
	bannerLabel->setAlignment(Qt::AlignHCenter);
	textLayout->addWidget(bannerLabel);

	QLabel *header = new QLabel("Feature Preview Build", this);
	header->setAlignment(Qt::AlignHCenter);
	header->setProperty("class", "text-heading");
	textLayout->addWidget(header);

	QLabel *label = new QLabel(
		"<br />This is an <b>unofficial</b> build with in-development features. There may be bugs, crashes or "
		"unexpected behaviour.<br /><br />Any new features being shown off in this build can and will change "
		"before release. There is no guarantee any feature seen here will be included in a future build of "
		"OBS Studio.<br /><br />If you have any thoughts or feedback, please join the "
		"<a href='https://obsproject.com/discord'>official OBS Discord</a> and direct it to <b>@Warchamp7</b> "
		"in the <b>#development</b> channel.<br /><br />"
		"<em style='color: #C01C37'><b>Do NOT bug our support volunteers for help with this build!!<br />"
		"<br />If I catch you reporting a bug or asking for support with this build in the support channels "
		"I will ban you and curse both sides of your pillows to never be cold.</b></em>",
		this);
	label->setAlignment(Qt::AlignHCenter);
	label->setWordWrap(true);
	label->setTextFormat(Qt::RichText);
	label->setOpenExternalLinks(true);
	textLayout->addWidget(label);

	layout->addWidget(textFrame);

	QHBoxLayout *footerLayout = new QHBoxLayout(this);

	layout->addLayout(footerLayout);

	QCheckBox *dontShowAgain = new QCheckBox(tr("DoNotShowAgain"), this);
	dontShowAgain->setProperty("class", "text-muted");
	footerLayout->addWidget(dontShowAgain);

	QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok, this);
	footerLayout->addWidget(buttons);

	buttons->setFocus();
	connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);

	connect(dontShowAgain, &QCheckBox::toggled, this, [this](bool checked) {
		int previewBuildIncrement = checked ? -1 : 0;

		config_set_int(App()->GetAppConfig(), "General", "PreviewBuildWarningIncrement", previewBuildIncrement);
		config_save_safe(App()->GetAppConfig(), "tmp", nullptr);
	});
}
