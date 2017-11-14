#include<iostream>
#include "agent_broker.h"
#include "hlds_agent.h"

using namespace std;

hlds_agent::hlds_agent()
{
	m_angle_x = 50.0f;              //Angle to rotate around X-axis(degree)
	m_angle_y = 0.0f;               //Angle to rotate around Y-axis(degree)
	m_angle_z = 0.0f;               //Angle to rotate around Z-axis(degree)
	m_height = 2700.0f;             //Height from floor(mm)
	m_dx = 648.0f;                  //Shift length to x-axis positive direction(mm)
	m_dy = 966.0f;                  //Shift length to y-axis positive direction(mm)
	m_zoom = 0.12f;                 //Zoom ratio
	m_bPoint = true;                //Mode to display points
	m_bBack = false;                //Mode to display footprint on background
	m_bCount = true;                //Mode to display human count
	m_bBoxShift = true;             //true: shift, false: change size in count area setting mode
	m_bSubDisplay = true;           //Mode to display sub display
	m_apphumanid = 0;
	m_stream_status = 0;			//STREAM OFF DEFAULT
	thread_flag = false;
	m_agent_run_flag = true; // for test
	m_Tof = new Tof;  
}


hlds_agent::~hlds_agent()
{
	m_Tof->Stop();
	m_Tof->Close();
	delete m_Tof;
}


void hlds_agent::SetDeviceInfo(DEVICEINFO device)
{
	m_deviceinfo = device;
	InitAgent();
}

void hlds_agent::SetThreadCount(int index)
{
    m_index = index;
}

bool hlds_agent:: ChangeAttribute(Tof* tof, float x, float y, float z, float rx, float ry, float rz)
{
    if (rx < 0) rx += 360;
    if (ry < 0) ry += 360;
    if (rz < 0) rz += 360;

    if (tof->SetAttribute(x, y, z, rx, ry, rz) != Result::OK){
        return false;
    }
    return true;
}


void hlds_agent::InitializeHumans(void)
{
    //Initialize humans in application
    m_apphumans.clear();
    //Human ID managed in application
    m_apphumanid = 0;
}


//Load ini file
bool hlds_agent::LoadIniFile(void)
{
    int  ret = 0;
	// to do load select database hlds_agent status //	
    return true;
}


