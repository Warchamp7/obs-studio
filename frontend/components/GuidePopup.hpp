#pragma once

#include <QWidget>
#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>

class GuidePopup : public QFrame {
	Q_OBJECT

public:
	GuidePopup(QWidget *parent = nullptr);
	~GuidePopup();
	void setAnchor(QWidget *anchor);

	void setTitle(QString text);
	void setInfo(QString text);

	void setMultipleSteps(bool enable);
	bool isMultipleSteps() { return multipleSteps; }

	void setAnchorCorner(Qt::Corner corner);
	void setOrientation(Qt::Orientation orientation_);

public slots:
	void dismiss();
	void next();

signals:
	void rejected();
	void accepted();

protected:
	bool eventFilter(QObject *obj, QEvent *ev) override;
	void paintEvent(QPaintEvent *event) override;

private:
	bool multipleSteps = false;

	QBoxLayout *mainLayout = nullptr;
	QWidget *arrowStart = nullptr;
	QWidget *arrowEnd = nullptr;

	QFrame *contents = nullptr;
	QHBoxLayout *contentsLayout = nullptr;

	QVBoxLayout *textLayout = nullptr;
	QWidget *header = nullptr;
	QHBoxLayout *headerLayout = nullptr;
	QPushButton *dismissButton = nullptr;
	QLabel *title = nullptr;
	QLabel *info = nullptr;

	QFrame *footer = nullptr;
	QHBoxLayout *footerLayout = nullptr;
	QPushButton *footerNext = nullptr;

	QWidget *anchor = nullptr;

	bool showOnLeft = false;
	bool showOnBottom = false;

	Qt::Orientation orientation = Qt::Horizontal;
	Qt::Corner anchorCorner = Qt::TopRightCorner;

	void updatePosition();
};
