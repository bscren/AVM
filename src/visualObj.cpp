#include "visualObj.hpp"
#include <GL/gl.h>

namespace kmod {

VisualObj::VisualObj( ):VAO(0), VBO(0), EBO(0), indexBuffer(0), framebuffer(0), renderbuffer(0), framebuffer_tex(0)
{

}
TestVis::TestVis()
{
    binit = false;
    framebuffer_tex = -1;
    _pshader = nullptr;
};
TestVis::~TestVis()
{
    _pshader->clear();
    clearBuffers();
};
void TestVis::Init()
{

    float vertices[] = {
        1.0f,1.0f,0.0f,	0.5f,0.5f,0.8f, 1.0f, 0.0f, // Top Right
        1.0f,-1.0f,0.0f,	0.5f,0.5f,0.8f, 1.0f, 1.0f, // Bottom Right
        -1.0f,-1.0f,0.0f,	0.2f,0.2f,0.5f, 0.0f, 1.0f, // Bottom Left
        -1.0f,1.0f,0.0f,	0.3f,0.3f,0.5f, 0.0f, 0.0f  // Top Left
    };


    // float vertices[] = {
    //     0.8f,0.8f,0.0f,	1.0f,0.0f,0.0f, 1.0f, 0.0f, // Top Right
    //     0.8f,0.6f,0.0f,	0.0f,1.0f,0.0f, 1.0f, 1.0f, // Bottom Right
    //     0.6f,0.6f,0.0f,	0.0f,0.0f,1.0f, 0.0f, 1.0f, // Bottom Left
    //     0.6f,0.8f,0.0f,	1.0f,0.0f,0.0f, 0.0f, 0.0f  // Top Left
    // };

    unsigned int indices[] = {
        0,1,3,
        1,2,3,
    };

    //	unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // 绑定VBO和VAO对象
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // 为当前绑定到target的缓冲区对象创建一个新的数据存储
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),indices, GL_STATIC_DRAW);
    // 释放
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    _pshader = std::make_shared<XShader>("../shaders/testvis.vs", "../shaders/testvis.frag");
    binit =true;

}
void TestVis::Prepare(const cv::Mat &imMat)
{
    if( !binit) return;

    if( imMat.cols != _width ||  imMat.rows != _height ) {

        _width = imMat.cols;
        _height = imMat.rows;

        if( -1 != framebuffer_tex )
            glDeleteTextures(1, &framebuffer_tex);
        glGenTextures(1, &framebuffer_tex);
        glBindTexture(GL_TEXTURE_2D, framebuffer_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE,  NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }


    _width = imMat.cols;
    _height = imMat.rows;
    // int pixellength = _width*_height * 3;
    // GLubyte* pixels = new GLubyte[pixellength];
    // memcpy(pixels, imMat.data, pixellength * sizeof(char));
    //glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framebuffer_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, imMat.data);
    // free(pixels);
    //glEnable(GL_TEXTURE_2D);

};
void TestVis::PaintGLOpaque()
{
    if( !binit) return;

    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framebuffer_tex);
    _pshader->Use();
    glUniform1i(glGetUniformLocation( _pshader->Program, "ourTexture"), 0);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

};
void TestVis::PaintGLTranslucent()
{

};
void TestVis::AfterPaint()
{

};




