#include <iostream>
#include <cstdlib>
#include <uEye.h>
#include <ueye.h>

using namespace std;

int main(int argc, char** argv)
{
    if (argc != 3) {
        cerr << "save_image <camera_id> <path>" << endl;
        return -1;
    }

    HIDS camera_id = atoi(argv[1]);
    const char* path = argv[2];

    INT status = is_InitCamera(&camera_id, NULL);
    if (status != IS_SUCCESS) {
        cerr << "Camera failed to initialize." << endl;
        return -1;
    }

    cout << "Camera ID: " << camera_id << endl;

    // is_EnableAutoExit(camera_id, IS_ENABLE_AUTO_EXIT);

    int mem_id;
    char* mem;

    INT size_x = 3840;
    INT size_y = 2748;

    if (is_AllocImageMem(camera_id, size_x, size_y, 32, &mem, &mem_id)
            != IS_SUCCESS) {
        cerr << "Error in allocating image memory." << endl;
        return -1;
    }

    if (is_SetImageMem(camera_id, mem, mem_id) != IS_SUCCESS) {
        cerr << "Error in setting image memory." << endl;
        return -1;
    }

    if (is_SetColorMode(camera_id, IS_SET_CM_RGB32) != IS_SUCCESS) {
        cerr << "Failed to set color mode." << endl;
        return -1;
    }

    if (is_SetPixelClock(camera_id, 15) != IS_SUCCESS) {
        char* ptr;
        is_GetError(camera_id, NULL, &ptr);
        cerr << "Error in setting pixel clock." << endl;
        cerr << ptr << endl;
        return -1;
    }

    if (is_SetDisplayMode(camera_id, IS_SET_DM_DIB) != IS_SUCCESS) {
        INT mode = is_SetDisplayMode(camera_id, IS_GET_DISPLAY_MODE);
        if (mode != IS_SET_DM_DIB) {
            char* ptr;
            is_GetError(camera_id, NULL, &ptr);
            cerr << "Error in setting display mode." << endl;
            cerr << ptr << endl;
            cerr << mode << endl;
        }
    }

    is_SetExternalTrigger(camera_id, IS_SET_TRIGGER_SOFTWARE);
     
    if (is_FreezeVideo(camera_id, IS_WAIT) != IS_SUCCESS) {
        cerr << "Error in freezing a frame." << endl;
        return -1;
    }

    if (is_SaveImageMemEx(camera_id, path, mem, mem_id, IS_IMG_BMP, 75) != IS_SUCCESS) {
        cerr << "Error in saving frame. " << endl;
        return -1;
    }

    is_CameraStatus(camera_id, IS_LAST_CAPTURE_ERROR, IS_GET_STATUS);

    is_FreeImageMem(camera_id, mem, mem_id);
    is_ExitCamera(camera_id);

    return 0;
}
