#include <QLabel>
#include <QHBoxLayout>
#include <QPainter>
#include <QPixmap>
#include "obs-app.hpp"
#include "window-basic-main.hpp"
#include "window-basic-status-bar.hpp"
#include "window-basic-main-outputs.hpp"

#define CONGESTION_UPDATE_SECONDS 10
#define EXCELLENT_THRESHOLD 0.0f
#define GOOD_THRESHOLD 0.3333f
#define MEDIOCRE_THRESHOLD 0.6667f
#define BAD_THRESHOLD 1.0f

StatusBarWidget::StatusBarWidget(QWidget *parent)
	: QWidget(parent), ui(new Ui::StatusBarWidget)
{
	ui->setupUi(this);
}

OBSBasicStatusBar::OBSBasicStatusBar(QWidget *parent)
	: QStatusBar(parent),
	  excellentPixmap(QIcon(":/res/images/network-excellent.svg")
				  .pixmap(QSize(16, 16))),
	  goodPixmap(
		  QIcon(":/res/images/network-good.svg").pixmap(QSize(16, 16))),
	  mediocrePixmap(QIcon(":/res/images/network-mediocre.svg")
				 .pixmap(QSize(16, 16))),
	  badPixmap(
		  QIcon(":/res/images/network-bad.svg").pixmap(QSize(16, 16))),
	  disconnectedPixmap(QIcon(":/res/images/network-disconnected.svg")
				     .pixmap(QSize(16, 16))),
	  inactivePixmap(QIcon(":/res/images/network-inactive.svg")
				 .pixmap(QSize(16, 16))),
	  recordingActivePixmap(QIcon(":/res/images/recording-active.svg")
					.pixmap(QSize(16, 16))),
	  recordingPausePixmap(QIcon(":/res/images/recording-pause.svg")
				       .pixmap(QSize(16, 16))),
	  recordingPauseInactivePixmap(
		  QIcon(":/res/images/recording-pause-inactive.svg")
			  .pixmap(QSize(16, 16))),
	  recordingInactivePixmap(QIcon(":/res/images/recording-inactive.svg")
					  .pixmap(QSize(16, 16))),
	  streamingActivePixmap(QIcon(":/res/images/streaming-active.svg")
					.pixmap(QSize(16, 16))),
	  streamingInactivePixmap(QIcon(":/res/images/streaming-inactive.svg")
					  .pixmap(QSize(16, 16)))
{
	mainWidget = new StatusBarWidget(this);
	mainWidget->ui->delayInfo->setText("");
	mainWidget->ui->droppedFrames->setText(
		QTStr("DroppedFrames").arg("0", "0.0"));
	mainWidget->ui->statusIcon->setPixmap(inactivePixmap);
	mainWidget->ui->streamIcon->setPixmap(streamingInactivePixmap);
	mainWidget->ui->streamTime->setDisabled(true);
	mainWidget->ui->recordIcon->setPixmap(recordingInactivePixmap);
	mainWidget->ui->recordTime->setDisabled(true);
	mainWidget->ui->delayFrame->hide();
	mainWidget->ui->issuesFrame->hide();
	mainWidget->ui->kbps->hide();

	addPermanentWidget(mainWidget);
}

void OBSBasicStatusBar::Activate()
{
	if (!active) {
		refreshTimer = new QTimer(this);
		connect(refreshTimer, SIGNAL(timeout()), this,
			SLOT(UpdateStatusBar()));

		int skipped = video_output_get_skipped_frames(obs_get_video());
		int total = video_output_get_total_frames(obs_get_video());

		totalStreamSeconds = 0;
		totalRecordSeconds = 0;
		lastSkippedFrameCount = 0;
		startSkippedFrameCount = skipped;
		startTotalFrameCount = total;

		refreshTimer->start(1000);
		active = true;

		if (streamOutput) {
			mainWidget->ui->statusIcon->setPixmap(inactivePixmap);
		}
	}

	if (streamOutput) {
		mainWidget->ui->streamIcon->setPixmap(streamingActivePixmap);
		mainWidget->ui->streamTime->setDisabled(false);
		mainWidget->ui->issuesFrame->show();
		mainWidget->ui->kbps->show();
		firstCongestionUpdate = true;
	}

	if (recordOutput) {
		mainWidget->ui->recordIcon->setPixmap(recordingActivePixmap);
		mainWidget->ui->recordTime->setDisabled(false);
	}
}

