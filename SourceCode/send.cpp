#include "QtBridge.h"
#include "fun.h"

void QtBridge::send()
{
    ui.pathshow->clear();
    ui.stepshow->clear();
    QString begin = ui.input1->toPlainText();
    std::string begin_addr0 = begin.toLatin1().data();
    QString end = ui.input2->toPlainText();
    std::string end_addr0 = end.toLatin1().data();
    std::string begin_addr,end_addr;
    if (!check_addr(begin_addr0,begin_addr,mac_tot) || !check_addr(end_addr0,end_addr,mac_tot)) {
        ui.pathshow->append("发出端口或目标端口输入有误，请重新输入");
        return;
    }
    int datano = getDatagram(mpos[begin_addr].up);
    moveDatagram(datano, mpos[begin_addr].down);
    // 开始发送
    lan_to_bridge(begin_addr, end_addr, mac_lan[begin_addr],datano, 0);
    // 打印路径
    QString path = QString::fromStdString(begin_addr+ "("+mac_tot[begin_addr]+")" + " -> ");
    ui.pathshow->append(path);
    for (int i = this->path_output.size() - 1; i >= 0; i--) {
        QString path = QString::fromStdString(this->path_output[i]);
        ui.pathshow->append(path);
    }
    path = QString::fromStdString(end_addr+"("+mac_tot[end_addr]+")");
    ui.pathshow->append(path);
    path_output.clear();
}

