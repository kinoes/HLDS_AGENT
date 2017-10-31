#pragma once 
#include <deque>
#include <unordered_map>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <turbojpeg.h>
#include "xthread.h"
#include "agent_broker.h"
#include "agent_client.h"
#include "stream_hlds.h"
#include "event_manager.h"
#include "command.h"

using namespace std;
using namespace cv;

class agent_broker : public xthread
{
	public:
		agent_broker();
		virtual ~agent_broker();
		void StreamHLDSClients(uint8_t* lpr_jpg_data, uint32_t lpr_jpg_len);
		int GetCurImage(Mat* out_img);
		void UpdateHLDSImage(Mat*img, bool use_lock);
		void lock_img_set() { m_img_set_lock.lock(); }
		void unlock_img_set() { m_img_set_lock.unlock(); }
		void SetStreamStatus(xpacket *recv);
		void SetFrame(xpacket *recv);
		void SetConfig(xpacket *recv);
		void SetPantiltData(xpacket *recv);
		void SetFocusInOut(xpacket *recv);
		void SetDeviceInfo(xpacket* recv);

		const static int DEFAULT_MAX_PACKET_UNIT_SIZE = 1024;
		const static int STREAM_IMG_WIDTH = 320;
		const static int STREAM_IMG_HEIGHT = 240;

		deque<agent_client*> m_clients;	
	private:
		virtual int Proc();
		
		//comm
		mutex m_clients_lock;
		mutex m_img_set_lock; //img_set components : lpr_img & context_img
		Mat* m_cur_hlds_img = nullptr;
};
