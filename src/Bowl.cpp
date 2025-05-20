#include "Bowl.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>


#include "Datadefine.h"

namespace kmod {

static void createImage(const std::vector< glm::vec3>&  interpoints, int nWidth = 2000, int nHeight= 2000, float dlt=0.001 )
{

    std::vector< glm::vec3>  outlinePoint(4);
    outlinePoint[0]=glm::vec3(0,0,0);//left xmin
    outlinePoint[1]=glm::vec3(0,0,0);//right xmax
    outlinePoint[2]=glm::vec3(0,0,0);//zmin
    outlinePoint[3]=glm::vec3(0,0,0);//zmax
    std::for_each(interpoints.begin(), interpoints.end(), [&](glm::vec3 v)
    {
        if(  outlinePoint[0].x > v.x )  outlinePoint[0] = v;
        if(  outlinePoint[1].x < v.x )  outlinePoint[1] = v;
        if(  outlinePoint[2].z > v.z )  outlinePoint[2] = v;
        if(  outlinePoint[3].z < v.z )  outlinePoint[3] = v;
    });

    std::for_each(outlinePoint.begin(), outlinePoint.end(), [&](glm::vec3 v)
    {
        std::cout <<v.x<<","<<v.y<<","<<v.z<< std::endl;
    });

    float centorx =  (outlinePoint[1].x + outlinePoint[0].x)/2.0f;
    float centory =  (outlinePoint[3].x + outlinePoint[2].z)/2.0f;

    // float dltx = (outlinePoint[1].x - outlinePoint[0].x)/nWidth;
    // float dlty = (outlinePoint[3].z - outlinePoint[2].z)/nHeight;

    // float dlt = dltx;

    // if( dlty>dlt ) dlt=dlty;

    cv::Mat image(nHeight,nWidth,CV_8UC1,cv::Scalar(0));

    std::for_each(interpoints.begin(), interpoints.end(), [&](glm::vec3 v) {

        // int x = (v.x - centorx)/dlt + nWidth/2;
        // int y = (v.z - centory)/dlt + nHeight/2;

        int x = v.x / dlt + nWidth / 2;
        int y = v.z / dlt + nHeight / 2;
        if( x>=0 && x<nWidth &&
                y>=0 && y<nHeight ) {
            image.at<uchar>(y,x) = 255;
        }
    });

    cv::imwrite("image.png",image);

}


//std::vector<cv::Point2f> transformedPoints(points);
//    cv::perspectiveTransform(points, transformedPoints, homography);
static void getOriImageUV(const std::vector< glm::vec3>&  interpoints,std::vector< glm::vec2>& interuv,cv::Mat invH,cv::Size imgsize,float dlt=0.001)
{

    std::vector< glm::vec3>  outlinePoint(4);
    outlinePoint[0]=glm::vec3(0,0,0);//left xmin
    outlinePoint[1]=glm::vec3(0,0,0);//right xmax
    outlinePoint[2]=glm::vec3(0,0,0);//zmin
    outlinePoint[3]=glm::vec3(0,0,0);//zmax
    std::for_each(interpoints.begin(), interpoints.end(), [&](glm::vec3 v)
    {
        if(  outlinePoint[0].x > v.x )  outlinePoint[0] = v;
        if(  outlinePoint[1].x < v.x )  outlinePoint[1] = v;
        if(  outlinePoint[2].z > v.z )  outlinePoint[2] = v;
        if(  outlinePoint[3].z < v.z )  outlinePoint[3] = v;
    });

    // std::for_each(outlinePoint.begin(), outlinePoint.end(), [&](glm::vec3 v)
    // {
    //     std::cout <<v.x<<","<<v.y<<","<<v.z<< std::endl;
    // });
    std::vector<cv::Point2f> points;




    int bigwidth=1080;
    int bigheight=1279;
    int nWidth = imgsize.width;
    int nHeight = imgsize.height;

    cv::Mat im(bigheight,bigwidth,CV_8UC1,cv::Scalar(0));
    cv::Mat image(imgsize,CV_8UC1,cv::Scalar(0));

    std::for_each(interpoints.begin(), interpoints.end(), [&](glm::vec3 v) {

        glm::vec3 inPnt;
        inPnt.x = v.x / dlt;
        inPnt.y = v.z / dlt;
        inPnt.z = 1.0;
        int xpos = (int)inPnt.x;
        int ypos = (int)inPnt.y;
        points.push_back(cv::Point2f( xpos, ypos));

        int ximgpos = (int)inPnt.x + bigwidth/2;
        int yimgpos = (int)inPnt.y + bigheight/2;

        if( ximgpos>=0 && ximgpos<bigwidth && yimgpos >=0 && yimgpos<bigheight ) {
            im.at<uchar>(yimgpos,ximgpos) = 100;
        }

        cv::Mat pnt = (cv::Mat_<double>(3,1)<< inPnt.x,inPnt.y,1.0);
        cv::Mat retpnt = invH * pnt;

        glm::vec2  uv;
        int xoripos = (int)(retpnt.at<double>(0)/retpnt.at<double>(2) + nWidth/2);
        int yoripos = (int)(retpnt.at<double>(1)/retpnt.at<double>(2) + nHeight/2);
        if( xoripos>=0 && xoripos<nWidth && yoripos >=0 &&  yoripos<nHeight ) {
            image.at<uchar>(yoripos,xoripos) = 255;
            im.at<uchar>(yimgpos,ximgpos) = 255;

            uv.x = xoripos/nWidth;
            uv.y = yoripos/nHeight;
        }



        interuv.push_back(uv);


    });
    cv::imwrite("iBmage.png",im);
    cv::imwrite("iOmage.png",image);

    // int nWidth = imgsize.width;
    // int nHeight = imgsize.height;
    // cv::Mat image(imgsize,CV_8UC1,cv::Scalar(0));
    // std::for_each(points.begin(), points.end(), [&](cv::Point2f v) {

    //     cv::Mat pnt = (cv::Mat_<double>(3,1)<< v.x,v.y,1.0);
    //     cv::Mat retpnt = invH*pnt;

    //     int xpos = (int)(retpnt.at<double>(0,0)/retpnt.at<double>(2,0) + nWidth/2);
    //     int ypos = (int)(retpnt.at<double>(1,0)/retpnt.at<double>(2,0) + nHeight/2);
    //     if( xpos>=0 && xpos<nWidth && ypos >=0 &&  ypos<nHeight ) {
    //         image.at<uchar>(xpos,ypos) = 255;
    //     }

    //     glm::vec2  uv;
    //     uv.x = xpos/nWidth;
    //     uv.y = ypos/nHeight;
    //     interuv.push_back(uv);


    // });
    //
    //cv::imwrite("ximage1.png",im);

    // std::vector<cv::Point2f> transformedPoints(points);
    // //cv::Mat invMat;
    // //cv::invert(H,invMat);

    // //pts_dst = cv2.transform(pts_src, H_inv.T)

    // //cv::transform( points,transformedPoints,invMat);

    // cv::perspectiveTransform(points, transformedPoints, invH);
    // //cv::projectPoints()

    // //cv::warpPerspective( points, transformedPoints, invH );

    // int nWidth = imgsize.width;
    // int nHeight = imgsize.height;
    // cv::Mat image(imgsize,CV_8UC1,cv::Scalar(0));
    // std::for_each(transformedPoints.begin(), transformedPoints.end(), [&](cv::Point2f v) {

    //     int xpos = (int)v.x + nWidth/2;
    //     int ypos = (int)v.y + nHeight/2;

    //     if( xpos>=0 && xpos<nWidth && ypos >=0 &&  ypos<nHeight ) {
    //         image.at<uchar>(v.y,v.x) = 255;
    //     }

    //     glm::vec2  uv;
    //     uv.x = xpos/nWidth;
    //     uv.y = ypos/nHeight;
    //     interuv.push_back(uv);

    // });
    // cv::imwrite("oriimage.png",image);


}

static void getOriImageUV(const std::vector< glm::vec3>&  interpoints,std::vector< glm::vec2>&  interuv,const glm::mat3 invH, const glm::vec2 imgsize,float dlt=0.001)
{

    std::vector< glm::vec3>  outlinePoint(4);
    outlinePoint[0]=glm::vec3(0,0,0);//left xmin
    outlinePoint[1]=glm::vec3(0,0,0);//right xmax
    outlinePoint[2]=glm::vec3(0,0,0);//zmin
    outlinePoint[3]=glm::vec3(0,0,0);//zmax
    std::for_each(interpoints.begin(), interpoints.end(), [&](glm::vec3 v)
    {
        if(  outlinePoint[0].x > v.x )  outlinePoint[0] = v;
        if(  outlinePoint[1].x < v.x )  outlinePoint[1] = v;
        if(  outlinePoint[2].z > v.z )  outlinePoint[2] = v;
        if(  outlinePoint[3].z < v.z )  outlinePoint[3] = v;
    });

    // std::for_each(outlinePoint.begin(), outlinePoint.end(), [&](glm::vec3 v)
    // {
    //     std::cout <<v.x<<","<<v.y<<","<<v.z<< std::endl;
    // });



    int nWidth = 1080;
    int nHeight = 1279;
    cv::Mat image(nHeight,nWidth,CV_8UC1,cv::Scalar(0));

    std::for_each(interpoints.begin(), interpoints.end(), [&](glm::vec3 v) {
        glm::vec3 inPnt;
        inPnt.x = v.x / dlt + nWidth  / 2.;
        inPnt.y = v.z / dlt + nHeight / 2.;
        inPnt.z = 1.0;
        glm::vec3 oPnt = invH * inPnt;
        oPnt /=oPnt.z;
        glm::vec2  uv;
        uv.x = oPnt.x/imgsize.x;
        uv.y = oPnt.y/imgsize.y;
        interuv.push_back(uv);

        int xpos = (int)inPnt.x;
        int ypos = (int)inPnt.y;

        if( xpos>=0 && xpos<nWidth && ypos >=0 && ypos<nHeight ) {
            image.at<uchar>(ypos,xpos) = 255;
        }

    });
    // cv::imwrite("ximage.png",image);
}


static bool  getPointFromLine2Plane( const glm::vec3 point1, const  glm::vec3 point2, const  glm::vec4 plane,glm::vec3& interpoint )
{
    glm::vec3  linedir  = glm::normalize( point2 - point1 );
    glm::vec3  planenorm = glm::normalize( glm::vec3(plane) );
    float dlt = glm::dot(linedir, planenorm);
    if( fabs(dlt) < 0.000001 ) return false; // 平行
    interpoint = point1 - linedir*( plane.w + glm::dot( planenorm, point1 ) ) / dlt;
    return true;
}

// static bool  getPointFromLine2Plane2( const glm::vec3 linepoint, const  glm::vec3 linenoraml, const  glm::vec4 plane,glm::vec3& interpoint )
// {
//     glm::vec3  linedir  = glm::normalize( linenoraml );
//     glm::vec3  planenorm = glm::normalize( glm::vec3(plane) );
//     float dlt = glm::dot(linedir, planenorm);
//     if( fabs(dlt) < 0.000001 ) return false; // 平行
//     interpoint = linepoint - linedir*( plane.w + glm::dot( planenorm, linepoint ) ) / dlt;


//     return true;
// }


bool Bowl::generate_mesh_(const float max_size_vert, std::vector<float>& vertices, std::vector<uint>& indices)
{
    if (fabs(param_a) <= epsilon || fabs(param_b) <= epsilon || fabs(param_c) <= epsilon)
        return false;
    if (rad <= 0.f || inner_rad <= 0.f)
        return false;
    if (set_hole && hole_rad <= 0.f)
        return false;
    auto a = param_a;
    auto b = param_b;
    auto c = param_c;

    vertices.clear();

    indices.clear();

    //范围
    float startpolar_coord =  -PI; //
    float endpolar_coord =  0;
    /*
            prepare grid mesh in polar coordinate with r - radius and theta - angle
    */
    // texture coordinates generate (u, v) [0, 1]

    std::vector<float> texture_u = meshgen::linspace(0.f, (1.f + eps_uv), max_size_vert);
    auto texture_v = texture_u;

    auto r = meshgen::linspace(hole_rad, rad, max_size_vert); // min_size = 0.f, max_size = 100.f,//半径方向
    auto theta = meshgen::linspace(startpolar_coord, endpolar_coord, max_size_vert);// 圆周方向

    auto mesh_pair = meshgen::meshgrid(r, theta); // 网格尺寸




    auto R = std::get<0>(mesh_pair);
    auto THETA = std::get<1>(mesh_pair);
    size_t grid_size = R.size();

    std::vector<float> x_grid;
    std::vector<float> y_grid;
    std::vector<float> z_grid;

    glm::vec3 p1(0,1.8,0);
    glm::vec4 plane1(0,1,0,0);

    float dlt = 0.001;
    std::vector< glm::vec3>  interpoints;
    //坐标系统，是XZ 平面  高度为y方向  ，XZY
    // Convert to rectangular coordinates
    // x = r*cos(theta), z = r*sin(theta), y/c = (x^2)/(a^2) + (z^2)/(b^2);
    glm::vec3 p3;
    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            auto x = R(i, j) * cos(THETA(i, j));
            auto z = R(i, j) * sin(THETA(i, j));
            auto y = c * (pow((x / a), 2) + pow((z / b), 2));
            x_grid.push_back(x);
            z_grid.push_back(z);
            y_grid.push_back(y);

            if( getPointFromLine2Plane(p1, glm::vec3 (x,y,z),plane1,p3) ) {
                interpoints.push_back( glm::vec3(p3));
            } else {
                interpoints.push_back( glm::vec3(p3));
            }
        }
    }

    createImage(interpoints,1080,1279,dlt);
    std::vector< glm::vec2>  interuv;

    cv::Mat invhmat;
    kmod::g_camerasParam[0]->cvinvHomo_norm.copyTo(invhmat);

    //cv::Size s = kmod::CameraParameter::imgSize; // kmod::CameraParameter::BigimgSize;
    // std::cout<<"hmat"<<hmat<<std::endl;
    // cv::invertAffineTransform( hmat,invhmat);
    std::cout<<"invhmat"<<invhmat<<std::endl;
    std::cout<<"type"<<invhmat.type()<<std::endl;

    if(invhmat.rows == 3 && invhmat.cols == 3 && invhmat.type() == CV_64F) {
        // glm::mat3 matGLM;
        // for (int i = 0; i < 3; ++i) {
        //     for (int j = 0; j < 3; ++j) {
        //         matGLM[i][j] = invhmat.at<double>(i, j);
        //         std::cout << matGLM[i][j] << " ";
        //     }
        // }
        //  glm::mat3 imat = glm::inverse(matGLM);

        //getOriImageUV(interpoints,interuv,matGLM,glm::vec2(s.width,s.height));
        getOriImageUV(interpoints,interuv,invhmat,kmod::CameraParameter::imgSize,dlt);
        // for (int i = 0; i < 3; ++i) {
        //     for (int j = 0; j < 3; ++j) {
        //         std::cout << imat[i][j] << " ";
        //     }
        //     std::cout << std::endl;
        // }

    }

    std::cout<<"interuv.size()" <<interuv.size() << std::endl;
    std::cout<<"grid_size*grid_size" <<grid_size*grid_size << std::endl;

    // std::cout<<interuv.size()<<std::endl;


    // 确保cv::Mat是3x3的浮点型矩阵
    //



    // // 确保cv::Mat是3x3的浮点数矩阵
    // if (invhmat.rows == 3 && invhmat.cols == 3 && invhmat.type() == CV_32F) {
    //     // 将数据从cv::Mat拷贝到glm::mat3
    //     memcpy(glm::value_ptr(matGLM), invhmat.data, 9 * sizeof(float));

    // }


    /*
            find start level - level when disk passes from to elliptic paraboloid
    */
    auto min_y = 0.f;
    auto idx_min_y = 0u; // index y - component when transition between disk and paraboloid
    auto max_y=0.f;
    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size; ++j) {

            auto x = x_grid[j + i * grid_size];
            auto z = z_grid[j + i * grid_size];

            if (lt_radius(x, z, inner_rad)) { // check level of paraboloid//找到在属于小于小圈内的Y方向的取值
                min_y = y_grid[j + i * grid_size];
                idx_min_y = i;
                break;
            }

        }
    }




    /*
            generate mesh vertices for disk and elliptic paraboloid
    */
    auto half_grid = grid_size / 2;
    auto vertices_size = 0;
    auto offset_idx_min_y = 0;
    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            auto x = x_grid[j + i * grid_size];
            auto z = z_grid[j + i * grid_size];


            auto y = min_y;
            if (gt_radius(x, z, inner_rad)) // check level of paraboloid
                y = y_grid[j + i * grid_size];

            vertices.push_back(x + cen[0]);
            vertices.push_back(y + cen[1]);
            vertices.push_back(z + cen[2]);
            vertices_size += 3;

            if (useUV) { // texture coordinates
                auto u = texture_u[j];
                auto v = texture_v[i];
                auto uv = interuv[j + i * grid_size];
                // if (i == 0 && j == 0 &&  !set_hole) // center disk
                //     u = texture_u[half_grid];
                //vertices.push_back(u);
                //vertices.push_back(v);
                vertices.push_back(uv.x);
                vertices.push_back(uv.y);
            }
        }
    }




    /*
            generate indices by y-order
    */

    idx_min_y -= offset_idx_min_y;
    int32 last_vert = vertices_size / _num_vertices;
    generate_indices(indices, grid_size, idx_min_y, last_vert);

    return true;
}


