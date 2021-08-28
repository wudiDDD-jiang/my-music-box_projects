#include "videoshow.h"
#include "ui_videoshow.h"
#include"QDebug"
#include"QMessageBox"


#define file_name "F:/KuGou/华语群星 - 少林英雄.mkv"


VideoShow::VideoShow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoShow),
    m_voiceOpenFlag(true),
    m_volumnNow(0),
    m_volumnLast(0)
{
    ui->setupUi(this);
    m_player = new VideoPlayer;
    connect(m_player,SIGNAL(SIG_GetOneImage(QImage)) , this , SLOT(slot_setOneImage(QImage))  );
    connect( m_player , SIGNAL( SIG_TotalTime(qint64))
             , this , SLOT( slot_getTotalTime(qint64))   );

    //m_player->setFileName( file_name);
    m_Timer = new QTimer(this);
    connect( m_Timer , SIGNAL(timeout()) , this , SLOT(slot_TimerTimeOut()) );
    m_Timer->setInterval( 500 );

    slot_PlayerStateChanged( PlayerState::Stop );


    ui->lw_videoList->setVisible(false);

    connect( ui->slider_videoProcess , SIGNAL(SIG_valueChanged(int)) ,
             this , SLOT(slot_videoSliderValueChanged(int)) );

    connect( m_player , SIGNAL(SIG_playerStateChange(int))
             , this , SLOT(slot_PlayerStateChanged(int)) );
}

VideoShow::~VideoShow()
{
    delete ui;

    if(m_player){

//        m_player->isquit = true;
        m_player->quit();
        m_player->wait(100);
        if( m_player ->isRunning() )
        {
            m_player->terminate();
            m_player->wait(100);
        }
        delete m_player;m_player = NULL;
    }
}

void VideoShow::slot_setOneImage(QImage img)
{
//    ui->lb_show->setPixmap( QPixmap::fromImage(img)  );
    ui->wdg_show->setImage( img );
}

void VideoShow::slot_PlayerStateChanged(int state){
     switch( state ){
        case PlayerState::Stop:
            m_Timer->stop();
            ui->slider_videoProcess->setValue(0);
            ui->lb_totalTime->setText("00:00:00");
            ui->lb_currentTime->setText("00:00:00");
            ui->lb_videoName->setText("");
            ui->pb_pause->hide();
            ui->pb_play->show();
            {
                QImage img;
                img.fill( Qt::black);
                ui->wdg_show->setImage( img );
            }
            this->update();
            isStop = true;
        break;
        case PlayerState::Playing:
             ui->pb_play->hide();
             ui->pb_pause->show();
             m_Timer->start();
             this->update();
             isStop = false;
        break;
     }
}

//获取当前视频时间定时器
void VideoShow::slot_TimerTimeOut()
{
    if( QObject::sender() == m_Timer )
    {
        qint64 Sec = m_player->getCurrentTime()/1000000;
        ui->slider_videoProcess->setValue(Sec);

        QString hStr = QString("00%1").arg(Sec/3600);
        QString mStr = QString("00%1").arg(Sec/60%60);
        QString sStr = QString("00%1").arg(Sec%60);

        QString str = QString("%1:%2:%3").arg(hStr.right(2)).arg(mStr.right(2)).arg(sStr.right(2));
//        QString("%1:%2:%3").arg( "1" , 2, QChar('0') );

        ui->lb_currentTime->setText(str);

        //播放结束 自动切换状态
        if(ui->slider_videoProcess->value() == ui->slider_videoProcess->maximum()
                && m_player->m_playerState == PlayerState::Stop)
        {
            slot_PlayerStateChanged( PlayerState::Stop );

        }else if(ui->slider_videoProcess->value() + 1  == ui->slider_videoProcess->maximum()
                && m_player->m_playerState == PlayerState::Stop)
        {
            slot_PlayerStateChanged( PlayerState::Stop );
        }
    }
}

//跳转到指定秒的位置
void VideoShow::slot_videoSliderValueChanged(int value)
{
//   m_player->seek( );
    if( QObject::sender() == ui->slider_videoProcess )
    {
        m_player->seek((qint64)value*1000000); //value 秒
    }
}

//点击关闭
void VideoShow::closeEvent(QCloseEvent *event)
{
    m_player->pause();
    if( QMessageBox::question( NULL , "提示","确认退出吗?") == QMessageBox::Yes )
    {
        event->accept();
        Q_EMIT SIG_close();
    }
}

