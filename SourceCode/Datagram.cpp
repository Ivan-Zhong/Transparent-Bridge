#include "QtBridge.h"
#include <QPropertyAnimation>

void QtBridge::initDatagram()
{
    speed = 5;
    for(int i = 0; i < 12; ++i)
    {
        datagrams[i] = new QLabel(ui.centralWidget);
        QPixmap pix(":/img/images/blue.png");
        datagrams[i]->resize(50, 50);
        datagrams[i]->setWindowFlags(Qt::WindowStaysOnTopHint);
        datagrams[i]->setGeometry(0, 0, 50, 50);
        datagrams[i]->setPixmap(pix.scaled(50, 50, Qt::KeepAspectRatio));
        datagrams[i]->setVisible(false);
    }
    memset(isFork, false, sizeof(isFork));
}

int QtBridge::getDatagram(const struct pos &startPos)
{
    int curId = 0;
    for(; curId < 12; ++curId)
    {
        if(datagrams[curId]->x() == 0 && datagrams[curId]->y() == 0)
            break;
    }
    datagrams[curId]->setGeometry(startPos.x, startPos.y, 50, 50);
    datagrams[curId]->setVisible(true);
    return curId;
}

void QtBridge::moveDatagram(int id, const struct pos &targetPos)
{
    animationTask task;
    task.type = moveD;
    task.targetpos = targetPos;
    if(tasks[id].isEmpty() && !isFork[id]) // 空队列，直接加入到末尾，并直接开始执行
    {
        tasks[id].push_back(task);
        executeTask(id);
    }
    else // 非空队列
    {
        tasks[id].push_back(task);
    }
}

void QtBridge::hideDatagram(int id)
{
    animationTask task;
    task.type = hideD;
    task.hideId = id;
    tasks[id].push_back(task);
}

int QtBridge::forkDatagram(int id)
{
    int forkId = 0;
    for(; forkId < 12; ++forkId)
    {
        if(datagrams[forkId]->x() == 0 && datagrams[forkId]->y() == 0 && !isFork[forkId])
            break;
    }
    isFork[forkId] = true;
    //qDebug()<<"forkId: "<<forkId<<endl;
    animationTask task;
    task.type = forkD;
    task.forkId = forkId;
    tasks[id].push_back(task);
    return forkId;
}

void QtBridge::closeDatagram(int id, bool reached)
{
    animationTask task;
    task.type = closeD;
    task.reached = reached;
    if(tasks[id].isEmpty() && !isFork[id]) // 空队列，直接加入到末尾，并直接开始执行
    {
        tasks[id].push_back(task);
        executeTask(id);
    }
    else // 非空队列
    {
        tasks[id].push_back(task);
    }
}

