#include <iostream>
#include <cstring>

#include "AbletonPushManager.h"


#define ABLETON_VENDOR_ID 0x2982
#define PUSH2_PRODUCT_ID  0x1967
#define PUSH2_BULK_EP_OUT 0x01
#define TRANSFER_TIMEOUT  1000

#define PUSH2_WIDTH 960
#define PUSH2_HEIGHT 160
#define PUSH2_LINE_PIXELS 960
#define PUSH2_LINE_BYTES 1920
#define PUSH2_LINE_FILLER 128
#define PUSH2_LINE_TOTAL_BYTES (PUSH2_LINE_BYTES + PUSH2_LINE_FILLER)
#define PUSH2_FRAME_LINES 160
#define PUSH2_FRAME_BYTES (PUSH2_LINE_TOTAL_BYTES * PUSH2_FRAME_LINES)
#define PUSH2_XOR_PATTERN 0xFFE7F3E7

AbletonPushManager::AbletonPushManager() {
    // Constructor implementation
}

AbletonPushManager::~AbletonPushManager() {
    ClosePush2Device();
    // Destructor implementation
}

bool AbletonPushManager::Init() {

    push2Handle = OpenPush2Device();
    if (!push2Handle) {
        std::cout << "Failed to open Ableton Push 2 device." << std::endl;
        return false;
    }
    AllocateTransferBuffer();
    std::cout << "Ableton Push 2 device opened successfully." << std::endl;
    // Initialization code
    return true;
}

bool AbletonPushManager::Send(const std::vector<uint8_t>& data) {
    // Send data to Ableton Push
    return true;
}


libusb_device_handle* AbletonPushManager::OpenPush2Device() 
{
    int result;

    if ((result = libusb_init(NULL)) < 0)
    {
        std::cout << "error: [" << result << "] could not initialize libusb" << std::endl;
        return NULL;
    }
    else{
        std::cout << "libusb initialized successfully." << std::endl;
    }

    libusb_set_debug(NULL, LIBUSB_LOG_LEVEL_ERROR);

    libusb_device** devices;
    ssize_t count;
    count = libusb_get_device_list(NULL, &devices);
    if (count < 0)
    {
        std::cout << "error: [" << count << "] could not get usb device list" << std::endl;
        return NULL;
    }
    if (count == 0)
    {
        std::cout << "error: no usb devices found" << std::endl;
        return NULL;
    }

    libusb_device* device;
    libusb_device_handle* device_handle = NULL;


    for (int i = 0; (device = devices[i]) != NULL; i++)
    {
        struct libusb_device_descriptor descriptor;
        if ((result = libusb_get_device_descriptor(device, &descriptor)) < 0)
        {
            std::cout << "error: [" << result << "] could not get usb device descriptor" << std::endl;
            continue;
        }

        if (descriptor.bDeviceClass == LIBUSB_CLASS_PER_INTERFACE
            && descriptor.idVendor == ABLETON_VENDOR_ID
            && descriptor.idProduct == PUSH2_PRODUCT_ID)
        {
            if ((result = libusb_open(device, &device_handle)) < 0)
            {
                std::cout << "error: [" << result << "] could not open Ableton Push 2 device" << std::endl;
            }
            else if ((result = libusb_claim_interface(device_handle, 0)) < 0)
            {
                std::cout << "error: [" << result << "] could not claim interface 0 of Push 2 device" << std::endl;
                libusb_close(device_handle);
                device_handle = NULL;
            }
            else
            {
                break; // successfully opened
            }
        }
    }


    libusb_free_device_list(devices, 1);
    return device_handle;
}

void AbletonPushManager::ClosePush2Device() 
{
    if (push2Handle) {
        libusb_release_interface(push2Handle, 0);
        libusb_close(push2Handle);
        push2Handle = NULL;
    }
}

// Callback for frame header transfer
void on_frame_header_transfer_finished(struct libusb_transfer* transfer)
{
    if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {
        std::cout << "Frame header transfer completed successfully." << std::endl;
    } else {
        std::cout << "Frame header transfer failed with status: " << transfer->status << std::endl;
    }
    libusb_free_transfer(transfer);
}

// Callback for buffer transfer
void on_buffer_transfer_finished(struct libusb_transfer* transfer)
{
    if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {
        std::cout << "Buffer transfer completed successfully." << std::endl;
    } else {
        std::cout << "Buffer transfer failed with status: " << transfer->status << std::endl;
    }
    libusb_free_transfer(transfer);
}