void OBSBasicStatusBar::Deactivate()
{
	OBSBasic *main = qobject_cast<OBSBasic *>(parent());
	if (!main)
		return;

	if (!streamOutput) {
		mainWidget->ui->streamTime->setText(QString("LIVE: 00:00:00"));
		mainWidget->ui->streamTime->setDisabled(true);
		mainWidget->ui->streamIcon->setPixmap(streamingInactivePixmap);
		mainWidget->ui->statusIcon->setPixmap(inactivePixmap);
		mainWidget->ui->delayFrame->hide();
		mainWidget->ui->issuesFrame->hide();
		mainWidget->ui->kbps->hide();
		totalStreamSeconds = 0;
		congestionArray.clear();
		disconnected = false;
		firstCongestionUpdate = false;
	}

	if (!recordOutput) {
		mainWidget->ui->recordTime->setText(QString("REC: 00:00:00"));
		mainWidget->ui->recordTime->setDisabled(true);
		mainWidget->ui->recordIcon->setPixmap(recordingInactivePixmap);
		totalRecordSeconds = 0;
	}

	if (main->outputHandler && !main->outputHandler->Active()) {
		delete refreshTimer;

		mainWidget->ui->delayInfo->setText("");
		mainWidget->ui->droppedFrames->setText(
			QTStr("DroppedFrames").arg("0", "0.0"));
		mainWidget->ui->kbps->setText("0 kbps");

		delaySecTotal = 0;
		delaySecStarting = 0;
		delaySecStopping = 0;
		reconnectTimeout = 0;
		active = false;
		overloadedNotify = true;

		mainWidget->ui->statusIcon->setPixmap(inactivePixmap);
	}
}

void OBSBasicStatusBar::UpdateDelayMsg()
{
	QString msg;

	if (delaySecTotal) {
		if (delaySecStarting && !delaySecStopping) {
			msg = QTStr("Basic.StatusBar.DelayStartingIn");
			msg = msg.arg(QString::number(delaySecStarting));

		} else if (!delaySecStarting && delaySecStopping) {
			msg = QTStr("Basic.StatusBar.DelayStoppingIn");
			msg = msg.arg(QString::number(delaySecStopping));

		} else if (delaySecStarting && delaySecStopping) {
			msg = QTStr("Basic.StatusBar.DelayStartingStoppingIn");
			msg = msg.arg(QString::number(delaySecStopping),
				      QString::number(delaySecStarting));
		} else {
			msg = QTStr("Basic.StatusBar.Delay");
			msg = msg.arg(QString::number(delaySecTotal));
		}
	}
	
	if (!mainWidget->ui->delayFrame->isVisible())
		mainWidget->ui->delayFrame->show();

	mainWidget->ui->delayInfo->setText(msg);
}

#define BITRATE_UPDATE_SECONDS 2

void OBSBasicStatusBar::UpdateBandwidth()
{
	if (!streamOutput)
		return;

	if (++bitrateUpdateSeconds < BITRATE_UPDATE_SECONDS)
		return;

	uint64_t bytesSent = obs_output_get_total_bytes(streamOutput);
	uint64_t bytesSentTime = os_gettime_ns();

	if (bytesSent < lastBytesSent)
		bytesSent = 0;
	if (bytesSent == 0)
		lastBytesSent = 0;

	uint64_t bitsBetween = (bytesSent - lastBytesSent) * 8;

	double timePassed =
		double(bytesSentTime - lastBytesSentTime) / 1000000000.0;

	double kbitsPerSec = double(bitsBetween) / timePassed / 1000.0;

	QString text;
	text += QString::number(kbitsPerSec, 'f', 0) + QString(" kbps");

	mainWidget->ui->kbps->setText(text);

	if (!mainWidget->ui->kbps->isVisible())
		mainWidget->ui->kbps->show();

	lastBytesSent = bytesSent;
	lastBytesSentTime = bytesSentTime;
	bitrateUpdateSeconds = 0;
}

