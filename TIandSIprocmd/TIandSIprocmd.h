/**
 * TIandSIprocmd
 * (TIandSI Professional CMD version)
 *
 * 雷霄骅 Lei Xiaohua,张晖 Zhang Hui
 * leixiaohua1020@126.com
 * zhanghuicuc@gmail.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 *
 * http://blog.csdn.net/leixiaohua1020
 * 
 * 本工程可以计算一个压缩视频的时间信息TI（Temporal perceptual Information，
 * 也可以称时间复杂度）和空间信息SI（Spatial perceptual Information，也可以
 * 称空间复杂度）。计算方法出自标准：ITU-R BT.1788
 *
 * Professional: 支持压缩码流作为输入
 *
 * This software can calculate a video bitstream's TI(Temporal perceptual Information) 
 * and SI(Spatial perceptual Information) according to ITU-R BT.1788.
 *
 * Professional: Support bitstreams (not just raw data such as YUV, RGB, etc.) as Input.
 *
 */
#ifndef TIANDSIPROCMD_H
#define TIANDSIPROCMD_H

#include <stdio.h>
#include <tchar.h>
#include <math.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <xmmintrin.h>
#include <emmintrin.h>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "sdl/SDL.h"
#include "sdl/SDL_main.h"
};
#endif