// Helper to fill buffer with blue/red checkerboard and apply XOR pattern
void FillPush2BufferWithCheckerboard(uint8_t* buffer) {
    for (int line = 0; line < PUSH2_FRAME_LINES; ++line) {
        uint8_t* line_ptr = buffer + line * PUSH2_LINE_TOTAL_BYTES;
        // Fill pixel data
        for (int pixel = 0; pixel < PUSH2_LINE_PIXELS; ++pixel) {
            bool is_blue = ((line + pixel) % 2 == 0);
            uint16_t pixel_value = is_blue ? 0b1111100000000000 : 0b0000000000011111; // blue or red, 16-bit RGB
            line_ptr[pixel * 2 + 0] = pixel_value & 0xFF;      // LSB
            line_ptr[pixel * 2 + 1] = (pixel_value >> 8) & 0xFF; // MSB
        }
        // Fill filler bytes (must be zero)
        memset(line_ptr + PUSH2_LINE_BYTES, 0, PUSH2_LINE_FILLER);
    }
    // XOR pattern
    const uint8_t xor_pattern[4] = {0xE7, 0xF3, 0xE7, 0xFF};
    for (size_t i = 0; i < PUSH2_FRAME_BYTES; ++i) {
        buffer[i] ^= xor_pattern[i % 4];
    }
}

void FillPush2BufferWithRed(uint8_t* buffer) {
    for (int line = 0; line < PUSH2_FRAME_LINES; ++line) {
        uint8_t* line_ptr = buffer + line * PUSH2_LINE_TOTAL_BYTES;
        for (int pixel = 0; pixel < PUSH2_LINE_PIXELS; ++pixel) {
            uint16_t pixel_value = 0b0000000000011111; // pure red, 16-bit RGB
            line_ptr[pixel * 2 + 0] = pixel_value & 0xFF;
            line_ptr[pixel * 2 + 1] = (pixel_value >> 8) & 0xFF;
        }
        memset(line_ptr + PUSH2_LINE_BYTES, 0, PUSH2_LINE_FILLER);
    }
    const uint8_t xor_pattern[4] = {0xE7, 0xF3, 0xE7, 0xFF};
    for (size_t i = 0; i < PUSH2_FRAME_BYTES; ++i) {
        buffer[i] ^= xor_pattern[i % 4];
    }
}

void AbletonPushManager::AllocateTransferBuffer()
{
    unsigned char frame_header[16] = {
    0xff, 0xcc, 0xaa, 0x88,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
    };

    libusb_transfer* frame_header_transfer;

    if ((frame_header_transfer = libusb_alloc_transfer(0)) == NULL)
    {
        std::cout << "error: could not allocate frame header transfer handle" << std::endl;
    }
    else
    {
    libusb_fill_bulk_transfer(
        frame_header_transfer,
        push2Handle,
        PUSH2_BULK_EP_OUT,
        frame_header,
        sizeof(frame_header),
        on_frame_header_transfer_finished,
        NULL,
        TRANSFER_TIMEOUT);
    }

    // Prepare checkerboard buffer
    static uint8_t checkerboard_buffer[PUSH2_FRAME_BYTES];
    FillPush2BufferWithCheckerboard(checkerboard_buffer);
    // Use checkerboard_buffer for pixel_data_transfer
    libusb_transfer* pixel_data_transfer;
    if ((pixel_data_transfer = libusb_alloc_transfer(0)) == NULL)
    {
        std::cout << "error: could not allocate transfer handle" << std::endl;
    }
    else
    {
        libusb_fill_bulk_transfer(
            pixel_data_transfer,
            push2Handle,
            PUSH2_BULK_EP_OUT,
            checkerboard_buffer,
            PUSH2_FRAME_BYTES,
            on_buffer_transfer_finished,
            NULL,
            TRANSFER_TIMEOUT);
    }
}

void AbletonPushManager::SendCheckerboardFrame() {
    if (!push2Handle) {
        std::cout << "Push 2 device not open." << std::endl;
        return;
    }
    // Frame header
    unsigned char frame_header[16] = {
        0xff, 0xcc, 0xaa, 0x88,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };
    int transferred = 0;
    int res = libusb_bulk_transfer(push2Handle, PUSH2_BULK_EP_OUT, frame_header, sizeof(frame_header), &transferred, TRANSFER_TIMEOUT);
    if (res != 0 || transferred != sizeof(frame_header)) {
        std::cout << "Error sending frame header: " << res << " transferred: " << transferred << std::endl;
        return;
    }
    // Prepare checkerboard buffer
    static uint8_t checkerboard_buffer[PUSH2_FRAME_BYTES];
    FillPush2BufferWithCheckerboard(checkerboard_buffer);
    // Send pixel data in 16kB chunks for efficiency
    const int chunk_size = 16 * 1024;
    int bytes_sent = 0;
    while (bytes_sent < PUSH2_FRAME_BYTES) {
        int to_send = std::min(chunk_size, (int)(PUSH2_FRAME_BYTES - bytes_sent));
        res = libusb_bulk_transfer(push2Handle, PUSH2_BULK_EP_OUT, checkerboard_buffer + bytes_sent, to_send, &transferred, TRANSFER_TIMEOUT);
        if (res != 0 || transferred != to_send) {
            std::cout << "Error sending pixel data chunk: " << res << " transferred: " << transferred << std::endl;
            return;
        }
        bytes_sent += transferred;
    }
    std::cout << "Checkerboard frame sent successfully." << std::endl;
}

