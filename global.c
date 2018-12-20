#include "global.h"

globalInfo_t gInfo;

/*
 *echo 参数释义：
 * 

-e: 打开反斜杠转义 (默认不打开) ,可以转义 “\n, \t” 等
-n: 在最后不自动换行

style:
    0：默认    1：设置高亮加粗    4：下划线    5：闪烁
background:
    40：黑    41: 红    47: 白
front:
    30: 黑     31：红    32：绿    33：黄    34：蓝    35：紫    36：青    37：白
info: msg
*/
void color_print(unsigned char style, unsigned char background, unsigned char front, char* info)
{
    char cmd[COLOR_PRINT_LENGTH + 64] = {};
    if(NULL != info)
    {
        sprintf(cmd, "echo -e \"\033[%d;%d;%dm %s \033[0m\n\"", style, background, front, info);
        system(cmd);
    }
}

//constructor func
void __attribute__((constructor)) initializer_before_main(){

    bzero(&gInfo, sizeof(globalInfo_t));
}

void initGlobalInfo(globalInfo_t* g, int argc, char* argv[])
{
    if(!g){
        ERR_PRINT("globalInfo is NULL");
        return;
    } 

	bzero(g, sizeof(globalInfo_t));
	
    g->fdCam = -1;
    g->width = CMOS_INPUT_WIDTH;
    g->height = CMOS_INPUT_HEIGHT;
    g->argc = argc;
    g->argv = argv;

    signal(SIGINT, emergency_sighandler);
    signal(SIGTERM, emergency_sighandler);
}

void emergency_sighandler(int signum)
{
    DBG_PRINT("signum: %d", signum);

    _releaseResourceCam(&gInfo);

    kill(getpid(), 9);

    exit(0);
}

int openCam(globalInfo_t* g)
{
    int ret = 0;
    if(!g)
    {
        ret = -EINVAL;
        ERR_PRINT("globalInfo is NULL");
    }else
    {
        g->fdCam = open(VIDEO_DEVICE, O_RDWR);	// | O_NONBLOCK);
        if(g->fdCam < 0)
        {
            ret = -EAGAIN;
            ERR_PRINT("open failed.");
        }
    }
    return ret;
}

int closeCam(globalInfo_t* g)
{
    if((NULL != g) && (g->fdCam > 0))
    {
        close(g->fdCam);
        g->fdCam = -1;
    }

    return 0;
}

int chooseCam(globalInfo_t* g, int input)
{
    int ret = 0;
    if(!g)
    {
        ret = -EINVAL;
        ERR_PRINT("globalInfo is Null");
        return ret;
    }
#if 0
    struct v4l2_capability cap;
    ret = ioctl(g->fdCam, VIDIOC_QUERYCAP, &cap);
    if(0 != ret)
    {
        ERR_PRINT("Can't get the information\n");
    }else
    {
    /**
             * struct v4l2_capability - Describes V4L2 device caps returned by VIDIOC_QUERYCAP
             *
             * @driver:	   name of the driver module (e.g. "bttv")
             * @card:	   name of the card (e.g. "Hauppauge WinTV")
             * @bus_info:	   name of the bus (e.g. "PCI:" + pci_name(pci_dev) )
             * @version:	   KERNEL_VERSION
             * @capabilities: capabilities of the physical device as a whole
             * @device_caps:  capabilities accessed via this particular device (node)
             * @reserved:	   reserved fields for future extensions
        struct v4l2_capability {
            __u8	driver[16];
            __u8	card[32];
            __u8	bus_info[32];
            __u32   version;
            __u32	capabilities;
            __u32	device_caps;
            __u32	reserved[3];
        };
            */

        DBG_PRINT("cap.driver: %s", cap.driver);
        DBG_PRINT("cap.card: %s", cap.card);
        DBG_PRINT("cap.bus_info: %s", cap.bus_info);
        DBG_PRINT("cap.version: %#x", cap.version);
        DBG_PRINT("cap.capabilities: %#x\n", cap.capabilities);
        //1. check capability
        if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
        {
            ret = -EAGAIN;
            ERR_PRINT("Error: This is not a video capture device\n");
        }
        if(!(cap.capabilities & V4L2_CAP_STREAMING))
        {
            ret = -EIO;
            ERR_PRINT("Error: This is not I/O flow control\n");
        }
        if(!(cap.capabilities & V4L2_CAP_READWRITE))
        {
            ret = -ENXIO;
            ERR_PRINT("Error: I/O flow can't read/write.\n");
        }
    }
#endif

    if(0 == ret)
    {
        ret = ioctl(g->fdCam, VIDIOC_S_INPUT, &input);
    }else
    {
        ERR_PRINT("Can't set input num.");
    }
    return ret;
}

