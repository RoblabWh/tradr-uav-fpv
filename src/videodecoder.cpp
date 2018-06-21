#include "videodecoder.h"

#include <QtConcurrentRun>

VideoDecoder* VideoDecoder::instance = nullptr;


VideoDecoder::VideoDecoder(bool isTransportStream, bool isIFrameNeccessary, uint8_t* iFrame, unsigned int iFrameSize)
{
    this->ready = false;

    VideoDecoder::instance = this;

    this->videoBuffer = nullptr;
    this->size = 0;


    this->logFile = fopen(LOGFILE, "a");
    this->logFileNumber = fileno(this->logFile);

    stringstream fifoPathSS;
    fifoPathSS << "/tmp/uavFPV_" << getpid() << ".pipe";
    string fifoPath = fifoPathSS.str();

    if ((mkfifo(fifoPath.c_str(), S_IRUSR | S_IWUSR)) == -1)
    {
        if(errno == EEXIST)
        {
            perror("mkfifo()");
        }
        else
        {
            perror("mkfifo()");
            exit(EXIT_FAILURE);
        }
    }

    // VLC options
    std::stringstream smem_options_stream;

    smem_options_stream << "#transcode{vcodec=RV24}" // Make stream smem compatible
                        << ":"
                           // Insert addresses of callbacks
                        << "smem{"
                        << "video-prerender-callback=" << (long long int)(intptr_t)(void*)&cbVideoPrerender
                        << ","
                        << "video-postrender-callback=" << (long long int)(intptr_t)(void*)&cbVideoPostrender
                        << ","
                        << "no-time-sync"
                        << "}";

    std::string smem_options = smem_options_stream.str();

    const char * const argumentsForTS[] = {
        "-I", "dummy",          // Don't use any interface
        "--ignore-config",      // Don't use VLC's config
        "--network-caching=0",  // Don't use buffer
        "--extraintf=logger",   // Log anything
        "--verbose=2",          // Be much more verbose then normal for debugging purpose
        "--sout", smem_options.c_str() // Stream to memory
    };

    const char * const argumentsForH264[] = {
        "-I", "dummy",          // Don't use any interface
        "--ignore-config",      // Don't use VLC's config
        "--demux", "h264",      // Decoder for h264
        "--network-caching=0",  // Don't use buffer
        "--extraintf=logger",   // Log anything
        "--verbose=2",          // Be much more verbose then normal for debugging purpose
        "--sout", smem_options.c_str() // Stream to memory
    };

    unsigned int numberOfArguments;

    numberOfArguments = isTransportStream
                      ? sizeof(argumentsForTS) / sizeof(argumentsForTS[0])
                      : sizeof(argumentsForH264) / sizeof(argumentsForH264[0]);


    // We launch VLC

    vlcInstance = libvlc_new(numberOfArguments, isTransportStream ? argumentsForTS : argumentsForH264);

    libvlc_media_t* media;

    media = libvlc_media_new_path(vlcInstance, fifoPath.c_str());

    // create a media play playing environment
    mp = libvlc_media_player_new_from_media(media);

    // no need to keep the media now
    libvlc_media_release(media);

    //mp = libvlc_media_player_new(vlcInstance);
    libvlc_media_player_play(mp);



    fifoWritePort = open(fifoPath.c_str(), O_WRONLY);
    if (fifoWritePort < 0)
    {
        fprintf(stderr, "could not open pipe (%s)", fifoPath.c_str());
        perror("open()");
        exit(EXIT_FAILURE);
    }

    if (isIFrameNeccessary)
    {
        bool written = false;
        while (!written)
        {
            int writtenBytes = write(fifoWritePort, iFrame, iFrameSize);

            if (writtenBytes > 0)
            {
                std::cout << "iFrame injected with length: " << iFrameSize << std::endl;
                written = true;
            }
            else
            {
                fprintf(stderr, "could not write iframe to pipe (length: %d)", iFrameSize);
                perror("write()");
            }
        }
    }

    this->ready = true;

}