int hlds_agent::InitAgent()
{
	
	//Initialize human counter
    memset(&m_Count, 0, sizeof(m_Count));
    m_Count.Square.left_x = -3235;
    m_Count.Square.top_y = -3219;
    m_Count.Square.right_x = 3263;
    m_Count.Square.bottom_y = -2937;
	
	//Initialize human counter
	m_TofInfo.tofid = m_deviceinfo.device_id;
	m_TofInfo.tofmac = m_deviceinfo.mac_address;
	m_TofInfo.tofip = m_deviceinfo.ip;
	m_TofInfo.rtp_port = 0;

	if(m_Tof->Open(m_TofInfo) != Result::OK){ 
    std::cout << "TOF ID " << m_TofInfo.tofid << " Open Error" << endl;
        return -1;
    }

	if(m_Tof->SetFrameRate(FrameRate::fr16fps) != Result::OK){
		cout<<"FRAME ERROR"<<endl;
		return -1;
	}
    
	//Set camera mode as Depth mode
    if (m_Tof->SetCameraMode(CameraMode::CameraModeDepth) != Result::OK){
        std::cout << "TOF ID " << m_Tof->tofinfo.tofid << " Set Camera Mode Error" << endl;
        return -1;
    }

    //Set camera pixel
    if (m_Tof->SetCameraPixel(CameraPixel::w320h240) != Result::OK){
        std::cout << "TOF ID " << m_Tof->tofinfo.tofid << " Set Camera Pixel Error" << endl;
        return -1;
    }
 
       //Set TOF sensor angle and height
    if (ChangeAttribute(m_Tof, 0, 0, m_height * -1, m_angle_x, m_angle_y, m_angle_z) == false){
        std::cout << "TOF ID " << m_Tof->tofinfo.tofid << " Set Camera Position Error" << endl;
        return -1;
    }

    //Noise reduction(Low signal cutoff)
    if (m_Tof->SetLowSignalCutoff(20) != Result::OK){
        std::cout << "TOF ID " << m_Tof->tofinfo.tofid << " Low Signal Cutoff Error" << endl;
        return -1;
    }

    //Edge noise reduction
    if (m_Tof->SetEdgeSignalCutoff(EdgeSignalCutoff::Enable) != Result::OK){
        std::cout << "TOF ID " << m_Tof->tofinfo.tofid << " Edge Noise Reduction Error" << endl;
        return -1;
    }

    //Start human detection
    if (m_Tof->Run(RunMode::HumanDetect) != Result::OK){
	std::cout << "TOF ID " << m_Tof->tofinfo.tofid << " Run Error" << endl;
        return -1;
    }
    std::cout << "TOF ID " << m_Tof->tofinfo.tofid << " Run OK" << endl;
    m_frame.CreateColorTable(0, 65530);

    //Initialize human information
    InitializeHumans();
    
	//Initialize background
	m_back = Mat::zeros(480*2, 640*2, CV_8UC3);
	return 0;
}
//Catch humans detected by Human Detect function in SDK
void hlds_agent::CatchHumans(FrameHumans *pframehumans)
{
      //Reset relation between humans managed in application and humans detected by SDK
    for (unsigned int ahno = 0; ahno < m_apphumans.size(); ahno++){
        m_apphumans[ahno].bEnable = false;
    }

    //Assign humans managed in application and humans detected by SDK
    for (int hno = 0; hno < pframehumans->numofhuman; hno++){

        bool bAssigned = false;
        for (unsigned int ahno = 0; ahno < m_apphumans.size(); ahno++){

            if (m_apphumans[ahno].id == pframehumans->humans[hno].id){
                //Found the current human --> assign

                m_apphumans[ahno].bEnable = true;
                m_apphumans[ahno].prex = m_apphumans[ahno].x;
                m_apphumans[ahno].prey = m_apphumans[ahno].y;
                m_apphumans[ahno].x = pframehumans->humans[hno].x;
                m_apphumans[ahno].y = pframehumans->humans[hno].y;
                m_apphumans[ahno].direction = pframehumans->humans[hno].direction;
                m_apphumans[ahno].headheight = pframehumans->humans[hno].headheight;
                m_apphumans[ahno].handheight = pframehumans->humans[hno].handheight;
               //Status
                HumanStatus status = pframehumans->humans[hno].status;

                //Update status
                m_apphumans[ahno].status = status;

                //Register to tracking data(ring queue)
                m_apphumans[ahno].track[m_apphumans[ahno].nexttrack].x = m_apphumans[ahno].prex;
                m_apphumans[ahno].track[m_apphumans[ahno].nexttrack].y = m_apphumans[ahno].prey;
                m_apphumans[ahno].nexttrack++;
                if (m_apphumans[ahno].nexttrack == MAX_TRACKS){
                    m_apphumans[ahno].nexttrack = 0;
                }
                m_apphumans[ahno].trackcnt++;
                if (m_apphumans[ahno].trackcnt > MAX_TRACKS){
                    m_apphumans[ahno].trackcnt = MAX_TRACKS;
                }

                bAssigned = true;
                break;
            }
        }
            if (!bAssigned){
            //No corresponded human managed in application (New human)

            //Make new human information managed in application
            AppHuman ah;
            memset(&ah, 0, sizeof(ah));
            ah.bEnable = true;
            ah.id = pframehumans->humans[hno].id;
            ah.status = HumanStatus::Walk;
            ah.x = pframehumans->humans[hno].x;
            ah.y = pframehumans->humans[hno].y;
            ah.prex = ah.x;
            ah.prey = ah.y;
            ah.direction = pframehumans->humans[hno].direction;
            ah.headheight = pframehumans->humans[hno].headheight;
            ah.handheight = pframehumans->humans[hno].handheight;
            ah.enterdir = COUNT_NO;
            ah.exitdir = COUNT_NO;
            m_apphumans.push_back(ah);
        }
    }
    //Delete human managed in application who was not assigned
    for (int ahno = m_apphumans.size() - 1; ahno >= 0; ahno--){
        if (m_apphumans[ahno].bEnable == false){
            //No assigend
            m_apphumans.erase(m_apphumans.begin() + ahno);
        }
    }
}


