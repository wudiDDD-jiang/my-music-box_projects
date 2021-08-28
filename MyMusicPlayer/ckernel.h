#ifndef CKERNEL_H
#define CKERNEL_H

#include <QObject>
#include"videoshow.h"
#include"onlinedialog.h"
#include"musicbox.h"
#include"logindialog.h"
#include"Packdef.h"
#include"qmytcpclient.h"
#include <QByteArray>
#include <QBuffer>
#include <QMenu>
#include <QMap>
#include"useritem.h"
#include "addfriends.h"
class CKernel;
typedef void ( CKernel::*PFUN)(char* buf,int nlen );

class CKernel : public QObject
{
    Q_OBJECT

private:
    explicit CKernel(QObject *parent = 0);
    ~CKernel(){}
    CKernel(const CKernel& kernel ){}


public slots:
    static CKernel* GetInstance(){
        static CKernel kernel;
        return &kernel;
    }
    void setNetPackMap();
    void DestoryInstance();

signals:

public slots:
    void InitConfig();


public:
    PFUN m_NetPackMap[ DEF_PACK_COUNT ];//协议映射数组
    QString m_serverIP;

    int m_id;
    QString m_UserName;

    QMyTcpClient *m_tcpClient;
    VideoShow* m_videoShow;
    OnlineDialog * m_online;
    MusicBox * m_musicBox;
    LoginDialog *m_loginDialog;
    addfriends *m_addFriends;

private slots:
    void slot_RegisterCommit(QString tel, QString password);
    void slot_LoginCommit(QString tel, QString password);
    void slot_dealRegisterRs(char *buf, int nlen);
    void slot_showLogin();
    void slot_showAddFriends();
    void slot_OpenVideo();
    void slot_dealLoginRs(char *buf, int nlen);
    void slot_dealData(char *buf,int nlen);
};

#endif // CKERNEL_H
