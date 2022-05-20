# 设置cmake最低版本
cmake_minimum_required(VERSION 3.10.2)

project("ccamera")

# 添加include目录路径
include_directories( include )

set(ffmpeg_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})

# 添加ffmpeg库
add_library( avcodec SHARED IMPORTED )
set_target_properties( avcodec
                       PROPERTIES IMPORTED_LOCATION
                       ${ffmpeg_lib_dir}/libavcodec.so )

add_library( avfilter SHARED IMPORTED )
set_target_properties( avfilter
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_lib_dir}/libavfilter.so )

add_library( avformat SHARED IMPORTED )
set_target_properties( avformat
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_lib_dir}/libavformat.so )

add_library( avutil SHARED IMPORTED )
set_target_properties( avutil
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_lib_dir}/libavutil.so )

add_library( swresample SHARED IMPORTED )
set_target_properties( swresample
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_lib_dir}/libswresample.so )

add_library( swscale SHARED IMPORTED )
set_target_properties( swscale
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_lib_dir}/libswscale.so )



# 添加yuv库
add_library(yuv SHARED IMPORTED )
set_target_properties( yuv
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libyuv.so )

add_library(CCMedia
            SHARED
            record/native-lib.cpp
            record/MediaMuxer.cpp
            record/AudioEncoder.cpp
            record/VideoEncoder.cpp
            record/MediaRecorder.cpp
            record/RecordParam.h
            record/Resampler.cpp
            record/AVMediaData.cpp
            record/MediaHeader.h
            record/YuvConvertor.cpp
            record/YuvData.cpp
            )

find_library(log-lib log)

target_link_libraries(CCMedia
                       -landroid
                       ${log-lib}
                       avcodec
                       avfilter
                       avformat
                       avutil
                       swresample
                       swscale
                       yuv)