void Bowl::generate_indices(std::vector<uint>& indices, const uint grid_size, const uint idx_min_y, const int32 last_vert) {
    bool oddRow = false;

    for (uint y = 0; y < grid_size - 1; ++y) {
        if (!oddRow) // even rows: y == 0, y == 2; and so on
        {
            for (uint x = 0; x < grid_size; ++x)
            {
                auto current = y * grid_size + x;
                auto next = (y + 1) * grid_size + x;
                /* change order when change disk to elliptic paraboloid */
                if (y == idx_min_y && x == 0) {
                    std::swap(current, next);
                    indices.push_back(current - grid_size);
                    indices.push_back(next);
                    indices.push_back(current);
                    continue;
                }
                if (set_hole && (current >= last_vert || next >= last_vert))
                    continue;
                indices.push_back(current);
                indices.push_back(next);
            }
        }
        else
        {
            for (int x = grid_size - 1; x >= 0; --x)
            {
                auto current = (y + 1) * grid_size + x;
                auto prev = y * grid_size + x;
                /* change order when change disk to elliptic paraboloid */
                if (y == idx_min_y && x == grid_size - 1) {
                    indices.push_back(current - grid_size);
                    indices.push_back(current);
                    indices.push_back(prev);
                    continue;
                }
                if (set_hole && (current >= last_vert || prev >= last_vert))
                    continue;
                indices.push_back(current);
                indices.push_back(prev);

            }
        }
        oddRow = !oddRow;
    }
}



