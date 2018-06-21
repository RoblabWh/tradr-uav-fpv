#ifndef ROSTHREAD_H
#define ROSTHREAD_H

#include <ros/ros.h>

#include <QThread>

using namespace ros;

class RosThread : public QThread
{
    Q_OBJECT

public:
    RosThread();

private:
    void run();
};

#endif // ROSTHREAD_H