int hlds_agent::CountDirection(float x, float y)
{

    //Linear equation from upper left to lower right of count area：y = a1 * x + b1
    float a1 = (m_Count.Square.bottom_y - m_Count.Square.top_y) / (m_Count.Square.right_x - m_Count.Square.left_x);
    float b1 = m_Count.Square.top_y - a1 * m_Count.Square.left_x;
    float y1 = a1 * x + b1;

    //Linear equation from lower left to upper right of count area：y = a2 * x + b2
    float a2 = (m_Count.Square.bottom_y - m_Count.Square.top_y) / (m_Count.Square.left_x - m_Count.Square.right_x);
    float b2 = m_Count.Square.top_y - a2 * m_Count.Square.right_x;
    float y2 = a2 * x + b2;

    int dir = COUNT_UP;

    if ((y <= y1) && (y <= y2)){
        dir = COUNT_UP;
    }
    else if ((y >= y1) && (y >= y2)){
        dir = COUNT_DOWN;
    }
    else if ((y >= y1) && (y <= y2)){
        dir = COUNT_LEFT;
    }
    else if ((y <= y1) && (y >= y2)){
        dir = COUNT_RIGHT;
    }
    return dir;
}

bool hlds_agent::InCountArea(float x, float y)
{
    bool ret = false;
    if ((x >= m_Count.Square.left_x) && (x <= m_Count.Square.right_x) &&
        (y >= m_Count.Square.top_y) && (y <= m_Count.Square.bottom_y)){
        ret = true;
    }
    return ret;
}


void hlds_agent::CountHumans(void)
{
	m_Count.InArea = 0;
	for (unsigned int ahno = 0; ahno < m_apphumans.size(); ahno++){

		if (InCountArea(m_apphumans[ahno].x, m_apphumans[ahno].y)){
			//In count area
			//countup
			m_Count.InArea++;

			if (!InCountArea(m_apphumans[ahno].prex, m_apphumans[ahno].prey)){
				//It was outside last time(Outside to inside)

				if (m_apphumans[ahno].enterdir != COUNT_NO){
					//Cancel previous entering count if already counted
					m_Count.Enter[m_apphumans[ahno].enterdir]--;
				}

				//Entering direction
				int dir = CountDirection(m_apphumans[ahno].prex, m_apphumans[ahno].prey);

				//countup
				m_Count.Enter[dir]++;

				//Register countup
				m_apphumans[ahno].enterdir = dir;
			}
		}
		else {
			//In outside of count area
			if (InCountArea(m_apphumans[ahno].prex, m_apphumans[ahno].prey)){
				//It was inside last time(Inside to outside)

				if (m_apphumans[ahno].exitdir != COUNT_NO){
					//Cancel previous exiting count if already counted
					m_Count.Exit[m_apphumans[ahno].exitdir]--;
				}

				//Exiting direction
				int dir = CountDirection(m_apphumans[ahno].x, m_apphumans[ahno].y);

				//countup
				m_Count.Exit[dir]++;

				//Register countup
				m_apphumans[ahno].exitdir = dir;
			}
		}
	}

	//Total number of human
	m_Count.TotalEnter = 0;
	m_Count.TotalExit = 0;
	
	if( m_Count.TotalEnter != 0)
	{
		printf("TotalEnter :%d TotalExit:%d\n", m_Count.TotalEnter, m_Count.TotalExit);
	}
	for (int dir = 0; dir < 4; dir++){
		m_Count.TotalEnter += m_Count.Enter[dir];
		m_Count.TotalExit += m_Count.Exit[dir];
	}
}