VideoDecoder::~VideoDecoder()
{

    std::cout << "before lock at \"destructor\" " << std::endl;
    renderLock.lock();
    std::cout << "after lock at \"destructor\" " << std::endl;

    libvlc_media_player_stop(mp);
    libvlc_media_player_release(mp);

    std::cout << "before unlock at \"destructor\" " << std::endl;
    renderLock.unlock();
    std::cout << "after unlock at \"destructor\" " << std::endl;

    std::cout << "before destroy at \"destructor\" " << std::endl;
    renderLock.~mutex();
    std::cout << "after destroy at \"destructor\" " << std::endl;
}

void VideoDecoder::pourData(uint8_t* h264DataUnit, unsigned int size)
{
    if (this->ready)
    {
        write(fifoWritePort, h264DataUnit, size);
        //write(logFileNumber, h264DataUnit, size);
    }
}


void VideoDecoder::cbVideoPrerender(void* videoData, uint8_t** pixelBuffer, int numberOfBytes)
{

    VideoDecoder::instance->on_vlcInstance_nextFrameWillBeDecoded(videoData, pixelBuffer, numberOfBytes);

    /*
    VideoDecoder* decoder = NULL;
    for (pair< VideoDecoder*, uint8_t*> instance : VideoDecoder::instanceList)
    {
        if (instance.second == *pixelBuffer)
        {
            decoder = instance.first;
            break;
        }
    }

    if (decoder != NULL)
    {
        decoder->on_vlcInstance_nextFrameWillBeDecoded(videoData, pixelBuffer, size);
    }
    */
}

void VideoDecoder::cbVideoPostrender(void* videoData, uint8_t* pixelBuffer, int width, int height, int bitsPerPixel, int numberOfBytes, int64_t pts)
{

    VideoDecoder::instance->on_vlcInstance_frameDecoded(videoData, pixelBuffer, width, height, bitsPerPixel, numberOfBytes, pts);
}

void VideoDecoder::on_vlcInstance_nextFrameWillBeDecoded(void* videoData, uint8_t** pixelBuffer, int numberOfBytes)
{
    std::cout << "before lock at \"nextFrameWillBeDecoded\" " << std::endl;
    this->renderLock.lock();
    std::cout << "after lock at \"nextFrameWillBeDecoded\" " << std::endl;

    if (this->size != numberOfBytes)
    {
        free(this->videoBuffer);
        this->videoBuffer = (uint8_t*) malloc(numberOfBytes);
        this->size = numberOfBytes;
    }
    *pixelBuffer = this->videoBuffer;
}

void VideoDecoder::on_vlcInstance_frameDecoded(void* videoData, uint8_t* pixelBuffer, int width, int height, int bitsPerPixel, int numberOfBytes, int64_t pts)
{
    /*
    std::cout << "bits per pixel: \t" << bitsPerPixel << std::endl;
    std::cout << "pts: \t" << pts << std::endl;
    std::cout << "number of Bytes: \t" << numberOfBytes << std::endl;
    std::cout << "bytes per line: \t" << (width * 3) << std::endl;
    std::cout << "width: \t" << width << std::endl;
    std::cout << "height: \t" << height << std::endl;
    */

    emit this->frameDecoded((uint8_t*) pixelBuffer, (uint) width, (uint) height, (uint) (width * 3));

    std::cout << "before unlock at \"frameDecoded\" " << std::endl;
    this->renderLock.unlock();
    std::cout << "after unlock at \"frameDecoded\" " << std::endl;
}

void VideoDecoder::writeIFrame()
{
    int writtenBytes = write(fifoWritePort, this->iFrame, this->iFrameSize);

    if (writtenBytes > 0)
    {
        std::cout << "iFrame injected with length: " << this->iFrameSize << std::endl;
    }
    else
    {
        fprintf(stderr, "could not write iframe to pipe (length: %d)", this->iFrameSize);
        perror("write()");
        exit(EXIT_FAILURE);
    }
}



