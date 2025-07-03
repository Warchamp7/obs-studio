#pragma once

#include <QWidget>
#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>

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

class GuidePopup : public QFrame {
	Q_OBJECT

	bool multipleSteps = false;

	QBoxLayout *mainLayout = nullptr;
	QWidget *arrowStart = nullptr;
	QWidget *arrowEnd = nullptr;

	QPoint arrowPosition;

	QWidget *parentWindow = nullptr;
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
	void updatePosition();

protected:
	bool eventFilter(QObject *obj, QEvent *ev) override;
	void paintEvent(QPaintEvent *event) override;

public:
	GuidePopup(QWidget *parent = nullptr);
	~GuidePopup();

	void setAnchorTarget(QWidget *anchorWidget);

	void setTitle(QString text);
	void setInfo(QString text);

	void setMultipleSteps(bool enable);
	bool isMultipleSteps() { return multipleSteps; }

	void setAnchorFrom(Anchor::Point corner);
	void setAnchorTo(Anchor::Point corner);
	void setOrientation(Qt::Orientation orientation_);

public slots:
	void dismiss();
	void next();

signals:
	void rejected();
	void accepted();
};
