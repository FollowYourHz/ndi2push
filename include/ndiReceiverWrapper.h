#include <iostream>

#include <Processing.NDI.Lib.h>

#include "AbletonPushManager.h"


class NdiReceiver 
{
    public:
        NdiReceiver();
        int initialize();
        void cleanup();
        void captureInLoop(std::string agrument);

        // Add getter and setter for AbletonPushManager pointer
        void setAbletonPushManagerPtr(AbletonPushManager* mgr);
        AbletonPushManager* getAbletonPushManagerPtr() const;
    

    private:

    NDIlib_recv_instance_t pNDI_recv;
    const std::uint16_t  frameWidth = 960;
    const std::uint16_t  frameHigh  = 160;
    const std::uint8_t   pixelSize = 4;

    uint16_t outputWidth;
    uint16_t outputHeight;

    const std::string threadName = "NDI Video Capture Thread";
    public:


    AbletonPushManager* pushManagerPtr = nullptr;

};

