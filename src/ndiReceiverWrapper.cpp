#include "ndiReceiverWrapper.h"


NdiReceiver::NdiReceiver()
{
    outputWidth = 960;
    outputHeight = 160;

}

int NdiReceiver::initialize()
{
    // Create a finder
	NDIlib_find_instance_t pNDI_find = NDIlib_find_create_v2();
	if (!pNDI_find) return 0;

	// Wait until there is one source
	uint32_t no_sources = 0;
	const NDIlib_source_t* p_sources = NULL;
	while (!no_sources)
	{	// Wait until the sources on the network have changed
		printf("Looking for sources ...\n");
		NDIlib_find_wait_for_sources(pNDI_find, 1000/* One second */);
		p_sources = NDIlib_find_get_current_sources(pNDI_find, &no_sources);
	}
	std::cout << "Found " << no_sources << " sources." << std::endl;

    // List all sources
    for (uint32_t i = 0; i < no_sources; ++i)
    {
        std::cout << "Source " << i << ": " << p_sources[i].p_ndi_name << std::endl;
    }

    // Find the source named "push2_out"
    const NDIlib_source_t* selected_source = nullptr;
    for (uint32_t i = 0; i < no_sources; ++i)
    {
        std::string source_name(p_sources[i].p_ndi_name);

        if (source_name.find("push2_out") != std::string::npos)
        {
            selected_source = &p_sources[i];
            std::cout << "Found source containing 'push2_out': " << source_name << std::endl;
            break;
        }
    }

    if (!selected_source)
    {
        std::cerr << "Source named 'push2_out' not found." << std::endl;
        NDIlib_find_destroy(pNDI_find);
        return 0;
    }

    NDIlib_recv_create_v3_t recv_desc;
    recv_desc.color_format = NDIlib_recv_color_format_BGRX_BGRA;

	// We now have the selected source, so we create a receiver to look at it.
	pNDI_recv = NDIlib_recv_create_v3(&recv_desc);

	if (!pNDI_recv) return 0;

	// Connect to the selected source
	NDIlib_recv_connect(pNDI_recv, selected_source);

	// Destroy the NDI finder. We needed to have access to the pointers to p_sources[0]
	NDIlib_find_destroy(pNDI_find);	


    return 0;
}

extern std::atomic<bool> running;

void NdiReceiver::captureInLoop(std::string agrument)
{
    NDIlib_video_frame_v2_t video_frame;
    std::vector<uint8_t> ndiBuffer(frameWidth * frameHigh * pixelSize, 0);

    while (running) {
        switch (NDIlib_recv_capture_v2(pNDI_recv, &video_frame, nullptr, nullptr, 1000)) {
            case NDIlib_frame_type_video:
                memcpy(ndiBuffer.data(), video_frame.p_data, frameWidth * frameHigh * pixelSize);
                if (pushManagerPtr) {
                    pushManagerPtr->SendFrame(ndiBuffer.data(), frameWidth, frameHigh);
                }
                NDIlib_recv_free_video_v2(pNDI_recv, &video_frame);
                break;

            // Add default case to handle other frame types and silence the warning.
            default:
                break;
        }
    }
}




void NdiReceiver::cleanup()
{
    // Destroy the receiver
	NDIlib_recv_destroy(pNDI_recv);

	// Not required, but nice
	NDIlib_destroy();
}

void NdiReceiver::setAbletonPushManagerPtr(AbletonPushManager* mgr) {
    pushManagerPtr = mgr;
}

AbletonPushManager* NdiReceiver::getAbletonPushManagerPtr() const {
    return pushManagerPtr;
}