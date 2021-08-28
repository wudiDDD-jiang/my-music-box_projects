#include "musicbox.h"
#include "ui_musicbox.h"
#include<qDebug>
#include<QTimer>

MusicBox::MusicBox(QWidget *parent) :
    CustomMoveDialog(parent),
    ui(new Ui::MusicBox),
    m_musicCount(0),m_musicStartFlag(false),
    m_musicPosChangedFlag(false),
    m_volumnNow(0),
    m_volumnLast(0),
    m_voiceOpenFlag(true)
{
    ui->setupUi(this);
    ui->pb_setUserInfo->hide();
    m_Player = new QMediaPlayer;
    QObject::connect( m_Player , SIGNAL(positionChanged(qint64)) /*&QMediaPlayer::positionChanged */
                      , this , SLOT( slot_musicPosChanged(qint64))  );   //1.发送者 2.信号 3.接收者 4.处理信号的槽函数 5.连接方式==默认

    m_sql = new CSqlite;
    loadSqlAndSet();
//    m_Player->setVolume(30);

    //设置默认窗口背景 ./ :/ 从资源文件里面取
    QPixmap pixmap = QPixmap(":/lib/4.jpg");
    pixmap = pixmap.scaled(this->width(),this->height()); // scaled 等比例缩放
    QPalette pal(this->palette()); // QPalette 画板 控件的背景
    pal.setBrush(QPalette::Background , QBrush(pixmap));
    this->setPalette( pal );

    m_downloadDlg = new DownloadWidget;
    connect( m_downloadDlg , SIGNAL(SIG_PlayOnline(QString,QString))
             , this , SLOT(slot_PlayOnline(QString,QString)) );
    connect( m_downloadDlg , SIGNAL(SIG_musicDownloaded(QString))
             , this , SLOT(slot_musicDownloaded(QString)) );

    m_downloadDlg->hide();

    //滚动字符串初始化
    m_moveString = ui->lb_currentMusicName->text();
    m_moveString += "      ";
    m_moveStringPos = 0;

    //滚动字符串定时器
    QTimer *timer = new QTimer(this); //子类由父类回收
    //定时器时间到了执行函数
    connect( timer , SIGNAL(timeout()) , this , SLOT( slot_moveString() ) );
//    timer->setInterval(1000); //设置定时器时间为1s
//    timer->start();
    timer->start(1000); //设置定时器时间为1s , 并且开始

    m_playlist=new QMediaPlaylist;
    m_playlist->setPlaybackMode(QMediaPlaylist::Loop);
    m_Player->setPlaylist(m_playlist);
}

MusicBox::~MusicBox()
{
    if( m_downloadDlg )
    {
        m_downloadDlg->close();
        delete m_downloadDlg;
        m_downloadDlg = NULL;
    }
    if(m_sql){
        delete m_sql;
        m_sql = NULL;
    }

    delete ui;
}

//定时器时间到了执行函数 滚动字符串
void MusicBox::slot_moveString()
{
    //取子串然后拼接  到末尾要回到头  mid(起始位置,长度)    mid(起始位置)-->取从起始到最后的
    if( m_moveStringPos < m_moveString.length() )
    {
        QString tmp = m_moveString.mid(m_moveStringPos) + m_moveString.mid(0,m_moveStringPos );
        ui->lb_currentMusicName->setText( tmp );
        //位置不断移动
        m_moveStringPos ++;
    }else
    {
        m_moveStringPos = 0;
    }
}

