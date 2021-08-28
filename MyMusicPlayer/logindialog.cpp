#include "logindialog.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}
//登录清空
void LoginDialog::on_pb_clear_clicked()
{
    ui->le_password->setText("");
    ui->le_tel->setText("");
}
//注册清空
void LoginDialog::on_pb_clear_register_clicked()
{
    ui->le_password_register->setText("");
    ui->le_tel_register->setText("");
    ui->le_confirm_register->setText("");
}
#include <QMessageBox>
#include <QRegExp>

//登录按钮
void LoginDialog::on_pb_login_clicked()
{
    QString tel = ui->le_tel->text();
    QString password = ui->le_password->text();

    //验证取得的信息的合法性
    if( tel.isEmpty() || password.isEmpty() )
    {
//        QMessageBox::about();
//        QMessageBox::information();
//        QMessageBox::warning();
//        QMessageBox::question();
         QMessageBox::about(this , "提示" , "手机号或密码不能为空");
         return;
    }
    QRegExp exp("[0-9]{1,11}");
    bool res = exp.exactMatch( tel );
    if(!res)
    {
        QMessageBox::about(this , "提示" , "手机号不正确！");
        return;
    }
    if(password.length() > 10)
    {
        QMessageBox::about(this , "提示" , "密码不正确！");
        return;
    }
    Q_EMIT SIG_LoginCommit(tel , password);
}
//注册按钮
void LoginDialog::on_pb_register_clicked()
{
    QString tel = ui->le_tel_register->text();
    QString password = ui->le_password_register->text();
    QString confirm = ui->le_confirm_register->text();

    //验证取得的信息的合法性
    if( tel.isEmpty() || password.isEmpty() || confirm.isEmpty())
    {

         QMessageBox::about(this , "提示" , "手机号或密码不能为空");
         return;
    }
    QRegExp exp("[0-9]{1,11}");
    bool res = exp.exactMatch( tel );

    if(!res)
    {
        QMessageBox::about(this , "提示" , "手机号不正确！");
        return;
    }
    if(password.length() > 10)
    {
        QMessageBox::about(this , "提示" , "密码不正确！");
        return;
    }
    if(password != confirm )
    {
        QMessageBox::about(this , "提示" , "两次密码不同！");
        return;
    }
    Q_EMIT SIG_RegisterCommit(tel , password);
}

void LoginDialog::closeEvent(QCloseEvent *event)
{
    event->accept();//执行事件
//    event->ignore();//忽略事件
    Q_EMIT SIG_close();
}


Ui::LoginDialog *LoginDialog::getUi() const
{
    return ui;
}


//点击全选
void LoginDialog::on_cb_all_clicked()
{
    ui->cb_all     ->setChecked(true);
    ui->cb_allnot  ->setChecked(false);

    ui->cb_ballad       ->setChecked(true);
    ui->cb_electronic   ->setChecked(true);
    ui->cb_lightmusic   ->setChecked(true);
    ui->cb_popular      ->setChecked(true);
    ui->cb_rap          ->setChecked(true);
    ui->cb_rock         ->setChecked(true);
    ui->cb_sir          ->setChecked(true);
    ui->cb_ancient_style->setChecked(true);
}

//点击全不选
void LoginDialog::on_cb_allnot_clicked()
{
    ui->cb_all  ->setChecked(false);
    ui->cb_allnot->setChecked(true);

    ui->cb_ballad       ->setChecked(false);
    ui->cb_electronic   ->setChecked(false);
    ui->cb_lightmusic   ->setChecked(false);
    ui->cb_popular      ->setChecked(false);
    ui->cb_rap          ->setChecked(false);
    ui->cb_rock         ->setChecked(false);
    ui->cb_sir          ->setChecked(false);
    ui->cb_ancient_style->setChecked(false);
}
//清空标签选项
void LoginDialog::on_pb_register_clear_clicked()
{
    slot_register_clear();
}
//注册的清空
void LoginDialog::slot_register_clear()
{
    ui->cb_all->setChecked(false);
    ui->cb_allnot->setChecked(false);

    ui->cb_ballad       ->setChecked(false);
    ui->cb_electronic   ->setChecked(false);
    ui->cb_lightmusic   ->setChecked(false);
    ui->cb_popular      ->setChecked(false);
    ui->cb_rap          ->setChecked(false);
    ui->cb_rock         ->setChecked(false);
    ui->cb_sir          ->setChecked(false);
    ui->cb_ancient_style->setChecked(false);
}
