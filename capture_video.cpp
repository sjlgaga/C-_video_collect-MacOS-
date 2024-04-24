#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <thread>
#include <functional>
#include <mutex>
#include <cstring>
#include <queue>
#define N 10000

using namespace std;
using namespace cv;
using namespace std::chrono;
using namespace std::__fs::filesystem;

class MyNode
{
    public:
    Mat frame;
    double timestamp;
    MyNode(Mat a,double b):frame(a),timestamp(b){};
};
bool sync = false;
queue<MyNode> q;
mutex mylock;

//producer
void data_collect(time_point<system_clock> start,int id,double fps,double collect_time)
{
    VideoCapture cap(id);
    if (!cap.isOpened()) {
        cerr << "ERROR: Unable to open the camera" << endl;
    }

    cap.set(CAP_PROP_FPS, fps);
    double actualFPS = cap.get(CAP_PROP_FPS);
    cout << "Requested FPS: " << fps << endl;
    cout << "Actual FPS set by camera: " << actualFPS << endl;

    sync = true;
    Mat frame;
    int sum=0;
    while (true) {
        cap >> frame;
        if (frame.empty()) {
            cerr << "ERROR: Unable to grab from the camera" << endl;
            break;
        }

        if ((waitKey(1) & 0xFF) == 'q') {
            break;
        }

        auto end = system_clock::now();
        duration<double> timestamp = end.time_since_epoch();
        duration<double> elapsed = end - start;
        if (elapsed.count() >= collect_time) {
            break;
        }

        sum++;
        if (q.size()<N)
        {
            mylock.lock();
            q.push(MyNode(frame,timestamp.count()));
            mylock.unlock();
        }
            
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }
    cout<<sum<<endl;
    cap.release();
}

//consumer
void data_write(time_point<system_clock> start,double collect_time)
{
    
    time_t now_c = system_clock::to_time_t(start);
    tm now_tm = *localtime(&now_c);
    stringstream ss;
    ss << put_time(&now_tm, "%Y-%m-%d-%H-%M-%S");
    string foldername = ss.str();
    if (!create_directory(foldername)) {
        cerr << "Error: Failed to create directory!" << std::endl;
    }

    //wait the camera to be initialized
    while(!sync);

    while(true)
    {
        if (!q.empty())
        {
            mylock.lock();
            auto node = q.front();
            q.pop();
            mylock.unlock();

            string filename = to_string(node.timestamp)+".jpg";
            string filepath = "./"+foldername+"/"+filename;
            if (node.frame.empty())
            {
                cout<<node.timestamp<<endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            if (!imwrite(filepath, node.frame)) 
            {
                cerr << "ERROR: Unable to save the image." << endl;
                break;
            }
        }
        else
        {
            auto end = system_clock::now();
            duration<double> timestamp = end.time_since_epoch();
            duration<double> elapsed = end - start;
            if (elapsed.count() >= collect_time) {
                break;
            }
            else
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
    }
}
int main(int argc, char* argv[]) {
    
    if (argc != 4) {
        std::cerr << "Error: Incorrect number of arguments.\n";
        std::cerr << "Usage: " << argv[0] << " <camera_id> <fps_set> <duration_time(seconds)>\n";
        return 1;
    }
    int camera_id = stoi(argv[1]);
    double fps_set = stod(argv[2]);
    double duration_time = stod(argv[3]);

    cout << "Start capturing and writing video" << endl;

    auto start = system_clock::now();

    thread collect(data_collect,start,camera_id,fps_set,duration_time);
    thread write(data_write,start,duration_time);

    collect.join();
    write.join();

    destroyAllWindows();

    return 0;
}