//加载数据库, 设置歌单和音量
//背景知识
//路径 QDir  exists( "path")   mkdir("path");
//文件 QFile exists("")   open()   close();
#include<QDir>
#include<QFile>
#include<QMessageBox>
void MusicBox::loadSqlAndSet()
{
    QString DBPath = QDir::currentPath() + "/sql/";
    QString FileName = "music.db";
    //先看路径存不存在 sql  music  lrc  images
    QDir tempDir;
    //不存在要创建
    if( !tempDir.exists( QDir::currentPath() + "/sql/") )
    {
        qDebug()<<"不存在sql路径";
        tempDir.mkdir( QDir::currentPath() + "/sql/" );//创建
    }
    if( !tempDir.exists( QDir::currentPath() + "/music/") )
    {
        qDebug()<<"不存在music路径";
        tempDir.mkdir( QDir::currentPath() + "/music/" );//创建
    }
    if( !tempDir.exists( QDir::currentPath() + "/lrc/") )
    {
        qDebug()<<"不存在lrc路径";
        tempDir.mkdir( QDir::currentPath() + "/lrc/" );//创建
    }
    if( !tempDir.exists( QDir::currentPath() + "/images/") )
    {
        qDebug()<<"不存在images路径";
        tempDir.mkdir( QDir::currentPath() + "/images/" );//创建
    }

    QFile file( DBPath + FileName );
    if( file.exists() )//数据库文件存不存在
    {//存在 select 查询 , 加载到内存和控件.
        m_sql->ConnectSql( DBPath + FileName );

        //查歌单
        QString sqlStr ="select musicName, musicPath from t_musicList;";
        QStringList resList;
        bool res = m_sql->SelectSql( sqlStr , 2, resList );
        if( !res ) return;
        for( int i = 0; i < resList.count(); i += 2)
        {
            ui->lw_musicList->addItem( resList[i] );//歌名
            m_musicList[m_musicCount++] = resList[i+1];  //路径
        }
        resList.clear();
        //音量
        sqlStr ="select volumn from t_volumn;";
        res = m_sql->SelectSql( sqlStr , 1, resList );
        if( !res ) return;
        if( resList.count() > 0 )
        {
            m_volumnLast = m_volumnNow = ( (QString)resList[0] ).toInt();

            if( m_volumnNow == 0 )
            {
                //进度条设置
                //ui->slider_volumn->setValue( m_volumnNow );  不好使 可能是构造  为0 不好使
                on_slider_volumn_valueChanged( m_volumnNow );
                m_volumnLast = 30; //默认
            }else
            {
                ui->slider_volumn->setValue( m_volumnNow );
                m_voiceOpenFlag = true;
            }
        }

        //查音量
    }else
    {//不存在要创建 并且建表
        if( !file.open( QIODevice::WriteOnly|QIODevice::Text ))
        {
            qDebug()<<"数据库创建失败, 存储歌曲会出问题";
            QMessageBox::about( this, "提示" ,"数据库创建失败, 存储歌曲会出问题");
        }else
        {
            file.close();
            m_sql->ConnectSql( DBPath + FileName );
            //建表   t_musicList(musicName, musicPath)   t_volumn
            QString sqlStr = "create table t_musicList( musicName varchar(260), musicPath varchar(260));";
            m_sql->UpdateSql(sqlStr);

            sqlStr = "create table t_volumn( volumn int );";
            m_sql->UpdateSql(sqlStr);

            sqlStr = "insert into t_volumn( volumn ) values ( 30 );";
            m_sql->UpdateSql(sqlStr);

            //设置音量
            ui->slider_volumn->setValue(30);
        }
    }

}
//关闭事件
void MusicBox::closeEvent(QCloseEvent *event)
{
    if( QMessageBox::question(this, "退出提示", "确认退出?") == QMessageBox::Yes)
    {
        //存储音量
        QString sql = QString("update t_volumn set volumn = '%1'").arg(m_volumnNow);
        m_sql->UpdateSql( sql );

        event->accept();
    }else
    {
        event->ignore();
    }
}

// 切换背景
void MusicBox::slot_setBackGroundImage()
{
    //切换背景  根据歌曲名字 先去找png有没有  没有就找jpg  还是没有就设置为默认
    QPixmap pixmap = QPixmap( QString("./images/%1.png").arg(m_currentMusicName) );
    if( !pixmap.isNull() )//根据歌曲名字 先去找png有没有
    {
        pixmap = pixmap.scaled(this->width(),this->height()); // scaled 等比例缩放
        QPalette pal(this->palette()); // QPalette 画板 控件的背景
        pal.setBrush(QPalette::Background , QBrush(pixmap));
        this->setPalette( pal );
    }else
    {//没有就找jpg
        QPixmap pixmap = QPixmap( QString("./images/%1.jpg").arg(m_currentMusicName) );
        if( !pixmap.isNull() )
        {
            pixmap = pixmap.scaled(this->width(),this->height()); // scaled 等比例缩放
            QPalette pal(this->palette()); // QPalette 画板 控件的背景
            pal.setBrush(QPalette::Background , QBrush(pixmap));
            this->setPalette( pal );
        }else//还是没有就设置为默认
        {
            QPixmap pixmap = QPixmap( QString(":/lib/4.jpg") );
            pixmap = pixmap.scaled(this->width(),this->height()); // scaled 等比例缩放
            QPalette pal(this->palette()); // QPalette 画板 控件的背景
            pal.setBrush(QPalette::Background , QBrush(pixmap));
            this->setPalette( pal );
        }
    }
}

