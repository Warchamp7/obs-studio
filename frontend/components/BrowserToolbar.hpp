#pragma once

#include "SourceToolbar.hpp"

class Ui_BrowserSourceToolbar;

class BrowserToolbar : public SourceToolbar {
	Q_OBJECT

	std::unique_ptr<Ui_BrowserSourceToolbar> ui;

public:
	BrowserToolbar(QWidget *parent, OBSSource source);
	~BrowserToolbar();

signals:
	void interactClicked();

public slots:
	void on_refresh_clicked();
	void on_interactButton_clicked();
};
