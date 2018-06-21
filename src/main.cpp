#include <ros/ros.h>

#include <iostream>

#include <QApplication>

#include "fpvwindow.h"

#include "rosthread.h"

int main(int argc, char** argv)
{
    stringstream ss;
    ss << "uav_fpv_" << getpid();
    string nodeName = ss.str();

    QApplication app(argc, argv);
    ros::init(argc, argv, nodeName.c_str());

    FPVWindow window;
    window.show();

    RosThread rosThread;
    rosThread.start();

    app.exec();

    rosThread.terminate();

    return 0;
}
