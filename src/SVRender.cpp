#include <SVRender.hpp>
#include <GL/gl.h>

// #include <opencv2/cudaimgproc.hpp>
// #include <opencv2/cudaarithm.hpp>
namespace kmod {
SVRender::SVRender(const int32 wnd_width_, const int32 wnd_height_) :
    wnd_width(wnd_width_), wnd_height(wnd_height_), aspect_ratio(0.f), texReady(false),
    white_luminance(1.0), tonemap_luminance(1.0)
{

}
// SVRender::SVRender() :
//     wnd_width(1280), wnd_height(720), aspect_ratio(1.0f), texReady(false),
//     white_luminance(1.0), tonemap_luminance(1.0)
// {

// }
void SVRender::render(std::shared_ptr<Camera> cam, const cv::Mat& frame)
{

    cv::cuda::GpuMat gpu_mat;
    gpu_mat.upload(frame);
    kmod::Camera c = *cam.get();
    render(c,gpu_mat);

}
void SVRender::render(const Camera& cam, const cv::Mat& frame)
{


    cv::cuda::GpuMat gpu_mat;
    gpu_mat.upload(frame);
    render(cam,gpu_mat);

}

void SVRender::render(const Camera& cam, const cv::cuda::GpuMat& frame)
{
    // render command
    //glBindTexture(GL_TEXTURE_2D, 0);
    drawBG(cam);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    //drawModel(cam);
    //drawBlackRect(cam);
    drawSurroundView(cam, frame);
    //glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind default framebuffer
    //glDisable(GL_DEPTH_TEST);
    //drawScreen(cam);
    // unbound
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}


bool SVRender::init(const ConfigBowl& cbowl, const std::string& shadersurroundvert, const std::string& shadersurroundfrag,
                    const std::string& shaderscreenvert, const std::string& shaderscreenfrag,
                    const std::string shaderblackrectvert, const std::string shaderblackrectfrag,
                    const std::string shadebgvert,  const std::string shadebgfrag)
{
    if (isInit)
        return isInit;

    aspect_ratio = static_cast<float>(wnd_width) / wnd_height;

    isInit = initBowl(cbowl, shadersurroundvert, shadersurroundfrag);
    if (!isInit)
        return false;

    if (!shaderblackrectvert.empty() && !shaderblackrectfrag.empty()) {
        isInit = initbowlBlackRect(shaderblackrectvert, shaderblackrectfrag);
        if (!isInit)
            return false;
    }

    isInit = initQuadRender(shaderscreenvert, shaderscreenfrag);
    if (!isInit)
        return false;




    isInit = initBGRender(shadebgvert, shadebgfrag);
    if (!isInit)
        return false;

    return isInit;
}


void SVRender::texturePrepare(const cv::cuda::GpuMat& frame)
{
    if (!texReady) {
        texReady = cuOgl.init(frame);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    auto ok = cuOgl.copyFrom(frame, 0);
}


void SVRender::drawSurroundView(const Camera& cam, const cv::cuda::GpuMat& frame)
{
    glm::mat4 model = bowlmodel.transformation;
    auto view = cam.getView();
    auto projection = glm::perspective(glm::radians(cam.getCamZoom()), aspect_ratio, 0.1f, 100.f);

#ifdef HEMISPHERE
    model = glm::scale(model, glm::vec3(3.f, 3.f, 3.f));
#else
    model = glm::scale(model, glm::vec3(5.f, 5.f, 5.f));
#endif
    OGLbowl.OGLShader.useProgramm();
    OGLbowl.OGLShader.setMat4("model", model);
    OGLbowl.OGLShader.setMat4("view", view);
    OGLbowl.OGLShader.setMat4("projection", projection);
    OGLbowl.OGLShader.setFloat("lum_white", white_luminance);
    OGLbowl.OGLShader.setFloat("lum_map", tonemap_luminance);

    texturePrepare(frame);

    glBindVertexArray(OGLbowl.VAO);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // glPolygonMode(GL_FRONT, GL_LINE);
    glDrawElements(GL_TRIANGLE_STRIP, OGLbowl.indexBuffer, GL_UNSIGNED_INT, 0);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //glBindTexture(GL_TEXTURE_2D, 0);

}

void SVRender::drawModel(const Camera& cam)
{
    glm::mat4 model(1.f);
    auto view = cam.getView();
    auto projection = glm::perspective(glm::radians(cam.getCamZoom()), aspect_ratio, 0.1f, 100.f);

    for(auto i = 0; i < models.size(); ++i) {
        model = modeltranformations[i];
        modelshaders[i]->useProgramm();
        modelshaders[i]->setMat4("model", model);
        modelshaders[i]->setMat4("view", view);
        modelshaders[i]->setMat4("projection", projection);
        models[i].Draw(*modelshaders[i]);
    }

}
void SVRender::drawBG(const Camera& cam)
{

    // if( !binit) return;
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, framebuffer_tex);
    //
    glDisable(GL_DEPTH_TEST);
    OGLBackGround.OGLShader.useProgramm();
    OGLblackRect.OGLShader.setInt("ourTexture", 0);
    glBindVertexArray(OGLBackGround.VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);



}

void SVRender::drawBlackRect(const Camera& cam)
{
    glm::mat4 model(1.f);

    glBindTexture(GL_TEXTURE_2D, 0);

#ifdef HEMISPHERE
    const float y_min = 0.08f;
#else
    constexpr auto bias = 1e-4;
    const float y_min = bowlmodel.y_start + bias;
#endif

    model = glm::translate(model, glm::vec3(0.f, y_min, 0.f));

    auto view = cam.getView();
    auto projection = glm::perspective(glm::radians(cam.getCamZoom()), aspect_ratio, 0.1f, 100.f);

    OGLblackRect.OGLShader.useProgramm();
    OGLblackRect.OGLShader.setMat4("model", model);
    OGLblackRect.OGLShader.setMat4("view", view);
    OGLblackRect.OGLShader.setMat4("projection", projection);

    glBindVertexArray(OGLblackRect.VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}


void SVRender::drawScreen(const Camera& cam)
{
    OGLquadrender.OGLShader.useProgramm();
    glBindVertexArray(OGLquadrender.VAO);
    glBindTexture(GL_TEXTURE_2D, OGLquadrender.framebuffer_tex);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


bool SVRender::addModel(const std::string& pathmodel, const std::string& pathvertshader,
                        const std::string& pathfragshader, const glm::mat4& mat_transform)
{
    bool res = pathmodel.empty() || pathvertshader.empty() || pathfragshader.empty();
    if (res) {
        std::cerr << "Error: empty path to model\n";
        return false;
    }

    Model m(pathmodel);
    res = m.getModelInit();
    if (!res) {
        std::cerr << "Error: fail load model from path\n";
        return false;
    }


    modelshaders.emplace_back(std::make_shared<Shader>());
    auto last_idx = modelshaders.size() - 1;
    res = modelshaders[last_idx]->initShader(pathvertshader.c_str(), pathfragshader.c_str());
    if (!res) {
        std::cerr << "Error: fail init shaders for load model\n";
        return false;
    }


    models.emplace_back(std::move(m));
    modeltranformations.emplace_back(mat_transform);

    return true;
}



bool SVRender::initBowl(const ConfigBowl& cbowl, const std::string& shadersurroundvert, const std::string& shadersurroundfrag)
{
    bool isinit = OGLbowl.OGLShader.initShader(shadersurroundvert.c_str(), shadersurroundfrag.c_str());

    if (!isinit)
        return false;

    glGenVertexArrays(1, &OGLbowl.VAO);
    glGenBuffers(1, &OGLbowl.VBO);
    glGenBuffers(1, &OGLbowl.EBO);

    bowlmodel = cbowl;
    std::vector<float> data;
    std::vector<uint> idxs;

    //

#ifdef HEMISPHERE
    HemiSphere bowl(128, 128);
    isinit = bowl.generate_mesh_uv(data, idxs);
#else
    Bowl bowl(bowlmodel);
    isinit = bowl.generate_mesh_uv_hole(cbowl.vertices_num, cbowl.hole_radius, data, idxs);
    //isinit = bowl.generate_mesh_uv(cbowl.vertices_num, data, idxs);
#endif

    //isinit = bowl.generate_mesh_uv(cbowl.vertices_num, data, idxs);

    if (!isinit)
        return false;

    OGLbowl.indexBuffer = idxs.size();

    constexpr auto stride = (3 + 2) * sizeof(float);


    glBindVertexArray(OGLbowl.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, OGLbowl.VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OGLbowl.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, OGLbowl.indexBuffer * sizeof(uint), &idxs[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

    return true;
}


bool SVRender::initbowlBlackRect(const std::string& fileblackrectvert, const std::string& fileblackrectfrag)
{

    bool isinit = OGLblackRect.OGLShader.initShader(fileblackrectvert.c_str(), fileblackrectfrag.c_str());

    if (!isinit)
        return false;


    const float rectvert[] = {
        0.4f,  0.0f,  0.525f,
        -0.4f,  0.0f,  0.525f,
        -0.4f,  0.0f, -0.525f,

        0.4f,  0.0f,  0.525f,
        -0.4f,  0.0f, -0.525f,
        0.4f,  0.0f, -0.525f,
    };

    glGenVertexArrays(1, &OGLblackRect.VAO);
    glGenBuffers(1, &OGLblackRect.VBO);
    glGenBuffers(1, &OGLblackRect.EBO);

    glBindVertexArray(OGLblackRect.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, OGLblackRect.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectvert), &rectvert, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);

    return true;
}

bool SVRender::initBGRender(const std::string& filevert, const std::string& filefrag)
{

    bool isinit = OGLBackGround.OGLShader.initShader(filevert.c_str(), filefrag.c_str());

    if (!isinit)
        return false;


    // float vertices[] = {
    //     1.0f,1.0f,0.0f,	  0.2f,0.2f,0.9f, 1.0f, 0.0f, // Top Right
    //     1.0f,-1.0f,0.0f,	0.8f,0.8f,0.8f, 1.0f, 1.0f, // Bottom Right
    //     -1.0f,-1.0f,0.0f,	0.8f,0.8f,0.8f, 0.0f, 1.0f, // Bottom Left
    //     -1.0f,1.0f,0.0f,	0.2f,0.2f,0.9f, 0.0f, 0.0f  // Top Left
    // };

    float vertices[] = {
        1.0f,1.0f,0.0f,	  0.2f,0.2f,0.2f, 1.0f, 0.0f, // Top Right
        1.0f,-1.0f,0.0f,	0.4f,0.4f,0.4f, 1.0f, 1.0f, // Bottom Right
        -1.0f,-1.0f,0.0f,	0.4f,0.4f,0.4f, 0.0f, 1.0f, // Bottom Left
        -1.0f,1.0f,0.0f,	0.2f,0.2f,0.2f, 0.0f, 0.0f  // Top Left
    };
    unsigned int indices[] = {
        0,1,3,
        1,2,3,
    };
    glGenVertexArrays(1, &OGLBackGround.VAO);
    glGenBuffers(1, &OGLBackGround.VBO);
    glGenBuffers(1, &OGLBackGround.EBO);

    glBindVertexArray(OGLBackGround.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, OGLBackGround.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    // glEnableVertexAttribArray(0);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);

    // 告诉显卡如何解析，第0个值，每个值是float，不需要标准化，每个步长
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // 开启VAO管理的第一个属性值
    glEnableVertexAttribArray(0);


    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(3 * sizeof(GLfloat)));
    // 开启VAO管理的第一个属性值
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(6 * sizeof(GLfloat)));
    // 开启VAO管理的第一个属性值
    glEnableVertexAttribArray(2);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OGLBackGround.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),indices, GL_STATIC_DRAW);
    // 释放
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    glBindVertexArray(0);

    return true;

}
bool SVRender::initQuadRender(const std::string& shaderscreenvert, const std::string& shaderscreenfrag)
{
    auto isinit = OGLquadrender.OGLShader.initShader(shaderscreenvert.c_str(), shaderscreenfrag.c_str());

    if (!isinit)
        return false;

    constexpr float quadvert[] = {
        -1.f, 1.f, 0.f, 1.f,
            -1.f, -1.f, 0.f, 0.f,
            1.f, 1.f, 1.f, 1.f,
            1.f, -1.f, 1.f, 0.f
        };

    glGenVertexArrays(1, &OGLquadrender.VAO);
    glGenBuffers(1, &OGLquadrender.VBO);
    glBindVertexArray(OGLquadrender.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, OGLquadrender.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadvert), &quadvert, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2 * sizeof(float)));

    glGenFramebuffers(1, &OGLquadrender.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, OGLquadrender.framebuffer);

    glGenTextures(1, &OGLquadrender.framebuffer_tex);
    glBindTexture(GL_TEXTURE_2D, OGLquadrender.framebuffer_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wnd_width, wnd_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, OGLquadrender.framebuffer_tex, 0);


    glGenRenderbuffers(1, &OGLquadrender.renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, OGLquadrender.renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, wnd_width, wnd_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, OGLquadrender.renderbuffer);



    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return false;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}
}

