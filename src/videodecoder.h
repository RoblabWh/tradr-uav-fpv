#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include <QObject>

#include <QThread>

#include <vlc/vlc.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#include <boost/thread/mutex.hpp>

#include <list>

class VideoDecoder;

#include "src/uav.h"

#define LOGFILE "/tmp/videostream.dat"

using namespace std;

class VideoDecoder : public QObject
{
    Q_OBJECT

private:
    static list< pair< VideoDecoder*, uint8_t* > > instanceList;

signals:
    void frameDecoded(uint8_t* frame, uint width, uint height, uint bytesPerLine);

public:
    VideoDecoder(bool isTransportStream, bool isIFrameNeccessary, uint8_t* iFrame, unsigned int iFrameSize);
    ~VideoDecoder();

    void pourData(uint8_t* h264DataUnit, unsigned int size);

private:
    static VideoDecoder* instance;

    int fifoReadPort;
    int fifoWritePort;

    int logFileNumber;

    FILE* pipe;
    FILE* logFile;

    // VLC pointers
    libvlc_instance_t*      vlcInstance;
    libvlc_media_player_t*  mp;
    uint8_t*                videoBuffer;
    int                     size;

    boost::mutex renderLock;

    bool isTransportStream;
    bool isIFrameNeccessary;
    uint8_t* iFrame;
    unsigned int iFrameSize;

    bool ready;


private:

    static void cbVideoPrerender(void* videoData, uint8_t** pixelBuffer, int numberOfBytes);
    static void cbVideoPostrender(void* videoData, uint8_t* pixelBuffer, int width, int height, int bitsPerPixel, int numberOfBytes, int64_t pts);

    void on_vlcInstance_nextFrameWillBeDecoded(void* videoData, uint8_t** pixelBuffer, int numberOfBytes);
    void on_vlcInstance_frameDecoded(void* videoData, uint8_t* pixelBuffer, int width, int height, int bitsPerPixel, int numberOfBytes, int64_t pts);

    void writeIFrame();
};

#endif // VIDEODECODER_H
