#include "useritem.h"
#include "ui_useritem.h"
#include <QBitmap>

UserItem::UserItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserItem),m_id(0),m_iconid(0)
{
    ui->setupUi(this);
}

UserItem::~UserItem()
{
    delete ui;
}

//单击头像
void UserItem::on_pb_icon_clicked()
{
    //Q_EMIT SIG_itemClicked(m_id);
}

//双击头像
void UserItem::mouseDoubleClickEvent(QMouseEvent *event)
{
    event->accept();

    Q_EMIT SIG_itemClicked(m_id);
}

//点击添加
void UserItem::on_pb_add_clicked()
{
    Q_EMIT SIG_itemClicked(m_id);
}


//设置用户信息
void UserItem::slot_setInfo(int id, QString name, int iconid)
{
    //设置属性
    m_id=id;
    m_iconid=iconid;
    m_name=name;

    //设置ui
    ui->lb_name->setText(m_name);
    //头像在线彩色 不在线黑白
//    if(m_state){
//        ui->pb_icon->setIcon(QIcon(QString(":/tx/%1.png").arg(m_iconid) ) );
//    }else{
//        QBitmap bt;
//        bt.load(QString(":/tx/%1.png").arg(m_iconid));
//        ui->pb_icon->setIcon(bt);
//    }
//    this->repaint(); //触发重绘事件 为了让彩色变成黑白

}


