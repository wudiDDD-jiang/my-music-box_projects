#include "downloadwidget.h"
#include "ui_downloadwidget.h"
//#include"musicwidget.h"
//#include "ui_musicwidget.h"
#include<QFile>
#include<QDir>

DownloadWidget::DownloadWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DownloadWidget)
{
    ui->setupUi(this);
    this->setWindowTitle("在线音乐");
    m_cook = new QNetworkCookieJar;

    QNetworkCookie cookie("kg_mid", QString("C3FF0AF142B6D3BE").toUtf8());
    QList<QNetworkCookie> cookies;
    cookies.append(cookie);
    m_cook->setCookiesFromUrl(cookies, QUrl("http://www.kugou.com/yy/index.php"));

    network_manager = new QNetworkAccessManager();
    network_request = new QNetworkRequest();            //发送请求一得到AlbumID和FileHash
    network_manager2 = new QNetworkAccessManager();
    network_request2 = new QNetworkRequest();			//发送请求二得到url和歌词等信息
    network_manager3 = new QNetworkAccessManager();
    network_request3 = new QNetworkRequest();			//发送请求三下载歌曲


    network_manager2->setCookieJar(m_cook);

    connect(network_manager, &QNetworkAccessManager::finished, this, &DownloadWidget::replyFinished);
    connect(network_manager2, &QNetworkAccessManager::finished, this, &DownloadWidget::replyFinished2);
    connect(network_manager3, &QNetworkAccessManager::finished, this, &DownloadWidget::replyFinished3);

    connect(this,SIGNAL(mediaAdd(QString )),this,SLOT(slot_addMedia(QString)));
    connect(this,SIGNAL(nameAdd(QString  )),this,SLOT(slot_addName(QString)));
    connect(this,SIGNAL(lrcAdd(QString )),this,SLOT(slot_addLrc(QString)));

    m_menu = new QMenu(this);
    m_menu->addAction("播放音乐");
    m_menu->addAction("下载到本地");
    connect( m_menu , SIGNAL(triggered(QAction*)),this,SLOT(slot_MenuAction(QAction*)));

}

DownloadWidget::~DownloadWidget()
{
    delete ui;
}

void DownloadWidget::search(QString str)
{
    if( str.isEmpty() ) return;
    if( m_key.compare(str) == 0 ) return ;
    ui->listWidget->clear();
    m_key = str;
    page = 0;

    m_LrcListCount = 0;
    m_MediaListCount = 0;
    //发送歌曲搜索请求
    QString KGAPISTR1 = QString("http://songsearch.kugou.com/song_search_v2?keyword="
                        "%1&page=&pagesize=&userid=-1&clientver=&platform=WebFilter&"
                                "tag=em&filter=2&iscorrection=1&privilege_filter=0").arg(str);

    network_request->setUrl(QUrl(KGAPISTR1));
    network_manager->get(*network_request);
}


