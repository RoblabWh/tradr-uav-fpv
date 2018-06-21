#include "rosthread.h"

RosThread::RosThread()
{

}

void RosThread::run()
{
    ros::spin();
}
