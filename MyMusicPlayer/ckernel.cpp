#include "ckernel.h"
#include"qDebug"
#include "ui_logindialog.h"

#define MD5_KEY "1234"
static QByteArray GetMD5(QString val)
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    QString tmp = QString("%1_%2").arg(val).arg(MD5_KEY);
    hash.addData(tmp.toStdString().c_str() , strlen(tmp.toStdString().c_str()));
    QByteArray  result = hash.result();
    return result.toHex();
}


CKernel::CKernel(QObject *parent) : QObject(parent),m_id(0),m_UserName("大帅哥")
{
    setNetPackMap();

    m_serverIP = "192.168.10.137";

    InitConfig();
    //tcp
    m_tcpClient = new QMyTcpClient;
    m_tcpClient->setIpAndPort( (char*)m_serverIP.toStdString().c_str(),8000);//c_str()默认为const char *
    connect(m_tcpClient, SIGNAL(SIG_ReadyData(char*,int)),
            this , SLOT(slot_dealData(char*,int)) );

    //视频界面
    m_videoShow = new VideoShow;
    //音乐播放
    m_musicBox = new MusicBox;
    connect( m_musicBox , SIGNAL(SIG_AddFriendsShow()) , this , SLOT(slot_showAddFriends()) );
    connect( m_musicBox , SIGNAL(SIG_Openvideo()) , this , SLOT(slot_OpenVideo())  );
    m_musicBox->show();

    //登录界面
    m_loginDialog = new LoginDialog;
    connect( m_musicBox , SIGNAL(SIG_Login()) , this , SLOT(slot_showLogin()) );
    connect( m_loginDialog , SIGNAL(SIG_RegisterCommit(QString,QString)) ,
            this , SLOT(slot_RegisterCommit(QString,QString)) );
    connect( m_loginDialog , SIGNAL(SIG_LoginCommit(QString,QString)) ,
            this , SLOT( slot_LoginCommit( QString,QString )) );
    //connect( m_loginDialog , SIGNAL(SIG_close()) , this , SLOT( slot_quit()) );



    //connect( m_videoShow , SIGNAL(SIG_close()) ,
            // this , SLOT(DestoryInstance())  );
    //m_videoShow->show();

    //网络界面
    m_online = new OnlineDialog;
    connect( m_videoShow , SIGNAL( SIG_showOnlineDialog()) ,
             m_online , SLOT( show() )   );

    connect( m_online , SIGNAL(SIG_playRtmp(QString)) ,
             m_videoShow , SLOT( slot_playRtmp(QString) ) );

    //测试推荐好友
    m_addFriends=new addfriends;

    UserItem * item1 = new UserItem;
    UserItem * item2 = new UserItem;
    UserItem * item3 = new UserItem;

    item1->slot_setInfo( 1,"风清扬" , 0);
    item2->slot_setInfo( 1,"风清扬" , 0);
    item3->slot_setInfo( 1,"风清扬" , 0);

    m_addFriends->slot_addUserItem( item1 );
    m_addFriends->slot_addUserItem( item2 );
    m_addFriends->slot_addUserItem( item3 );


//    UserItem * item4 = new UserItem;
//    item4->slot_setInfo( 1,QString("friendName"),26);
//    m_addFriends->slot_addUser(item4);
//    UserItem * item5 = new UserItem;
//    item5->slot_setInfo( 1,QString("friendName"),26);
//    m_addFriends->slot_addUser(item5);
//    connect( item1 , SIGNAL(SIG_ItemClicked()) ,
//             this, SLOT( slot_UserItemClicked()) );

}

#define NetPackMap(a) m_NetPackMap[ (a)- DEF_PACK_BASE ]//重定义消息映射函数
void CKernel::setNetPackMap()
{
    //清空数组
    memset(m_NetPackMap ,0 ,sizeof(m_NetPackMap));

    NetPackMap(DEF_PACK_REGISTER_RS      ) = &CKernel::slot_dealRegisterRs;
    NetPackMap(DEF_PACK_LOGIN_RS         ) = &CKernel::slot_dealLoginRs;
}

