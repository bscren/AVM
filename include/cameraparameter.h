#ifndef CAMERAPARAMETER_H
#define CAMERAPARAMETER_H

#include <opencv2/opencv.hpp>
#include <string>

namespace kmod {
class CameraParameter
{
public:
    CameraParameter();
    ~CameraParameter() {;};

    cv::Mat cvIntrinsic;
    cv::Mat cvExtrinsic;
    cv::Mat cvDistCoeffs;
    cv::Mat cvFusion;
    cv::Mat cvHomo;
    cv::Mat cvinvHomo;
    cv::Mat cvHomo_norm;
    cv::Mat cvinvHomo_norm;
    cv::Mat cvmask;
    float   fscale;
    bool intrinsicDistReady;
    bool extrinsicReady;
    bool fusionReady;
    std::string name;
    int devId;
    static bool changed;
    static cv::Size imgSize;
    static cv::Size boardSize;
    static float squareSize;
    static float imageWidthToHeight;
    static cv::Size BigimgSize;
    static cv::Rect cvOverlayRC[4];
    static std::vector<cv::Mat> cvOverlayMask;
    static std::vector<cv::Mat> cvOverlayMaskInv;
    static cv::Rect cvCentorRC;
};


class CameraParameters
{
public:

    explicit CameraParameters() {;};
    static void saveCamerasParam( std::string cmrscfg, std::vector<std::shared_ptr<CameraParameter>> &cmrParams );
    static void loadCamerasParam( std::string cmrscfg, std::vector<std::shared_ptr<CameraParameter>> &cmrParams );
    static void loadCmrParam(std::string path,std::vector<std::shared_ptr<CameraParameter>> &cmrParams );
    static void Fusion(cv::Mat &mapImage,std::vector<cv::Mat> imgs);

};


}

#endif // CAMERAPARAMETER_H
