#pragma once 
#include <deque>
#include <unordered_map>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <turbojpeg.h>
#include "xthread.h"
#include "agent_broker.h"
#include "agent_client.h"
#include "tof.h"
#include "hlds_agent.h"
#include "stream_hlds.h"
#include "event_manager.h"
#include "command.h"
#include "gmark.h"
#include "send_packet.h"

using namespace std;
using namespace cv;

/*
struct CAMERAANGLE
{
	float height;
	float angle_x;
	float angle_y;
	float angle_z;
};

struct ETCCONFIG
{
	int bg_interval;
	int direction;
	int noise;
};

struct DEVICEINFO
{
	char ip[30];
	char mac_address[30];
	char device_id[30];
	char area_id[30];
	char area[30];
};
*/

class agent_broker : public xthread
{
	public:
		agent_broker();
		virtual ~agent_broker();
		void InitFalg();
		void StreamHLDSClients(uint8_t* lpr_jpg_data, uint32_t lpr_jpg_len);
		int GetCurImage(Mat* out_img);
		void UpdateHLDSImage(Mat*img, bool use_lock);
		void lock_img_set() { m_img_set_lock.lock(); }
		void unlock_img_set() { m_img_set_lock.unlock(); }
		void SetStreamStatus(xpacket *recv);
		uint8_t GetStreamStatus();
		void SetDeviceCheck(xpacket *recv);
		void SetFrame(xpacket *recv);
		void SetConfig(xpacket *recv);
		void SetPantiltData(xpacket *recv);
		void SetFocusInOut(xpacket *recv);
		float GetFocusInOut();
		void SetDeviceInfo(xpacket* recv);
		void InitStatusRes();
		void AddClient(agent_client* client);
		void AddAgent(hlds_agent* agent);
		void DelAgent();
		void SetCameraAngle(xpacket* recv);
		CAMERAANGLE GetCameraAngle();
		void SetEtcConfigSet(xpacket* recv);
		ETCCONFIG GetEtcConfig();
		void ManageDeviceAddDel(xpacket* recv);
		bool GetDeviceInfo();

		bool GetFocusFlag(){return m_zoom_flag;}
		bool GetCameraAngleFlag(){return m_cam_angle_flag;}
		bool GetCamMoveFlag(){return m_cam_move_flag;}
		bool GetEtcConfigFlag(){return m_etc_config_flag;}
		uint8_t  GetDeviceManageFlag(){return m_device_manage_flag;}
		uint8_t GetCamMoveStatus();
		void GetAgentStatus(xpacket *packet);
		const static int DEFAULT_MAX_PACKET_UNIT_SIZE = 1024;
		const static int STREAM_IMG_WIDTH = 320;
		const static int STREAM_IMG_HEIGHT = 240;

		deque<agent_client*> m_clients;	
		deque<hlds_agent*> m_agents;
	private:
		virtual int Proc();
		
		//comm
		uint8_t m_stream_onoff;
		float m_zoom;
		bool m_zoom_flag;
		bool m_cam_angle_flag;
		bool m_cam_move_flag;
		bool m_etc_config_flag;
		uint8_t m_device_manage_flag;
		uint8_t m_cam_status;
		bool m_device_exits_flag;
	
		CAMERAANGLE m_camera_angle_data;
		ETCCONFIG m_etc_config_data;
		DEVICEINFO m_device_info;
		mutex m_camera_angle_lock;
		mutex m_focus_mode_lock;
		mutex m_stream_mode_lock;
		mutex m_agent_clients_lock;
		mutex m_camera_move_lock;
		mutex m_etc_config_data_lock;
		mutex m_img_set_lock; //img_set components : lpr_img & context_img
		mutex m_device_manage_lock;
		mutex m_agentcheck_manage_lock; 
		mutex m_device_check_lock;
		mutex  m_device_exits_lock;
		Mat* m_cur_hlds_img = nullptr;
};
