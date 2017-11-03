#include<iostream>
#include "hlds_agent.h"

using namespace std;

hlds_agent::hlds_agent()
{
    m_angle_x = 90.0f;              //Angle to rotate around X-axis(degree)
    m_angle_y = 0.0f;               //Angle to rotate around Y-axis(degree)
    m_angle_z = 0.0f;               //Angle to rotate around Z-axis(degree)
    m_height = 1000.0f;             //Height from floor(mm)
    m_dx = 600.0f;                  //Shift length to x-axis positive direction(mm)
    m_dy = 900.0f;                  //Shift length to y-axis positive direction(mm)
    m_zoom = 0.12f;                 //Zoom ratio
    m_bPoint = true;                //Mode to display points
    m_bBack = true;                //Mode to display footprint on background
    m_bCount = true;                //Mode to display human count
    m_bBoxShift = true;             //true: shift, false: change size in count area setting mode
    m_bSubDisplay = true;           //Mode to display sub display
    m_apphumanid = 0;
    m_stream_status = 1;			//STREAM OFF DEFAULT
	m_Tof = new Tof;  
    InitAgent();
}

hlds_agent::~hlds_agent()
{
    delete m_Tof;
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
    string stBuffer;

    /*stBuffer = _GetPrivateProfileString(inisection,"ANGLE_X", to_string(m_angle_x), &ret, inifilename);
    if (ret != 0){
        m_angle_x = stof(stBuffer);
    }
    stBuffer = _GetPrivateProfileString(inisection, "ANGLE_Y", to_string(m_angle_y), &ret, inifilename);
    if (ret != 0){
        m_angle_y = stof(stBuffer);
    }
    stBuffer = _GetPrivateProfileString(inisection, "ANGLE_Z", to_string(m_angle_z), &ret, inifilename);
    if (ret != 0){
        m_angle_z = stof(stBuffer);
    }
    stBuffer = _GetPrivateProfileString(inisection,"HEIGHT", to_string(m_height), &ret, inifilename);
    if (ret != 0){
        m_height = stof(stBuffer);
    }
    stBuffer = _GetPrivateProfileString(inisection,"SHIFT_X", to_string(m_dx), &ret, inifilename);
    if (ret != 0){
        m_dx = stof(stBuffer);
    }

    stBuffer = _GetPrivateProfileString(inisection,"SHIFT_Y", to_string(m_dy), &ret, inifilename);
    if (ret != 0){
        m_dy = stof(stBuffer);
    }

    stBuffer = _GetPrivateProfileString(inisection,"ZOOM", to_string(m_zoom), &ret, inifilename);
    if (ret != 0){
        m_zoom = stof(stBuffer);
    }

    stBuffer = _GetPrivateProfileString(inisection,"COUNT_LEFT_X", to_string(Count.Square.left_x), &ret, inifilename);
    if (ret != 0){
        Count.Square.left_x = stof(stBuffer);
    }

    stBuffer = _GetPrivateProfileString(inisection,"COUNT_TOP_Y", to_string(Count.Square.top_y), &ret, inifilename);
    if (ret != 0){
        Count.Square.top_y = stof(stBuffer);
    }

    stBuffer = _GetPrivateProfileString(inisection,"COUNT_RIGHT_X", to_string(Count.Square.right_x), &ret, inifilename);
    if (ret != 0){
        Count.Square.right_x = stof(stBuffer);
    }

    stBuffer = _GetPrivateProfileString(inisection,"COUNT_BOTTOM_Y", to_string(Count.Square.bottom_y),&ret,inifilename);
    if (ret != 0){
        Count.Square.bottom_y = stof(stBuffer);
    }*/
    return true;
}


int hlds_agent::InitAgent()
{
   // for test //
    testm_TofInfo.tofid="EC:2E:4E:01:02:55";
    testm_TofInfo.tofmac="EC:2E:4E:01:02:55";
    testm_TofInfo.tofip="172.16.100.228";
    testm_TofInfo.rtp_port=0;

    if(m_Tof->Open(testm_TofInfo) != Result::OK){ 
    std::cout << "TOF ID " << testm_TofInfo.tofid << " Open Error" << endl;
        return -1;
    }

    //Set camera mode as Depth mode
    if (m_Tof->SetCameraMode(CameraMode::CameraModeDepth) != Result::OK){
        std::cout << "TOF ID " << m_Tof->tofinfo.tofid << " Set Camera Mode Error" << endl;
        return -1;
    }

    //Set camera pixel
    if (m_Tof->SetCameraPixel(CameraPixel::w320h240) != Result::OK){
        //  if (tof.SetCameraPixel(CameraPixel::w160h120) != Result::OK){
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
    m_back = Mat::zeros(240 * 2, 320 * 2, CV_8UC3);
  
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

int hlds_agent::Proc()
{
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

		if (m_bBack){
			m_img = m_back.clone();
		}
		else {
			m_img = Mat::zeros(240 * 2, 320 * 2, CV_8UC3);
		}
		//Catch detected humans
		CatchHumans(&m_framehumans);
		if(m_stream_status == STREAM_ON) 
		{
			Mat* hlds_img = new Mat();
			*hlds_img = m_img;
			((agent_broker*)m_arg)->UpdateHLDSImage(hlds_img, true);
		}
	}    
	return 0;
}

