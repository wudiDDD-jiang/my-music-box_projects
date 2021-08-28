#include "videoshow.h"
#include <QApplication>

#include"ckernel.h"


#undef main
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    VideoShow w;
//    w.show();

    CKernel::GetInstance();

    return a.exec();
}
