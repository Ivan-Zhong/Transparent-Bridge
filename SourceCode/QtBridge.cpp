#include <string>
#include <string.h>
#include<cstring>
#include "QtBridge.h"
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QTextCodec>
#include <map>
#include<cmath>
#include "fun.h"
#include<Windows.h>
#include<QTime>
#include<QMessageBox>
#include<QDebug>
using namespace std;
QtBridge::QtBridge(QWidget *parent)
    : QMainWindow(parent)
{
    // 设置背景颜色
    QWidget *m_pWidget = new QWidget(this);
    m_pWidget->setGeometry(0, 0, 1027, 836);
    QPalette pal(m_pWidget->palette());
    //pal.setColor(QPalette::Background, QColor(255,248,225));
    pal.setColor(QPalette::Background, QColor(241,254,255));
    m_pWidget->setAutoFillBackground(true);
    m_pWidget->setPalette(pal);
    m_pWidget->show();
    ui.setupUi(this);
    // 输入框
    ui.input1->setPlaceholderText("请输入数据包的发送端(单个大写字母或完整mac)");
    ui.input2->setPlaceholderText("请输入数据包的接收端(单个大写字母或完整mac)");
    QTextCodec* codec = QTextCodec::codecForName("GBK");//或者"GBK",不分大小写
    QTextCodec::setCodecForLocale(codec);
    // 设置表格
    this-> t1 = new QStandardItemModel();
    t1->setHorizontalHeaderLabels({ "      MAC addr      ","port" });
    ui.table1->setModel(t1);
    ui.table1->setColumnWidth(0,220);
    ui.table1->setColumnWidth(1,80);
    this-> t2 = new QStandardItemModel();
    t2->setHorizontalHeaderLabels({ "      MAC addr      ","port" });
    ui.table2->setModel(t2);
    ui.table2->setColumnWidth(0,220);
    ui.table2->setColumnWidth(1,80);
    // 机器在哪个LAN上
    this->mac_lan["A"] = 1;
    this->mac_lan["B"] = 1;
    this->mac_lan["C"] = 2;
    this->mac_lan["D"] = 4;
    this->mac_lan["E"] = 3;
    this->mac_lan["F"] = 3;
    // 每个机器的完整mac
    this->mac_tot["A"] = "E8:6A:64:F7:DD:0A";
    this->mac_tot["B"] = "E8:6A:64:F7:DD:0B";
    this->mac_tot["C"] = "E8:6A:64:F7:DD:0C";
    this->mac_tot["D"] = "88:40:3B:A1:C8:0D";
    this->mac_tot["E"] = "88:40:3B:A1:C8:0E";
    this->mac_tot["F"] = "88:40:3B:A1:C8:0F";
    // 在ui种展示机器对应mac
    QStandardItemModel* tm= new QStandardItemModel();
    tm->setHorizontalHeaderLabels({ "机器","完整的mac addr" });
    for(int i=0;i<='F'-'A';i++){
        string machine="A";
        machine[0]+=i;
        tm->setItem(i, 1, new QStandardItem(QString::fromStdString(this->mac_tot[machine])));
        tm->setItem(i, 0, new QStandardItem(QString('A'+i)));
    }
    ui.maclist->setModel(tm);
    ui.maclist->setColumnWidth(0,55);
    ui.maclist->setColumnWidth(1,145);
    // 每个LAN的两端对应哪个bridge，以及连接的port
    this->lan[1].bridge1 = 1;
    this->lan[1].port1 = 1;

    this->lan[2].bridge1 = 1;
    this->lan[2].port1 = 2;
    this->lan[2].bridge2 = 2;
    this->lan[2].port2 = 1;

    this->lan[3].bridge1 = 2;
    this->lan[3].port1 = 2;
    this->lan[4].bridge1 = 2;
    this->lan[4].port1 = 3;
    // bridge对应的端口数，端口连接的lan是哪个
    this->bridgeinfo[1].ports_num = 2;
    this->bridgeinfo[1].ports_lan[1] = 1;
    this->bridgeinfo[1].ports_lan[2] = 2;

    this->bridgeinfo[2].ports_num = 3;
    this->bridgeinfo[2].ports_lan[1] = 2;
    this->bridgeinfo[2].ports_lan[2] = 3;
    this->bridgeinfo[2].ports_lan[3] = 4;

    connect(ui.OK, SIGNAL(clicked()), this, SLOT(send()));
    connect(ui.RESET, SIGNAL(clicked()), this, SLOT(restart()));
    initDatagram();
    initialPos();

    QPixmap pix1(":/img/images/machine.png");
    QPixmap pix2(":/img/images/machine2.png");
    QPixmap pix3(":/img/images/machine3.png");
    QPixmap pix4(":/img/images/Bridge1.png");
    QPixmap pix5(":/img/images/Bridge2.png");
    ui.A->setPixmap(pix1.scaled(80, 80, Qt::KeepAspectRatio));
    ui.B->setPixmap(pix1.scaled(80, 80, Qt::KeepAspectRatio));
    ui.C->setPixmap(pix2.scaled(80, 80, Qt::KeepAspectRatio));
    ui.D->setPixmap(pix2.scaled(80, 80, Qt::KeepAspectRatio));
    ui.E->setPixmap(pix3.scaled(80, 80, Qt::KeepAspectRatio));
    ui.F->setPixmap(pix3.scaled(80, 80, Qt::KeepAspectRatio));
    ui.B1->setPixmap(pix4.scaled(120, 120, Qt::KeepAspectRatio));
    ui.B2->setPixmap(pix5.scaled(120, 120, Qt::KeepAspectRatio));
}