void hlds_agent::DrawHumans(void)
{
	//11 colors if different colors are assigned for each human
	cv::Scalar color[11] = { cv::Scalar(0, 0, 255),
		cv::Scalar(0, 255, 0),
		cv::Scalar(0, 255, 255),
		cv::Scalar(255, 0, 0),
		cv::Scalar(255, 0, 255),
		cv::Scalar(255, 255, 0),
		cv::Scalar(0, 127, 255),
		cv::Scalar(0, 255, 127),
		cv::Scalar(255, 0, 127),
		cv::Scalar(255, 127, 0),
		cv::Scalar(127, 0, 255),
	};
	//Draw each human
	for (unsigned int ahno = 0; ahno < m_apphumans.size(); ahno++){

		//Colors
		cv::Scalar backcolor = cv::Scalar(255, 255, 255);       //Footprint on background : White
		cv::Scalar footcolor = cv::Scalar(255, 255, 0);         //Tracking line : Light blue
		cv::Scalar color_el = cv::Scalar(0, 255, 0);            //L of human cursor : Yellow-green
		cv::Scalar color_plus = cv::Scalar(0, 255, 255);        //Circle of human cursor : Yellow

#ifdef HUMAN_COLOR
		//Change color depending on ID
		cv::Scalar idcolor = color[m_apphumans[ahno].id % 11];    //Different color for each ID
		backcolor = idcolor;
		footcolor = idcolor;
		color_el = idcolor;
#endif //HUMAN_COLOR

#ifndef NO_FOOTPRINT
		//Draw footprint on background
		cv::line(m_back, cv::Point((int)(m_apphumans[ahno].prex * m_zoom + m_dx), (int)(m_apphumans[ahno].prey * m_zoom + m_dy)),
				cv::Point((int)(m_apphumans[ahno].x * m_zoom + m_dx), (int)(m_apphumans[ahno].y * m_zoom + m_dy)), backcolor, 1, CV_AA, 0);

		//Draw tracking line
		int tno = 0;
		if (m_apphumans[ahno].trackcnt == MAX_TRACKS){
			tno = m_apphumans[ahno].nexttrack;
		}
		int prex;
		int prey;
		for (int tcnt = 0; tcnt < m_apphumans[ahno].trackcnt; tcnt++){
			int x = (int)(m_apphumans[ahno].track[tno].x * m_zoom + m_dx);
			int y = (int)(m_apphumans[ahno].track[tno].y * m_zoom + m_dy);
			if (tcnt > 0){
				cv::line(m_img, cv::Point(prex, prey), cv::Point(x, y), footcolor, 2, CV_AA, 0);
			}
			prex = x;
			prey = y;

			tno++;
			if (tno == MAX_TRACKS){
				tno = 0;
			}
		}
#endif  //NO_FOOTPRINT
#ifndef NO_HUMAN_CURSOR
		//Draw human cursor
		int hx = (int)(m_apphumans[ahno].x * m_zoom + m_dx);
		int hy = (int)(m_apphumans[ahno].y * m_zoom + m_dy);
		cv::Point point1;
		cv::Point point2;

		if ((m_apphumans[ahno].status == HumanStatus::Crouch) || (m_apphumans[ahno].status == HumanStatus::CrouchHand)){
			//Crouching
			color_plus = cv::Scalar(0, 128, 255);   //Orange
		}

		//L of upper left
		point1.x = hx - (int)(HUMAN_CURSOR_SIZE * m_zoom / 2);
		point1.y = hy - (int)(HUMAN_CURSOR_SIZE * m_zoom / 2);
		point2.x = point1.x + (int)(HUMAN_CURSOR_SIZE * m_zoom / 3);
		point2.y = point1.y;
		cv::line(m_img, point1, point2, color_el, 2, CV_AA, 0);


		point2.x = point1.x;
		point2.y = point1.y + (int)(HUMAN_CURSOR_SIZE * m_zoom / 3);
		cv::line(m_img, point1, point2, color_el, 2, CV_AA, 0);

		//L of upper right
		point1.x = hx + (int)(HUMAN_CURSOR_SIZE * m_zoom / 2);
		point1.y = hy - (int)(HUMAN_CURSOR_SIZE * m_zoom / 2);
		point2.x = point1.x - (int)(HUMAN_CURSOR_SIZE * m_zoom / 3);
		point2.y = point1.y;
		cv::line(m_img, point1, point2, color_el, 2, CV_AA, 0);

		point2.x = point1.x;
		point2.y = point1.y + (int)(HUMAN_CURSOR_SIZE * m_zoom / 3);
		cv::line(m_img, point1, point2, color_el, 2, CV_AA, 0);

		//L of lower right
		point1.x = hx + (int)(HUMAN_CURSOR_SIZE * m_zoom / 2);
		point1.y = hy + (int)(HUMAN_CURSOR_SIZE * m_zoom / 2);
		point2.x = point1.x - (int)(HUMAN_CURSOR_SIZE * m_zoom / 3);
		point2.y = point1.y;
		cv::line(m_img, point1, point2, color_el, 2, CV_AA, 0);

		point2.x = point1.x;
		point2.y = point1.y - (int)(HUMAN_CURSOR_SIZE * m_zoom / 3);
		cv::line(m_img, point1, point2, color_el, 2, CV_AA, 0);

		//L of lower left
		point1.x = hx - (int)(HUMAN_CURSOR_SIZE * m_zoom / 2);
		point1.y = hy + (int)(HUMAN_CURSOR_SIZE * m_zoom / 2);
		point2.x = point1.x + (int)(HUMAN_CURSOR_SIZE * m_zoom / 3);
		point2.y = point1.y;
		cv::line(m_img, point1, point2, color_el, 2, CV_AA, 0);

		point2.x = point1.x;
		point2.y = point1.y - (int)(HUMAN_CURSOR_SIZE * m_zoom / 3);
		cv::line(m_img, point1, point2, color_el, 2, CV_AA, 0);

		point1.x = hx - (int)(2 * HUMAN_CURSOR_SIZE * m_zoom / 6 * sin(deg2rad(m_apphumans[ahno].direction)));
		point1.y = hy + (int)(2 * HUMAN_CURSOR_SIZE * m_zoom / 6 * cos(deg2rad(m_apphumans[ahno].direction)));
		point2.x = hx - (int)(HUMAN_CURSOR_SIZE * m_zoom / 6 * sin(deg2rad(m_apphumans[ahno].direction)));
		point2.y = hy + (int)(HUMAN_CURSOR_SIZE * m_zoom / 6 * cos(deg2rad(m_apphumans[ahno].direction)));
		cv::line(m_img, point1, point2, color_plus, 2, CV_AA, 0);

		point1.x = hx;
		point1.y = hy;
		cv::circle(m_img, point1, (int)(HUMAN_CURSOR_SIZE * m_zoom / 6), color_plus, 2);

#endif //NO_HUMAN_CURSOR
	}
}

