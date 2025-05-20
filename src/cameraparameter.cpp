#include "cameraparameter.h"

namespace kmod
{

cv::Size CameraParameter::imgSize(1280, 720);
cv::Size CameraParameter::boardSize(9, 6);
float CameraParameter::squareSize(26);
bool CameraParameter::changed(false);
float CameraParameter::imageWidthToHeight(1.8);
cv::Size CameraParameter::BigimgSize(1080, 1279);

cv::Rect CameraParameter::cvOverlayRC[4]= {cv::Rect(0,0,1280, 720)};
cv::Rect CameraParameter::cvCentorRC=cv::Rect(0,0,1280, 720);

std::vector<cv::Mat> CameraParameter::cvOverlayMask;
std::vector<cv::Mat> CameraParameter::cvOverlayMaskInv;

CameraParameter::CameraParameter() :
    intrinsicDistReady(false),
    extrinsicReady(false),
    fusionReady(false),
    devId(-1)
{

}

void CameraParameters::saveCamerasParam(std::string cmrscfg, std::vector<std::shared_ptr<CameraParameter>> &cmrParams )
{
    cv::FileStorage fs(cmrscfg,cv::FileStorage::WRITE);
    fs << "chessboardSetting" << "{"
       << "width" << CameraParameter::boardSize.width
       << "height" << CameraParameter::boardSize.height
       << "squareSize" << CameraParameter::squareSize
       << "}";

    fs << "imageResolution" << "{"
       << "width" << CameraParameter::imgSize.width
       << "height" << CameraParameter::imgSize.height
       << "}";

    fs << "parameters" << "{";
    for( const std::shared_ptr<CameraParameter> p:cmrParams) {
        fs << p->name << "{"
           << "devId" << p->devId
           << "intrinsicReady" << p->intrinsicDistReady
           << "extrinsicReady" << p->extrinsicReady
           << "fusionReady" << p->fusionReady
           << "intrinsic" << p->cvIntrinsic
           << "dist" << p->cvDistCoeffs
           << "extrinsic" << p->cvExtrinsic
           << "cvFusion" << p->cvFusion
           << "}";

    }
    fs << "}";
}

void CameraParameters::loadCamerasParam(std::string cmrscfg, std::vector<std::shared_ptr<CameraParameter>> &cmrParams )
{
    cv::FileStorage fs(cmrscfg,cv::FileStorage::READ);
    cv::FileNode n;
    n = fs["chessboardSetting"];
    n["width"] >> CameraParameter::boardSize.width;
    n["height"] >> CameraParameter::boardSize.height;
    n["squareSize"] >> CameraParameter::squareSize;

    n = fs["imageResolution"];
    n["width"] >> CameraParameter::imgSize.width;
    n["height"] >> CameraParameter::imgSize.height;
    float w = CameraParameter::imgSize.width;
    float h = CameraParameter::imgSize.height;
    CameraParameter::imageWidthToHeight = w / h;

    n = fs["parameters"];

    for( const std::shared_ptr<CameraParameter> p:cmrParams) {
        cv::FileNode sn = n[p->name];
        sn["devId"] >> p->devId;
        sn["intrinsicReady"] >> p->intrinsicDistReady;
        sn["extrinsicReady"] >> p->extrinsicReady;
        sn["fusionReady"] >> p->fusionReady;
        sn["intrinsic"] >> p->cvIntrinsic;
        sn["dist"] >> p->cvDistCoeffs;
        sn["extrinsic"] >> p->cvExtrinsic;
        sn["cvFusion"] >> p->cvFusion;
    }


}

// cv::Mat AA;
// cv::Mat BB;
// int row = outmat.rows;
// int col = outmat.cols;
// //对各个像素遍历进行取反
// for (int i = 0; i < row; i++) {
//     for (int j = 0; j < col; j++) {

//         outmat.at<cv::Vec3b>(i, j)[0] = (uchar)(A.at<cv::Vec3b>(i, j)[0] *         weight.at<cv::Vec3b>(i, j)[0]/255.0f +
//                                                 B.at<cv::Vec3b>(i, j)[0] * (1.0f - weight.at<cv::Vec3b>(i, j)[0]/255.0f));

//         outmat.at<cv::Vec3b>(i, j)[1] = (uchar)(A.at<cv::Vec3b>(i, j)[1]*         weight.at<cv::Vec3b>(i, j)[1]/255.0f +
//                                                 B.at<cv::Vec3b>(i, j)[1]* (1.0f - weight.at<cv::Vec3b>(i, j)[1]/255.0f));

//         outmat.at<cv::Vec3b>(i, j)[2] = (uchar)(A.at<cv::Vec3b>(i, j)[2] *         weight.at<cv::Vec3b>(i, j)[2]/255.0f +
//                                                 B.at<cv::Vec3b>(i, j)[2] * (1.0f - weight.at<cv::Vec3b>(i, j)[2]/255.0f));

//     }
// }

//
// cv::Mat inverseColor(cv::Mat srcImage) {
//     cv::Mat tempImage = srcImage.clone();
//     int row = tempImage.rows;
//     int col = tempImage.cols;
//     //对各个像素遍历进行取反
//     for (int i = 0; i < row; i++) {
//         for (int j = 0; j < col; j++) {
//             //分别对各个通道进行反色处理
//             tempImage.at<uchar>(i, j) = 255 - tempImage.at<uchar>(i, j);

//         }
//     }
//     return tempImage;
// }


static cv::Mat merge2channel( cv::Mat &A, cv::Mat &B, cv::Mat& weight, cv::Mat& invweight,cv::Mat& outmat )
{
    A.convertTo(A,CV_32FC3);
    B.convertTo(B,CV_32FC3);
    cv::Mat  X = A.mul(weight) + B.mul(invweight);
    X.convertTo(outmat, CV_8UC3);
    return outmat;
}

void CameraParameters::Fusion(cv::Mat &mapImage,std::vector<cv::Mat> imgs)
{
    //right top

    cv::Mat outA;
    cv::Mat weight =   CameraParameter::cvOverlayMask[0];
    cv::Mat invweight =   CameraParameter::cvOverlayMaskInv[0];
    cv::Mat A = imgs[0]( CameraParameter::cvOverlayRC[0]);
    cv::Mat B = imgs[1]( CameraParameter::cvOverlayRC[0]);
    merge2channel(A,B,weight,invweight,outA);
    outA.copyTo(mapImage(CameraParameter::cvOverlayRC[0]));

//    //left top
    weight =   CameraParameter::cvOverlayMask[1];
    invweight =   CameraParameter::cvOverlayMaskInv[1];
    A = imgs[0]( CameraParameter::cvOverlayRC[1]);
    B = imgs[3]( CameraParameter::cvOverlayRC[1]);
    merge2channel(A,B,weight,invweight,outA);
    outA.copyTo(mapImage(CameraParameter::cvOverlayRC[1]));


//     //right bottom
    weight =   CameraParameter::cvOverlayMask[2];
    invweight =   CameraParameter::cvOverlayMaskInv[2];
    A = imgs[2]( CameraParameter::cvOverlayRC[2]);
    B = imgs[1]( CameraParameter::cvOverlayRC[2]);
    merge2channel(A,B,weight,invweight,outA);
    outA.copyTo(mapImage(CameraParameter::cvOverlayRC[2]));

//     //left bottom
    weight =   CameraParameter::cvOverlayMask[3];
    invweight =   CameraParameter::cvOverlayMaskInv[3];
    A = imgs[2]( CameraParameter::cvOverlayRC[3]);
    B = imgs[3]( CameraParameter::cvOverlayRC[3]);
    merge2channel(A,B,weight,invweight,outA);
    outA.copyTo(mapImage(CameraParameter::cvOverlayRC[3]));

}



void CameraParameters::loadCmrParam(std::string path,std::vector<std::shared_ptr<CameraParameter>> &cmrParams )
{

    std::string cmrfile = path;
    std::string cmrhfile = path;
    std::string overlayfile = path+"/overlay.yaml";



    std::string maskpath=path;
    std::string weightpath=path;

    cv::FileStorage fs = cv::FileStorage(overlayfile, cv::FileStorage::READ);
    if (fs.isOpened()) {
        cv::Mat resolution;
        fs["resolution"]>>resolution;
        cv::Mat rt,lt,rb,lb,centm;
        std::string  weightname,maskname;
        fs["rtrc"]>>rt;
        fs["ltrc"]>>lt;
        fs["rbrc"]>>rb;
        fs["lbrc"]>>lb;
        fs["weight"]>>weightname;
        fs["mask"]>>maskname;
        fs["centrc"]>>centm;
        fs.release();
        weightpath +="/"+weightname;
        maskpath +="/"+maskname;

        //std::cout<<rt.size()<<rt.at<int32_t>(0)<<std::endl;

        CameraParameter::BigimgSize.width = resolution.at<int32_t>(0);
        CameraParameter::BigimgSize.height = resolution.at<int32_t>(1);
        std::cout<<CameraParameter::BigimgSize<<std::endl;
        //std::cout<<"lt:"<< lt<<" "<<lt.type()<<"  "<<lt.at<double>(2)<<std::endl;
        //大图的重叠区域
        CameraParameter::cvOverlayRC[0]=cv::Rect( rt.at<double>(0),rt.at<double>(1),rt.at<double>(2),rt.at<double>(3));
        CameraParameter::cvOverlayRC[1]=cv::Rect( lt.at<double>(0),lt.at<double>(1),lt.at<double>(2),lt.at<double>(3));
        CameraParameter::cvOverlayRC[2]=cv::Rect( rb.at<double>(0),rb.at<double>(1),rb.at<double>(2),rb.at<double>(3));
        CameraParameter::cvOverlayRC[3]=cv::Rect( lb.at<double>(0),lb.at<double>(1),lb.at<double>(2),lb.at<double>(3));
        //
        //重叠区域输出
        // for(int j=0; j<4; j++)
        //     std::cout<<j<<":"<< CameraParameter::cvOverlayRC[j].x<<","<< CameraParameter::cvOverlayRC[j].y<<";"<<
        //              CameraParameter::cvOverlayRC[j].width<<","<< CameraParameter::cvOverlayRC[j].height<<std::endl;
        //中心的空白区域大小
        CameraParameter::cvCentorRC = cv::Rect( centm.at<double>(0),centm.at<double>(1),centm.at<double>(2),centm.at<double>(3));

        // std::cout<<"cvCentorRC:"<< CameraParameter::cvCentorRC.x<<","<< CameraParameter::cvCentorRC.y<<","<<
        //          CameraParameter::cvCentorRC.width<<","<< CameraParameter::cvCentorRC.height<<std::endl;
        // std::cout<<weightpath<<std::endl;
        // std::cout<<maskpath<<std::endl;
    }



    //std::cout<<"maskMatsize:"<<maskMat.size()<<"type:"<<maskMat.type()<<std::endl;
    //std::cout<<"weightMatsize:"<<weightMat.size()<<"type:"<<maskMat.type()<<std::endl;

    cv::Mat weightMat = cv::imread(weightpath,cv::IMREAD_UNCHANGED);

    //读入权重参数，四个通道放在一起
    std::vector<cv::Mat> cmrweights;
    cv::split(weightMat, cmrweights);

    CameraParameter::cvOverlayMask.clear();

    //std::cout<<"chweight:"<<cmrweights.size() <<std::endl;
    if( cmrweights.size() == 4) {
        for( int i=0; i<4; i++) {
            cv::Mat tempmat, overmask;

            //取出重叠部分的区域

            tempmat = cmrweights[i]( CameraParameter::cvOverlayRC[i]);
            cv::Mat whitemat( tempmat.size(),CV_8UC1, cv::Scalar(255));
            cv::Mat invweight = whitemat - tempmat;

            //单通道变为3通道
            std::vector<cv::Mat> channels_r;
            tempmat.convertTo(tempmat,CV_32FC1,1.0/255.0);
            channels_r.push_back(tempmat.clone());
            channels_r.push_back(tempmat.clone());
            channels_r.push_back(tempmat.clone());
            cv::merge(channels_r,overmask);
            CameraParameter::cvOverlayMask.push_back(overmask.clone());

            //单通道变为3通道
            channels_r.clear();
            invweight.convertTo(invweight,CV_32FC1,1.0/255.0);
            channels_r.push_back(invweight.clone());
            channels_r.push_back(invweight.clone());
            channels_r.push_back(invweight.clone());
            cv::merge(channels_r,overmask);
            CameraParameter::cvOverlayMaskInv.push_back(overmask.clone());
        }
    }

    cv::Mat maskMat = cv::imread(maskpath,cv::IMREAD_UNCHANGED);
    std::vector<cv::Mat> cmrMasks;
    cv::split(maskMat, cmrMasks);
    //std::cout<<"maskweight:"<<cmrMasks.size() <<std::endl;

    for( int i=0; i<4; i++) {
        cmrfile = path + "/video"+std::to_string(i)+".yaml";
        cmrhfile = path + "/video"+std::to_string(i)+"H.yaml";

        cv::FileStorage fsc(cmrfile,cv::FileStorage::READ);
        if (fsc.isOpened()) {
            cv::Mat resolution;
            fsc["resolution"]>>resolution;
            fsc["camera_matrix"]>>cmrParams[i]->cvIntrinsic;
            fsc["dist_coeffs"]>>cmrParams[i]->cvDistCoeffs;
            fsc["scale"]>>cmrParams[i]->fscale;
            fsc.release();

        }
        cv::FileStorage fsh(cmrhfile,cv::FileStorage::READ);
        if (fsh.isOpened()) {
            cv::Mat resolution;
            fsh["wholeres"]>>resolution;

            fsh["H"]>>cmrParams[i]->cvHomo;
            fsh["invH"]>>cmrParams[i]->cvinvHomo;

            fsh["H_norm"]>>cmrParams[i]->cvHomo_norm;
            fsh["intH_norm"]>>cmrParams[i]->cvinvHomo_norm;

            fsh.release();
            //std::cout<<resolution<<std::endl;
            //cv::invert(cmrParams[i]->cvHomo,cmrParams[i]->cvinvHomo);
        }

        if(  cmrMasks.size() == 4 ) {
            cmrParams[i]->cvmask = cmrMasks[i].clone();
            //cv::imwrite( std::to_string(i)+"mask.png",cmrParams[i]->cvmask);
            //cv::imwrite( std::to_string(i)+"overlay.png",CameraParameter::cvOverlayMask[i]);
            //cv::Mat white1mat( CameraParameter::cvOverlayMask[i].size(),CV_8UC1, cv::Scalar(255 ) );
        }

        // std::cout<<i <<" inner_camera:"<<cmrParams[i]->cvIntrinsic<<std::endl;
        // std::cout<<i <<" dist_camera:"<<cmrParams[i]->cvDistCoeffs<<std::endl;
        std::cout<<i <<" scale:"<<cmrParams[i]->fscale<<std::endl;
        std::cout<<i <<" H:"<<cmrParams[i]->cvHomo<<std::endl;
        std::cout<<i <<" invH:"<<cmrParams[i]->cvinvHomo<<std::endl;
        std::cout<<i <<" H_norm:"<<cmrParams[i]->cvHomo_norm<<std::endl;
        std::cout<<i <<" intH_norm:"<<cmrParams[i]->cvinvHomo_norm<<std::endl;

        //Mat mo = Mat::ones(10,12,CV_32FC3);
        //Mat mo = Mat::ones(10,12,CV_32FC3);
        //Mat mo = Mat::ones(10,12,CV_32FC3);


    }

}

}
