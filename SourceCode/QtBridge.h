#ifndef QTBRIDGE_H
#define QTBRIDGE_H
#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtBridge.h"
#include <map>
#include <QStandardItemModel>
#include<QTimer>
#include<QQueue>
#include<QMap>
#define MAXPORT 10
// 每个LAN最多连接两个bridge，分别对应一个端口
// 如果起点是其中一个端口，那么终点就是另一个
// 如果起点是LAN上的机器，就两边都发
struct LAN {
    int port1=-1;
    int bridge1=-1;
    int port2=-1;
    int bridge2=-1;
};
struct Bridgeinfo {
    int ports_num = 0;
    int ports_lan[MAXPORT]; // ports号对应一个LAN号
    // 储存table中的内容，用mac addr索引port
    std::map <std::string, int> bridge_map;
};

struct pos {
    int x;
    int y;
};
struct machine_pos {
    pos up;
    pos down;
};
struct bridge_pos {
    pos center;
    pos up;
    pos down;
};

enum animationType
{
    moveD, closeD, forkD, hideD
};

struct animationTask
{
    animationType type;
    int hideId;
    int forkId;
    bool reached;
    pos targetpos;
};

class QtBridge : public QMainWindow
{
    Q_OBJECT

public:
    QtBridge(QWidget *parent = Q_NULLPTR);
signals:;	// 自定义信号
public slots:
    void send();   // 自定义槽函数，在.cpp中实现
    void restart();
private:
    Ui::QtBridgeClass ui;
    Bridgeinfo bridgeinfo[3];
    // 图案位置
    bridge_pos bpos[3];
    std::map<std::string, machine_pos>mpos;
    std::map<std::string,std::string>mac_tot;// 通过ABC等索引完整的mac地址
    std::map <std::string,int> mac_lan; // 机器在哪个lan上
    LAN lan[5];
    std::vector<std::string>path_output; // 储存打印路径用的
    QStandardItemModel* t1; // 用来设置表的内容
    QStandardItemModel* t2;
    void update_table(int table,std::string mac_addr, int port);
    // 从lan发送到bridge，并展示动画和更新发送表
    bool lan_to_bridge(std::string begin_addr,std::string end_addr,int lan_no,int data_no=0,int port_choose=0,int inundate=0);
    void initialPos();
    void initDatagram();
    int getDatagram(struct pos const& startPos);
    void moveDatagram(int id, struct pos const& targetPos);
    void hideDatagram(int id);
    int forkDatagram(int id);
    void closeDatagram(int id, bool reached);
    void executeTask(int id);
    QLabel* datagrams[12];
    QQueue<animationTask> tasks[24];
    bool isFork[12];
    int speed;

private slots:
    void on_timer0_timeout();
    void on_timer1_timeout();
    void on_timer2_timeout();
    void on_timer3_timeout();
    void on_timer4_timeout();
    void on_timer5_timeout();
    void on_timer6_timeout();
    void on_timer7_timeout();
    void on_timer8_timeout();
    void on_timer9_timeout();
    void on_timer10_timeout();
    void on_timer11_timeout();

//private slots:
//    void on_t_timeout();
    void on_horizontalSlider_valueChanged(int value);
    void on_pushButton_clicked();
};

#endif // QTBRIDGE_H