void hlds_agent::DrawCount()
{
 	    //Display count area
    float x = m_Count.Square.left_x * m_zoom + m_dx +100;
    float y = m_Count.Square.top_y * m_zoom + m_dy +100;
    float lx = m_Count.Square.right_x * m_zoom + m_dx - x +100;
    float ly = m_Count.Square.bottom_y * m_zoom + m_dy - y +100;
    cv::rectangle(m_img, cv::Rect((int)x, (int)y, (int)lx, (int)ly), cv::Scalar(255, 255, 255), 2, CV_AA);

    string text;
    int tx1 = 50;
    int tx2 = 170;
    int ty = 100;
    int tdy = 20;

    /*text = "IN AREA";
    cv::putText(m_img, text, cv::Point(tx1, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);

    text = ": " + std::to_string(Count.InArea);
    cv::putText(m_img, text, cv::Point(tx2, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);
    ty += tdy;
    ty += tdy;

    text = "ENTER COUNTER";
    cv::putText(m_img, text, cv::Point(tx1, ty), cv::FONT_HERSHEY_TRIPLEX, 0.5, cv::Scalar(255, 255, 255), 2, CV_AA);
    ty += tdy;

    text = "UP";
    cv::putText(m_img, text, cv::Point(tx1, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);
    text = ": " + std::to_string(Count.Enter[COUNT_UP]);
    cv::putText(m_img, text, cv::Point(tx2, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);
    ty += tdy;

    text = "DOWN";
    cv::putText(m_img, text, cv::Point(tx1, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);

    text = ": " + std::to_string(Count.Enter[COUNT_DOWN]);
    cv::putText(m_img, text, cv::Point(tx2, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);
    ty += tdy;

    text = "LEFT";
    cv::putText(m_img, text, cv::Point(tx1, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);

    text = ": " + std::to_string(Count.Enter[COUNT_LEFT]);
    cv::putText(m_img, text, cv::Point(tx2, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);
    ty += tdy;

    text = "RIGHT";
    cv::putText(m_img, text, cv::Point(tx1, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);

    text = ": " + std::to_string(Count.Enter[COUNT_RIGHT]);
    cv::putText(m_img, text, cv::Point(tx2, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);
    ty += tdy;

    text = "TOTAL";
    cv::putText(m_img, text, cv::Point(tx1, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);

    text = ": " + std::to_string(Count.TotalEnter);
    cv::putText(m_img, text, cv::Point(tx2, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);
    ty += tdy;
    ty += tdy;
    text = "EXIT COUNTER";
    cv::putText(m_img, text, cv::Point(tx1, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);
    ty += tdy;

    text = "UP";
    cv::putText(m_img, text, cv::Point(tx1, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);

    text = ": " + std::to_string(Count.Exit[COUNT_UP]);
    cv::putText(m_img, text, cv::Point(tx2, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);
    ty += tdy;

    text = "DOWN";
    cv::putText(m_img, text, cv::Point(tx1, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);

    text = ": " + std::to_string(Count.Exit[COUNT_DOWN]);
    cv::putText(m_img, text, cv::Point(tx2, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);
    ty += tdy;

    text = "LEFT";
    cv::putText(m_img, text, cv::Point(tx1, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);

    text = ": " + std::to_string(Count.Exit[COUNT_LEFT]);
    cv::putText(m_img, text, cv::Point(tx2, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);
    ty += tdy;

    text = "RIGHT";
    cv::putText(m_img, text, cv::Point(tx1, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);

    text = ": " + std::to_string(Count.Exit[COUNT_RIGHT]);
    cv::putText(m_img, text, cv::Point(tx2, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);
    ty += tdy;

    text = "TOTAL";
    cv::putText(m_img, text, cv::Point(tx1, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);

    text = ": " + std::to_string(Count.TotalExit);
    cv::putText(m_img, text, cv::Point(tx2, ty), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 255, 255), 2, CV_AA);
    ty += tdy;
*/
}