void DownloadWidget::replyFinished(QNetworkReply *reply)        //发送搜索请求完成，接受到信息，然后进行解析
{
    //获取响应的信息，状态码为200表示正常
    QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    //无错误返回
    if(reply->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = reply->readAll();  //获取字节
//        qDebug()<<bytes;
        QString result(bytes);  //转化为字符串
        parseJson_getAlbumID(result);  //自定义方法，解析歌曲数据

        qDebug()<<result;
    }
    else
    {
        //处理错误
        qDebug()<<"处理错误";
    }
}
void DownloadWidget::replyFinished2(QNetworkReply *reply)       //发送搜索请求完成，接受到信息，然后进行解析         通过歌曲ID搜索
{
    //获取响应的信息，状态码为200表示正常
    QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    //无错误返回
    if(reply->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = reply->readAll();  //获取字节
        QString result(bytes);  //转化为字符串
         qDebug()<< result;
        parseJson_getplay_url(result);  //自定义方法，解析歌曲数据
    }
    else
    {
        //处理错误
        qDebug()<<"处理错误";
    }
}
void DownloadWidget::replyFinished3(QNetworkReply *reply)       //发送搜索请求完成，接受到信息，然后进行解析         通过歌曲ID搜索
{
    //获取响应的信息，状态码为200表示正常
    QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    //无错误返回
    if(reply->error() == QNetworkReply::NoError)
    {
        //todo
        QString FileName = ui->listWidget->currentItem()->text()+".mp3";
        QString FilePath = QDir::currentPath()+"/music/"+FileName;
        QFile file(FilePath);
        if( !file.open(QIODevice::WriteOnly))
            return;
        file.write( reply->readAll());
        file.close();

        emit SIG_musicDownloaded( FilePath );
    }
    else
    {
        //处理错误
        qDebug()<<"处理错误";
    }
}
void DownloadWidget::parseJson_getAlbumID(QString json)     //解析接收到的歌曲信息，得到歌曲ID
{
    QByteArray byte_array;
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(byte_array.append(json), &json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        if(parse_doucment.isObject())
        {
            QJsonObject rootObj = parse_doucment.object();
            if(rootObj.contains("data"))
            {
                QJsonValue valuedata = rootObj.value("data");
                if(valuedata.isObject())
                {
                    QJsonObject valuedataObject = valuedata.toObject();
                    if(valuedataObject.contains("lists"))
                    {
                        QJsonValue valueArray = valuedataObject.value("lists");
                        if (valueArray.isArray())
                        {
                            QJsonArray array = valueArray.toArray();
                            int size = array.size();
                            for(int i = 0;i < /*1*/size;i++)
                            {
                                QJsonValue value = array.at(i);
                                if(value.isObject())
                                {
                                    QJsonObject object = value.toObject();
                                    if(object.contains("AlbumID"))
                                    {
                                        QJsonValue AlbumID_value = object.take("AlbumID");
                                        if(AlbumID_value.isString())
                                        {
                                            album_idStr = AlbumID_value.toString();             //歌曲ID信息
                                        }
                                    }
                                    if(object.contains("FileHash"))
                                    {
                                        QJsonValue FileHash_value = object.take("FileHash");
                                        if(FileHash_value.isString())
                                        {
                                            hashStr = FileHash_value.toString();                //hash
                                        }
                                    }

                                    //通过歌曲ID发送请求，得到歌曲url和歌词
                                    QString KGAPISTR1 = QString("http://www.kugou.com/yy/index.php?r=play/getdata&hash=%1&album_id=%2&_=1497972864535")
                                            .arg(hashStr).arg(album_idStr);


//                                    qDebug()<<KGAPISTR1;

                                    network_request2->setUrl(QUrl(KGAPISTR1));
                                    network_manager2->get(*network_request2);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        qDebug()<<json_error.errorString();
    }
}
//todo
void DownloadWidget::parseJson_getplay_url(QString json)        //解析得到歌曲
{
    QByteArray byte_array;
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(byte_array.append(json), &json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        if(parse_doucment.isObject())
        {
            QJsonObject rootObj = parse_doucment.object();
            if(rootObj.contains("data"))
            {
                QJsonValue valuedata = rootObj.value("data");
                if(valuedata.isObject())
                {
                    QJsonObject valuedataObject = valuedata.toObject();
                    QString play_urlStr("");
                    if(valuedataObject.contains("play_url"))
                    {
                        QJsonValue play_url_value = valuedataObject.take("play_url");
                        if(play_url_value.isString())
                        {
                            play_urlStr = play_url_value.toString();                    //歌曲的url

                            if(play_urlStr!="")
                            {
                                emit mediaAdd(play_urlStr);
                            }

                        }
                    }
                    if(valuedataObject.contains("audio_name"))
                    {
                        QJsonValue play_name_value = valuedataObject.take("audio_name");
                        if(play_name_value.isString())
                        {
                            QString play_name = play_name_value.toString();                //歌曲名字
                            if(play_urlStr!="")
                            {
                                emit nameAdd(play_name);
                            }

                        }
                    }
                    if(valuedataObject.contains("lyrics"))                                  //lrc
                    {
                        QJsonValue play_url_value = valuedataObject.take("lyrics");
                        if(play_url_value.isString())
                        {
                            QString play_lrcStr = play_url_value.toString();
                            if(play_urlStr!="")
                            {
                                if(play_lrcStr != "")
                                {
                                    emit lrcAdd(play_lrcStr);
                                }
                                else
                                {
                                    emit lrcAdd("没有歌词");
                                }
                            }

                        }
                    }
                }
            }
        }
    }
}

void DownloadWidget::slot_addMedia(QString strMedia)
{
    qDebug()<<"strMedia"<<strMedia;
    m_mediaList[ m_MediaListCount++ ] = strMedia;
}

void DownloadWidget::slot_addName(QString strName)
{
    qDebug()<<"strName"<<strName;
    ui->listWidget->addItem(strName);
}

void DownloadWidget::slot_addLrc(QString strLrc)
{
    qDebug()<<"strLrc"<<strLrc;
    m_LrcList[ m_LrcListCount++ ] = strLrc;
}



void DownloadWidget::on_listWidget_clicked(const QModelIndex &index)
{
    //添加菜单
    m_menu->exec(QCursor::pos());
}

void DownloadWidget::slot_MenuAction(QAction* action)
{
    if( "播放音乐" == action->text() )
    {
        int idx = ui->listWidget->currentIndex().row();

        QString currentTxt =ui->listWidget->currentItem()->text();
        qDebug()<< currentTxt;
        if( m_LrcList[idx] != QString("没有歌词"))
        {
            QFile lrcFile("lrc/"+ currentTxt +".txt");

            if( lrcFile.open(QIODevice::WriteOnly))
            {
                QTextStream lrcFileStream(&lrcFile);
                lrcFileStream.setCodec("UTF-8");
                lrcFileStream << m_LrcList[idx];
                lrcFile.close();
            }
        }

        emit SIG_PlayOnline( m_mediaList[idx]  ,  ui->listWidget->currentItem()->text());

//        on_listWidget_doubleClicked( QModelIndex());
    }else if("下载到本地" == action->text()) //下载网络请求
    {
        qDebug()<<"下载到本地";
        //保存歌词
        int idx = ui->listWidget->currentIndex().row();
        QString currentTxt = ui->listWidget->currentItem()->text();
        if( m_LrcList[idx] != QString("没有歌词"))
        {
            QFile lrcFile("lrc/"+ currentTxt +".txt");

            if( lrcFile.open(QIODevice::WriteOnly))
            {
                QTextStream lrcFileStream(&lrcFile);
                lrcFileStream.setCodec("UTF-8");
                lrcFileStream << m_LrcList[idx];
                lrcFile.close();
            }
        }

        // url  https://webfs.yun.kugou.com/202003051440/e558971180cdc2c2add4ee87b8864933/G173/M09/18/0D/TYcBAF2vRz6AJfImAB30vj0K0VY201.mp3
        // 酷狗 音乐云 https  ssl加密 下载文件

        QSslConfiguration conf =  network_request3->sslConfiguration();
        conf.setPeerVerifyMode( QSslSocket::VerifyNone);
        conf.setProtocol(QSsl::TlsV1SslV3);
        network_request3->setSslConfiguration( conf );

        //网络请求
        network_request3->setUrl(QUrl( m_mediaList[ui->listWidget->currentIndex().row()]));
        network_manager3->get(*network_request3);     
    }
}

void DownloadWidget::on_listWidget_doubleClicked(const QModelIndex &index)
{
//    int idx = ui->listWidget->currentIndex().row();

    qDebug()<< ui->listWidget->currentItem()->text();

//    QFile lrcFile("lrc/"+ui->listWidget->currentItem()->text()+".txt");

//    if( lrcFile.open(QIODevice::WriteOnly))
//    {
//        QTextStream lrcFileStream(&lrcFile);
//        lrcFileStream.setCodec("UTF-8");
//        lrcFileStream << m_LrcList[idx];
//        lrcFile.close();
//    }
//    emit SIG_PlayOnline( m_mediaList[idx]  ,  ui->listWidget->currentItem()->text());
}
