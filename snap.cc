#include <iostream>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <unistd.h>
#include <ueye.h>
#include <zmq.hpp>

#define debug cout

using namespace std;

// Standard resolutions, if you need more then
// specify: http://www.ids-imaging.de/frontend/files/uEyeManuals/Manual_eng/uEye_Manual/index.html?is_imageformat.html
enum Resolution {
    R_8M = 1,
    R_8M_3_2,
    R_8M_16_9,
    R_5M,
    R_3M,
    R_FULL_HD_16_9,
    R_2M,
    R_1_2M_4_3,
    R_HD_16_9,
    R_WVGA_2_1,
    R_WVGA,
    R_VGA,
    R_VGA_16_9,
    R_WQVGA
};


// http://www.ids-imaging.de/frontend/files/uEyeManuals/Manual_eng/uEye_Manual/index.html?is_imageformat.html
class Camera {
public:
    char* snap_buffer;
    int buffer_id;
    int mode;
    int cam_size;
    int width;
    int height;
    int depth;
    HIDS handle;
    UINT format_count;
    IMAGE_FORMAT_LIST* resolutions;
    bool connected;

    Camera() : snap_buffer(0), handle(0), resolutions(0), connected(false)
    {
        // empty
    }

    ~Camera()
    {
        shutdown();
    }

    void create(const int device_id)
    {
        handle = device_id;
        handle |= IS_USE_DEVICE_ID;
        if (is_InitCamera(&handle, NULL) != IS_SUCCESS) {
            cerr << "Camera(" << handle << ") failed to initialize." << endl;
            connected = false;
            return;
        }
        connected = true;

        debug << "Initialized camera: " << handle << endl;
    }

    void shutdown()
    {
        if (resolutions != NULL) {
            free(resolutions);
            debug << "Cleaned up resolutions." << endl;
            resolutions = 0;
        }

        if (snap_buffer != NULL) {
            is_FreeImageMem(handle , snap_buffer, buffer_id);
            debug << "Cleaned up allocated snap buffer." << endl;
            snap_buffer = 0;
        }

        if (connected && handle) {
            is_ExitCamera(handle);
            debug << "is_ExitCamera called." << endl;
            connected = false;
            handle = 0;
        }
    }

    bool stop_video(int video_mode)
    {
        debug << "Video mode: " << video_mode << endl;
        return is_StopLiveVideo(handle, video_mode) == IS_SUCCESS;
    }

    bool external_trigger(int trigger)
    {
        debug << "External trigger: " << trigger << endl;
        return is_SetExternalTrigger(handle, trigger) == IS_SUCCESS;
    }

    bool color_mode(int color)
    {
        debug << "Color mode: " << color << endl;
        return is_SetColorMode(handle, color) == IS_SUCCESS;
    }

    bool display_mode(int mode)
    {
        debug << "Display mode: " << mode << endl;
        return is_SetDisplayMode(handle, mode) == IS_SUCCESS;
    }

    bool refresh_formats_available()
    {
        debug << "Refreshing camera image formats." << endl;
        // Get number of available formats and size of list
        UINT bytes_needed = sizeof(IMAGE_FORMAT_LIST);
        INT n_ret = is_ImageFormat(handle, IMGFRMT_CMD_GET_NUM_ENTRIES, &format_count, 4);
        bytes_needed += (format_count - 1) * sizeof(IMAGE_FORMAT_INFO);
        void* ptr = malloc(bytes_needed);
         
        // Create and fill list
        resolutions = (IMAGE_FORMAT_LIST*) ptr;
        resolutions->nSizeOfListEntry = sizeof(IMAGE_FORMAT_INFO);
        resolutions->nNumListElements = format_count;

        n_ret = is_ImageFormat(handle, IMGFRMT_CMD_GET_LIST, resolutions, bytes_needed);
        stop_video(IS_WAIT);
        external_trigger(IS_SET_TRIGGER_SOFTWARE);

        debug << "Formats found: " << format_count << endl;
        return n_ret == IS_SUCCESS;
    }