#include <QCoreApplication>
#include <QFileInfo>
#include <QSettings>
//设置配置文件或者获取配置文件
void CKernel::InitConfig()
{
    // 路径  ../xx.ini
    QString path =  QCoreApplication::applicationDirPath() +"/config.ini";
    // 看有没有这个文件
    QFileInfo info(path);
    if(info.exists())
    {// 有  获取
        QSettings settings( path , QSettings::IniFormat , NULL);
        settings.beginGroup("Net");
        QVariant ip =  settings.value("IP");
        settings.endGroup();
        QString strIP = ip.toString();
        if( !strIP.isEmpty() ) m_serverIP = strIP;
    }else
    {// 没有 创建 设置默认
        QSettings settings( path , QSettings::IniFormat , NULL);
        settings.beginGroup("Net");
        settings.setValue("IP" , m_serverIP );
        settings.endGroup();

    }
}
void CKernel::DestoryInstance()
{
    qDebug() << __func__;
    if(m_musicBox){
        delete m_musicBox; m_musicBox = NULL;
    }
    if( m_videoShow ){
        m_videoShow->hide();
        delete m_videoShow; m_videoShow = NULL;
    }
    if(m_loginDialog){
        m_loginDialog->hide();
        delete m_loginDialog;
        m_loginDialog=NULL;
    }
    if(m_tcpClient){
        delete m_tcpClient;
        m_tcpClient=NULL;
    }
    if(m_online){
        delete m_online;
        m_online=NULL;
    }
}

//发送登录请求
void CKernel::slot_LoginCommit(QString tel, QString password)
{
    STRU_LOGIN_RQ rq;
    strcpy(rq.m_szUser,tel.toStdString().c_str()); //如果tel不包含中文
    QByteArray arrary=GetMD5(password);
    memcpy(rq.m_szPassword,arrary.data(),arrary.size());

    qDebug()<<rq.m_szPassword;
    m_tcpClient->SendData((char*)&rq,sizeof(rq));

}

//发送注册请求
void CKernel::slot_RegisterCommit(QString tel, QString password)
{
    STRU_REGISTER_RQ rq;
    strcpy(rq.m_szUser,tel.toStdString().c_str()); //如果tel不包含中文
    QByteArray arrary=GetMD5(password);
    memcpy(rq.m_szPassword,arrary.data(),arrary.size());

    qDebug()<<rq.m_szPassword;
    m_tcpClient->SendData((char*)&rq,sizeof(rq));
}

#include <QMessageBox>
//登录回复的处理
void CKernel::slot_dealLoginRs(char *buf, int nlen)
{
    //拆包
    STRU_LOGIN_RS* rs=(STRU_LOGIN_RS*)buf;
    switch (rs->m_lResult) {            //根据结果 显示
    case userid_no_exist:
        QMessageBox::about(m_loginDialog,"提示","用户不存在，登录失败！");
        break;
    case password_error:
        QMessageBox::about(m_loginDialog,"提示","密码错误");
        break;
    case user_online:
        QMessageBox::about(m_loginDialog,"提示","用户已经在线");
        break;
    case login_success:
        m_loginDialog->hide();
        m_id=rs->m_UserID;
        m_musicBox->slot_setInfo(25,m_UserName);
        break;
    }
}
//处理数据
void CKernel::slot_dealData(char *buf, int nlen)
{
    PackType type=*(int*)buf;
    if(type >= DEF_PACK_BASE && type < DEF_PACK_BASE+DEF_PACK_COUNT){
        PFUN fun=m_NetPackMap[type - DEF_PACK_BASE];
        if(fun){
            (this->*fun)(buf,nlen);
        }
    }
    delete [] buf;
}

//注册回复的处理
void CKernel::slot_dealRegisterRs(char *buf, int nlen)
{
    //拆包
    STRU_REGISTER_RS* rs=(STRU_REGISTER_RS*)buf;
    switch (rs->m_lResult) {            //根据结果 显示
    case userid_is_exist:
        QMessageBox::about(m_loginDialog,"提示","用户已存在！");
        break;
    case register_success:
        QMessageBox::about(m_loginDialog,"提示","注册成功");
        m_loginDialog->getUi()->tw_login_register->setCurrentIndex(0);
        break;
    }
}

//显示登录界面
void CKernel::slot_showLogin()
{
    m_loginDialog->show();
}

void CKernel::slot_showAddFriends()
{
    m_addFriends->show();
}
void CKernel::slot_OpenVideo(){
    m_videoShow->show();
}
