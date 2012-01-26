#include <iostream>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <ueye.h>
#include <zmq.hpp>
#include <unistd.h>

using namespace std;

int FORMAT = 9;

int main(int argc, char** argv)
{
    if (argc != 2) {
        cerr << "save_image <camera_id>" << endl;
        return -1;
    }

    HIDS camera_id = atoi(argv[1]);

    std::cout << "Initializing camera: " << camera_id << std::endl;

    INT status = is_InitCamera(&camera_id, NULL);
    if (status != IS_SUCCESS) {
        cerr << "Camera failed to initialize." << endl;
        return -1;
    }

    cout << "Camera ID: " << camera_id << endl;

    //  Prepare our context and socket
    cout << "Preparing ZMQ" << endl;
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);
    socket.bind ("tcp://*:5555");
    
    // Get number of available formats and size of list
    UINT count;
    UINT bytesNeeded = sizeof(IMAGE_FORMAT_LIST);
    INT nRet = is_ImageFormat(camera_id, IMGFRMT_CMD_GET_NUM_ENTRIES, &count, 4);
    bytesNeeded += (count - 1) * sizeof(IMAGE_FORMAT_INFO);
    void* ptr = malloc(bytesNeeded);
     
    // Create and fill list
    IMAGE_FORMAT_LIST* pformatList = (IMAGE_FORMAT_LIST*) ptr;
    pformatList->nSizeOfListEntry = sizeof(IMAGE_FORMAT_INFO);
    pformatList->nNumListElements = count;
    nRet = is_ImageFormat(camera_id, IMGFRMT_CMD_GET_LIST, pformatList, bytesNeeded);
     
    // Activate trigger mode for capturing high resolution images (USB uEye XS)
    nRet = is_StopLiveVideo(camera_id, IS_WAIT);
    nRet = is_SetExternalTrigger(camera_id, IS_SET_TRIGGER_SOFTWARE);

    IMAGE_FORMAT_INFO formatInfo;
    for (UINT i = 0; i < count; i++) {
        if (pformatList->FormatInfo[i].nFormatID == FORMAT) {
            formatInfo = pformatList->FormatInfo[i];
            break;
        }
    }

    if (is_ImageFormat(camera_id, IMGFRMT_CMD_SET_FORMAT, &formatInfo.nFormatID, 4) != IS_SUCCESS) {
        char* ptr;
        is_GetError(camera_id, NULL, &ptr);
        cerr << "Error in setting display mode." << endl;
        cerr << ptr << endl;
        return -1;
    }

    int mem_id;
    char* mem;

    INT size_x = formatInfo.nWidth;
    INT size_y = formatInfo.nHeight;
    INT depth = 24;
    int send_size = size_x * size_y * depth;

    std::cout << "Camera is operating at " << formatInfo.strFormatName << std::endl;

    if (is_AllocImageMem(camera_id, size_x, size_y, depth, &mem, &mem_id)
            != IS_SUCCESS) {
        cerr << "Error in allocating image memory." << endl;
        return -1;
    }

    if (is_SetImageMem(camera_id, mem, mem_id) != IS_SUCCESS) {
        cerr << "Error in setting image memory." << endl;
        return -1;
    }

    std::cout << "Preparing for data" << std::endl;
    while (true) {
        zmq::message_t request;
        //  Wait for next request from client
        std::cout << "====== Waiting for command =====" << std::endl;
        socket.recv (&request);
        char* data = (char*)request.data();
        data[request.size()] = '\0';
        std::cout << "Received Command: " << data << std::endl;
        if (strcmp(data, "snap") == 0) {
            std::cout << "======= Client wants to snap ======" << std::endl;
            zmq::message_t reply (send_size);

            std::cout << "Freezing frame" << std::endl;
            if (is_FreezeVideo(camera_id, IS_WAIT) != IS_SUCCESS) {
                cerr << "Error in freezing a frame." << endl;
                return -1;
            }

            //  Send reply back to client
            std::cout << "Saving image to data" << std::endl;
            is_CopyImageMem(camera_id, mem, mem_id, (char*)reply.data());

            socket.send (reply);
            std::cout << "Data sent: " << send_size << std::endl;
        } else if (strcmp(data, "exit") == 0) {
            break;
        } else {
            std::cout << "Unknown command: " << data << std::endl;
        }
    }
    is_FreeImageMem(camera_id, mem, mem_id);
    is_ExitCamera(camera_id);

    return 0;
}