    bool resolution(Resolution mode)
    {
        if (!refresh_formats_available()) {
            cerr << "Camera image formats could not be generated." << endl;
            return false;
        }

        debug << "Resolution mode: " << mode << endl;
        bool format_found = false;
        IMAGE_FORMAT_INFO format_info;
        for (UINT i = 0; i < format_count; i++) {
            if (resolutions->FormatInfo[i].nFormatID == mode) {
                format_info = resolutions->FormatInfo[i];
                format_found = true;
                break;
            }
        }

        if (!format_found) {
            cerr << "Resolution was not found: " << mode << endl;
            return false;
        }

        debug << "Resolution format found: " << format_info.strFormatName << endl;
        if (is_ImageFormat(handle, IMGFRMT_CMD_SET_FORMAT, &format_info.nFormatID, 4) != IS_SUCCESS) {
            cerr << "Failed to set resolution mode." << endl;
            return false;
        }

        width = format_info.nWidth;
        height = format_info.nHeight;
        depth = 24;
        cam_size = width * height * depth;

        debug << "Allocation size: " << cam_size << endl;

        if (snap_buffer) {
            debug << "Freeing memory for camera." << endl;
            is_FreeImageMem(handle, snap_buffer, buffer_id);
        }

        if (is_AllocImageMem(handle, width, height, depth, &snap_buffer, &buffer_id)
                != IS_SUCCESS) {
            cerr << "Error in allocating image memory." << endl;
            return false;
        }

        debug << "Setting memory for camera" << endl;
        if (is_SetImageMem(handle, snap_buffer, buffer_id) != IS_SUCCESS) {
            cerr << "Error in setting image memory." << endl;
            return false;
        }
        debug << "Memory set." << endl;
        return true;
    }

    bool snap(zmq::socket_t* socket)
    {
        static int message_count = 0;
        zmq::message_t message(cam_size);

        if (is_FreezeVideo(handle, IS_WAIT) != IS_SUCCESS) {
            cerr << "Error in freezing a frame." << endl;
            return false;
        }

        is_CopyImageMem(handle, snap_buffer, buffer_id, (char*)message.data());
        socket->send(message);
        debug << "[" << ++message_count << "] Message sent (" << cam_size << ")" << endl;
        return true;
    }
};

// oh baby!
Camera camera;

void sig_exit_handler(int sig)
{
    // gracefully shutdown the IDS camera
    cerr << "caught SIGHUP or SIGINT, shutting down gracefully..." << endl;
    camera.shutdown();
    exit(130);
}

void sig_ttystop_handler(int sig)
{
    // alert the user to take more appropriate action
    cerr << "SIGTSTP is disabled, use SIGHUP or SIGINT to terminate" << endl;
}

int main(int argc, char** argv)
{
    if (argc < 4) {
        cerr << argv[0] << ": <camera> <resolution-mode[1-19]> <delay> <port>" << endl;
        exit(1);
    }

    int devid = atoi(argv[1]);
    int resolution = atoi(argv[2]);
    int delay = atoi(argv[3]);
    string tcp = "tcp://*:";
    tcp += argv[4];

    signal(SIGHUP, sig_exit_handler);
    signal(SIGINT, sig_exit_handler);
    signal(SIGTSTP, sig_ttystop_handler);

    camera.create(devid);
    if (!camera.connected) {
        cerr << "Camera not connected" << endl;
        exit(2);
    }

    if (!camera.resolution(Resolution(resolution))) {
        cerr << "Camera failed to set resolution" << endl;
        exit(3);
    }

    debug << "Preparing ZMQ:PUSH on: " << tcp << endl;
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_PUSH);
    socket.bind(tcp.c_str());
    
    while (true) {
        camera.snap(&socket);
        sleep(delay);
    }
    
    return 0;
}
