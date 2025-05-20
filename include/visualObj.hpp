#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <GLES3/gl32.h>
#include <EGL/egl.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3ext.h>
#include <GLES3/gl3.h>

#include <glm/glm.hpp>

#include "xShader.h"


namespace kmod {

class VisualObj
{

public:

    //VisualObj(GLuint va0=0, GLuint vbo=0,  GLuint ebo=0,uint indbuf=0,GLuint frmbuf=0, GLuint rdbuf=0,GLuint frmtex=0);
    //VisualObj()=default;
    VisualObj();
    ~VisualObj() {
        clearBuffers();
    }
    virtual void Init()=0;
    virtual void Prepare(const cv::Mat &imMat)=0;
    virtual void PaintGLOpaque()=0;
    virtual void PaintGLTranslucent()=0;
    virtual void AfterPaint()=0;
    bool IsInit() {
        return binit;
    }
    //  VisualObj& operator=(const VisualObj&) = delete;
    ///  VisualObj(const VisualObj&) = delete;

protected:
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    uint indexBuffer;
    GLuint framebuffer;
    GLuint renderbuffer;
    GLuint framebuffer_tex;
    bool   binit = false;

public:
    void clearBuffers()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteFramebuffers(1, &framebuffer);
        glDeleteRenderbuffers(1, &renderbuffer);
        glDeleteTextures(1, &framebuffer_tex);
        binit = false;
    }
};

class TestVis: public VisualObj
{
public:
    TestVis();
    ~TestVis();
    virtual void Init();
    virtual void Prepare(const cv::Mat &imMat);
    virtual void PaintGLOpaque();
    virtual void PaintGLTranslucent();
    virtual void AfterPaint();

private:

    float _w2hrate = 0.0;
    float _width = 0;
    float _height = 0;
    //unsigned shaderProgram;
    std::shared_ptr<XShader> _pshader;
};

class RectVis: public VisualObj
{
public:
    RectVis();
    ~RectVis();
    virtual void Init();
    virtual void Prepare(const cv::Mat &imMat);
    virtual void PaintGLOpaque();
    virtual void PaintGLTranslucent();
    virtual void AfterPaint();

private:
    float _w2hrate = 0.0;
    float _width = 0;
    float _height = 0;
    std::shared_ptr<XShader> _pshader;
};
}