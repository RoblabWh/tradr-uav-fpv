#ifndef UAV_H
#define UAV_H

#include <QObject>

#include <QThread>

#include <ros/ros.h>

#include <bridge_path_interface_msgs/GetUavIdent.h>
#include <bridge_path_interface_msgs/UavIdent.h>
#include <bridge_path_interface_msgs/GetUavState.h>
#include <bridge_path_interface_msgs/UavState.h>

#include <uav_videostream_msgs/VideostreamStartReq.h>
#include <uav_videostream_msgs/VideostreamStopReq.h>
#include <uav_videostream_msgs/VideostreamData.h>
#include <uav_videostream_msgs/VideostreamStartRsp.h>

#include <string>

#include <thread>

class Uav;

#include "videodecoder.h"

using namespace ros;
using namespace std;
using namespace bridge_path_interface_msgs;
using namespace uav_videostream_msgs;

class Uav : public QObject
{
    Q_OBJECT

public:
    enum MainState
    {
        MAINSTATE_INIT,
        MAINSTATE_WAIT_FOR_REQUEST,
        MAINSTATE_REGISTERED
    };

    enum StatusState
    {
        STATUSSTATE_INIT,
        STATUSSTATE_UAV_MONITORING
    };

    enum StreamState
    {
        STREAMSTATE_INIT,
        STREAMSTATE_RECEIVING_VIDEO_DATA_OFF,
        STREAMSTATE_WAIT_FOR_RESPONSE,
        STREAMSTATE_RECEIVING_VIDEO_DATA_ON
    };

    enum TaskState
    {
        TASKSTATE_INIT,
        TASKSTATE_WAIT_FOR_TASK,
        TASKSTATE_WAIT_FOR_RESPONSE,
        TASKSTATE_EXECUTION
    };

private:
    NodeHandle* nodeHandle;

    ServiceClient getUavIdentServiceClient;
    ServiceClient getUavStateServiceClient;

    Subscriber stateChangedNotificationSubscriber;

    Publisher startVideoStreamReqPublisher;
    Subscriber startVideoStreamRspSubscriber;

    Publisher stopVideoStreamReqPublisher;

    Subscriber videoStreamDataSubscriber;

    unsigned int id;
    string topicNamespace;
    QString name;
    QString model;

    MainState mainState;
    StatusState statusState;
    StreamState streamState;
    TaskState taskState;

    VideoDecoder* videoDecoder;
    boost::mutex videoDecoderLock;

    long number;

signals:
    void frameReceived(uint8_t* frame, uint width, uint height, uint bytesPerLine);

    void stateChanged(MainState mainState, StatusState statusState, StreamState streamState, TaskState taskState);

    void videostreamStateChanged(bool isLiveStreamOn);

public:
    Uav(NodeHandle* nodeHandle, unsigned int uavID);
    ~Uav();

    unsigned int getID();
    QString getName();
    QString getModel();

    MainState getMainState();
    StatusState getStatusState();
    StreamState getStreamState();
    TaskState getTaskState();

    void startLiveStream();
    void stopLiveStream();

    bool isLiveStreamOn();

    void on_stateChangedNotificationSubscriber_messageReceived(const UavStateConstPtr& msg);

    void on_videoStreamDataSubscriber_messageReceived(const VideostreamDataConstPtr& data);
    void on_videoStreamStartRspSubscriber_messageReceived(const VideostreamStartRspConstPtr& startRsp);

public slots:
    void on_videoDecoder_frameDecoded(uint8_t* frame, uint width, uint height, uint bytesPerLine);
};

#endif // UAV_H
