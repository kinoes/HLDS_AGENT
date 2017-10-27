#pragma once 
#include <opencv2/opencv.hpp>
#include <deque>
#include <vector>
#include <thread>
#include <mutex>

#include "xthread.h"
#include "tof.h"

#define MAX_TRACKS              (100)           //Max points in a track
#define COUNT_NO    (-1)    //Not counted
#define COUNT_UP    (0)     //Move to up direction
#define COUNT_RIGHT (1)     //Move to right direction
#define COUNT_DOWN  (2)     //Move to down direction
#define COUNT_LEFT  (3)     //Move to left direction


using namespace std;
using namespace hlds;
using namespace cv;

//Human count
struct {
    int Enter[4];           //Human count who enter to the area from each direction(Use COUNT_XXX macro)
    int Exit[4];            //Human count who exit from the area to each direction(Use COUNT_XXX macro)
    int TotalEnter;         //Total number of humans entering
    int TotalExit;          //Total number of humans exiting
    int InArea;             //Total number of humans in count area

    //Count area(Counted when a human exits from the area)
    struct {
        float left_x;
        float top_y;
        float right_x;
        float bottom_y;
    } Square;
} Count;

//Human information
struct AppHuman {

    long id;                    //Human ID managed in HumanDetect function of SDK
    int appid;                  //Human ID managed in application(HumanCounter.cpp)
    bool bEnable;               //false : Candidate(Not recognized as a human yet)
    HumanStatus status;         //Status

    float x;                    //X-coordinate of center of gravity of the human
    float y;                    //Y-coordinate of center of gravity of the human
    float prex;                 //Previous X-coordinate of center of gravity of the human
    float prey;                 //Previous Y-coordinate of center of gravity of the human
    float direction;            //Direction of body(0 degree to 359 degree) --> Positive direction of Y-axis is 0 degree, and clockwise
    float headheight;           //Head height from floor(mm)
    float handheight;           //Hand height from floor(mm)(bHand == true)

    //Track data(Ring queue)
    struct {
        float x;                //X-cordinate of one point in the track
        float y;                //Y-cordinate of one point in the track
    } track[MAX_TRACKS];
    int nexttrack;              //Next inserting point in ring buffer
    int trackcnt;               //Number of points in the track

    int enterdir;               //Direction entering to the count area(COUNT_XXX macro)
    int exitdir;                //Direction exiting from the count area(COUNT_XXX macro)
};


class hlds_agent : public xthread
{
    public:
        hlds_agent();
        virtual ~hlds_agent();
        int InitAgent();
        void SetThreadCount(int index);
        Tof* GetTofInstance() { return m_Tof; }
    private:
        int m_index;
        float m_angle_x;              //Angle to rotate around X-axis(degree)
        float m_angle_y;               //Angle to rotate around Y-axis(degree)
        float m_angle_z;               //Angle to rotate around Z-axis(degree)
        float m_height;             //Height from floor(mm)
        float m_dx;                  //Shift length to x-axis positive direction(mm)
        float m_dy;                  //Shift length to y-axis positive direction(mm)
        float m_zoom;                 //Zoom ratio
        bool m_bPoint;                 //Mode to display points
        bool m_bBack;                 //Mode to display footprint on background
        bool m_bCount;                 //Mode to display human count
        bool m_bBoxShift;              //true: shift, false: change size in count area setting mode
        bool m_bSubDisplay;            //Mode to display sub display

        Tof *m_Tof = nullptr;
        TofInfo testm_TofInfo;
        
        //Create instances for reading frames
        FrameDepth m_frame;
        //Create instances for 3D data after conversion
        Frame3d m_frame3d;
        //Create instances for reading human frames
        FrameHumans m_framehumans;
        vector<AppHuman> m_apphumans;
        int m_apphumanid;

        //Display image
        Mat m_img;
        //Background image
        Mat m_back;

        bool LoadIniFile(void);
        void CatchHumans(FrameHumans *pframehumans);
        bool ChangeAttribute(Tof* tof, float x, float y, float z, float rx, float ry, float rz);
        void InitializeHumans(void);
        virtual int Proc();
};
