#ifndef VIDEOSHOW_H
#define VIDEOSHOW_H

#include <QWidget>
#include<QMediaPlayer>
#include "videoplayer.h"
#include"QTimer"
#include<QCloseEvent>


namespace Ui {
class VideoShow;
}

class VideoShow : public QWidget
{
    Q_OBJECT

public:
    explicit VideoShow(QWidget *parent = 0);
    ~VideoShow();

signals:
    void SIG_close();
    void SIG_showOnlineDialog();

public slots:
    void slot_setOneImage(QImage img);
    void slot_PlayerStateChanged(int state);
    void slot_TimerTimeOut();
    void slot_getTotalTime(qint64 uSec);
    void slot_videoSliderValueChanged(int value);

    void closeEvent(QCloseEvent *event);
private slots:
    void on_pb_stop_clicked();

    void on_pb_listShow_clicked();

    void on_pb_open_clicked();

    void on_pb_play_clicked();

    void on_pb_pause_clicked();

    void on_pb_online_clicked();

    void slot_playRtmp(QString url);

    void on_pb_volumn_clicked();

    void on_slider_volumn_valueChanged(int value);

private:
    Ui::VideoShow *ui;

    VideoPlayer * m_player;
    QMediaPlayer *m_QMediaPlayerr;
    //音量
    int     m_volumnNow;
    int     m_volumnLast;
    bool    m_voiceOpenFlag;
    int isStop;//代表当前是否是停止状态
    QTimer *m_Timer;

};

#endif // VIDEOSHOW_H
