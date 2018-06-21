#ifndef UAVMANAGER_H
#define UAVMANAGER_H

#include <ros/ros.h>

#include <bridge_path_interface_msgs/RegistrationNotification.h>
#include <bridge_path_interface_msgs/RegistrationList.h>
#include <bridge_path_interface_msgs/GetRegistrationList.h>

#include <QString>
#include <QObject>

#include <list>
#include <iostream>

#include "src/uav.h"

using namespace std;
using namespace ros;
using namespace bridge_path_interface_msgs;

class UavManager : public QObject
{
    Q_OBJECT

private:
    list<Uav*> uavList;

    NodeHandle* nodeHandle;
    Subscriber uavRegistrationNotificationSubscriber;
    ServiceClient getRegistrationListServiceClient;


signals:
    void uavRegistered(unsigned int index, Uav* uav);
    void uavUnregistered(unsigned int index, Uav* uav);
    void uavReseted();

public:
    UavManager();

    void update();

private:
    void on_uavRegistrationNotificationSubscriber_messageReceived(const RegistrationNotificationConstPtr& notification);

private:
    void insertUav(unsigned int uavID);
    void deleteUav(unsigned int uavID);
    void reset();
};

#endif // UAVMANAGER_H
