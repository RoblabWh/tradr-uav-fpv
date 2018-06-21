#include "uavmanager.h"

UavManager::UavManager()
{
    this->nodeHandle = new NodeHandle();

    this->uavRegistrationNotificationSubscriber = this->nodeHandle->subscribe("/uav_registration_notification", 1, &UavManager::on_uavRegistrationNotificationSubscriber_messageReceived, this);

    this->getRegistrationListServiceClient = this->nodeHandle->serviceClient<GetRegistrationList>("getRegistrationList");
}

void UavManager::update()
{
    this->reset();

    bridge_path_interface_msgs::GetRegistrationListRequest req;
    bridge_path_interface_msgs::GetRegistrationListResponse rsp;
    this->getRegistrationListServiceClient.call(req, rsp);

    for (unsigned int uavID : rsp.registrationList.id)
    {
        this->insertUav(uavID);
    }
}

void UavManager::on_uavRegistrationNotificationSubscriber_messageReceived(const RegistrationNotificationConstPtr& notification)
{
    switch (notification->type)
    {
    case RegistrationNotification::TYPE_RESET:
        this->reset();
        break;
    case RegistrationNotification::TYPE_REGISTRATION:
        this->insertUav(notification->id);
        break;
    case RegistrationNotification::TYPE_UNREGISTRATION:
        this->deleteUav(notification->id);
        break;
    }
}

void UavManager::insertUav(unsigned int uavID)
{
    for (Uav* uav : this->uavList)
    {
        if (uav->getID() == uavID)
        {
            return;
        }
    }

    Uav* uav = new Uav(this->nodeHandle, uavID);

    unsigned int index = this->uavList.size();
    this->uavList.push_back(uav);

    emit this->uavRegistered(index, uav);
}

void UavManager::deleteUav(unsigned int uavID)
{    
    unsigned int i = 0;
    for (Uav* uav : this->uavList)
    {
        if (uav->getID() == uavID)
        {
            emit this->uavUnregistered(i, uav);
            this->uavList.remove(uav);
            delete uav;
            break;
        }
        i++;
    }
}

void UavManager::reset()
{
    emit this->uavReseted();

    for (Uav* uav : this->uavList)
    {
        delete uav;
    }

    this->uavList.clear();
}


