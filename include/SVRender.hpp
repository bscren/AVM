#pragma once
#include <vector>
#include <Bowl.hpp>
#include <Virtcam.hpp>
#include <Model.hpp>
#include <SVCudaOGL.hpp>

namespace kmod {
class SVRender
{
private:
    bool initBowl(const ConfigBowl& cbowl, const std::string& filesurroundvert, const std::string& filesurroundfrag);
    bool initbowlBlackRect(const std::string& fileblackrectvert, const std::string& fileblackrectfrag);
    bool initQuadRender(const std::string& filescreenvert, const std::string& filescreenfrag);
    bool initBGRender(const std::string&, const std::string& );
protected:
    void texturePrepare(const cv::cuda::GpuMat& frame);
    void drawSurroundView(const Camera& cam, const cv::cuda::GpuMat& frame);
    void drawModel(const Camera& cam);
    void drawScreen(const Camera& cam);
    void drawBlackRect(const Camera& cam);
    void drawBG(const Camera& cam);
public:
    bool getInit() const {
        return isInit;
    }
    bool addModel(const std::string& pathmodel, const std::string& pathvertshader,
                  const std::string& pathfragshader, const glm::mat4& mat_transform);
    float getWhiteLuminance() const {
        return white_luminance;
    }
    void setWhiteLuminance(const float white_luminance_) {
        white_luminance = white_luminance_;
    }
    float getToneLuminance() const {
        return tonemap_luminance;
    }
    void setToneLuminance(const float tone_luminance_) {
        tonemap_luminance = tone_luminance_;
    }

public:
    SVRender(const int32 wnd_width_, const int32 wnd_height_);

    SVRender& operator=(const SVRender&) = delete;
    SVRender(const SVRender&) = delete;

    bool init(const ConfigBowl& cbowl, const std::string& shadersurroundvert, const std::string& shadersurroundfrag,
              const std::string& shaderscreenvert, const std::string& shaderscreenfrag,
              const std::string shaderblackrectvert = std::string(), const std::string shaderblackrectfrag=std::string(),
              const std::string shadebgvert = std::string(), const std::string shadebgfrag=std::string());
    void render(const Camera& cam, const cv::cuda::GpuMat& frame);
    void render(const Camera& cam, const cv::Mat& frame);
    void render(std::shared_ptr<Camera> cam, const cv::Mat& frame);

private:
    ConfigBowl bowlmodel;
    OGLBuffer OGLbowl;
    OGLBuffer OGLblackRect;
    OGLBuffer OGLquadrender;
    OGLBuffer OGLBackGround;
    float aspect_ratio;
    int32  wnd_width;
    int32  wnd_height;
    CUDA_OGL cuOgl;
    float white_luminance, tonemap_luminance;

private:
    std::vector<Model> models;
    std::vector<std::shared_ptr<Shader>> modelshaders;
    std::vector<glm::mat4> modeltranformations;
    bool isInit = false;
    bool texReady;
};


}



