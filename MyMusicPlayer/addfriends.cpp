#include "addfriends.h"
#include "ui_addfriends.h"

addfriends::addfriends(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::addfriends)
{
    ui->setupUi(this);
    //用代码更改子控件，子控件回收随父控件，不必回收子控件
    m_mainLayout = new QVBoxLayout;

    //设置默认的外间距
    m_mainLayout->setContentsMargins(0,0,0,0); //左上右下
    m_mainLayout->setSpacing( 2 );

    ui->wdg_UserList->setLayout( m_mainLayout );
}

addfriends::~addfriends()
{
    delete ui;
}

void addfriends::slot_addUserItem(QWidget *item)
{
    m_mainLayout->addWidget( item );
}
void addfriends::slot_removeUserItem(QWidget *item)
{
    m_mainLayout->removeWidget( item );
}