bool hlds_agent::CompareDeviceInfo(DEVICEINFO data)
{
	if(memcmp(&m_deviceinfo,&data,sizeof(DEVICEINFO))== 0)
	{
		return true; 
	}
	else
	{
		return false; 
	}
}

void hlds_agent::SetRunFlag(bool flag)
{
	m_agent_run_flag = flag;
}

void hlds_agent::StatusInfo()
{
	if(m_agent_run_flag == true) 
	{
		m_stream_status = ((agent_broker*)m_arg)->GetStreamStatus();

		if( ((agent_broker*)m_arg)->GetFocusFlag() == true)
		{
			float focus_ratio = ((agent_broker*)m_arg)->GetFocusInOut();
			m_zoom = m_zoom + focus_ratio;
		}

		if( ((agent_broker*)m_arg)->GetCameraAngleFlag() == true)
		{
			CAMERAANGLE recv = ((agent_broker*)m_arg)->GetCameraAngle();
			memcpy(&m_angle_x,&recv.angle_x,4);
			memcpy(&m_angle_y,&recv.angle_y,4);
			memcpy(&m_angle_z,&recv.angle_z,4);
			memcpy(&m_height,&recv.height,4);
		}

		if( ((agent_broker*)m_arg)->GetCamMoveFlag() == true)
		{
			uint8_t cammove = ((agent_broker*)m_arg)->GetCamMoveStatus();
			if(cammove == 1)
			{
				m_dy -= 100 * m_zoom; //up
			}
			else if(cammove == 2)
			{
				m_dy += 100 * m_zoom; //down 
			}
			else if(cammove ==3)
			{
				m_dx -= 100 * m_zoom; // left
			}
			else if(cammove == 4)
			{
				m_dx += 100 * m_zoom; // right
			}
		}
	}
}

