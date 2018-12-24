#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <sys/time.h>

#define APP_VERSION         "V0.0.2"

#define VIDEO_DEVICE        "/dev/video0"
#define VIDEO_INPUT_NUM     0           //0 -> csi0:vip0； 1 -> csi0: vip1
#define CMOS_INPUT_WIDTH    1920        //sensor固定输出1920*1080的图像
#define CMOS_INPUT_HEIGHT   1080
#define IMAGE_SIZE          (CMOS_INPUT_WIDTH * CMOS_INPUT_WIDTH * 2)
#define BUFFER_COUNT        5           //最多申请5个缓冲区
/* CMOS_PIX_FMT
 * V4L2_PIX_FMT_SBGGR10     //backup: 10bit raw格式 for imx291(parallel 10-bit raw) and ov5647(mipi 2lane raw)
 * V4L2_PIX_FMT_NV16        //backup: just for test  
 * V4L2_PIX_FMT_YUYV        //backup: just for test
*/
#define CMOS_PIX_FMT    V4L2_PIX_FMT_UYVY
/* CMOS_FIELD
 * 指明扫描方式，已经去除驱动中相关代码，此处必须指定(默认为 V4L2_FIELD_NONE)
 * V4L2_FIELD_INTERLACED
 * V4L2_FIELD_ANY
*/
#define CMOS_FIELD  V4L2_FIELD_NONE  

//image save path
#define IMG_SAVE_PATH "/opt/1080p.yuv"

#define DEBUG_SWITCH 1
#if(DEBUG_SWITCH >= 3)
#define DBG_PRINT(fmt, args...) printf("Debug: %s(+ %d)" fmt "\n", __func__, __LINE__, ##args)
#define WARN_PRINT(fmt, args...) printf("Warning: %s(+ %d)" fmt "\n", __func__, __LINE__, ##args)
#define ERR_PRINT(fmt, args...) printf("Error: %s(+ %d)" fmt "\n", __func__, __LINE__, ##args)

#elif(DEBUG_SWITCH >= 2)
#define DBG_PRINT(fmt, args...) 
#define WARN_PRINT(fmt, args...) printf("Warning: %s(+ %d)" fmt "\n", __func__, __LINE__, ##args)
#define ERR_PRINT(fmt, args...) printf("Error: %s(+ %d)" fmt "\n", __func__, __LINE__, ##args)

#else
#define DBG_PRINT(fmt, args...) 
#define WARN_PRINT(fmt, args...) 
#define ERR_PRINT(fmt, args...) printf("Error: %s(+ %d)" fmt "\n", __func__, __LINE__, ##args)
#endif

#define COLOR_PRINT_LENGTH  256
#define USING_ECHO_COLOR_PRINT 0    //0: printf, 1: echo
#define COLOR_PRINT_BLACK_RED(fmt, args...) do{ \
                                                char buf[COLOR_PRINT_LENGTH] = {}; \
                                                sprintf(buf, fmt, ##args); \
                                                color_print(0, 40, 31, buf); \
                                                }while(0)
#define COLOR_PRINT_BLACK_GREEN(fmt, args...) do{ \
                                                char buf[COLOR_PRINT_LENGTH] = {}; \
                                                sprintf(buf, fmt, ##args); \
                                                color_print(0, 40, 32, buf); \
                                                }while(0)
#define COLOR_PRINT_UNDERLINE_RED_YELLOW(fmt, args...) do{ \
                                                char buf[COLOR_PRINT_LENGTH] = {}; \
                                                sprintf(buf, fmt, ##args); \
                                                color_print(4, 41, 33, buf); \
                                                }while(0)

typedef struct _globalInfo{
    int width;
    int height;
    int size;
    int argc;
    char** argv;

    //pthread_mutex mutex;
    int fdCam;
    int err;
    unsigned char* vbufp[BUFFER_COUNT]; //video buffer pointer

	//frame buf for img
	unsigned char frameBuf[IMAGE_SIZE];
	int frameSize;
    //time for saving img at start or end
	struct timeval tv[2];
}globalInfo_t;

#ifdef __cplusplus
extern "C"{
#endif
    extern void color_print(unsigned char style, unsigned char background, unsigned char front, char* info);
    extern globalInfo_t gInfo;
    extern void __attribute__((constructor)) initializer_before_main();
    extern void initGlobalInfo(globalInfo_t* g, int argc, char* argv[]);
    extern void emergency_sighandler(int signum);

    extern int _requestResourceCam(globalInfo_t* g);
    extern int _releaseResourceCam(globalInfo_t* g);
    extern int _getImageFrame(globalInfo_t* g);
	extern int _saveImgFrame(unsigned char* dst, unsigned char* src, int size);
#ifdef __cplusplus
}
#endif

#endif // GLOBAL_H
