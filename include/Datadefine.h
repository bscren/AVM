/*

author: chenjinbo  date:2022/10/05
*/
#pragma once
#include <vector>
#include <time.h>
#include <opencv2/opencv.hpp>
#include "rtspplayer.h"
#include "cameraparameter.h"
#include "SVRender.hpp"
#include "Virtcam.hpp"
namespace kmod {
struct DataPrivate {
public:
    std::vector<std::shared_ptr<rtspPlayer>>	mCameras;
public:
    void	setWorkMode(int mode) {
        std::lock_guard<std::mutex> lock(mutex);
        mProcessStatus = mode;
    }
    int		getWorkMode() {
        std::lock_guard<std::mutex> lock(mutex);
        return mProcessStatus;
    }
private:
    unsigned  char	mProcessStatus;//0 at work 1 config
    std::mutex mutex;
};
struct InfosPrivate {
    bool append(const std::string  v) {
        struct tm* newtime;
        time_t lt1;
        time(&lt1);
        char buf[128];
        newtime = localtime(&lt1);
        strftime(buf, 128, "%Y-%m-%d %H:%M:%S---", newtime);
        std::lock_guard<std::mutex> lock(mutex);

        if (infos.size() > 10) {
            infos.pop_front();
            infos.push_back(v);
        }
        std::string s = buf;
        infos.push_back(s + v);
        bnew = true;
        return true;
    }
    std::list<std::string> &getinfos() {
        std::lock_guard<std::mutex> lock(mutex);
        //TraceL  << "get:" << infos.size();
        bnew = false;
        return infos;
    }
    bool isNewer() {
        return bnew;
    }
private:
    std::list<std::string> infos;
    std::mutex mutex;
    bool bnew = false;
};
const int CAMERA_WIDTH =1280;
const int CAMERA_HEIGHT =720;

struct SVAppConfig
{
    int cam_width = CAMERA_WIDTH;
    int cam_height = CAMERA_HEIGHT;
    int calib_width = CAMERA_WIDTH;
    int calib_height = CAMERA_HEIGHT;
    std::string undist_folder {"camparameters/video"};//{"calibrationData/1280/video"};
    std::string calib_folder = "camparameters/";
    std::string car_model = "../models/Dodge Challenger SRT Hellcat 2015.obj";
    std::string car_vert_shader = "../shader/modelshadervert.glsl";
    std::string car_frag_shader = "../shader/modelshaderfrag.glsl";
    std::string win1{"Cam0"}; // window name
    std::string win2{"Cam1"};
    int numbands = 4;
    float scale_factor = 0.65;
    int limit_iteration_init = 5000;
    int num_pool_threads = 2;
    std::chrono::seconds time_recompute_photometric_gain{10};
    std::chrono::seconds time_recompute_photometric_luminance{7};
    ConfigBowl cbowl;
    std::string surroundshadervert = "../shader/surroundvert.glsl";
    std::string surroundshaderfrag = "../shader/surroundfrag.glsl";
    std::string screenshadervert = "../shader/frame_screenvert.glsl";
    std::string screenshaderfrag = "../shader/frame_screenfrag.glsl";
    std::string blackrectshadervert = "../shader/blackrectshadervert.glsl";
    std::string blackrectshaderfrag = "../shader/blackrectshaderfrag.glsl";
    std::string bgshadervert = "../shader/bgvert.glsl";
    std::string bgshaderfrag = "../shader/bgfrag.glsl";
    bool usePedestrianDetection = false;

    int nMode=0;

};

extern "C" void  addCar(std::shared_ptr<SVRender>& view_, const SVAppConfig& svcfg);
extern "C" void addBowlConfig(ConfigBowl& cbowl);
extern "C" void resetCameraState();

extern std::vector<std::shared_ptr<kmod::CameraParameter>> g_camerasParam;

extern std::shared_ptr<DataPrivate>  g_pDevDatav;
extern std::shared_ptr<InfosPrivate> g_infos;
extern std::shared_ptr<SVRender>     g_pRender;
extern std::shared_ptr<Camera>       g_pcamera;
extern SVAppConfig                   g_svcfg;


}


