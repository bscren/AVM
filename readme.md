sudo apt-get install liblapack-dev libsuitesparse-dev libcxsparse3 libgflags-dev libgoogle-glog-dev libgtest-dev

sudo updatedb  
locate eigen3  
 pkg-config --modversion eigen3

dowload eigen3

mkdir build
cd build
cmake ..
sudo make install

git clone  https://github.com/fmtlib/fmt.git
cd fmt
mkdir build
cd build
cmake ..
make
sudo make install

git clone https://github.com/strasdat/Sophus.git
cd Sophus/
mkdir build
cd build
cmake ..
make
sudo make install

git clone https://github.com/ceres-solver/ceres-solver.git

cd ceres-solver
mkdir build
cd build
cmake ..
make -j4
sudo make install

sudo apt-get install qt5-qmake qt5-default libqglviewer-dev-qt5 libsuitesparse-dev libcxsparse3 libcholmod3

git clone https://github.com/RainerKuemmerle/g2o.git   # 克隆最新版本g2o
cd g2o
mkdir build
cd build
cmake ..
make -j6      # 注意，这里尽量使用更多的j，否则g2o安装很慢（-j4  -j6等等）
sudo make install


sudo apt-get install libassimp-dev
sudo apt-get install libassipm3
sudo apt-egt install assimp-utils

sudo apt install libzzip-dev zlib1g-dev

nanovg for ui



// // #define lowp
// // #define mediump
// // #define highp
// // #line 3
// // #ifdef GL_FRAGMENT_PRECISION_HIGH
// //   precision highp float;
// // #else
// //   precision mediump float;
// // #endif

// // #ifdef GL_FRAGMENT_PRECISION_HIGH
// //   //precision highp float;
// //   precision mediump float;
// // #else
// //   //precision mediump float;
// //   precision mediump float;
// // #endif

//layout (std140, row_major) uniform transformBlock {
//    mat3 intrinsic;
//    vec4 discoeffs;
//    mat3 H;
//};