// 注释  ctrl+/
// 自动调整  ctrl+A   ctrl+i
// 跳转到声明  shift +F2   调整到定义 F2
// 帮助文档 F1

//回到初始状态
void MusicBox::slot_clearToBegin()
{
    //歌曲要暂停
    m_Player->stop();
    //ui 播放键切换  QIcon 图标类 使用导入资源 ->  ":/images/bfzn_pause.png"
    ui->pb_play->setIcon( QIcon(":/lib/1bofang.png") );
    m_musicStartFlag = false;
    m_currentMusicName = "";
    ui->lb_currentMusicName->setText( m_currentMusicName );
}
#include<QTime>
#include<QFile>
//播放进度
void MusicBox::slot_musicPosChanged(qint64 val)
{
    static bool lyricBeginFlag = false;
    //判断是否在播放
    if( m_Player->state() == QMediaPlayer::PlayingState )
    {
        if( m_Player->duration() )
        {
           //m_Player->duration()  歌曲持续时间  m_Player->position() //歌曲当前时间 以ms为单位的数
            m_musicPosChangedFlag = true;
            //更新 进度条
            ui->slider_musicProcess->setValue( m_Player->position()*100/m_Player->duration() );
            //更新 时间
//计算时间   四舍五入
            QTime duration1( 0, m_Player->position()/(60*1000) ,
                             qRound( m_Player->position()%(60*1000)/1000.0 ) , 0 ); // h, m, s, ms  --> 当前时间
            QTime duration2( 0, m_Player->duration()/(60*1000) ,
                             qRound( m_Player->duration()%(60*1000)/1000.0 ) , 0 ); //总时间
            //QStirng  .arg() 对字符串格式化
            ui->lb_musicTime->setText(  QString("%1/%2").arg( duration1.toString("mm:ss"))
                                        .arg(duration2.toString("mm:ss")) );

            //播放结束 -- 切换下一曲
            if( duration1 == duration2 )
            {
                on_pb_next_clicked();
            }
            //显示歌词
            if( m_lyricList[0] != m_currentMusicName )
            {
                QString lyricPath = QString("./lrc/%1.txt").arg(m_currentMusicName);
                QFile lyricFile( lyricPath );
                if( lyricFile.open( QIODevice::ReadOnly ))
                {//加载歌词 -- 打开文件
                    m_lyricCount = 0;
                    m_lyricList[m_lyricCount++] = m_currentMusicName;
                    ui->lw_lyric->clear();

                    lyricBeginFlag = true;
                    //读文件-->数组
                    QTextStream stream(&lyricFile); // 文件和文件流关联
                    stream.setCodec("UTF-8");//设置读取编码
                    while( !stream.atEnd() )
                    {
                       m_lyricList[m_lyricCount++] =  lyricFile.readLine();
                       // 存储举例[00:00.00] 作曲 : 暗杠
                    }

                    lyricFile.close();
                }else
                { //没有歌词
                    ui->lw_lyric->clear();
                    ui->lw_lyric->addItem("");
                    ui->lw_lyric->addItem("当前歌曲无歌词");
                    ui->lw_lyric->addItem("");

                    //设置字体和高亮
                    ui->lw_lyric->setCurrentRow(1);
                    ui->lw_lyric->currentItem()->setTextAlignment(Qt::AlignCenter);
                    ui->lw_lyric->currentItem()->setFont( QFont("微软雅黑",18) );
                    ui->lw_lyric->currentItem()->setTextColor(Qt::yellow);
                }
            }else
            {
                //首次加载, 避免错过时间匹配没有歌词
                if( lyricBeginFlag )
                {

                    for( int i = 1 ; i <= 9 ;++i)
                    {
                        QListWidgetItem *item = new QListWidgetItem("" , ui->lw_lyric ); //文本和父类
                        item->setFont( QFont("微软雅黑", 12 ));
                        item->setTextAlignment(Qt::AlignCenter);

                        if( i < m_lyricCount )
                        {
                            QString tmp = m_lyricList[i];
                            QStringList lst = tmp.split(']');  //[00:00.00] 作曲 : 暗杠
                            if( lst.size() == 2 )
                            {
                                item->setText( lst[1] ); //lst[0] --> [00:00.00  lst[1] --> 作曲 : 暗杠
                            }else
                            {
                                item->setText( m_lyricList[i] );
                            }
                        }else
                        {
                            item->setText( "" );
                        }
                        ui->lw_lyric->addItem( item );
                    }
                    lyricBeginFlag = false;
                }
                //显示歌词
                //匹配时间 匹配不到 不做显示调整
                int currentLyricRow = 0;

                for( currentLyricRow = 0; currentLyricRow < m_lyricCount ; ++currentLyricRow)
                {
                    //看歌词数组每一行 的时间 -->匹配  [00:19.200]
                    QString mm = m_lyricList[currentLyricRow].mid(1,2);//mid 取子串 从哪取多长  00
                    QString ss = m_lyricList[currentLyricRow].mid(4,2);//19

                    QTime lyricTime(0, mm.toInt() , ss.toInt() , 0 );
                    if( lyricTime == duration1 )
                        break;  //找到匹配行 --> currentLyricRow
                }
                //匹配到 调整歌词
                if( currentLyricRow < m_lyricCount ) //匹配到
                {
                    ui->lw_lyric->clear();
                    //取 上4行 和下4行
                    for( int i = currentLyricRow - 4; i <= currentLyricRow + 4; ++i  )
                    {
                        QListWidgetItem *item = new QListWidgetItem("" , ui->lw_lyric ); //文本和父类
                        item->setFont( QFont("微软雅黑", 12 ));
                        item->setTextAlignment(Qt::AlignCenter);

                        if( i > 0 && i < m_lyricCount ) // 防止越界
                        {
                            //从数组中取出
                            QString tmp = m_lyricList[i];
                            QStringList lst = tmp.split(']');
                            if( lst.size() == 2 )
                            {
                                item->setText( lst[1] );
                            }else
                            {
                                item->setText( m_lyricList[i] );
                            }
                        }else
                        {
                           //写空的item
                            item->setText( "" );
                        }
                        ui->lw_lyric->addItem( item );
                    }
                    //选中行的高亮
                    ui->lw_lyric->setCurrentRow(4);
                    ui->lw_lyric->currentItem()-> setFont( QFont("微软雅黑", 18 ) );
                    ui->lw_lyric->currentItem()->setTextColor(Qt::yellow);
                }
            }
            ui->lw_lyric->setCurrentRow(-1);
        }
    }

}