void QtBridge::initialPos() {
    this->bpos[1].center.x = ui.B1->x() + ui.B1->width() / 2 - 25;
    this->bpos[1].center.y = ui.B1->y() + ui.B1->height() / 2 - 25;
    this->bpos[2].center.x = ui.line_up->x();
    this->bpos[2].center.y = ui.B2->y() + ui.B2->height() / 2 - 25;
    this->bpos[2].up.x = ui.line_up->x();
    this->bpos[2].up.y = ui.line_up->y() - 25;
    this->bpos[2].down.x = ui.line_down->x();
    this->bpos[2].down.y = ui.line_F->y() - 25;
    this->mpos["A"].up.x = ui.line_A->x();
    this->mpos["A"].up.y = ui.A->y();
    this->mpos["A"].down.x = ui.line_A->x();
    this->mpos["A"].down.y = ui.B1->y() + ui.B1->height() / 2 - 25;
    this->mpos["B"].up.x = ui.line_B->x();
    this->mpos["B"].up.y = ui.B->y();
    this->mpos["B"].down.x = ui.line_B->x();
    this->mpos["B"].down.y = ui.B1->y() + ui.B1->height() / 2 - 25;
    this->mpos["C"].up.x = ui.line_C->x();
    this->mpos["C"].up.y = ui.C->y();
    this->mpos["C"].down.x = ui.line_C->x();
    this->mpos["C"].down.y = ui.B1->y() + ui.B1->height() / 2 - 25;
    this->mpos["D"].up.x = ui.line_D->x();
    this->mpos["D"].up.y = ui.D->y();
    this->mpos["D"].down.x = ui.line_D->x();
    this->mpos["D"].down.y = ui.line_up->y() - 25;
    this->mpos["E"].up.x = ui.line_E->x();
    this->mpos["E"].up.y = ui.E->y();
    this->mpos["E"].down.x = ui.line_E->x();
    this->mpos["E"].down.y = ui.line_F->y() - 25;
    this->mpos["F"].up.x = ui.line_F->x();
    this->mpos["F"].up.y = ui.F->y();
    this->mpos["F"].down.x = ui.line_F->x();
    this->mpos["F"].down.y = ui.line_F->y() - 25;
}



void QtBridge::update_table(int table,string mac_addr,int portno) {
    string mac=mac_addr+"("+mac_tot[mac_addr]+")";
    if (table == 1) {
        int row=bridgeinfo[table].bridge_map.size()-1;
        t1->setItem(row, 1, new QStandardItem(QString('0'+portno)));
        t1->setItem(row, 0, new QStandardItem(QString::fromStdString( mac)));
        ui.table1->setModel(t1);
    }
    if (table == 2) {
        int row=bridgeinfo[table].bridge_map.size()-1;
        t2->setItem(row, 1, new QStandardItem(QString('0'+portno)));
        t2->setItem(row, 0, new QStandardItem(QString::fromStdString( mac)));
        ui.table2->setModel(t2);
    }
}

void QtBridge::restart()
{
    // 清空表格
    this->t1 = new QStandardItemModel();
    t1->setHorizontalHeaderLabels({ "MAC addr","port" });
    ui.table1->setModel(t1);
    this->t2 = new QStandardItemModel();
    t2->setHorizontalHeaderLabels({ "MAC addr","port" });
    ui.table2->setModel(t2);
    ui.pathshow->clear();
    ui.stepshow->clear();
    ui.input1->clear();
    ui.input2->clear();
    bridgeinfo[1].bridge_map.clear();
    bridgeinfo[2].bridge_map.clear();
}

void QtBridge::on_horizontalSlider_valueChanged(int value)
{
    speed = value;
}

void QtBridge::on_pushButton_clicked()
{
    QString helpmsg = " 透明网桥的工作原理：\n";
    helpmsg += "1. 网桥内部使用一张转发表，表中记录各个目的地址在网桥的哪个端口上;网桥用帧的目的地址查找转发表，若目的地址所在端口与帧的输入端口相同，丢弃帧；\n";
    helpmsg += "2. 转发表初始为空，网桥通过逆向学习法获知各个目的地址所在端口，逐步建立转发表；\n";
    helpmsg += "3. 逆向学习：网桥通过检查帧的源地址及输入端口来发现网络中的节点及所在的端口；\n";
    helpmsg += "4. 对于每个发向未知目的地址对应端口的帧，使用泛洪算法(flooding algorithm)转发到除源端口外的所有端口，学习成功后则不再泛洪而只转发到正确端口；\n";
    helpmsg += "\n本程序的用户使用说明：\n";
    helpmsg += "输入与图片中对应的发送端和接收端(A~F,大写,或完整mac地址)，点击发送，将演示发送的动画：报文到达某个机器后若变为灰色，则代表不接收，若为红色，代表接收。\n";
    helpmsg += "程序输出发送路径，左下角输出学习和转发的全过程，右下角两个表分别是Bridge1的转发表和Bridge2的转发表。\n";
    helpmsg += "同时提供滑块来控制动画的速度。\n";
    helpmsg += "点击右上角“重置”将清空重置。\n";
    QMessageBox::information(this, "帮助", helpmsg);
}