void OBSBasicStatusBar::UpdateCPUUsage()
{
	OBSBasic *main = qobject_cast<OBSBasic *>(parent());
	if (!main)
		return;

	QString text;
	text += QString("CPU: ") +
		QString::number(main->GetCPUUsage(), 'f', 1) + QString("%");

	mainWidget->ui->cpuUsage->setText(text);
}

void OBSBasicStatusBar::UpdateCurrentFPS()
{
	OBSBasic *main = qobject_cast<OBSBasic *>(parent());
	if (!main)
		return;

  struct obs_video_info ovi;

  QString targetFPS = "--";
  if (obs_get_video_info(&ovi)) {
	  double framerate = (double)ovi.fps_num / (double)ovi.fps_den;

	  // Limit target framerate to a max of 2 decimals without being fixed length
	  targetFPS = QString::number(floor(framerate * 100) / 100);
  }

	QString text;
  text += QString::number(obs_get_active_fps(), 'f', 2) + QString(" / ") +
	  targetFPS + QString(" FPS");

	mainWidget->ui->fpsCurrent->setText(text);
}

void OBSBasicStatusBar::UpdateStreamTime()
{
	totalStreamSeconds++;

	int seconds = totalStreamSeconds % 60;
	int totalMinutes = totalStreamSeconds / 60;
	int minutes = totalMinutes % 60;
	int hours = totalMinutes / 60;

	QString text = QString::asprintf("LIVE: %02d:%02d:%02d", hours, minutes,
					 seconds);
	mainWidget->ui->streamTime->setText(text);
	if (streamOutput && !mainWidget->ui->streamTime->isEnabled())
		mainWidget->ui->streamTime->setDisabled(false);

	if (reconnectTimeout > 0) {
		QString msg = QTStr("Basic.StatusBar.Reconnecting")
				      .arg(QString::number(retries),
					   QString::number(reconnectTimeout));
		showMessage(msg);
		disconnected = true;
		mainWidget->ui->statusIcon->setPixmap(disconnectedPixmap);
		congestionArray.clear();
		reconnectTimeout--;

	} else if (retries > 0) {
		QString msg = QTStr("Basic.StatusBar.AttemptingReconnect");
		showMessage(msg.arg(QString::number(retries)));
	}

	if (delaySecStopping > 0 || delaySecStarting > 0) {
		if (delaySecStopping > 0)
			--delaySecStopping;
		if (delaySecStarting > 0)
			--delaySecStarting;
		UpdateDelayMsg();
	}
}

extern volatile bool recording_paused;

void OBSBasicStatusBar::UpdateRecordTime()
{
	bool paused = os_atomic_load_bool(&recording_paused);

	if (!paused) {
		totalRecordSeconds++;

		int seconds = totalRecordSeconds % 60;
		int totalMinutes = totalRecordSeconds / 60;
		int minutes = totalMinutes % 60;
		int hours = totalMinutes / 60;

		QString text = QString::asprintf("REC: %02d:%02d:%02d", hours,
						 minutes, seconds);

		mainWidget->ui->recordTime->setText(text);
		if (recordOutput && !mainWidget->ui->recordTime->isEnabled())
			mainWidget->ui->recordTime->setDisabled(false);
	} else {
		mainWidget->ui->recordIcon->setPixmap(
			streamPauseIconToggle ? recordingPauseInactivePixmap
					      : recordingPausePixmap);

		streamPauseIconToggle = !streamPauseIconToggle;
	}
}