//进度条快进 快退   进入这个函数 有两种可能 一种是 由于播放设置的进度 另一种是手动设置的位置
//由于播放设置的进度 去屏蔽掉 只处理手动设置的位置  --> 设置一个标志位
void MusicBox::on_slider_musicProcess_valueChanged(int value)
{
    if( !m_musicPosChangedFlag )
    { //处理手动设置的位置
        m_Player->pause();
        //设置播放器跳转的时间
        m_Player->setPosition( value*m_Player->duration()/100 );

        if( m_musicStartFlag )
            m_Player->play();

        //拖拽时间跟着改变 todo
        QTime duration1( 0, m_Player->position()/(60*1000) ,
                         qRound( m_Player->position()%(60*1000)/1000.0 ) , 0 ); // h, m, s, ms  --> 当前时间
        QTime duration2( 0, m_Player->duration()/(60*1000) ,
                         qRound( m_Player->duration()%(60*1000)/1000.0 ) , 0 ); //总时间
        //QStirng  .arg() 对字符串格式化
        ui->lb_musicTime->setText(  QString("%1/%2").arg( duration1.toString("mm:ss"))
                                    .arg(duration2.toString("mm:ss")) );

        m_musicPosChangedFlag = true;
    }else
    {// 由于播放设置的进度
        m_musicPosChangedFlag = false;
    }
}
//点击播放按钮
void MusicBox::on_pb_play_clicked()
{
    qDebug()<<__func__;
    //播放先要设置媒体文件   F:/KuGou/xhy.mp3   F:\\KuGou\\xhy.mp3
//    m_Player->setMedia( QUrl::fromLocalFile("F:/KuGou/xhy.mp3") );
//    m_Player->play();

    //播放和暂停
    //异常
    if( m_musicCount == 0 )
    {
        //变回初始
        slot_clearToBegin();
        return;
    }

    int inx = ui->lw_musicList->currentRow();
    if( inx < 0) return;

    //判断是什么状态  首次播放  暂停到播放  播放到暂停
    if( !m_musicStartFlag ) // 首次播放  暂停到播放
    {
        //播放歌曲
        //1. player setMedia 设置路径 并播放
        if( m_Player->state() == QMediaPlayer::StoppedState //首次播放
           ||  m_currentMusicName != ui->lw_musicList->currentItem()->text() ) //暂停到播放 暂停时选别的歌了
        {
            this->slot_setMedia( m_musicList[inx] );
        }

        m_Player->play();
        //2. 当前歌曲
        m_currentMusicName = ui->lw_musicList->currentItem()->text();
        m_moveString = m_currentMusicName +"      ";
        m_moveStringPos = 0;
        //3. ui界面调整 3.1 当前歌曲 3.2 播放按钮
     //   ui->lb_currentMusicName->setText( m_currentMusicName );
        ui->pb_play->setIcon( QIcon(":/lib/1zantingtingzhi.png"));

        //4. 播放标志位true
        m_musicStartFlag = true;
        //5. 背景切换
        slot_setBackGroundImage();
    }else //播放到暂停
    {
        //歌曲要暂停
        m_Player->pause();
        //ui 播放键切换  QIcon 图标类 使用导入资源 ->  ":/lib/1bofang.png"
        ui->pb_play->setIcon( QIcon(":/lib/1bofang.png") );

        m_musicStartFlag = false;
    }
}
#include<QFileDialog>
//添加歌曲
void MusicBox::on_pb_addMusic_clicked()
{
    // ./ 当前相对路径  如果是debug时,  E:\2020fall\1130\build-MusicBox-Desktop_Qt_5_6_2_MinGW_32bit-Debug
    // 点击exe 启动  ./ 是exe同级路径
    QStringList lst = QFileDialog::getOpenFileNames(this , "打开音乐","./music");
    if( lst.count() <= 0  ) return;
    //遍历链表 , 存储绝对路径  设置ui的歌单
    //去重 -- 以绝对路径去去重
    bool hasSame = false;
    for( int i = 0 ; i < lst.count() ; ++i )
    {
        hasSame = false;
       //去重 -- 以绝对路径去去重
        QString path = lst[i];
        // 与 m_musicList[] 数组比较
        for( int j = 0; j < m_musicCount ; ++j)
        {
            if( path == m_musicList[j] )
            {
                hasSame = true;
                break;
            }
        }
        if( !hasSame ) //存储绝对路径  设置ui的歌单
        {
            m_musicList[ m_musicCount++ ] = path;
            QFileInfo info(path);
            //info.baseName();  123.mp3 --> basename --> 123
            this->ui->lw_musicList->addItem( info.baseName() );

            //加入数据库存储
            QString sql = QString("insert into t_musicList(musicName , musicPath) values ('%1','%2')")
                    .arg( info.baseName() ).arg( path);
            m_sql->UpdateSql( sql );
        }
    }
    if( m_musicCount != 0 )
    {
        ui->lw_musicList->setCurrentRow(0); //默认选中项
    }
}
//player setMedia 设置路径
void MusicBox::slot_setMedia( QString path)
{
    //要判断当前是本地文件还是网址(http  https)
    if( path.contains("http")||path.contains("https") )  //或者通过正则表达式
    {
        m_Player->setMedia( QUrl( path ) );  //网络url
    }else
    {
        m_Player->setMedia( QUrl::fromLocalFile( path ) );//本地
    }
}