void QtBridge::executeTask(int id)
{
    if(!tasks[id].isEmpty())
    {
        animationTask task = tasks[id].front();
        int misec;
        if(task.type == moveD)
        {
            QPropertyAnimation* animation = new QPropertyAnimation(datagrams[id], "geometry");
            int curx = datagrams[id]->x();
            int cury = datagrams[id]->y();
            misec = speed * (abs(task.targetpos.x - curx) + abs(task.targetpos.y - cury));
            animation->setDuration(misec);
            animation->setStartValue(QRect(curx, cury, 50, 50));
            animation->setEndValue(QRect(task.targetpos.x, task.targetpos.y, 50, 50));
            animation->start();
        }
        else if(task.type == closeD)
        {
            if(task.reached)
            {
                QPixmap pix(":/img/images/red.png");
                datagrams[id]->setPixmap(pix.scaled(50, 50, Qt::KeepAspectRatio));
                misec = 5000;
            }
            else
            {
                QPixmap pix(":/img/images/gray.png");
                datagrams[id]->setPixmap(pix.scaled(50, 50, Qt::KeepAspectRatio));
                misec = 1000;
            }
        }
        else if(task.type == forkD)
        {
            datagrams[task.forkId]->setGeometry(datagrams[id]->x(), datagrams[id]->y(), 50, 50);
            datagrams[task.forkId]->setVisible(true);
            executeTask(task.forkId);
            misec = 10;
        }
        else if(task.type == hideD)
        {
            datagrams[id]->setVisible(false);
            misec = 10;
        }
        switch(id)
        {
        case 0:
            QTimer::singleShot(misec, this, SLOT(on_timer0_timeout()));
            break;
        case 1:
            QTimer::singleShot(misec, this, SLOT(on_timer1_timeout()));
            break;
        case 2:
            QTimer::singleShot(misec, this, SLOT(on_timer2_timeout()));
            break;
        case 3:
            QTimer::singleShot(misec, this, SLOT(on_timer3_timeout()));
            break;
        case 4:
            QTimer::singleShot(misec, this, SLOT(on_timer4_timeout()));
            break;
        case 5:
            QTimer::singleShot(misec, this, SLOT(on_timer5_timeout()));
            break;
        case 6:
            QTimer::singleShot(misec, this, SLOT(on_timer6_timeout()));
            break;
        case 7:
            QTimer::singleShot(misec, this, SLOT(on_timer7_timeout()));
            break;
        case 8:
            QTimer::singleShot(misec, this, SLOT(on_timer8_timeout()));
            break;
        case 9:
            QTimer::singleShot(misec, this, SLOT(on_timer9_timeout()));
            break;
        case 10:
            QTimer::singleShot(misec, this, SLOT(on_timer10_timeout()));
            break;
        case 11:
            QTimer::singleShot(misec, this, SLOT(on_timer11_timeout()));
            break;
        }
    }
}

void QtBridge::on_timer0_timeout()
{
    animationTask task = tasks[0].front();
    if(task.type == closeD)
    {
        QPixmap pix(":/img/images/blue.png");
        datagrams[0]->setPixmap(pix.scaled(50, 50, Qt::KeepAspectRatio));
        datagrams[0]->setVisible(false);
        datagrams[0]->setGeometry(0, 0, 50, 50);
        isFork[0] = false;
    }
    tasks[0].pop_front();
    if(!tasks[0].isEmpty())
    {
        executeTask(0);
    }
}

void QtBridge::on_timer1_timeout()
{
    animationTask task = tasks[1].front();
    if(task.type == closeD)
    {
        QPixmap pix(":/img/images/blue.png");
        datagrams[1]->setPixmap(pix.scaled(50, 50, Qt::KeepAspectRatio));
        datagrams[1]->setVisible(false);
        datagrams[1]->setGeometry(0, 0, 50, 50);
        isFork[1] = false;
    }
    tasks[1].pop_front();
    if(!tasks[1].isEmpty())
    {
        executeTask(1);
    }
}

void QtBridge::on_timer2_timeout()
{
    animationTask task = tasks[2].front();
    if(task.type == closeD)
    {
        QPixmap pix(":/img/images/blue.png");
        datagrams[2]->setPixmap(pix.scaled(50, 50, Qt::KeepAspectRatio));
        datagrams[2]->setVisible(false);
        datagrams[2]->setGeometry(0, 0, 50, 50);
        isFork[2] = false;
    }
    tasks[2].pop_front();
    if(!tasks[2].isEmpty())
    {
        executeTask(2);
    }
}

void QtBridge::on_timer3_timeout()
{
    animationTask task = tasks[3].front();
    if(task.type == closeD)
    {
        QPixmap pix(":/img/images/blue.png");
        datagrams[3]->setPixmap(pix.scaled(50, 50, Qt::KeepAspectRatio));
        datagrams[3]->setVisible(false);
        datagrams[3]->setGeometry(0, 0, 50, 50);
        isFork[3] = false;
    }
    tasks[3].pop_front();
    if(!tasks[3].isEmpty())
    {
        executeTask(3);
    }
}

void QtBridge::on_timer4_timeout()
{
    animationTask task = tasks[4].front();
    if(task.type == closeD)
    {
        QPixmap pix(":/img/images/blue.png");
        datagrams[4]->setPixmap(pix.scaled(50, 50, Qt::KeepAspectRatio));
        datagrams[4]->setVisible(false);
        datagrams[4]->setGeometry(0, 0, 50, 50);
        isFork[4] = false;
    }
    tasks[4].pop_front();
    if(!tasks[4].isEmpty())
    {
        executeTask(4);
    }
}

