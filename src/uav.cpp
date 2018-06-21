#include "uav.h"

#include <QtConcurrentRun>

Uav::Uav(NodeHandle* nodeHandle, unsigned int uavID)
{
    this->nodeHandle = nodeHandle;

    this->id = uavID;

    this->topicNamespace = "uav_" + std::to_string(uavID);

    this->getUavIdentServiceClient = this->nodeHandle->serviceClient<GetUavIdent>(this->topicNamespace + "/getUavIdent");
    this->getUavStateServiceClient = this->nodeHandle->serviceClient<GetUavState>(this->topicNamespace + "/getUavState");

    this->stateChangedNotificationSubscriber = this->nodeHandle->subscribe<UavState>(this->topicNamespace + "/stateChangedNotification", 1, &Uav::on_stateChangedNotificationSubscriber_messageReceived, this);

    this->startVideoStreamReqPublisher = this->nodeHandle->advertise<VideostreamStartReq>(this->topicNamespace + "/videostream/startReq", 1);
    this->stopVideoStreamReqPublisher = this->nodeHandle->advertise<VideostreamStopReq>(this->topicNamespace + "/videostream/stopReq", 1);
    this->startVideoStreamRspSubscriber = this->nodeHandle->subscribe<VideostreamStartRsp>(this->topicNamespace + "/videostream/startRsp", 1, &Uav::on_videoStreamStartRspSubscriber_messageReceived, this);
    this->videoStreamDataSubscriber = this->nodeHandle->subscribe<VideostreamData>(this->topicNamespace + "/videostream/data", 1000, &Uav::on_videoStreamDataSubscriber_messageReceived, this);

    GetUavIdentRequest identReq;
    GetUavIdentResponse identRsp;
    this->getUavIdentServiceClient.call(identReq, identRsp);

    this->name = QString(identRsp.uavIdent.name.c_str());
    this->model = QString(identRsp.uavIdent.model.c_str());

    GetUavStateRequest stateReq;
    GetUavStateResponse stateRsp;
    this->getUavStateServiceClient.call(stateReq, stateRsp);

    this->mainState = (MainState) stateRsp.uavState.mainState;
    this->statusState = (StatusState) stateRsp.uavState.statusState;
    this->streamState = (StreamState) stateRsp.uavState.streamState;
    this->taskState = (TaskState) stateRsp.uavState.taskState;

    this->videoDecoder = NULL;
}

Uav::~Uav()
{
    this->videoDecoderLock.lock();
    if (this->videoDecoder != NULL)
    {
        delete this->videoDecoder;
    }
    this->videoDecoderLock.unlock();

    this->videoDecoderLock.~mutex();
}

unsigned int Uav::getID()
{
    return this->id;
}

QString Uav::getName()
{
    return this->name;
}

QString Uav::getModel()
{
    return this->model;
}

Uav::MainState Uav::getMainState()
{
    return this->mainState;
}

Uav::StatusState Uav::getStatusState()
{
    return this->statusState;
}

Uav::StreamState Uav::getStreamState()
{
    return this->streamState;
}

Uav::TaskState Uav::getTaskState()
{
    return this->taskState;
}

void Uav::startLiveStream()
{
    VideostreamStartReq startReq;

    startReq.cameraID = 0;

    this->startVideoStreamReqPublisher.publish(startReq);
}

void Uav::stopLiveStream()
{
    VideostreamStopReq stopReq;

    stopReq.stopCode = 0;

    this->stopVideoStreamReqPublisher.publish(stopReq);

    this->videoDecoderLock.lock();
    if (this->videoDecoder != NULL)
    {
        delete this->videoDecoder;
        this->videoDecoder = NULL;
    }
    this->videoDecoderLock.unlock();

    emit this->videostreamStateChanged(false);
}

bool Uav::isLiveStreamOn()
{
    std::cout << "StreamState: " << this->streamState << std::endl;
    return (this->streamState == STREAMSTATE_RECEIVING_VIDEO_DATA_ON);
}

void Uav::on_stateChangedNotificationSubscriber_messageReceived(const UavStateConstPtr& msg)
{
    this->mainState   = (MainState)   msg->mainState;
    this->statusState = (StatusState) msg->statusState;
    this->streamState = (StreamState) msg->streamState;
    this->taskState   = (TaskState)   msg->taskState;

    emit this->stateChanged(this->mainState, this->statusState, this->streamState, this->taskState);

    if (this->streamState == STREAMSTATE_RECEIVING_VIDEO_DATA_OFF)
    {
        this->videoDecoderLock.lock();
        if (this->videoDecoder != NULL)
        {
            delete this->videoDecoder;
            this->videoDecoder = NULL;
        }
        this->videoDecoderLock.unlock();

        emit this->videostreamStateChanged(false);
    }
    else if (this->streamState == STREAMSTATE_RECEIVING_VIDEO_DATA_ON)
    {
        emit this->videostreamStateChanged(true);
    }
}

void Uav::on_videoStreamDataSubscriber_messageReceived(const VideostreamDataConstPtr& data)
{
    if (this->videoDecoder)
    {
        if (this->number != data->number - 1)
        {
            std::cout << "packet lost" << std::endl;
        }
        this->number = data->number;

        this->videoDecoder->pourData((unsigned char*) &(data->data)[0], data->size);
    }
}

void Uav::on_videoStreamStartRspSubscriber_messageReceived(const VideostreamStartRspConstPtr& startRsp)
{
    this->videoDecoder = new VideoDecoder(startRsp->isTransportStream, startRsp->iframeNecessary, (unsigned char*) &(startRsp->iframe)[0], startRsp->iframe.size());

    bool isConnected = connect(this->videoDecoder, SIGNAL(frameDecoded(uint8_t*, uint, uint, uint)), this, SLOT(on_videoDecoder_frameDecoded(uint8_t*, uint, uint, uint)), Qt::DirectConnection);

    if (isConnected)
    {
        std::cout << "decoder connected" << std::endl;
    }
    else
    {
        std::cerr << "decoder not connected" << std::endl;
    }

    emit this->videostreamStateChanged(true);
}

void Uav::on_videoDecoder_frameDecoded(uint8_t* frame, uint width, uint height, uint bytesPerLine)
{
    emit this->frameReceived(frame, width, height, bytesPerLine);
}