//双击歌单 播放歌曲
void MusicBox::on_lw_musicList_doubleClicked(const QModelIndex &index)
{
    qDebug()<<__func__;
    //异常
    int inx = ui->lw_musicList->currentRow();
    if( inx < 0) return;
    //播放歌曲
    //1. player setMedia 设置路径 并播放
    this->slot_setMedia( m_musicList[inx] );
    m_Player->play();
    //2. 当前歌曲
    m_currentMusicName = ui->lw_musicList->currentItem()->text();
    m_moveString = m_currentMusicName +"      ";
    m_moveStringPos = 0;
    //3. ui界面调整 3.1 当前歌曲 3.2 播放按钮
  //  ui->lb_currentMusicName->setText( m_currentMusicName );
    ui->pb_play->setIcon( QIcon(":/lib/1zantingtingzhi.png"));
    //4. 播放标志位true
    m_musicStartFlag = true;
    //5. 背景切换
    slot_setBackGroundImage();
}

//下一曲
void MusicBox::on_pb_next_clicked()
{
    //顺序为例
    //异常
    if( m_musicCount == 0 )
    {
        //变回初始
        slot_clearToBegin();
        return;
    }
    //异常
    int inx = ui->lw_musicList->currentRow();
    if( inx < 0) return;
    //播放歌曲
    if(m_playlist->playbackMode()==QMediaPlaylist::Loop){//循环播放
        //1. player setMedia 设置路径 并播放   下一曲   最后一首的下一首是第一手
        this->slot_setMedia( m_musicList[ (inx+1)%m_musicCount ] );
        ui->lw_musicList->setCurrentRow( (inx+1)%m_musicCount ); //歌单切换选中项
    }
    else if(m_playlist->playbackMode()==QMediaPlaylist::Random){//随机播放
        this->slot_setMedia(m_musicList[ rand()% m_musicCount ]);
        ui->lw_musicList->setCurrentRow( rand()% m_musicCount );
    }
    else if(m_playlist->playbackMode()==QMediaPlaylist::CurrentItemInLoop){//单曲循环
        this->slot_setMedia( m_musicList[inx] );
    }

    m_Player->play();
    //2. 当前歌曲
    m_currentMusicName = ui->lw_musicList->currentItem()->text();
    m_moveString = m_currentMusicName +"      ";
    m_moveStringPos = 0;
    //3. ui界面调整 3.1 当前歌曲 3.2 播放按钮
   // ui->lb_currentMusicName->setText( m_currentMusicName );
    ui->pb_play->setIcon( QIcon(":/lib/1zantingtingzhi.png"));
    //4. 播放标志位true
    m_musicStartFlag = true;
    //5. 背景切换
    slot_setBackGroundImage();
}
//上一曲
void MusicBox::on_pb_pre_clicked()
{
    //顺序为例
    //异常
    if( m_musicCount == 0 )
    {
        //变回初始
        slot_clearToBegin();
        return;
    }
    //异常
    int inx = ui->lw_musicList->currentRow();
    if( inx < 0) return;
    //播放歌曲
    if(m_playlist->playbackMode()==QMediaPlaylist::Loop){//循环播放
        if( inx == 0)
         {
             this->slot_setMedia( m_musicList[ m_musicCount - 1 ] );
             ui->lw_musicList->setCurrentRow(   m_musicCount - 1  ); //歌单切换选中项
         }else
         {
             this->slot_setMedia( m_musicList[ inx - 1 ] );
             ui->lw_musicList->setCurrentRow(   inx - 1  ); //歌单切换选中项
         }
    }
    else if(m_playlist->playbackMode()==QMediaPlaylist::Random){//随机播放
        this->slot_setMedia(m_musicList[ rand()% m_musicCount ]);
        ui->lw_musicList->setCurrentRow(   rand()% m_musicCount );
    }
    else if(m_playlist->playbackMode()==QMediaPlaylist::CurrentItemInLoop){//单曲循环
        this->slot_setMedia( m_musicList[inx] );
    }


    m_Player->play();
    //2. 当前歌曲
    m_currentMusicName = ui->lw_musicList->currentItem()->text();
    m_moveString = m_currentMusicName +"      ";
    m_moveStringPos = 0;
    //3. ui界面调整 3.1 当前歌曲 3.2 播放按钮
 //   ui->lb_currentMusicName->setText( m_currentMusicName );
    ui->pb_play->setIcon( QIcon(":/lib/1zantingtingzhi.png"));
    //4. 播放标志位true
    m_musicStartFlag = true;
    //5. 背景切换
    slot_setBackGroundImage();
}


