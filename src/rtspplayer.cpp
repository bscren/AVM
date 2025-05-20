#include "rtspplayer.h"
#include <iostream>
#include <Util/logger.h>
#include "KMsg.h"

using namespace std;
using namespace toolkit;

rtspPlayer::rtspPlayer(std::string path)
    : bConnected(false)
{
    this->path = path;
    pFormatCtx = NULL;
    pCodecCtx = NULL;
    pCodec = NULL;
    pFrame = NULL;
    pFrameBGR = NULL;
    packet = NULL;
    outBuffer = NULL;
    imgConvertCtx = NULL;
}

rtspPlayer::~rtspPlayer()
{
    destroy();
}

void rtspPlayer::connect()
{

    bConnected = false;

    avformat_network_init();
    AVDictionary *avdic = NULL;
    pFormatCtx = avformat_alloc_context();

    av_dict_set(&avdic, "rtsp_transport", "tcp", 0); // milliseconds
    // av_dict_set(&avdic, "max_delay", "50", 0); // milliseconds
    av_dict_set(&avdic, "stimeout", "2*1000*1000", 0); // milliseconds

    // std::cout<<"connect2"<<std::endl;
    if (avformat_open_input(&pFormatCtx, path.c_str(), NULL, &avdic) != 0)
    {
        av_dict_free(&avdic);
        bConnected = false;
        return;
    }

    std::cout << "connect3" << std::endl;

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        // std::cout << "can't find stream infomation" << std::endl;
        av_dict_free(&avdic);
        bConnected = false;
        return;
    }

    videoStream = -1;

    for (unsigned int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        // if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
        }
    }

    if (videoStream == -1)
    {
        // std::cout << "can't find a video stream" << std::endl;
        av_dict_free(&avdic);
        avformat_free_context(pFormatCtx);
        bConnected = false;
        return;
    }

    // AVCodecID ss;
    // pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    pCodecCtx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoStream]->codecpar);
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    pCodecCtx->bit_rate = 0;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 10;
    pCodecCtx->frame_number = 1;

    if (pCodec == NULL)
    {
        // std::cout << "can't find a codec" << std::endl;
        av_dict_free(&avdic);
        avcodec_free_context(&pCodecCtx);
        avformat_free_context(pFormatCtx);
        bConnected = false;
        return;
    }

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        // std::cout << "can't open a codec" << std::endl;
        av_dict_free(&avdic);
        avcodec_free_context(&pCodecCtx);
        avformat_free_context(pFormatCtx);
        bConnected = false;
        return;
    }

    pFrame = av_frame_alloc();
    pFrameBGR = av_frame_alloc();

    AVPixelFormat pixfmt = pCodecCtx->pix_fmt;

    switch (pCodecCtx->pix_fmt)
    {
    case AV_PIX_FMT_YUVJ420P:
        pixfmt = AV_PIX_FMT_YUV420P;
        break;
    case AV_PIX_FMT_YUVJ422P:
        pixfmt = AV_PIX_FMT_YUV422P;
        break;
    case AV_PIX_FMT_YUVJ444P:
        pixfmt = AV_PIX_FMT_YUV444P;
        break;

    default:
        break;
    }

    imgConvertCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pixfmt,
                                   pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_BGR24,
                                   SWS_BICUBIC, NULL, NULL, NULL);

    // std::cout << "deodecer:"<<pCodecCtx->pix_fmt << std::endl;

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height, 1);
    outBuffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(pFrameBGR->data, pFrameBGR->linesize, outBuffer, AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height, 1);

    nWidth = pCodecCtx->width;
    nHeight = pCodecCtx->height;

    // packet = (AVPacket *) malloc(sizeof(AVPacket));
    // av_new_packet(packet, pCodecCtx->width * pCodecCtx->height);
    bConnected = true;
    if (avdic != NULL)
        av_dict_free(&avdic);

    // av_free(outBuffer);
    // InfoL<<"bConnected";

    // std::cout<<"decoder name:="<<pCodec->name<<std::endl;
    // std::cout<<"pCodecCtx->codec_id:="<<pCodecCtx->codec_id<<std::endl;
    // std::cout<<"pCodecCtx->pix_fmt:="<<pCodecCtx->pix_fmt<<std::endl;

    // int numBytes = avpicture_get_size(AV_PIX_FMT_BGR24, pCodecCtx->width,pCodecCtx->height);
    // int numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

    // avpicture_fill((AVPicture *) pFrameBGR, outBuffer, AV_PIX_FMT_BGR24,  pCodecCtx->width, pCodecCtx->height);
    // av_image_fill_arrays(pFrameBGR->data, pFrameBGR->linesize, outBuffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
}

void rtspPlayer::close()
{
    bConnected = false;
}