int hlds_agent::Proc()
{
		StatusInfo();
		long frameno;
		TimeStamp timestamp;
		m_Tof->GetFrameStatus(&frameno, &timestamp);

		if (frameno != m_frame.framenumber){
			//Read a new frame only if frame number is changed(Old data is shown if it is not changed.)
			//Read a frame of humans data
			Result ret = Result::OK;
			ret = m_Tof->ReadFrame(&m_framehumans);
			if (ret != Result::OK) {
				std::cout << "read frame error" << endl;
				return 0;
			}

			//Read a frame of depth data
			ret = Result::OK;
			ret = m_Tof->ReadFrame(&m_frame);
			if (ret != Result::OK) {
				std::cout << "read frame error" << endl;
				return 0;
			}	

			//3D conversion(with lens correction)
			m_frame3d.Convert(&m_frame);

			//3D rotation(to top view)
			m_frame3d.Rotate(m_angle_x, m_angle_y, m_angle_z);

			//Initialize Z-buffer
			memset(m_z_buffer, 0, sizeof(m_z_buffer));

			if (m_bBack){
				m_img = m_back.clone();
			}
			else {
				m_img = Mat::zeros(480 * 2, 640 * 2, CV_8UC3);
			}

			if (m_bSubDisplay){
				//Initialize sub display
				m_subdisplay = cv::Mat::zeros(SUB_DISPLAY_HEIGHT, SUB_DISPLAY_WIDTH, CV_8UC3);
			}

			//Catch detected humans
			for (int y = 0; y < m_frame3d.height; y++){
				for (int x = 0; x < m_frame3d.width; x++){

					if ((m_frame.CalculateLength(m_frame.databuf[y * m_frame.width + x]) >= m_framehumans.distance_min) &&
							(m_frame.CalculateLength(m_frame.databuf[y * m_frame.width + x]) <= m_framehumans.distance_max)){
						//Valid data(Only data in specific distance for sensor is valid)

						TofPoint point;     //Coordinate after rotation

						//Get coordinates after 3D conversion
						point.x = m_frame3d.frame3d[y * m_frame3d.width + x].x;
						point.y = m_frame3d.frame3d[y * m_frame3d.width + x].y;
						point.z = m_frame3d.frame3d[y * m_frame3d.width + x].z;

						//Zoom
						point.x *= m_zoom;
						point.y *= m_zoom;

						//Shift to X/Y direction on display
						point.x += m_dx;
						point.y += m_dy;

						if ((point.x >= 0) && (point.x < m_img.size().width) &&
								(point.y >= 0) && (point.y < m_img.size().height)){

							if ((point.z >= m_framehumans.z_min) && (point.z < m_framehumans.z_max)){
								//Within range of Z direction

								if ((m_z_buffer[(int)point.x][(int)point.y] == 0) ||
										(m_z_buffer[(int)point.x][(int)point.y] > point.z)){	

									//Register to Z-buffer
									m_z_buffer[(int)point.x][(int)point.y] = point.z;

									//Register color to display image based on distance of Z direction
									long color = (long)(65530 * (point.z - m_framehumans.z_min) / ((m_framehumans.z_max - m_framehumans.z_min)));

									cv::Vec3b v;
									v.val[0] = m_frame.ColorTable[0][color];
									v.val[1] = m_frame.ColorTable[1][color];
									v.val[2] = m_frame.ColorTable[2][color];

									if (m_bPoint){
										m_img.at<cv::Vec3b>((int)point.y, (int)point.x) = v;
									}
								}

								if (m_bSubDisplay){
									//Sub display
									cv::Vec3b v;
									v.val[0] = m_frame.ColorTable[0][m_frame.databuf[y * m_frame.width + x]];
									v.val[1] = m_frame.ColorTable[1][m_frame.databuf[y * m_frame.width + x]];
									v.val[2] = m_frame.ColorTable[2][m_frame.databuf[y * m_frame.width + x]];
									m_subdisplay.at<cv::Vec3b>(y * SUB_DISPLAY_HEIGHT / m_frame3d.height,x * SUB_DISPLAY_WIDTH / m_frame3d.width) = v;

								}
							}
						}
					}
					else {
						//Invalid point is (x,y,z) = (0,0,0)
						m_frame3d.frame3d[y * m_frame3d.width + x].x = 0;
						m_frame3d.frame3d[y * m_frame3d.width + x].y = 0;
						m_frame3d.frame3d[y * m_frame3d.width + x].z = 0;
					}
				}
			}
			CatchHumans(&m_framehumans);
			//Human count
			CountHumans();
			//Draw humans
			DrawHumans();

			//Draw human counter
			if (m_bCount){
				DrawCount();
			}

			if (m_bSubDisplay){
				//Sub display
				cv::Mat roi = m_img(cv::Rect(SUB_DISPLAY_X, SUB_DISPLAY_Y, SUB_DISPLAY_WIDTH, SUB_DISPLAY_HEIGHT));
				cv::resize(m_subdisplay, roi, roi.size(), cv::INTER_LINEAR);
			}

			if(m_stream_status == 1)
			{
				Mat* hlds_img = new Mat();
				*hlds_img = m_img.clone();
				((agent_broker*)m_arg)->UpdateHLDSImage(hlds_img, true);
			}
		}
		return 0;
}