void QtBridge::on_timer5_timeout()
{
    animationTask task = tasks[5].front();
    if(task.type == closeD)
    {
        QPixmap pix(":/img/images/blue.png");
        datagrams[5]->setPixmap(pix.scaled(50, 50, Qt::KeepAspectRatio));
        datagrams[5]->setVisible(false);
        datagrams[5]->setGeometry(0, 0, 50, 50);
        isFork[5] = false;
    }
    tasks[5].pop_front();
    if(!tasks[5].isEmpty())
    {
        executeTask(5);
    }
}

void QtBridge::on_timer6_timeout()
{
    animationTask task = tasks[6].front();
    if(task.type == closeD)
    {
        QPixmap pix(":/img/images/blue.png");
        datagrams[6]->setPixmap(pix.scaled(50, 50, Qt::KeepAspectRatio));
        datagrams[6]->setVisible(false);
        datagrams[6]->setGeometry(0, 0, 50, 50);
        isFork[6] = false;
    }
    tasks[6].pop_front();
    if(!tasks[6].isEmpty())
    {
        executeTask(6);
    }
}

void QtBridge::on_timer7_timeout()
{
    animationTask task = tasks[7].front();
    if(task.type == closeD)
    {
        QPixmap pix(":/img/images/blue.png");
        datagrams[7]->setPixmap(pix.scaled(50, 50, Qt::KeepAspectRatio));
        datagrams[7]->setVisible(false);
        datagrams[7]->setGeometry(0, 0, 50, 50);
        isFork[7] = false;
    }
    tasks[7].pop_front();
    if(!tasks[7].isEmpty())
    {
        executeTask(7);
    }
}

void QtBridge::on_timer8_timeout()
{
    animationTask task = tasks[8].front();
    if(task.type == closeD)
    {
        QPixmap pix(":/img/images/blue.png");
        datagrams[8]->setPixmap(pix.scaled(50, 50, Qt::KeepAspectRatio));
        datagrams[8]->setVisible(false);
        datagrams[8]->setGeometry(0, 0, 50, 50);
        isFork[8] = false;
    }
    tasks[8].pop_front();
    if(!tasks[8].isEmpty())
    {
        executeTask(8);
    }
}

void QtBridge::on_timer9_timeout()
{
    animationTask task = tasks[9].front();
    if(task.type == closeD)
    {
        QPixmap pix(":/img/images/blue.png");
        datagrams[9]->setPixmap(pix.scaled(50, 50, Qt::KeepAspectRatio));
        datagrams[9]->setVisible(false);
        datagrams[9]->setGeometry(0, 0, 50, 50);
        isFork[9] = false;
    }
    tasks[9].pop_front();
    if(!tasks[9].isEmpty())
    {
        executeTask(9);
    }
}

void QtBridge::on_timer10_timeout()
{
    animationTask task = tasks[10].front();
    if(task.type == closeD)
    {
        QPixmap pix(":/img/images/blue.png");
        datagrams[10]->setPixmap(pix.scaled(50, 50, Qt::KeepAspectRatio));
        datagrams[10]->setVisible(false);
        datagrams[10]->setGeometry(0, 0, 50, 50);
        isFork[10] = false;
    }
    tasks[10].pop_front();
    if(!tasks[10].isEmpty())
    {
        executeTask(10);
    }
}

void QtBridge::on_timer11_timeout()
{
    animationTask task = tasks[11].front();
    if(task.type == closeD)
    {
        QPixmap pix(":/img/images/blue.png");
        datagrams[11]->setPixmap(pix.scaled(50, 50, Qt::KeepAspectRatio));
        datagrams[11]->setVisible(false);
        datagrams[11]->setGeometry(0, 0, 50, 50);
        isFork[11] = false;
    }
    tasks[11].pop_front();
    if(!tasks[11].isEmpty())
    {
        executeTask(11);
    }
}
