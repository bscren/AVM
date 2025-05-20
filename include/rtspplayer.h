#ifndef RTSPPLAYER_H
#define RTSPPLAYER_H

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>

#include <opencv2/opencv.hpp>

extern "C"
{
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
#include <libavdevice/avdevice.h>
}

class rtspPlayer
{
public:
    explicit rtspPlayer(std::string);
    ~rtspPlayer();
    void connect();
    void decode(int nid);
    void destroy();
    void close();
    bool isConncected() const { return bConnected; }
    std::string getpath() const { return path; }
    int getWidth() const { return nWidth; }
    int getHeight() const { return nHeight; }
    std::deque<cv::Mat> decodedImgBuf;
    std::mutex mtx;

private:
    AVFormatContext *pFormatCtx = NULL;
    AVCodecContext *pCodecCtx = NULL;
    AVCodec *pCodec = NULL;
    AVFrame *pFrame = NULL;
    AVFrame *pFrameBGR = NULL;
    AVPacket *packet = NULL;
    uint8_t *outBuffer = NULL;
    SwsContext *imgConvertCtx = NULL;
    int videoStream = -1;
    bool bConnected = false;
    std::string path;
    int nWidth = 1920;
    int nHeight = 1080;
};

#endif // RTSPPLAYER_H
