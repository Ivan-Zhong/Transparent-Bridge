#include "QtBridge.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    QtBridge w;
    w.setWindowTitle(QObject::tr(" 透明网桥的工作原理演示"));
    w.show();
    return a.exec();
}
