#include <iostream>
#include <string>
#include <cstring>
#include <iomanip>
#include <thread>
#include <atomic>
#include <csignal>

#include "ndiReceiverWrapper.h"
#include "AbletonPushManager.h"

using namespace std::chrono;

const uint32_t push2Width = 960;
const uint32_t push2Height = 160;

std::atomic<bool> running(true);

void signalHandler(int signum) {
    running = false;
}

int main(int argc, char* argv[])
{	
    AbletonPushManager *pushManager = new AbletonPushManager();
    if (!pushManager->Init()) {
        std::cerr << "Failed to initialize Ableton Push Manager." << std::endl;
        return -1;
    }

    NdiReceiver *recv = new NdiReceiver();
    recv->initialize();
    recv->setAbletonPushManagerPtr(pushManager);

    std::signal(SIGINT, signalHandler);
    std::thread ndiCaptureThread(&NdiReceiver::captureInLoop, recv, std::string("NDI Video Capture Thread"));
    
    while(running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    ndiCaptureThread.join();
    recv->cleanup();
    std::cout << "Exiting..." << std::endl; 
    delete recv;
    delete pushManager;
    return 0;
}