int configCam(globalInfo_t* g)
{
    unsigned int i = 0;
    int ret = 0;
    struct v4l2_format format;
    bzero(&format, sizeof(struct v4l2_format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//帧类型，用于视频捕获设备
    //V4L2_PIX_FMT_SBGGR10;//10bit raw格式 for imx291(parallel 10-bit raw) and ov5647(mipi 2lane raw)
    //V4L2_PIX_FMT_NV16 
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_SBGGR10;
    format.fmt.pix.width    = CMOS_INPUT_WIDTH;//分辨率
    format.fmt.pix.height   = CMOS_INPUT_HEIGHT;
    //指明扫描方式，已经去除驱动中相关代码，此处必须指定
    format.fmt.pix.field    = V4L2_FIELD_NONE;//V4L2_FIELD_INTERLACED;//V4L2_FIELD_ANY;
	/*
	   //videodev2.h
	   enum v4l2_field {
	   	V4L2_FIELD_ANY           = 0, // driver can choose from none, top, bottom, interlaced depending on whatever it thinks is approximate ... 
		V4L2_FIELD_NONE          = 1, // this device has no fields ... 
		V4L2_FIELD_TOP           = 2, // top field only 
		V4L2_FIELD_BOTTOM        = 3, // bottom field only 
		V4L2_FIELD_INTERLACED    = 4, // both fields interlaced 
		V4L2_FIELD_SEQ_TB        = 5, // both fields sequential into one buffer, top-bottom order 
		V4L2_FIELD_SEQ_BT        = 6, // same as above + bottom-top order
		V4L2_FIELD_ALTERNATE     = 7, // both fields alternating into separate buffers 
		V4L2_FIELD_INTERLACED_TB = 8, // both fields interlaced, top field first and the top field is transmitted first
		V4L2_FIELD_INTERLACED_BT = 9, // both fields interlaced, top field first and the bottom field is transmitted first
	};
	*/

    //包含vidioc_try_fmt设置当前窗口格式，扫描方式, vidioc_s_fmt_vid_cap获取当前mipi数据信息, mipi bsp的使能
    ret = ioctl(g->fdCam, VIDIOC_S_FMT, &format);
    if (ret != 0)
    {
    ERR_PRINT("ioctl(VIDIOC_S_FMT) failed %d(%s)", errno, strerror(errno));
    }else
    {
        //申请缓冲区
        struct v4l2_requestbuffers req;
        req.count = BUFFER_COUNT;//缓冲帧个数
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//缓冲帧数据格式
        req.memory = V4L2_MEMORY_MMAP;//内存映射方式
        ret = ioctl(g->fdCam, VIDIOC_REQBUFS, &req);//申请缓冲区
        if (ret != 0)
        {
            ERR_PRINT("ioctl(VIDIOC_REQBUFS) failed %d(%s)", errno, strerror(errno));
        }else
        {
            DBG_PRINT("req.count: %d", req.count);
            if (req.count < BUFFER_COUNT)
            {
                ret = -ENOMEM;
                ERR_PRINT("request buffer failed");
            }else
            {
                struct v4l2_buffer buffer;
                bzero(&buffer, sizeof(struct v4l2_buffer));
                buffer.type = req.type;
                buffer.memory = V4L2_MEMORY_MMAP;
                for (i=0; i<req.count; i++)
                {
                    buffer.index = i;
                    ret = ioctl(g->fdCam, VIDIOC_QUERYBUF, &buffer);//查询已经分配的V4L2的视频缓冲区的相关信息，包括视频缓冲区的使用状态、在内核空间的偏移地址、缓冲区长度等
                    if(ret != 0)
                    {
                        ERR_PRINT("ioctl(VIDIOC_QUERYBUF) failed %d(%s)", errno, strerror(errno));
                    }else
                    {
                        gInfo.vbufp[i] = (unsigned char*)mmap(NULL, buffer.length, PROT_READ|PROT_WRITE, MAP_SHARED, gInfo.fdCam, buffer.m.offset);//调用函数mmap把内核空间地址映射到用户空间，这样应用程序才能够访问位于内核空间的视频缓冲区
                        if(gInfo.vbufp[i] == MAP_FAILED)
                        {
                            ret = -ENOMEM;
                            ERR_PRINT("mmap() failed %d(%s)", errno, strerror(errno));
                        }else
                        {
                            DBG_PRINT("Frame buffer ptr:%p\n", gInfo.vbufp[i]);
                            DBG_PRINT("buffer.length: %d", buffer.length);
                            DBG_PRINT("buffer.m.offset: %d", buffer.m.offset);
                            DBG_PRINT("buffer.type: %d = V4L2_BUF_TYPE_VIDEO_CAPTURE(%d)", buffer.type, V4L2_BUF_TYPE_VIDEO_CAPTURE);
                            DBG_PRINT("buffer.memory: %d = V4L2_MEMORY_MMAP(%d)", buffer.memory, V4L2_MEMORY_MMAP);
                            DBG_PRINT("buffer.index: %d = i(%d)", buffer.index, i);
                            ret = ioctl(g->fdCam, VIDIOC_QBUF, &buffer);//入队内存mapped的buffer, VIDIOC_BUF的用法1
                            if(ret)
                            {
                                ERR_PRINT("ioctl(VIDIOC_QBUF) failed1 %d(%s)", errno, strerror(errno));
                            }
                        }
                    }
                }

                if(!ret)
                {
                    enum v4l2_buf_type buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    ret = ioctl(g->fdCam, VIDIOC_STREAMON, &buffer_type);//启动数据流
                    if (ret)
                    {
                        ERR_PRINT("ioctl(VIDIOC_STREAMON) failed %d(%s)", errno, strerror(errno));
                    }else
                    {
                        DBG_PRINT("cam init done.");
                    }
                }
                else
                {
                    ERR_PRINT("VIDIOC_QUERYBUF or VIDIOC_QBUF failed.\n");
                }
            }
        }
    }
        
    return ret;
}

int _requestResourceCam(globalInfo_t* g)
{
    int ret = 0;
    if(!g)
    {
        ret = -EINVAL;
        ERR_PRINT("gloabalInfo is NULL.");
        return ret;
    }

    ret = openCam(g);
    if(ret)
    {
        ERR_PRINT("open camera failed.");
        g->fdCam = -1;
        return ret;
    }

    //2. 设置输入源, *input: input num
    ret = chooseCam(g, VIDEO_INPUT_NUM);
    if(ret)
    {
        ERR_PRINT("choose camera interface failed.");
        closeCam(g);
        return ret;
    }

    ret = configCam(g);
    if(ret)
    {
        ERR_PRINT("choose camera interface failed.");
        closeCam(g);
    }
        
    return ret;
}

int _releaseResourceCam(globalInfo_t* g)
{
    return closeCam(g);
}

int _getImageFrame(globalInfo_t* g)
{
    int ret = 0;
    struct v4l2_buffer buffer;
    
    if(!g)
    {
        ret = -EINVAL;
        ERR_PRINT("globalInfo is NULL.");
        return ret;
    }

    bzero(&buffer, sizeof(buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    //buffer.index = BUFFER_COUNT;
    
    DBG_PRINT("Before VIDIOC_DQBUF");
    ret = ioctl(g->fdCam, VIDIOC_DQBUF, &buffer);//从队列中取出一帧
    if(ret != 0)
    {
        ERR_PRINT("ioctl(VIDIOC_DQBUF) failed.");
    }else
    {
        if(buffer.index >= BUFFER_COUNT)
        {
            ret = -EINVAL;
            ERR_PRINT("invalid buffer index: %d", buffer.index);
        }else
        {
            DBG_PRINT("dequeue done, index: %d, length: %d", buffer.index, buffer.length);
            g->frameSize = buffer.length;
            memcpy(g->frameBuf, g->vbufp[buffer.index], g->frameSize);//缓冲帧数据拷贝出来
            DBG_PRINT("copy done.");
            ret = ioctl(g->fdCam, VIDIOC_QBUF, &buffer);//缓冲帧放入队列， 用法2
            if(ret != 0)
            {
                ERR_PRINT("ioctl(VIDIOC_QBUF) failed2.");
            }else
            {
                DBG_PRINT("enqueue done.");
            }
        }
    }
    return ret;
}

//save image
int _saveImgFrame(unsigned char* dst, unsigned char* src, int size)
{
	int ret = 0;
	unsigned char cmd[64] = "rm ";
	//rm old file
	strcat(cmd, dst);
    DBG_PRINT("%s(old file)", cmd);

    system(cmd);

	DBG_PRINT();
    int fd = open(dst, O_WRONLY|O_CREAT, 00700);//保存图像数据
    if(fd >= 0)
	{
		ret = write(fd, src, size);
		if(0 == ret)
		{
			ret = -EIO;
			ERR_PRINT("Save an img frame failed.\n");
 		}else{
            ret = 0;
        }
		
		close(fd);
	}else 
	{
		ret = -EAGAIN;
		ERR_PRINT("open dst file failed.");
	}
	
	return ret;
}
