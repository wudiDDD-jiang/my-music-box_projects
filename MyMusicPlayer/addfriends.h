#ifndef ADDFRIENDS_H
#define ADDFRIENDS_H

#include <QWidget>
#include<QVBoxLayout>

namespace Ui {
class addfriends;
}

class addfriends : public QWidget
{
    Q_OBJECT

public:
    explicit addfriends(QWidget *parent = 0);
    ~addfriends();
public slots:

    void slot_addUserItem(QWidget *item);
    void slot_removeUserItem(QWidget *item);
private:
    Ui::addfriends *ui;
    QVBoxLayout* m_mainLayout;
};

#endif // ADDFRIENDS_H