void AbletonPushManager::SendRedFrame() {
    if (!push2Handle) {
        std::cout << "Push 2 device not open." << std::endl;
        return;
    }
    unsigned char frame_header[16] = {
        0xff, 0xcc, 0xaa, 0x88,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };
    int transferred = 0;
    int res = libusb_bulk_transfer(push2Handle, PUSH2_BULK_EP_OUT, frame_header, sizeof(frame_header), &transferred, TRANSFER_TIMEOUT);
    if (res != 0 || transferred != sizeof(frame_header)) {
        std::cout << "Error sending frame header: " << res << " transferred: " << transferred << std::endl;
        return;
    }
    static uint8_t red_buffer[PUSH2_FRAME_BYTES];
    FillPush2BufferWithRed(red_buffer);
    const int chunk_size = 16 * 1024;
    int bytes_sent = 0;
    while (bytes_sent < PUSH2_FRAME_BYTES) {
        int to_send = std::min(chunk_size, (int)(PUSH2_FRAME_BYTES - bytes_sent));
        res = libusb_bulk_transfer(push2Handle, PUSH2_BULK_EP_OUT, red_buffer + bytes_sent, to_send, &transferred, TRANSFER_TIMEOUT);
        if (res != 0 || transferred != to_send) {
            std::cout << "Error sending pixel data chunk: " << res << " transferred: " << transferred << std::endl;
            return;
        }
        bytes_sent += transferred;
    }
    std::cout << "Red frame sent successfully." << std::endl;
}

bool AbletonPushManager::SendRawBuffer(const uint8_t* buffer, size_t size) {
    if (!push2Handle) {
        std::cout << "Push 2 device not open." << std::endl;
        return false;
    }
    // Send frame header
    unsigned char frame_header[16] = {
        0xff, 0xcc, 0xaa, 0x88,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };
    int transferred = 0;
    int res = libusb_bulk_transfer(push2Handle, PUSH2_BULK_EP_OUT, frame_header, sizeof(frame_header), &transferred, TRANSFER_TIMEOUT);
    if (res != 0 || transferred != sizeof(frame_header)) {
        std::cout << "Error sending frame header: " << res << " transferred: " << transferred << std::endl;
        return false;
    }
    // Send pixel data in 16kB chunks for efficiency
    const int chunk_size = 16 * 1024;
    int bytes_sent = 0;
    while (bytes_sent < size) {
        int to_send = std::min(chunk_size, (int)(size - bytes_sent));
        // Remove const qualifier for libusb_bulk_transfer
        res = libusb_bulk_transfer(push2Handle, PUSH2_BULK_EP_OUT, const_cast<uint8_t*>(buffer + bytes_sent), to_send, &transferred, TRANSFER_TIMEOUT);
        if (res != 0 || transferred != to_send) {
            std::cout << "Error sending pixel data chunk: " << res << " transferred: " << transferred << std::endl;
            return false;
        }
        bytes_sent += transferred;
    }
    return true;
}

bool AbletonPushManager::SendFrame(const uint8_t* bgraBuffer, int width, int height) {
    if (width != PUSH2_WIDTH || height != PUSH2_HEIGHT) {
        std::cout << "SendFrame: Invalid size, expected 960x160 BGRA." << std::endl;
        return false;
    }
    std::vector<uint8_t> push2Buffer(PUSH2_FRAME_BYTES, 0);
    for (int y = 0; y < PUSH2_HEIGHT; ++y) {
        for (int x = 0; x < PUSH2_WIDTH; ++x) {
            int srcIdx = (y * PUSH2_WIDTH + x) * 4;
            uint8_t r = bgraBuffer[srcIdx + 0];
            uint8_t g = bgraBuffer[srcIdx + 1];
            uint8_t b = bgraBuffer[srcIdx + 2];
            // Correct RGB565 conversion
            uint16_t rgb = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
            int dstIdx = x * 2 + y * PUSH2_LINE_TOTAL_BYTES;
            push2Buffer[dstIdx + 0] = rgb & 0xFF;
            push2Buffer[dstIdx + 1] = (rgb >> 8) & 0xFF;
        }
        int fillerStart = y * PUSH2_LINE_TOTAL_BYTES + PUSH2_LINE_BYTES;
        memset(push2Buffer.data() + fillerStart, 0, PUSH2_LINE_FILLER);
    }
    // XOR pattern
    const uint8_t xor_pattern[4] = {0xE7, 0xF3, 0xE7, 0xFF};
    for (size_t i = 0; i < push2Buffer.size(); ++i) {
        push2Buffer[i] ^= xor_pattern[i % 4];
    }
    return SendRawBuffer(push2Buffer.data(), push2Buffer.size());
}