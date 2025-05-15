#include "OBSSourceWidget.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

#include <util/platform.h>
#include <OBSApp.hpp>
#include <utility/display-helpers.hpp>
#include <utility/platform.hpp>
#include <widgets/OBSBasic.hpp>

#include <qt-wrappers.hpp>

#include <QScreen>
#include <QScrollArea>
#include <QWindow>

#include "moc_OBSSourceWidget.cpp"

OBSSourceWidget::OBSSourceWidget(QWidget *parent_) : QFrame(parent_), fixedAspectRatio(0.0)
{
	setAttribute(Qt::WA_TransparentForMouseEvents);

	layout = new QVBoxLayout();
	setLayout(layout);

	layout->setContentsMargins(0, 0, 0, 0);
	setMinimumSize(QSize(240, 135));
	resize(240, 135);

	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

	if (window()) {
		// window()->installEventFilter(this);
	}

	if (parent_) {
		parent_->installEventFilter(this);
	}

	QObject *checkParent = parent();
	while (checkParent) {
		QScrollArea *scrollParent = qobject_cast<QScrollArea *>(checkParent);
		if (scrollParent && scrollParent->widget()) {
			scrollParent->installEventFilter(this);
			scrollParent->widget()->installEventFilter(this);
		}

		if (!checkParent->parent() || checkParent->parent() == checkParent) {
			// blog(LOG_INFO, "Parent loop");
			break;
		}

		checkParent = checkParent->parent();
	}

	connect(&updateTimer, &QTimer::timeout, this, &OBSSourceWidget::resizeSourceView);
	updateTimer.setSingleShot(true);

	show();
}

OBSSourceWidget::OBSSourceWidget(QWidget *parent, obs_source_t *source) : OBSSourceWidget(parent)
{
	setSource(source);
}

void OBSSourceWidget::setFixedAspectRatio(double ratio)
{
	if (ratio > 0.0) {
		fixedAspectRatio = ratio;
	} else {
		fixedAspectRatio = 0;
	}
}

void OBSSourceWidget::setSource(obs_source_t *source)
{
	if (!sourceView) {
		//*
		sourceView = new OBSSourceWidgetView(this, source);
		sourceView->disableMouseEvents();
		layout->addWidget(sourceView);
		sourceView->show();
		blog(LOG_INFO, "Widget size %d %d", width(), height());
		/**/
		const char *name = obs_source_get_name(source);
		sourceName = name;

		connect(sourceView, &OBSSourceWidgetView::viewReady, this, [this, name]() { queueViewUpdate(); });
	}

	if (sourceView) {
		sourceView->setSource(source);
	}
}

void OBSSourceWidget::resizeSourceView()
{
	if (!sourceView) {
		return;
	}

	if (sourceView->sourceWidth() <= 0 || sourceView->sourceHeight() <= 0) {
		return;
	}

	double aspectRatio = fixedAspectRatio > 0
				     ? fixedAspectRatio
				     : (double)sourceView->sourceWidth() / (double)sourceView->sourceHeight();

	// Widget only expands in one direction
	bool singleExpandDirection = (sizePolicy().horizontalPolicy() & QSizePolicy::ExpandFlag) !=
				     (sizePolicy().verticalPolicy() & QSizePolicy::ExpandFlag);

	int scaledWidth = std::floor(height() / aspectRatio);
	int scaledHeight = std::floor(width() / aspectRatio);

	if (fixedAspectRatio) {
		setMaximumWidth(QWIDGETSIZE_MAX);
		setMaximumHeight(scaledHeight);
	} else if (singleExpandDirection) {
		setMaximumWidth(QWIDGETSIZE_MAX);
		setMaximumHeight(QWIDGETSIZE_MAX);

		if ((sizePolicy().horizontalPolicy() & QSizePolicy::ExpandFlag) == QSizePolicy::ExpandFlag) {
			setMaximumHeight(scaledHeight);
		} else {
			setMaximumWidth(scaledWidth);
		}
	}

	updateViewClipping();
}

void OBSSourceWidget::updateViewPosition()
{
	QRegion visible = sourceView->visibleRegion();

	if (!visible.isNull()) {
		QPoint newPosition = mapTo(window(), QPoint());
		if (newPosition.x() == windowPosition.x() && newPosition.y() == windowPosition.y()) {
			return;
		}

		windowPosition = newPosition;
	}
}

void OBSSourceWidget::updateViewClipping()
{
	bool render = false;

	QRegion visible = sourceView->visibleRegion();
	QWindow *nativeWindow = sourceView->windowHandle();

	if (nativeWindow) {
		QPoint position = sourceView->mapTo(sourceView->nativeParentWidget(), QPoint());
		QPoint altPosition = mapTo(window(), QPoint());
		nativeWindow->setGeometry(QRect(altPosition, sourceView->geometry().size()));

		if (!visible.isNull()) {
			QRect visibleRect = visible.boundingRect();

			if (visible.boundingRect().width() > 0 && visible.boundingRect().height() > 0) {
				render = true;
				nativeWindow->setMask(visibleRect);

				sourceView->CreateDisplay();
				/*blog(LOG_INFO, "Position set to %d %d", position.x(), position.y());
				blog(LOG_INFO, "Window Map Position %d %d", altPosition.x(), altPosition.y());
				blog(LOG_INFO, "Mask set to %d %d", visibleRect.width(), visibleRect.height());*/
			}
		} else {
			nativeWindow->setMask(QRegion(0, -1, sourceView->width(), 1));
		}
	}

	sourceView->enableRendering(render);
}