void OBSBasicStatusBar::UpdateDroppedFrames()
{
	if (!streamOutput)
		return;

	int totalDropped = obs_output_get_frames_dropped(streamOutput);
	int totalFrames = obs_output_get_total_frames(streamOutput);
	double percent = (double)totalDropped / (double)totalFrames * 100.0;

	if (!totalFrames)
		return;

	QString text = QTStr("DroppedFrames");
	text = text.arg(QString::number(totalDropped),
			QString::number(percent, 'f', 1));
	mainWidget->ui->droppedFrames->setText(text);
	
	if (!mainWidget->ui->issuesFrame->isVisible())
		mainWidget->ui->issuesFrame->show();
	

	/* ----------------------------------- *
	 * calculate congestion color          */

	float congestion = obs_output_get_congestion(streamOutput);
	float avgCongestion = (congestion + lastCongestion) * 0.5f;
	if (avgCongestion < congestion)
		avgCongestion = congestion;
	if (avgCongestion > 1.0f)
		avgCongestion = 1.0f;

	lastCongestion = congestion;

	if (disconnected)
		return;

	bool update = firstCongestionUpdate;
	float congestionOverTime = avgCongestion;

	if (congestionArray.size() >= CONGESTION_UPDATE_SECONDS) {
		congestionOverTime = accumulate(congestionArray.begin(),
						congestionArray.end(), 0.0f) /
				     (float)congestionArray.size();
		congestionArray.clear();
		update = true;
	} else {
		congestionArray.emplace_back(avgCongestion);
	}

	if (update) {
		if (congestionOverTime <= EXCELLENT_THRESHOLD + EPSILON)
			mainWidget->ui->statusIcon->setPixmap(excellentPixmap);
		else if (congestionOverTime <= GOOD_THRESHOLD)
			mainWidget->ui->statusIcon->setPixmap(goodPixmap);
		else if (congestionOverTime <= MEDIOCRE_THRESHOLD)
			mainWidget->ui->statusIcon->setPixmap(mediocrePixmap);
		else if (congestionOverTime <= BAD_THRESHOLD)
			mainWidget->ui->statusIcon->setPixmap(badPixmap);

		firstCongestionUpdate = false;
	}
}

void OBSBasicStatusBar::OBSOutputReconnect(void *data, calldata_t *params)
{
	OBSBasicStatusBar *statusBar =
		reinterpret_cast<OBSBasicStatusBar *>(data);

	int seconds = (int)calldata_int(params, "timeout_sec");
	QMetaObject::invokeMethod(statusBar, "Reconnect", Q_ARG(int, seconds));
}

void OBSBasicStatusBar::OBSOutputReconnectSuccess(void *data, calldata_t *)
{
	OBSBasicStatusBar *statusBar =
		reinterpret_cast<OBSBasicStatusBar *>(data);

	QMetaObject::invokeMethod(statusBar, "ReconnectSuccess");
}

void OBSBasicStatusBar::Reconnect(int seconds)
{
	OBSBasic *main = qobject_cast<OBSBasic *>(parent());

	if (!retries)
		main->SysTrayNotify(
			QTStr("Basic.SystemTray.Message.Reconnecting"),
			QSystemTrayIcon::Warning);

	reconnectTimeout = seconds;

	if (streamOutput) {
		delaySecTotal = obs_output_get_active_delay(streamOutput);
		UpdateDelayMsg();

		retries++;
	}
}

void OBSBasicStatusBar::ReconnectClear()
{
	retries = 0;
	reconnectTimeout = 0;
	bitrateUpdateSeconds = -1;
	lastBytesSent = 0;
	lastBytesSentTime = os_gettime_ns();
	delaySecTotal = 0;
	UpdateDelayMsg();
}

