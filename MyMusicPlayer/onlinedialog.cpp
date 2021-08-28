#include "onlinedialog.h"
#include "ui_onlinedialog.h"

OnlineDialog::OnlineDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OnlineDialog)
{
    ui->setupUi(this);
}

OnlineDialog::~OnlineDialog()
{
    delete ui;
}

//点击提交 发送链接的信号
void OnlineDialog::on_pb_rtmpCommit_clicked()
{
    QString strContent = ui->le_rtmp->text();
    if( strContent.isEmpty() ) return;

    Q_EMIT SIG_playRtmp( strContent );
}
