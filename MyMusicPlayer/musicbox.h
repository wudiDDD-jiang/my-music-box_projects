#ifndef MUSICBOX_H
#define MUSICBOX_H

#include <QDialog>
#include<QMediaPlayer>
#include"csqlite.h"
#include<QCloseEvent>
#include"customwidget.h"
#include"downloadwidget.h"
#include <QMediaPlaylist>
#include <QPushButton>


namespace Ui {
class MusicBox;
}
#define DEF_MAX_MUSIC_COUNT (500)
#define DEF_MAX_LYRIC_COUNT (1000)

class MusicBox : public CustomMoveDialog
{
    Q_OBJECT
signals:
    void SIG_close();
    void SIG_Login();
    void SIG_AddFriendsShow();
    void SIG_Openvideo();
public:
    explicit MusicBox(QWidget *parent = 0);
    ~MusicBox();

private slots:
    void on_pb_play_clicked();

    void on_pb_addMusic_clicked();

    void on_lw_musicList_doubleClicked(const QModelIndex &index);

    void slot_setMedia(QString path);
    void slot_clearToBegin();
    void on_pb_next_clicked();

    void on_pb_pre_clicked();

    void slot_musicPosChanged(qint64 val);
    void on_slider_musicProcess_valueChanged(int value);

    void slot_setBackGroundImage();
    void on_slider_volumn_valueChanged(int value);

    void on_pb_volumn_clicked();

    void loadSqlAndSet();

    void closeEvent(QCloseEvent *event);
    void on_pb_deleteMusic_clicked();

    void on_pb_min_clicked();

    void on_pb_close_clicked();

    void slot_moveString();
    void slot_PlayOnline(QString strUrl, QString musicName);
    void slot_musicDownloaded(QString FilePath);
    void on_pb_search_clicked();
    void on_pb_CaiDan_clicked();//显示菜单
    void on_pb_GeDan_clicked();
    void on_pb_DengLu_clicked();

    void on_pb_BoFangMoShi_clicked();

    void on_pb_XiaZai_clicked();

    void on_pb_GeCi_clicked();

    void on_pb_AddFriend_clicked();

    void on_pb_SheZhi_clicked();

    void on_pb_PaiHang_clicked();

public slots:
    void slot_setInfo(int icon, QString name);

    //void changeStackWidgetIndex(int index);
private:

    Ui::MusicBox *ui;
    QMediaPlayer *m_Player;

    QString  m_musicList[DEF_MAX_MUSIC_COUNT]; //存储歌曲的绝对路径与歌单一一对应
    int     m_musicCount;//歌曲数量
    bool    m_musicStartFlag;//开始播放的标志
    QString  m_currentMusicName;//当前歌曲名

    //进度标志位
    bool m_musicPosChangedFlag;

    ///歌词
    /// m_lyricList[] 歌词缓存数组
    /// m_lyricCount 当前歌曲多少行歌词
    QString m_lyricList[DEF_MAX_LYRIC_COUNT];
    int m_lyricCount;

    //音量
    int     m_volumnNow;
    int     m_volumnLast;
    bool    m_voiceOpenFlag;

    CSqlite *m_sql;

    //滚动歌曲名
    QString m_moveString;
    int m_moveStringPos;

    DownloadWidget *m_downloadDlg;

    QMediaPlaylist * m_playlist;
    QTimer *timerShow;
};

#endif // MUSICBOX_H
