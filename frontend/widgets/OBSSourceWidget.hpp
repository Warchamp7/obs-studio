#pragma once

#include "OBSQTDisplay.hpp"

#include <QFrame>
#include <QTimer>
#include <QVBoxLayout>

class OBSSourceWidgetView;

class OBSSourceWidget : public QFrame {
	Q_OBJECT

private:
	OBSSourceWidgetView *sourceView = nullptr;
	QVBoxLayout *layout;

	double fixedAspectRatio;
	QPoint windowPosition;

	QTimer updateTimer;
	void queueViewUpdate();

	void showEvent(QShowEvent *event) override;
	void moveEvent(QMoveEvent *event) override;
	void resizeEvent(QResizeEvent *event) override;
	void paintEvent(QPaintEvent *event) override;

public:
	OBSSourceWidget(QWidget *parent);
	OBSSourceWidget(QWidget *parent, obs_source_t *source);
	~OBSSourceWidget();

	void setFixedAspectRatio(double ratio);
	void setSource(obs_source_t *source);

	const char *sourceName;

	void resizeSourceView();
	void updateViewPosition();
	void updateViewClipping();

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;
};

class OBSSourceWidgetView : public OBSQTDisplay {
	Q_OBJECT

private:
	OBSWeakSourceAutoRelease weakSource;

	static void OBSRender(void *data, uint32_t cx, uint32_t cy);

	QRect prevGeometry;

	const char *sourceName;

	int32_t sourceWidth_;
	int32_t sourceHeight_;

	bool render = true;

public:
	OBSSourceWidgetView(OBSSourceWidget *parent, obs_source_t *source);
	~OBSSourceWidgetView();

	void setSource(obs_source_t *source);
	void setSourceSize(int width, int height);
	int sourceWidth() { return sourceWidth_; }
	int sourceHeight() { return sourceHeight_; }

	void disableMouseEvents();
	void enableRendering(bool enable);

	OBSSource GetSource();

signals:
	void viewReady();
};