void OBSBasicStatusBar::ReconnectSuccess()
{
	OBSBasic *main = qobject_cast<OBSBasic *>(parent());

	QString msg = QTStr("Basic.StatusBar.ReconnectSuccessful");
	showMessage(msg, 4000);
	main->SysTrayNotify(msg, QSystemTrayIcon::Information);
	ReconnectClear();

	if (streamOutput) {
		delaySecTotal = obs_output_get_active_delay(streamOutput);
		UpdateDelayMsg();
		disconnected = false;
		firstCongestionUpdate = true;
	}
}

void OBSBasicStatusBar::UpdateStatusBar()
{
	OBSBasic *main = qobject_cast<OBSBasic *>(parent());

	UpdateBandwidth();

	if (streamOutput)
		UpdateStreamTime();

	if (recordOutput)
		UpdateRecordTime();

	UpdateDroppedFrames();

	int skipped = video_output_get_skipped_frames(obs_get_video());
	int total = video_output_get_total_frames(obs_get_video());

	skipped -= startSkippedFrameCount;
	total -= startTotalFrameCount;

	int diff = skipped - lastSkippedFrameCount;
	double percentage = double(skipped) / double(total) * 100.0;

	if (diff > 10 && percentage >= 0.1f) {
		showMessage(QTStr("HighResourceUsage"), 4000);
		if (!main->isVisible() && overloadedNotify) {
			main->SysTrayNotify(QTStr("HighResourceUsage"),
					    QSystemTrayIcon::Warning);
			overloadedNotify = false;
		}
	}

	lastSkippedFrameCount = skipped;
}

void OBSBasicStatusBar::StreamDelayStarting(int sec)
{
	OBSBasic *main = qobject_cast<OBSBasic *>(parent());
	if (!main || !main->outputHandler)
		return;

	streamOutput = main->outputHandler->streamOutput;

	delaySecTotal = delaySecStarting = sec;
	UpdateDelayMsg();
	Activate();
}

void OBSBasicStatusBar::StreamDelayStopping(int sec)
{
	delaySecTotal = delaySecStopping = sec;
	UpdateDelayMsg();
}

void OBSBasicStatusBar::StreamStarted(obs_output_t *output)
{
	streamOutput = output;

	signal_handler_connect(obs_output_get_signal_handler(streamOutput),
			       "reconnect", OBSOutputReconnect, this);
	signal_handler_connect(obs_output_get_signal_handler(streamOutput),
			       "reconnect_success", OBSOutputReconnectSuccess,
			       this);

	retries = 0;
	lastBytesSent = 0;
	lastBytesSentTime = os_gettime_ns();
	Activate();
}

void OBSBasicStatusBar::StreamStopped()
{
	if (streamOutput) {
		signal_handler_disconnect(
			obs_output_get_signal_handler(streamOutput),
			"reconnect", OBSOutputReconnect, this);
		signal_handler_disconnect(
			obs_output_get_signal_handler(streamOutput),
			"reconnect_success", OBSOutputReconnectSuccess, this);

		ReconnectClear();
		streamOutput = nullptr;
		clearMessage();
		Deactivate();
	}
}

void OBSBasicStatusBar::RecordingStarted(obs_output_t *output)
{
	recordOutput = output;
	Activate();
}

void OBSBasicStatusBar::RecordingStopped()
{
	recordOutput = nullptr;
	Deactivate();
}

void OBSBasicStatusBar::RecordingPaused()
{
	QString text = mainWidget->ui->recordTime->text() +
		       QStringLiteral(" (PAUSED)");
	mainWidget->ui->recordTime->setText(text);

	if (recordOutput) {
		mainWidget->ui->recordIcon->setPixmap(recordingPausePixmap);
		streamPauseIconToggle = true;
	}
}

void OBSBasicStatusBar::RecordingUnpaused()
{
	if (recordOutput) {
		mainWidget->ui->recordIcon->setPixmap(recordingActivePixmap);
	}
}