//音量设置 ui->slider_volumn->setValue->自动调用on_slider_volumn_valueChanged
void MusicBox::on_slider_volumn_valueChanged(int value)
{
    //音频要改变
    m_Player->setVolume( value );
    //变量要改变
    if( value != m_volumnNow )
    {
        m_volumnLast = m_volumnNow;
        m_volumnNow = value;
    }
    //ui要改变 label 和 图标
    ui->lb_volumn->setText( QString("%1%").arg(value) );

    if( value == 0) //静音 和 有音 可能会更新图标
    {//静音
        if( m_voiceOpenFlag )//非静音切换静音才切换
            ui->pb_volumn->setIcon( QIcon(":/images/voice_close.png") );
        m_voiceOpenFlag = false;
    }else
    {//有音
        if( !m_voiceOpenFlag )//静音切换非静音才切换
            ui->pb_volumn->setIcon( QIcon(":/images/voice_open.png") );
        m_voiceOpenFlag = true;
    }
}

//切换静音和非静音
void MusicBox::on_pb_volumn_clicked()
{
    //根据标志位设置静音和非静音
    if( m_voiceOpenFlag )
    {
        // --> 静音
        ui->slider_volumn->setValue( 0 );
    }else
    {
        // --> 非静音
        ui->slider_volumn->setValue( m_volumnLast );
    }
}