bool HemiSphere::generate_mesh_(std::vector<float>& vertices, std::vector<uint>& indices)
{
    if (set_hole && hole_rad <= 0.f)
        return false;

    vertices.clear();

    indices.clear();

    for(int y = 0; y <= y_segment; ++y) {
        for(int x = 0; x <= x_segment; ++x) {
            float xSegm = (float)x / (float)x_segment;
            float ySegm = (float)y / (float)y_segment;
            float xPos = std::cos(xSegm * 2.0 * PI) * std::sin(ySegm * half_pi);
            float yPos = 1.0f - std::cos(ySegm * half_pi);
            float zPos = std::sin(xSegm * 2.0 * PI) * std::sin(ySegm * half_pi);

            vertices.push_back(xPos);
            vertices.push_back(yPos);
            vertices.push_back(zPos);
            vertices.push_back(xSegm);
            vertices.push_back(ySegm);
        }

    }


    bool oddRow = false;
    for(uint y = 0; y < y_segment - 1; ++y) {
        if (!oddRow) {

            for(uint x = 0; x <= x_segment; ++x) {
                indices.push_back(y * (x_segment + 1) + x);
                indices.push_back((y + 1)* (x_segment + 1) + x);
            }
        }
        else {

            for(int x = x_segment; x >= 0; --x) {
                indices.push_back((y + 1)* (x_segment + 1) + x);
                indices.push_back(y * (x_segment + 1) + x);
            }
        }
        oddRow = !oddRow;
    }


    return true;
}
}