//获取视频全部时间
void VideoShow::slot_getTotalTime(qint64 uSec)
{
    qint64 Sec = uSec/1000000;

    ui->slider_videoProcess->setRange(0,Sec);//精确到秒

    QString hStr = QString("00%1").arg(Sec/3600);
    QString mStr = QString("00%1").arg(Sec/60);
    QString sStr = QString("00%1").arg(Sec%60);

    QString str = QString("%1:%2:%3").arg(hStr.right(2)).arg(mStr.right(2)).arg(sStr.right(2));
    ui->lb_totalTime->setText(str);
}

void VideoShow::on_pb_stop_clicked()
{
    //m_player->isquit = true;

    m_player->stop( true );
    slot_PlayerStateChanged( PlayerState::Stop );
}

//显示列表
void VideoShow::on_pb_listShow_clicked()
{
    if( ui->lw_videoList->isVisible() )
    {
        ui->lw_videoList->setVisible(false);
    }else
    {
        ui->lw_videoList->setVisible(true);
    }
}

#include<QFileDialog>
#include<QMessageBox>

//打开文件并播放
void VideoShow::on_pb_open_clicked()
{
//    QFileDialog::getOpenFileName();
    //打开文件 弹出对话框 参数:父窗口, 标题, 默认路径, 筛选器
    QString path = QFileDialog::getOpenFileName(this,"选择要播放的文件" , "F:/",
            "视频文件 (*.flv *.rmvb *.avi *.MP4 *.mkv);; 所有文件(*.*);;");
    if(!path.isEmpty())
    {
        qDebug()<< path ;
        QFileInfo info(path);
        if( info.exists() )
        {
            m_player->stop( true ); //如果播放 你要先关闭
            m_player->setFileName(path);
            ui->lb_videoName->setText( info.baseName() );
            slot_PlayerStateChanged(PlayerState::Playing);
        }
        else
        {
            QMessageBox::information( this, "提示" , "打开文件失败");
        }
    }
}


//恢复播放
void VideoShow::on_pb_play_clicked()
{
//    m_player->play();
    if( isStop ) return;

    m_player->play();

    if( m_player->playerState() == PlayerState::Playing )
    {
        ui->pb_play->hide();
        ui->pb_pause->show();
        this->update();
    }
}

//暂停
void VideoShow::on_pb_pause_clicked()
{
    if( isStop ) return;//停止状态自然不能播放

    m_player->pause();

    if( m_player->playerState() == PlayerState::Pause)
    {
        ui->pb_play->show();
        ui->pb_pause->hide();
        this->update();
    }

}

void VideoShow::on_pb_online_clicked()
{
    Q_EMIT SIG_showOnlineDialog();
}

//播放rtmp链接
void VideoShow::slot_playRtmp(QString url)
{
    m_player->stop( true ); //如果播放 你要先关闭
    m_player->setFileName(url);
//    ui->lb_videoName->setText( info.baseName() );
    slot_PlayerStateChanged(PlayerState::Playing);
}
//点击声音图标
void VideoShow::on_pb_volumn_clicked()
{
    //根据标志位设置静音和非静音
    if( m_voiceOpenFlag ){//非静音切换静音才切换
        ui->pb_volumn->setIcon( QIcon(":/lib/jingyin.png") );
        m_voiceOpenFlag = false;}
    else
    {//有音

        ui->pb_volumn->setIcon( QIcon(":/lib/shengyin.png") );
        m_voiceOpenFlag = true;
    }
    //TODO  改变声音大小
}

void VideoShow::on_slider_volumn_valueChanged(int value)
{
//    //音频要改变
//    m_QMediaPlayerr->setVolume( value );

//    //变量要改变
//    if( value != m_volumnNow )
//    {
//        m_volumnLast = m_volumnNow;
//        m_volumnNow = value;
//    }
//    //ui要改变 label 和 图标

//    if( value == 0) //静音 和 有音 可能会更新图标
//    {//静音
//        if( m_voiceOpenFlag )//非静音切换静音才切换
//            ui->pb_volumn->setIcon( QIcon(":/lib/jingyin.png") );
//        m_voiceOpenFlag = false;
//    }else
//    {//有音
//        if( !m_voiceOpenFlag )//静音切换非静音才切换
//            ui->pb_volumn->setIcon( QIcon(":/lib/shengyin.png") );
//        m_voiceOpenFlag = true;
//    }
}
