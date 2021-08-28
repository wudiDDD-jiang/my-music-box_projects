#ifndef ONLINEDIALOG_H
#define ONLINEDIALOG_H

#include <QDialog>

namespace Ui {
class OnlineDialog;
}

class OnlineDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OnlineDialog(QWidget *parent = 0);
    ~OnlineDialog();
signals:
    void SIG_playRtmp( QString strContent );
private slots:
    void on_pb_rtmpCommit_clicked();

private:
    Ui::OnlineDialog *ui;
};

#endif // ONLINEDIALOG_H
