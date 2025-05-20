cmake_minimum_required(VERSION 3.5)
project(cameraFusion VERSION 0.1 LANGUAGES CXX CUDA)

set(CMAKE_BUILD_TYPE "Release") #Release需加引号
#add_compile_options("/utf-8")



set(CMAKE_INCLUDE_CURRENT_DIR ON)

#set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)




find_package(CUDA REQUIRED)

enable_language(CUDA)
set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)
set(
         CMAKE_NVCC_FLAGS
         ${CMAKE_NVCC_FLAGS};
         -Xcompiler
         -fopenmp
         -O3
         -use_fast_math
         -gencode arch=compute_62,code=sm_62
)
set(CMAKE_CUDA_FLAGS)


if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
set(CMAKE_CUDA_ARCHITECTURES 70 75 80)
endif(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)

find_package(OpenMP REQUIRED)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS})

function(FolderTraRCS_GetUi result rootdir)
	file(GLOB_RECURSE allcode "${rootdir}/*.ui")
	set(${result} ${allcode} PARENT_SCOPE)
endfunction()
#Source Code From .../Qt/Qt5.12.2/5.12.2/winrt_x64_msvc2017/lib/cmake/Qt5Widgets/Qt5WidgetsMacros   QT5_WRAP_UI
#How to Use: QT5_WRAP_UI_DIY(<filelist>(return) UIDIR <.ui dir> OUTDIR <gen .h dir>)
function(QT5_WRAP_UI_DIY outfiles )

    set(options)
    set(oneValueArgs UIDIR OUTDIR)
    set(multiValueArgs OPTIONS)
	
	cmake_parse_arguments(_WRAP_UI "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	
	set(ui_rootdir ${_WRAP_UI_UIDIR})
    set(ui_options ${_WRAP_UI_OPTIONS})
	set(ui_outdir ${_WRAP_UI_OUTDIR})
	
	FolderTraRCS_GetUi(ui_files ${ui_rootdir})
	
	message("\n")
	message("--------------QT5_WRAP_UI_DIY-------------")
    foreach(it ${ui_files})
		
		get_filename_component(dir ${it} DIRECTORY)
		string(REPLACE "${ui_rootdir}" "" reldir ${dir})
		get_filename_component(filename ${it} NAME_WE)
		
        get_filename_component(infile ${it} ABSOLUTE)
		message("${it}---------->Gen: ${ui_outdir}${reldir}/ui_${filename}.h")
        set(outfile "${ui_outdir}${reldir}/ui_${filename}.h")
		
        add_custom_command(OUTPUT ${outfile}
          COMMAND ${Qt5Widgets_UIC_EXECUTABLE}
          ARGS ${ui_options} -o ${outfile} ${infile}
          MAIN_DEPENDENCY ${infile} VERBATIM)
        set_source_files_properties(${infile} PROPERTIES SKIP_AUTOUIC ON)
        set_source_files_properties(${outfile} PROPERTIES SKIP_AUTOMOC ON)
        set_source_files_properties(${outfile} PROPERTIES SKIP_AUTOUIC ON)
        list(APPEND ${outfiles} ${outfile})
		
    endforeach()
	message("\n")
    set(${outfiles} ${${outfiles}} PARENT_SCOPE)
	
endfunction()


#Source Code From .../Qt/Qt5.12.2/5.12.2/winrt_x64_msvc2017/lib/cmake/Qt5Core/Qt5CoreMacros.cmake   QT5_WRAP_CPP
#How to Use: QT5_WRAP_CPP_DIY(<filelist>(return) HEADERDIR <.h dir> OUTDIR <gen .cpp dir>)
function(QT5_WRAP_CPP_DIY outfiles )

    # get include dirs
    qt5_get_moc_flags(moc_flags)

    set(options)
    set(oneValueArgs TARGET HEADERDIR CPPDIR OUTDIR)
    set(multiValueArgs OPTIONS DEPENDS)

    cmake_parse_arguments(_WRAP_CPP "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(moc_options ${_WRAP_CPP_OPTIONS})
    set(moc_target ${_WRAP_CPP_TARGET})
    set(moc_depends ${_WRAP_CPP_DEPENDS})
	
	set(moc_headerdir ${_WRAP_CPP_HEADERDIR})
	set(moc_outdir ${_WRAP_CPP_OUTDIR})
	FolderTraRCS_GetCode(moc_files ${moc_headerdir})
	
	message("\n")
	message("--------------QT5_WRAP_CPP_DIY-------------")
    foreach(it ${moc_files})
	
		get_filename_component(dir ${it} DIRECTORY)
		string(REPLACE "${moc_headerdir}" "" reldir ${dir})
        
		get_filename_component(it ${it} ABSOLUTE)
        qt5_make_output_file(${it} moc_ cpp outfile)
		get_filename_component(filename ${outfile} NAME)
		
		message("${it}---------->Gen: ${moc_outdir}${reldir}/${filename}")
		set(outfile "${moc_outdir}${reldir}/${filename}")
		
        qt5_create_moc_command(${it} ${outfile} "${moc_flags}" "${moc_options}" "${moc_target}" "${moc_depends}")
        list(APPEND cppfiles ${outfile})
		
    endforeach()
	message("\n")
	
	
    set(${outfiles} ${cppfiles} PARENT_SCOPE)
	
endfunction()




#if (POLICY CMP0072)
#  set(OpenGL_GL_PREFERENCE LEGACY)
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)

set(OpenCV_DIR "/home/k/usr/local/lib/cmake/opencv4/")

find_package( OpenCV REQUIRED ) # 找到需求的库或者组件
# 矩阵相关库
find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)
#find_package(Eigen3 REQUIRED)
#find_package(GLUT REQUIRED)
#find_package(GLU REQUIRED)
#find_package(FreeGLUT REQUIRED)
#find_package(ZLToolKit REQUIRED)
#link_directories(/home/ps/devlib/lib/)
# 窗口管理库
#find_package(glfw3 REQUIRED)