RectVis::RectVis()
{
    binit = false;
    framebuffer_tex = -1;
};
RectVis::~RectVis()
{
    clearBuffers();
};
void RectVis::Init()
{


    // float bufdata[32]= {1.0};
    // for( int i=0; i < 4; i++) {

    //     bufdata[8*i]=  int(i%2)*1024-512;
    //     bufdata[8*i+1]= int(i/2)*768-384;
    //     bufdata[8*i+2]= 1.0;//Z
    //     //bufdata[8*i+3]= ;
    //     //  bufdata[8*i+4]=
    //     //   bufdata[8*i+5]=
    //     bufdata[8*i+6]= i%2;
    //     bufdata[8*i+7]=int(i/2);
    // }

    // unsigned int indices[] = {//注意索引从0开始
    //     0,1,3,//第一个三角形
    //     1,2,3 //第二个三角形
    // };
    // //std::cout<<_width<<","<<_height<<std::endl;

    // glGenVertexArrays(1, &VAO);
    // glBindVertexArray(VAO);

    // glGenBuffers(1, &VBO);
    // // // // glGenBuffers(1, &EBO);
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(bufdata), bufdata, GL_STATIC_DRAW);


    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);	//绑定索引缓冲对象
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    // // //position
    // glEnableVertexAttribArray(0);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);

    // // //color
    // glEnableVertexAttribArray(1);
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (GLvoid*) (3 * sizeof(float)));

    // // // //uv
    // glEnableVertexAttribArray(2);
    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (GLvoid*)( 6 * sizeof(float)));



    GLfloat vertices[] = {
        // Positions          // Colors           // Texture Coords
        0.5f,0.5f,0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Top Right
        0.5f,-0.5f,0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // Bottom Right
        -0.5f,-0.5f,0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Bottom Left
        -0.5f,0.5f,0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // Top Left
    };
    GLuint indices[] = { // 注意索引从0开始!
        0, 1, 3, // 第一个三角形
        1, 2, 3  // 第二个三角形
    };

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // TexCoord attribute
    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    // glEnableVertexAttribArray(2);



    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    _pshader = std::make_shared<XShader>("/home/k/AVM/cameraFusion/shaders/draw_texture.vs", "/home/k/AVM/cameraFusion/shaders/draw_texture.frag");

    binit =true;


}
void RectVis::Prepare(const cv::Mat &imMat)
{
    if( !binit) return;

    // if( imMat.cols != _width ||  imMat.rows != _height ) {

    //     _width = imMat.cols;
    //     _height = imMat.rows;

    //     if( -1 != framebuffer_tex )
    //         glDeleteTextures(1, &framebuffer_tex);

    //     glGenTextures(1, &framebuffer_tex);
    //     glBindTexture(GL_TEXTURE_2D, framebuffer_tex);
    //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE,  NULL);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // }


    // _width = imMat.cols;
    // _height = imMat.rows;

    // int pixellength = _width*_height * 3;
    // GLubyte* pixels = new GLubyte[pixellength];
    // memcpy(pixels, imMat.data, pixellength * sizeof(char));
    // glBindTexture(GL_TEXTURE_2D, framebuffer_tex);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
    // free(pixels);
    // glEnable(GL_TEXTURE_2D);

};
void RectVis::PaintGLOpaque()
{


    if( !binit) return;


    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, framebuffer_tex);

    //_pshader->useProgramm();
    // _pshader->setInt("ourTexture", framebuffer_tex );
    _pshader->Use();
    //glUniform1i(glGetUniformLocation( _pshader->Program, "ourTexture"), 0);
    glBindVertexArray(VAO);			//绑定VAO
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);//使用线框模式绘制三角形，默认为glPolygonMode(GL_FRONT_AND_BACK, GL_FILL)
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


    // //绘制正方形开始
    // glBegin(GL_QUADS);

    // //前面
    // glTexCoord2f( 0.0, 0.0 );
    // glColor3f(1.0f,1.0f,1.0f);
    // glVertex3f( -50, -50, 1.0 );
    // glTexCoord2f( 1.0, 0.0 );
    // glColor3f(1.0f,1.0f,1.0f);
    // glVertex3f( 50, -50, 1.0 );
    // glTexCoord2f( 1.0, 1.0 );
    // glColor3f(1.0f,1.0f,1.0f);
    // glVertex3f( 50, 50, 1.0 );
    // glTexCoord2f( 0.0, 1.0 );
    // glColor3f(1.0f,1.0f,1.0f);
    // glVertex3f( -50, 50, 1.0 );

    // glEnd();

};
void RectVis::PaintGLTranslucent()
{

};
void RectVis::AfterPaint()
{

};
}