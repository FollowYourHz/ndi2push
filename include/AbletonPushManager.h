#pragma once
#include <cstdint>
#include <vector>
#include "libusb.h"

class AbletonPushManager {
public:
    AbletonPushManager();
    ~AbletonPushManager();

    bool Init();
    bool Send(const std::vector<uint8_t>& data);
    void SendCheckerboardFrame();
    void SendRedFrame();
    bool SendRawBuffer(const uint8_t* buffer, size_t size);
    bool SendFrame(const uint8_t* bgraBuffer, int width, int height); // Added SendFrame declaration


private:
    libusb_device_handle* push2Handle;
    libusb_device_handle* OpenPush2Device();    // Add private members for device handle, etc.
    void ClosePush2Device();
    void  AllocateTransferBuffer();
    
};