find_library(AVFORMAT_LIB avformat)
find_library(AVCODEC_LIB avcodec)
find_library(AVUTIL_LIB avutil)
find_library(AVSWSCALE_LIB swscale)
find_library(AVSWRESAMPLE_LIB swresample)
find_library(AVDEVICE_LIB avdevice)
find_library(AVAVFILTER_LIB avfilter)

# add_subdirectory (libs)
# include_directories(
#     libs/assimp-3.3.1/include/
# )
set(TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/resource/cameraFusion_yue_CN.ts)



include_directories( ${OpenCV_INCLUDE_DIRS}
                     #${EIGEN3_INCLUDE_DIR}   
                     ${CUDA_INCLUDE_DIRS}  
                     ${CMAKE_CURRENT_BINARY_DIR}/include  
                     ${CMAKE_CURRENT_BINARY_DIR} 
                     ${CMAKE_CURRENT_SOURCE_DIR}/include
                     ${PROJECT_SOURCE_DIR}/include                   
                     /usr/local/include                    
                     #${GLUT_INCLUDE_DIR}
                     ) # 添加一个用于包含的文件去构建的文件夹/home/ps/devlib/include/
 #                    include_directories()


set(MOC_HEADERS
        include/ImageWidget.h
        include/extrinsicdialog.h
        include/extrinsiclabel.h
        include/qgpurender.h
        include/aboutdlg.h
        include/calibratewidget.h
        include/cameralistwidget.h
        include/cameraCalibrator.h
        include/mainwindow.h
)#add moc

                     

file(GLOB file_ui ${CMAKE_CURRENT_SOURCE_DIR}/ui/*.ui)


QT5_WRAP_UI(ui_FILES ${file_ui} )
QT5_WRAP_CPP( moc_SRCS ${MOC_HEADERS} )

file(GLOB file_qrc  ${CMAKE_CURRENT_SOURCE_DIR}/resource/*.qrc)
file(GLOB file_ts   ${CMAKE_CURRENT_SOURCE_DIR}/resource/*.ts)
file(GLOB file_cpp  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB file_cu  ${CMAKE_CURRENT_SOURCE_DIR}/cusrc/*.cu)




# 设置库文件路径
#link_directories(${GLUT_LIBRARY_DIR})


set( PROJECT_SOURCES 	 
        ${file_cpp}
        ${file_qrc}
        ${file_ts}
        ${file_ui}
        ${moc_cu}  
        ${moc_SRCS}  
        )


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(cameraFusion
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET cameraFusion APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(cameraFusion SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(cameraFusion
            ${PROJECT_SOURCES}
        )
    endif()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()
#GLU glfw
target_link_libraries(cameraFusion PRIVATE Qt${QT_VERSION_MAJOR}::Widgets pthread  glm::glm EGL GL assimp z 
                   libZLToolKit.a
                   ${OpenCV_LIBS}
                   ${GLUT_LIBRARIES}
                   ${AVFORMAT_LIB}
                   ${AVCODEC_LIB}
                   ${AVUTIL_LIB}
                   ${AVSWSCALE_LIB}
                   ${AVSWRESAMPLE_LIB}
                   ${AVDEVICE_LIB}
                   ${AVAVFILTER_LIB}
                   ${CUDA_LIBRARIES}
                   )
#target_link_libraries(cameraFusion  ) # 链接的库

set_target_properties(cameraFusion PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(cameraFusion)
endif()


set_target_properties(cameraFusion 
			PROPERTIES OUTPUT_NAME "cameraFusion" 
			CUDA_SEPARABLE_COMPILATION ON
)
set_property(TARGET cameraFusion PROPERTY CUDA_STANDARD 14)
target_compile_options(cameraFusion PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:${CMAKE_NVCC_FLAGS}>)