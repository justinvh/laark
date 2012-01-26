#include <iostream>
#include <string>
#include <sstream>
#include <locale>

#include <ctime>

#include <boost/program_options.hpp>

#include <zmq.hpp>

using namespace std;
namespace po = boost::program_options;

// A more succint way of computing time deltas
// http://stackoverflow.com/a/380446
int64_t timespecDiff(const timespec& timeA_p, const timespec& timeB_p)
{
    return ((timeA_p.tv_sec * 1000000000) + timeA_p.tv_nsec) -
        ((timeB_p.tv_sec * 1000000000) + timeB_p.tv_nsec);
}

int main(int argc, char** argv)
{
    po::options_description options("Options");

    options.add_options()
        ("help", "create help message")
        ("width", po::value<int>(), 
         "width of image to simulate. Default: 500")
        ("height", po::value<int>(), 
         "height of image to simulate. Default: 800")
        ("bits", po::value<int>(), 
         "bit size of image to simulate. Default: 32")
        ("port", po::value<int>(), 
         "port to serve on. Set PORT in env alternatively. Default: 9876")
        ("fps", po::value<int>(),
         "fps of the camera. Defaults to 60")
        ("header", po::value<string>(),
         "set the first n-bytes to this string. Default: 0xdeadbeef")
        ("socket", po::value<string>(),
         "Compatible peer socket type. This includes: DEALER, REQ, REP, PUB, PUSH, and PAIR. Default: PUSH")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << options << "\n";
        return 1;
    }

    int port = 9876;
    int fps = 60;
    int width = 800;
    int height = 500;
    int bits = 32;
    int socket_type = ZMQ_PUSH;
    string header = "0xdeadbeef";
    string socket_str = "PUSH";

    if (vm.count("width")) width = vm["width"].as<int>();
    if (vm.count("height")) height = vm["height"].as<int>();
    if (vm.count("bits")) bits = vm["bits"].as<int>();
    if (vm.count("fps")) fps = vm["fps"].as<int>();
    if (vm.count("header")) header = vm["header"].as<string>();

    // Figure out the socket we are using if provided.
    if (vm.count("socket")) {
        const string& s = vm["socket"].as<string>();
        if (s == "DEALER") socket_type = ZMQ_DEALER;
        else if (s == "REQ") socket_type = ZMQ_REQ;
        else if (s == "REP") socket_type = ZMQ_REP;
        else if (s == "PUB") socket_type = ZMQ_PUB;
        else if (s == "PUSH") socket_type = ZMQ_PUSH;
        else if (s == "PAIR") socket_type = ZMQ_PAIR;
        else {
            cout << options << endl;
            return 1;
        }
        socket_str = s;
    }
    
    // Read the environment variable if port is not set. Otherwise just
    // default the port to 9876.
    if (vm.count("port")) {
        port = vm["port"].as<int>();
    } else {
        char* port_str = getenv("PORT");
        if (port_str) {
            cout << "Port is being set from $PORT" << endl;
            port = atoi(port_str);
        }
    }

    stringstream ss;
    ss << "tcp://*:" << port;
    const string& socket_binding = ss.str();
    const int size = bits * width * height;
    double fps_time = 1/((double)fps) * 1000;

    cout.imbue(locale(""));

    cout << "Benchmark Profile:"
         << "\n\tImage dimensions: " << width << "x" << height << "x" << bits
         << "\n\tImage size: " << size / 1024 << "kb"
         << "\n\tHeader: " << header << " (first " << header.size() << " bytes)"
         << "\n\tFPS: " << fps << " (or " << fps_time << " ms per frame)"
         << "\n\tCompat Peer Socket: ZMQ_" << socket_str
         << "\n\tServing: " << socket_binding
         << endl;

    zmq::context_t context(1);
    zmq::socket_t socket(context, socket_type);
    socket.bind(socket_binding.c_str());
    int packet = 1;
    while (true) {
        timespec start, end;
        uint64_t diff0, diff1, diff2;
        static bool valid_time[3] = { false };

        zmq::message_t reply(size);

        clock_gettime(CLOCK_MONOTONIC, &start);
        char* msg = new char[size];
        memcpy(msg, header.c_str(), header.size());
        usleep(fps_time);
        clock_gettime(CLOCK_MONOTONIC, &end);

        diff0 = timespecDiff(end, start);
        valid_time[0] = diff0 > 0;

        clock_gettime(CLOCK_MONOTONIC, &start);
        memcpy((void*)reply.data(), msg, size);
        clock_gettime(CLOCK_MONOTONIC, &end);

        diff1 = timespecDiff(end, start);
        valid_time[1] = diff1 > 0;


        clock_gettime(CLOCK_MONOTONIC, &start);
        socket.send(reply);;
        clock_gettime(CLOCK_MONOTONIC, &end);

        diff2 = timespecDiff(end, start);
        valid_time[2] = diff2 > 0;

        delete[] msg;

        if (valid_time[0] && valid_time[1] && valid_time[2]) {
            cout << "Push: " << packet << endl;
            cout << "Allocating     : " << diff0 << "ms" << endl;
            cout << "Copying Header : " << diff1 << "ms" << endl;
            cout << "Sending Data   : " << diff2 << "ms\n" << endl;
            packet++;
            valid_time[0] = valid_time[1] = valid_time[2] = false;
        }
    }
}
