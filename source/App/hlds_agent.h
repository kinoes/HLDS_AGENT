#pragma once 
#include <opencv2/opencv.hpp>
#include <deque>
#include <vector>
#include <thread>
#include <mutex>

#include "xthread.h"
#include "tof.h"
#include "send_packet.h"

#define MAX_TRACKS              (100)           //Max points in a track
#define COUNT_NO    (-1)    //Not counted
#define COUNT_UP    (0)     //Move to up direction
#define COUNT_RIGHT (1)     //Move to right direction
#define COUNT_DOWN  (2)     //Move to down direction
#define COUNT_LEFT  (3)     //Move to left direction



//sub display
#define SUB_DISPLAY_X           (10)            //X-coordinate of upper left of sub display
#define SUB_DISPLAY_Y           (710)           //Y-coordinate of upper left of sub display
#define SUB_DISPLAY_WIDTH       (320)           //Width of sub display
#define SUB_DISPLAY_HEIGHT      (240)           //Height of sub display



//Parameter to display human
#define HUMAN_CURSOR_SIZE       (600.0f)        //Size of human cursor(Length of one side from L to L)
#define MAX_TRACKS              (100)           //Max points in a track
//
//Hand height display
#define HAND_INDICATOR_MIN      (0)             //min.
#define HAND_INDICATOR_MAX      (2000)          //max.

#if defined (_WIN64) || defined(_WIN32)
#define M_PI (3.14159265358979) //Pi
#endif
#define deg2rad(d) ( (d) / 180.0 * M_PI )   //Macro function to convert deg to rad



using namespace std;
using namespace hlds;
using namespace cv;

//Human count

struct Count{
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
};


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


typedef struct __TofEvent {
	char	wszIp[20];				//tof ip
	int		inout;					//피플카운트 입(1)출(2)확인/위험구역 출입(2)해제(3) 확인
	time_t  time;				    //이벤트 감지 시간
	char	wszDeviceID[10];		//장치 ID
	char	wszAreaID[10];			//구역 ID
	char	wszAreaName[256];	//구역명
	int		direction;
}TofEvent, *PTofEvent;


class hlds_agent : public xthread
{
    public:
        hlds_agent();
		void SetDeviceInfo(DEVICEINFO device);	
		virtual ~hlds_agent();
        int InitAgent();
        void SetThreadCount(int index);
        Tof* GetTofInstance() { return m_Tof; }
		TofInfo GETTofInfo(){return m_TofInfo;}
		bool CompareDeviceInfo(DEVICEINFO data);
		void SetRunFlag(bool flag);
		bool GetRunFlag(){return m_agent_run_flag;}
	private:
        mutex  m_agent_lock;
		int m_agent_run_flag;
		int m_index;
        uint8_t m_stream_status;			// 0 : off  1: on
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
		Count m_Count;
		bool thread_flag;
        Tof *m_Tof = nullptr;
        TofInfo m_TofInfo;
        
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
		Mat m_subdisplay;
		//Z-buffer(to understand before and behind)
		float m_z_buffer[640 * 2][480 * 2];
		DEVICEINFO m_deviceinfo;
		void StatusInfo();
		bool LoadIniFile(void);
        void CatchHumans(FrameHumans *pframehumans);
        bool ChangeAttribute(Tof* tof, float x, float y, float z, float rx, float ry, float rz);
        void InitializeHumans(void);
		void DrawHumans(void);
		void CountHumans(void);
		int CountDirection(float x, float y);
		bool InCountArea(float x, float y);
		void DrawCount();
		virtual int Proc();
};