// port_choose代表选择lan两边的哪个端口发送，如果为0的话代表两边都发
// 返回值代表往这条路能不能发送到目标地址处 能返回1
bool QtBridge::lan_to_bridge(string begin_addr, string end_addr, int lan_no,int data_no,int port_choose,int inundate) {
    // qDebug()<<data_no;
    // 记录一下数据包的初始位置
    bool flag=false;
    if (((port_choose==0||port_choose==1)&&lan[lan_no].bridge1 != -1)==0&&((port_choose == 0 || port_choose == 2) && lan[lan_no].bridge2 != -1)==0)
        hideDatagram(data_no);
    // 记录有没有新的包要分裂出来
    int count = 0;
        for (int i = 0; i <= 'F' - 'A'; i++) {
             string addr = "A";
             addr[0] += i;
             // 如果在这条线上
             if (addr!=begin_addr&&mac_lan[addr] == lan_no) {
                 int endflag = 0;
                 if (addr == end_addr){
                     endflag = 1;
                     flag=true;
                 }

                     int no=forkDatagram(data_no);
                     moveDatagram(no, mpos[addr].down);
                     moveDatagram(no, mpos[addr].up);
                     closeDatagram(no, endflag);
                 count++;
             }
        }

    // 如果目标地址就在这个lan上
    string lanstr="0";
    lanstr[0]+=lan_no;
    string choosestr="0";
    choosestr[0]+=port_choose;
    string output="数据包经过LAN"+lanstr;//+"port choose = "+choosestr;
    QString path = QString::fromStdString(output);
    ui.stepshow->append(path);
    int data_no2=data_no;    // 一端
    if ((port_choose==0||port_choose==1)&&lan[lan_no].bridge1 != -1) {
        // 如果另一端也要的话就先fork一个出来
        if ((port_choose == 0 || port_choose == 2) && lan[lan_no].bridge2 != -1) {
            data_no2=forkDatagram(data_no);
            // datagrams[data_no2]->setVisible(false);
        }
        int bridge = lan[lan_no].bridge1;
        // 更新网桥1中的转发表,如果发送端没有被记录的话
        if (bridgeinfo[bridge].bridge_map.find(begin_addr) == bridgeinfo[bridge].bridge_map.end()) {
            bridgeinfo[bridge].bridge_map[begin_addr] = lan[lan_no].port1;
            update_table(bridge, begin_addr, lan[lan_no].port1);
            string bridgestr="0";
            bridgestr[0]+=bridge;
            string output="发送端"+begin_addr+"("+mac_tot[begin_addr]+")"+"不在bridge"+bridgestr+"的转发表上，更新bridge"+bridgestr;
            QString path = QString::fromStdString(output);
            ui.stepshow->append(path);
        }

        // 移动数据包到桥上
        int datano_bridge = data_no; // 到桥的包的号
        if (lan_no != 3 && lan_no != 4) {
                moveDatagram(data_no, bpos[bridge].center);
        }
        else {// 拐弯
            struct pos pos1 = bpos[bridge].center;
            if (lan_no == 4)
                pos1 = bpos[bridge].up;
            else
                pos1 = bpos[bridge].down;
                moveDatagram(data_no, pos1);
                moveDatagram(data_no, bpos[bridge].center);
        }
        count++;
        // 如果目标地址在源端口，不做别的处理
        if (bridgeinfo[bridge].bridge_map.find(end_addr) != bridgeinfo[bridge].bridge_map.end()&&bridgeinfo[bridge].bridge_map[end_addr] == lan[lan_no].port1)
        {
            closeDatagram(datano_bridge,false);
            // 查看目标地址是不是不需要泛洪，并且到了，那么返回
            if (mac_lan[end_addr]==lan_no){
                QString path= QString::fromStdString("到达目标地址,成功接收");
                ui.stepshow->append(path);
                return true;
            }
        }
        else{
            // 如果目标地址不在转发表里,泛洪
            if (bridgeinfo[bridge].bridge_map.find(end_addr) == bridgeinfo[bridge].bridge_map.end()) {
                int subcount = 0;// 计数，如果为0九用datano_bridge
                string bridgestr="0";
                bridgestr[0]+=bridge;
                string portstr="0";
                portstr[0]+=lan[lan_no].port1;
                string output = "接收端"+end_addr+"("+mac_tot[end_addr]+")"+"不在bridge"+bridgestr+"的转发表上，向除了port"+portstr+"以外的端口泛洪";
                QString path = QString::fromStdString(output);
                ui.stepshow->append(path);
                hideDatagram(datano_bridge);
                for (int i = 1; i <= bridgeinfo[bridge].ports_num; i++) {
                    int sub_datano = datano_bridge;
                    if (i != lan[lan_no].port1) { // 发送到所有非源端口
                        // datagrams[datano_bridge]->setVisible(true);
                        string output = "泛洪:从bridge0的port0发出报文,目标mac地址为"+end_addr;
                        output[16]+=bridge;
                        output[24]+=i;
                        QString path = QString::fromStdString(output);
                        ui.stepshow->append(path);
                        int next_lan_port = 0;
                        // 如果这个bridge相对要发送的lan是顶点1，则要发到顶点2
                        if (lan[bridgeinfo[bridge].ports_lan[i]].bridge1 == bridge)
                            next_lan_port = 2;
                        else if (lan[bridgeinfo[bridge].ports_lan[i]].bridge2 == bridge)
                            next_lan_port = 1;
                        // 如果是bridge2出来要拐弯的，在这里拐弯一下
                            sub_datano = forkDatagram(datano_bridge);
                        subcount++;
                        if (bridgeinfo[bridge].ports_lan[i] == 3 || bridgeinfo[bridge].ports_lan[i] == 4) {
                            struct pos pos1 = bpos[bridge].center;
                            if (bridgeinfo[bridge].ports_lan[i] == 4)
                                pos1 = bpos[bridge].up;
                            else
                                pos1 = bpos[bridge].down;
                            moveDatagram(sub_datano, pos1);
                        }
                        bool ret = lan_to_bridge(begin_addr, end_addr, bridgeinfo[bridge].ports_lan[i],sub_datano, next_lan_port,1);
                        // 如果目标地址在这条路上
                        if (ret) {
                            flag = true;
                            string bridgestring = "bridge0 -> port0 -> ";
                            bridgestring[6] += bridge;
                            bridgestring[15] += i;
                            path_output.push_back(bridgestring);
                        }
                    }
                }
                closeDatagram(datano_bridge,false);
            }
            // 在转发表里，发送
            else {
                int next_lan_port = 0;
                int send_port = bridgeinfo[bridge].bridge_map[end_addr];
                int send_lan = bridgeinfo[bridge].ports_lan[send_port];
                // 如果这个bridge相对要发送的lan是顶点1，则要发到顶点2
                if (lan[send_lan].bridge1 == bridge)
                    next_lan_port = 2;
                else if(lan[send_lan].bridge2 == bridge)
                    next_lan_port = 1;
                // 如果是bridge2出来到34的要拐一下
                if (send_lan == 3 || send_lan == 4) {
                    struct pos pos1 = bpos[bridge].center;
                    if (send_lan == 4)
                        pos1 = bpos[bridge].up;
                    else
                        pos1 = bpos[bridge].down;
                    moveDatagram(datano_bridge, pos1);
                }
                string bridgestr="0";
                bridgestr[0]+=bridge;
                string portstr="0";
                portstr[0]+=send_port;
                string output = "接收端"+end_addr+"("+mac_tot[end_addr]+")"+"在bridge"+bridgestr+"的转发表上，根据转发表向port"+portstr+"发送";
                QString path = QString::fromStdString(output);
                ui.stepshow->append(path);
                bool ret=lan_to_bridge(begin_addr, end_addr,send_lan ,datano_bridge, next_lan_port);
                if (ret == true) {
                    flag = true;
                    string bridgestring = "bridge0 -> port0 -> ";
                    bridgestring[6] += bridge;
                    bridgestring[15] += send_port;
                    path_output.push_back(bridgestring);
                }
            }
        }
    }
    // 另一端
    if ((port_choose == 0 || port_choose == 2) && lan[lan_no].bridge2 != -1) {
        //datagrams[data_no2]->setVisible(true);
        int bridge = lan[lan_no].bridge2;
        // 更新网桥2中的转发表
        if (bridgeinfo[bridge].bridge_map.find(begin_addr) == bridgeinfo[bridge].bridge_map.end()) {
            bridgeinfo[bridge].bridge_map[begin_addr] = lan[lan_no].port2;
            update_table(bridge, begin_addr, lan[lan_no].port2);
            string bridgestr="0";
            bridgestr[0]+=bridge;
            string output="发送端"+begin_addr+"("+mac_tot[begin_addr]+")"+"不在bridge"+bridgestr+"的转发表上，更新bridge"+bridgestr;
            QString path = QString::fromStdString(output);
            ui.stepshow->append(path);
        }

        // 移动数据包到桥上
        int datano_bridge = data_no2; // 到桥的包的号
        if (lan_no != 3 && lan_no != 4) {
                moveDatagram(data_no2, bpos[bridge].center);
        }
        else {// 拐弯
            struct pos pos1 = bpos[bridge].center;
            if (lan_no == 4)
                pos1 = bpos[bridge].up;
            else
                pos1 = bpos[bridge].down;
                moveDatagram(data_no2, pos1);
                moveDatagram(data_no2, bpos[bridge].center);
        }

        // 如果目标地址在源端口，不做别的处理
        if (bridgeinfo[bridge].bridge_map.find(end_addr) != bridgeinfo[bridge].bridge_map.end()&&bridgeinfo[bridge].bridge_map[end_addr] == lan[lan_no].port2)
        {
            closeDatagram(datano_bridge,false);
            // 查看目标地址是不是不需要泛洪，并且到了，那么返回
            if (mac_lan[end_addr]==lan_no){
                QString path= QString::fromStdString("到达目标地址，成功接收");
                ui.stepshow->append(path);
                closeDatagram(datano_bridge,false);
                return true;
            }
        }
        else{
            // 如果目标地址不在转发表里,泛洪
            if (bridgeinfo[bridge].bridge_map.find(end_addr) == bridgeinfo[bridge].bridge_map.end()) {
                string bridgestr="0";
                bridgestr[0]+=bridge;
                string portstr="0";
                portstr[0]+=lan[lan_no].port2;
                string output = "接收端"+end_addr+"("+mac_tot[end_addr]+")"+"不在bridge"+bridgestr+"的转发表上，向除了port"+portstr+"以外的端口泛洪";
                QString path = QString::fromStdString(output);
                ui.stepshow->append(path);
                int subcount = 0;// 计数，如果为0九用datano_bridge
                hideDatagram(datano_bridge);
                for (int i = 1; i <= bridgeinfo[bridge].ports_num; i++) {
                    int sub_datano = datano_bridge;
                    if (i != lan[lan_no].port2) { // 发送到所有非源端口
                        // datagrams[datano_bridge]->setVisible(true);
                        string output="泛洪:从bridge0的port0发出报文,目标mac地址为"+end_addr;
                        output[16]+=bridge;
                        output[24]+=i;
                        QString path = QString::fromStdString(output);
                        ui.stepshow->append(path);
                        int next_lan_port = 0;
                        // 如果这个bridge相对要发送的lan是顶点1，则要发到顶点2
                        if (lan[bridgeinfo[bridge].ports_lan[i]].bridge1 == bridge)
                            next_lan_port = 2;
                        else if (lan[bridgeinfo[bridge].ports_lan[i]].bridge2 == bridge)
                            next_lan_port = 1;
                        // 如果是bridge2出来要拐弯的，在这里拐弯一下
                            sub_datano = forkDatagram(datano_bridge);
                        subcount++;
                        // datagrams[datano_bridge]->setVisible(false);
                        if (bridgeinfo[bridge].ports_lan[i] == 3 || bridgeinfo[bridge].ports_lan[i] == 4) {
                            struct pos pos1 = bpos[bridge].center;
                            if (bridgeinfo[bridge].ports_lan[i] == 4)
                                pos1 = bpos[bridge].up;
                            else
                                pos1 = bpos[bridge].down;
                            moveDatagram(sub_datano, pos1);
                        }
                        bool ret = lan_to_bridge(begin_addr, end_addr, bridgeinfo[bridge].ports_lan[i], sub_datano, next_lan_port, 1);
                        // 如果目标地址在这条路上
                        if (ret) {
                            // bridgeinfo[bridge].bridge_map[end_addr] = i;
                            flag = true;
                            string bridgestring = "bridge0 -> port0 -> ";
                            bridgestring[6] += bridge;
                            bridgestring[15] += i;
                            path_output.push_back(bridgestring);
                            // update_table(bridge, end_addr, i);
                        }
                    }
                }
                closeDatagram(datano_bridge,false);
            }
            // 在转发表里，发送
            else {
                int next_lan_port = 0;
                int send_port = bridgeinfo[bridge].bridge_map[end_addr];
                int send_lan = bridgeinfo[bridge].ports_lan[send_port];
                // 如果这个bridge相对要发送的lan是顶点1，则要发到顶点2
                if (lan[send_lan].bridge1 == bridge)
                    next_lan_port = 2;
                else if (lan[send_lan].bridge2 == bridge)
                    next_lan_port = 1;
                // 如果是bridge2出来到34的要拐一下
                if (send_lan == 3 || send_lan == 4) {
                    struct pos pos1 = bpos[bridge].center;
                    if (send_lan == 4)
                        pos1 = bpos[bridge].up;
                    else
                        pos1 = bpos[bridge].down;
                    moveDatagram(datano_bridge, pos1);
                }
                string bridgestr="0";
                bridgestr[0]+=bridge;
                string portstr="0";
                portstr[0]+=send_port;
                string output = "接收端"+end_addr+"("+mac_tot[end_addr]+")"+"在bridge"+bridgestr+"的转发表上，根据转发表向port"+portstr+"发送";
                QString path = QString::fromStdString(output);
                ui.stepshow->append(path);
                bool ret = lan_to_bridge(begin_addr, end_addr, send_lan, datano_bridge,next_lan_port);
                if (ret == true) {
                    flag = true;
                    string bridgestring = "bridge0 -> port0 -> ";
                    bridgestring[6] += bridge;
                    bridgestring[15] += send_port;
                    path_output.push_back(bridgestring);
                }
            }
        }
    }
    if (((port_choose==0||port_choose==1)&&lan[lan_no].bridge1 != -1)==0&&((port_choose == 0 || port_choose == 2) && lan[lan_no].bridge2 != -1)==0)
        closeDatagram(data_no,false);
    return flag;
};