//删除歌曲  先从数据库中移除, 然后在从数组中移除, 不然找不到了
void MusicBox::on_pb_deleteMusic_clicked()
{
    //异常
    if( m_musicCount <= 0 ) return;
    int inx = ui->lw_musicList->currentRow();//选中歌曲下标
    if( inx < 0) return;

    //数据库更新
    QString sqlstr = QString("delete from t_musicList where musicPath = '%1'")
            .arg( m_musicList[inx] );
    m_sql->UpdateSql( sqlstr );

    //从数组中移除  -->数组移动
    for( int i = inx ; i < m_musicCount -1; i++)
    {
        m_musicList[i] = m_musicList[i+1];
    }
    //count --;
    m_musicCount--;
    //ui删除  listwidget-> takeitem  移除某一行,下面的移动
    ui->lw_musicList->takeItem( inx);

    //避免反复删除同一个下标
    ui->lw_musicList->setCurrentRow(-1);
}
//最小化
void MusicBox::on_pb_min_clicked()
{
    this->showMinimized();
}


//关闭
void MusicBox::on_pb_close_clicked()
{
    //-------------------------------------------------------------------
    m_Player->pause();
    ui->pb_play->setIcon( QIcon(":/lib/1bofang.png") );
    m_musicStartFlag = false;
    this->close();
    Q_EMIT SIG_close(); // 触发关闭事件  closeEvent
}

//点击下载模块播放在线音乐  -->不会写入数据库
void MusicBox::slot_PlayOnline(QString strUrl , QString musicName )
{
    //去重 路径的数组不可以有重复项

    bool hasSame = false;

   //去重 -- 以绝对路径去去重
    QString path = strUrl;
    // 与 m_musicList[] 数组比较
    for( int j = 0; j < m_musicCount ; ++j)
    {
        if( path == m_musicList[j] )
        {
            hasSame = true;
            //找到 就跳到歌单这个位置
            ui->lw_musicList->setCurrentRow( j );
            break;
        }
    }
    if( !hasSame ) //存储绝对路径  设置ui的歌单
    {
        m_musicList[ m_musicCount++ ] = strUrl;
        ui->lw_musicList->addItem(musicName);
        ui->lw_musicList->setCurrentRow( m_musicCount -1 );
    }

    //设置媒体文件并播放  与播放逻辑类似
    //1. player setMedia 设置路径 并播放
    m_Player->setMedia( QUrl(strUrl) );
    m_Player->play();
    //2. 当前歌曲
    m_currentMusicName = ui->lw_musicList->currentItem()->text();
    m_moveString = m_currentMusicName +"      ";
    m_moveStringPos = 0;
    //3. ui界面调整 3.1 当前歌曲 3.2 播放按钮
  //  ui->lb_currentMusicName->setText( m_currentMusicName );
    ui->pb_play->setIcon( QIcon(":/lib/1zantingtingzhi.png"));
    //4. 播放标志位true
    m_musicStartFlag = true;
    //5. 背景切换
    slot_setBackGroundImage();

}