void rtspPlayer::decode(int nid)
{
    std::chrono::milliseconds duration(10);
    // std::cout << "av_read_frame q.\n" << std::endl;
    packet = av_packet_alloc();
    // while (av_read_frame(pFormatCtx, packet) >= 0){

    while (bConnected)
    {

        // std::cout << "av_read_frame error.\n" << std::endl;
        if (av_read_frame(pFormatCtx, packet) < 0)
        {
            break;
        }

        if (packet->stream_index == videoStream)
        {
            int got_picture = 0;
            // int ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,packet);
            int ret = avcodec_send_packet(pCodecCtx, packet);
            got_picture = avcodec_receive_frame(pCodecCtx, pFrame); // got_picture = 0 success, a frame was returned

            if (ret != 0)
            { //*if (ret < 0)
                // std::cout << "decode error.\n" << std::endl;
                break;
            }
            // std::cout << "decode error:"<<got_picture << std::endl;
            if (0 == got_picture)
            { // *if (got_picture)
                sws_scale(imgConvertCtx,
                          (uint8_t const *const *)pFrame->data,
                          pFrame->linesize, 0, pCodecCtx->height, pFrameBGR->data,
                          pFrameBGR->linesize);

                cv::Mat img(pFrame->height,
                            pFrame->width,
                            CV_8UC3,
                            pFrameBGR->data[0]);
                kmod::MatTimeStamp outdata = {img, 0, nid};

                if (kmod::g_ImgbufList[nid].push(outdata))
                {
                    ; // std::cout<<"ch="<<nid<<std::endl;
                }
            }
        }
        av_packet_unref(packet); //*av_free_packet(packet);
        std::this_thread::sleep_for(duration);
    }
    // av_packet_unref(packet);//*av_free_packet(packet);
    // std::cout << "av_read_frame q."<< nid << std::endl;
    bConnected = false;
}

void rtspPlayer::destroy()
{

    // InfoL<<"destroy";
    try
    {

        if (imgConvertCtx)
            sws_freeContext(imgConvertCtx);
        if (outBuffer)
            av_free(outBuffer);
        if (pFrameBGR)
            av_frame_free(&pFrameBGR);
        if (pFrame)
            av_frame_free(&pFrame);
        if (packet)
            av_packet_unref(packet);

        if (pCodecCtx)
            avcodec_free_context(&pCodecCtx);
        if (pFormatCtx)
            avformat_free_context(pFormatCtx);
    }
    catch (const std::exception &e)
    {
        ErrorL << "Fatal Error:" << e.what();
        ;
    }
}

// avcodec_close(pCodecCtx);
// avformat_close_input(&pFormatCtx);

// av_free(pFrameBGR);
// av_free(pFrame);
//     AVFormatContext *pFormatCtx;
//     AVCodecContext *pCodecCtx;
//     AVCodec *pCodec;
//     AVFrame *pFrame, *pFrameBGR;
//     AVPacket *packet;
//     uint8_t *outBuffer;
//     SwsContext *imgConvertCtx;
//     AVFrame *pFrame, *pFrameBGR;
//     av_packet_free(&packet);
//
//
//     //avcodec_close(pCodecCtx);
//  avcodec_close(pCodecCtx);
//     //
//

//          if(NULL != outputFormatContext) {
//     if(NULL != outputFormatContext->pb) {
//         avio_close(outputFormatContext->pb);
//     }
//     avformat_free_context(outputFormatContext);
// }
// ​
// if(NULL != inputFormatContext) {
//     avformat_close_input(&inputFormatContext);
// }

// av_frame_free(&pFrameBGR);
// av_frame_free(&pFrame);
// av_packet_unref(packet);
//

// avcodec_close(pCodecCtx);
// avcodec_free_context(&pCodecCtx);
// // avformat_close_input(&pFormatCtx);
// avformat_free_context(pFormatCtx);

/* switch (pCodecCtx->codec_id) {
     case AV_CODEC_ID_H263:
         //g_stFormat.codec = cudaVideoCodec_MPEG4;
         break;

     case AV_CODEC_ID_H264:
        // g_stFormat.codec = cudaVideoCodec_H264;
         break;

     case AV_CODEC_ID_HEVC:
         //g_stFormat.codec = cudaVideoCodec_HEVC;
         break;

     case AV_CODEC_ID_MJPEG:
        // g_stFormat.codec = cudaVideoCodec_JPEG;
         break;

     case AV_CODEC_ID_MPEG1VIDEO:
         //g_stFormat.codec = cudaVideoCodec_MPEG1;
         break;

     case AV_CODEC_ID_MPEG2VIDEO:
         //g_stFormat.codec = cudaVideoCodec_MPEG2;
         break;

     case AV_CODEC_ID_MPEG4:
         //g_stFormat.codec = cudaVideoCodec_MPEG4;
         break;

     case AV_CODEC_ID_VP8:
         //g_stFormat.codec = cudaVideoCodec_VP8;
         break;

     case AV_CODEC_ID_VP9:
         //g_stFormat.codec = cudaVideoCodec_VP9;
         break;

     case AV_CODEC_ID_VC1:
         //g_stFormat.codec = cudaVideoCodec_VC1;
         break;
     default:
         return ;
     }

     //这个地方的FFmoeg与cuvid的对应关系不是很确定，不过用这个参数似乎最靠谱
     switch (pCodecCtx->sw_pix_fmt)
     {
     case AV_PIX_FMT_YUV420P:
         //g_stFormat.chroma_format = cudaVideoChromaFormat_420;
         break;
     case AV_PIX_FMT_YUV422P:
         //g_stFormat.chroma_format = cudaVideoChromaFormat_422;
         break;
     case AV_PIX_FMT_YUV444P:
         //g_stFormat.chroma_format = cudaVideoChromaFormat_444;
         break;
     default:
         //g_stFormat.chroma_format = cudaVideoChromaFormat_420;
         break;
     }*/
// if (mtx.try_lock()) {
//     decodedImgBuf.push_back(img);
//     std::cout<<"size:"<<decodedImgBuf.size()<<std::endl;
//     mtx.unlock();
// }

// pCodec->