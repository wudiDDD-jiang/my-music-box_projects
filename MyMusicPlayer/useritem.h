#ifndef USERITEM_H
#define USERITEM_H

#include <QWidget>
#include <QMouseEvent>

namespace Ui {
class UserItem;
}

class UserItem : public QWidget
{
    Q_OBJECT

public:
    explicit UserItem(QWidget *parent = nullptr);
    ~UserItem();
signals:
    void SIG_itemClicked(int id);

public slots:
    void on_pb_icon_clicked();
    void mouseDoubleClickEvent(QMouseEvent* event);
    void slot_setInfo(int id,QString name,int iconid=26);

private:
    Ui::UserItem *ui;
public:
    int m_id;
    int m_iconid;
    int m_state;
    QString m_name;
private slots:
    void on_pb_add_clicked();
};

#endif // USERITEM_H