//在线歌曲已下载
void MusicBox::slot_musicDownloaded(QString FilePath)
{
    //添加到控件
    bool hasSame = false;

   //去重 -- 以绝对路径去去重
    QString path = FilePath;
    // 与 m_musicList[] 数组比较
    for( int j = 0; j < m_musicCount ; ++j)
    {
        if( path == m_musicList[j] )
        {
            hasSame = true;
            break;
        }
    }
    if( !hasSame ) //存储绝对路径  设置ui的歌单
    {
        //写表 数组里面写 写控件上
        m_musicList[ m_musicCount++ ] = path;
        QFileInfo info(path);
        //info.baseName();  123.mp3 --> basename --> 123
        this->ui->lw_musicList->addItem( info.baseName() );

        //加入数据库存储
        QString sql = QString("insert into t_musicList(musicName , musicPath) values ('%1','%2')")
                .arg( info.baseName() ).arg( path);
        m_sql->UpdateSql( sql );

        //弹窗告诉已经下载完成
        QMessageBox::about( this , "提示" , QString("歌曲%1 下载完成, 已添加到歌单").arg(info.baseName()) );
    }
    else
    {
        //提示已下载
        QMessageBox::about( this , "提示" , QString("歌曲已下载过") );
    }


}

//点击搜索
void MusicBox::on_pb_search_clicked()
{
    //搜索
    if( !ui->le_search->text().isEmpty() )//空白字符串不搜  "" --> isEmpty()是真
    {
        //也可以加入正则表达式  TODO
        m_downloadDlg->search( ui->le_search->text() );
    }
    //显示搜索对话框
    m_downloadDlg->show();

    ui->label_17->show();
    ui->label_28->show();
    ui->stackedWidget->setCurrentIndex(2);
    //得到数据，然后再进行操作
//    search(ui->lineEdit_2->text());
}
//显示菜单
void MusicBox::on_pb_CaiDan_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
//    if(ui->lw_musicList->isVisible()){
//        ui->lw_musicList->setVisible(false);
//    }else
//    {
//        ui->lw_musicList->setVisible(true);
//    }
}
//显示歌单
void MusicBox::on_pb_GeDan_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
//    if(ui->lw_musicList->isVisible()){
//        ui->lw_musicList->setVisible(false);
//    }else
//    {
//        ui->lw_musicList->setVisible(true);
//    }
}

void MusicBox::on_pb_DengLu_clicked()
{
    Q_EMIT SIG_Login();
}

//显示用户信息
void MusicBox::slot_setInfo(int icon, QString name)
{
    //设置属性
    int m_conid=icon;
    QString m_name=name;

    //设置ui
    ui->pb_icon->setIcon( QIcon(QString(":/tx/%1.png").arg(m_conid) ) );
    ui->pb_DengLu->hide();
    ui->pb_setUserInfo->show();
    ui->pb_setUserInfo->setText(m_name);

}
//更改播放模式
void MusicBox::on_pb_BoFangMoShi_clicked()
{
    if(m_playlist->playbackMode()==QMediaPlaylist::Loop){
        ui->pb_BoFangMoShi->setToolTip(u8"随机播放");
        ui->pb_BoFangMoShi->setIcon(QIcon(":/lib/suiji.png"));

        m_playlist->setPlaybackMode(QMediaPlaylist::Random);
    }
    else if(m_playlist->playbackMode()==QMediaPlaylist::Random){
        ui->pb_BoFangMoShi->setToolTip(u8"单曲循环");
        ui->pb_BoFangMoShi->setIcon(QIcon(":/lib/danquxunhuan.png"));

        m_playlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
    }
    else if(m_playlist->playbackMode()==QMediaPlaylist::CurrentItemInLoop){
        ui->pb_BoFangMoShi->setToolTip(u8"循环播放");
        ui->pb_BoFangMoShi->setIcon(QIcon(":/lib/xunhuan.png"));
        m_playlist->setPlaybackMode(QMediaPlaylist::Loop);
    }
}
//显示下载管理
void MusicBox::on_pb_XiaZai_clicked()
{
//     ui->stackedWidget->setCurrentIndex(2);
}

//歌词
void MusicBox::on_pb_GeCi_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}
//点击添加好友
void MusicBox::on_pb_AddFriend_clicked()
{
    Q_EMIT SIG_AddFriendsShow();
}
//打开视频video
void MusicBox::on_pb_SheZhi_clicked()
{
    Q_EMIT SIG_Openvideo();
}
//排行榜
void MusicBox::on_pb_PaiHang_clicked()
{
    //m_downloadDlg->show();
}