bool OBSSourceWidget::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::Resize) {
		/*blog(LOG_INFO, "Event filter RESIZE %s inside %s", obj->metaObject()->className(),
		     obj->parent()->metaObject()->className());*/
		queueViewUpdate();
	} else if (event->type() == QEvent::Move) {
		/*blog(LOG_INFO, "Event filter MOVE %s inside %s", obj->metaObject()->className(),
		     obj->parent()->metaObject()->className());*/
		queueViewUpdate();
	}

	return false;
}

void OBSSourceWidget::queueViewUpdate()
{
	if (!updateTimer.isActive()) {
		updateTimer.start(0);
	}
}

void OBSSourceWidget::showEvent(QShowEvent *event)
{
	queueViewUpdate();
}

void OBSSourceWidget::moveEvent(QMoveEvent *event)
{
	blog(LOG_INFO, "SourceWidget MOVE (%s) %d %d", sourceName, pos().x(), pos().y());
	queueViewUpdate();
}

void OBSSourceWidget::resizeEvent(QResizeEvent *event)
{
	blog(LOG_INFO, "SourceWidget RESIZE (%s) %d, %d", sourceName, width(), height());
	if (sourceView && width() > 16 && height() > 16) {
		queueViewUpdate();
	}
}

void OBSSourceWidget::paintEvent(QPaintEvent *event) {}

OBSSourceWidget::~OBSSourceWidget() {}

OBSSourceWidgetView::OBSSourceWidgetView(OBSSourceWidget *widget, obs_source_t *source_)
	: OBSQTDisplay(widget, Qt::Widget),
	  weakSource(OBSGetWeakRef(source_))
{
	//setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	OBSSource source = GetSource();
	if (source)
		obs_source_inc_showing(source);

	auto addDrawCallback = [this]() {
		obs_display_add_draw_callback(GetDisplay(), OBSRender, this);
	};

	enum obs_source_type type = obs_source_get_type(source);
	bool drawable_type = type == OBS_SOURCE_TYPE_INPUT || type == OBS_SOURCE_TYPE_SCENE;

	uint32_t caps = obs_source_get_output_flags(source);
	if ((caps & OBS_SOURCE_VIDEO) != 0) {
		if (drawable_type) {
			connect(this, &OBSQTDisplay::DisplayCreated, addDrawCallback);
		}
	}

	CreateDisplay();
	show();
}

OBSSourceWidgetView::~OBSSourceWidgetView()
{
	obs_display_remove_draw_callback(GetDisplay(), OBSRender, this);

	OBSSource source = GetSource();
	if (source)
		obs_source_dec_showing(source);
}

void OBSSourceWidgetView::setSourceSize(int width, int height)
{
	if (sourceWidth() == width && sourceHeight() == height) {
		return;
	}

	sourceWidth_ = width;
	sourceHeight_ = height;
	emit viewReady();
}

void OBSSourceWidgetView::disableMouseEvents()
{
	setAttribute(Qt::WA_TransparentForMouseEvents);
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

#ifdef _WIN32
	HWND hwnd = (HWND)winId();
	LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
	exStyle |= WS_EX_LAYERED | WS_EX_TRANSPARENT;
	SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);
#endif

#ifdef __APPLE__
	NSView *nsView = (NSView *)winId();
	NSWindow *nsWindow = [nsView window];
	[nsWindow setIgnoresMouseEvents:YES];
#endif

#ifdef __linux__
	// X11 input transparency using XShape or XFixes
	Display *display = QX11Info::display();
	Window win = winId();

	// Make the input shape empty — allows mouse events to pass through
	XRectangle rect = {0, 0, 0, 0};
	XserverRegion region = XFixesCreateRegion(display, &rect, 1);
	XFixesSetWindowShapeRegion(display, win, ShapeInput, 0, 0, region);
	XFixesDestroyRegion(display, region);
#endif
}

void OBSSourceWidgetView::enableRendering(bool enable)
{
	if (render != enable) {
		if (!enable) {
			blog(LOG_INFO, "[OBSSourceWidget] (%s) View went offscreen. Disabling rendering", sourceName);
		} else {
			blog(LOG_INFO, "[OBSSourceWidget] (%s) View became visible. Enabling rendering", sourceName);
		}
	}

	render = enable;
	setRender(render);
}

OBSSource OBSSourceWidgetView::GetSource()
{
	return OBSGetStrongRef(weakSource);
}

void OBSSourceWidgetView::OBSRender(void *data, uint32_t cx, uint32_t cy)
{
	OBSSourceWidgetView *widget = reinterpret_cast<OBSSourceWidgetView *>(data);

	OBSSource source = widget->GetSource();

	uint32_t sourceCX = std::max(obs_source_get_width(source), 1u);
	uint32_t sourceCY = std::max(obs_source_get_height(source), 1u);

	widget->setSourceSize(sourceCX, sourceCY);

	if (!widget->render) {
		// return;
	}

	int x, y;
	int newCX, newCY;
	float scale;

	GetScaleAndCenterPos(sourceCX, sourceCY, cx, cy, x, y, scale);

	newCX = int(scale * float(sourceCX));
	newCY = int(scale * float(sourceCY));

	gs_viewport_push();
	gs_projection_push();
	const bool previous = gs_set_linear_srgb(true);

	gs_ortho(0.0f, float(sourceCX), 0.0f, float(sourceCY), -100.0f, 100.0f);
	gs_set_viewport(x, y, newCX, newCY);
	obs_source_video_render(source);

	gs_set_linear_srgb(previous);
	gs_projection_pop();
	gs_viewport_pop();
}

void OBSSourceWidgetView::setSource(obs_source_t *source)
{
	const char *name = obs_source_get_name(source);
	sourceName = name;

	uint32_t sourceCX = std::max(obs_source_get_width(source), 1u);
	uint32_t sourceCY = std::max(obs_source_get_height(source), 1u);

	setSourceSize(sourceCX, sourceCY);

	weakSource = OBSGetWeakRef(source);
}
