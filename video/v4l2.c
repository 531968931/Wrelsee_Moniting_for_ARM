#include <config.h>
#include <video_manager.h>
#include <disp_manager.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


static T_VideoOpr g_tV4l2VideoOpr;

static int g_aiSupportFormats[] = {V4L2_PIX_FMT_YUYV, 
									V4L2_PIX_FMT_MJPEG, 
									 V4L2_PIX_FMT_RGB565};

/**********************************************************************
 * 函数名称： isSupportThisFormat
 * 功能描述： 判断传进来的视频类型本程序能否支持
 * 输入参数： int iPixelFormat - 视频类型描述符
 * 输出参数： 无
 * 返 回 值： 1 - 可以支持, 0 - 无法支持
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
static int isSupportThisFormat(int iPixelFormat)
{
	int i;
	for (i = 0; i < sizeof(g_aiSupportFormats)/sizeof(g_aiSupportFormats[0]); i++)
	{
		if (g_aiSupportFormats[i] == iPixelFormat)
			return 1;
	}
	return 0;
}

/**********************************************************************
 * 函数名称： V4l2GetFrameForReadWrite
 * 功能描述： 将PT_VideoDevice中的视频数据的结构传到PT_VideoBuf中
 * 输入参数： PT_VideoDevice - 视频设备结构体
 * 			  PT_VideoBuf - 存储视频数据的buf
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
static int V4l2GetFrameForReadWrite(PT_VideoDevice ptVideoDevice, PT_VideoBuf ptVideoBuf)
{
    int iRet;

    iRet = read(ptVideoDevice->iFd, ptVideoDevice->pucVideoBuf[0], ptVideoDevice->iVideoBufMaxLen);
    if (iRet <= 0)
    {
        return -1;
    }
    
    ptVideoBuf->iPixelFormat        = ptVideoDevice->iPixelFormat;
    ptVideoBuf->tPixelDatas.iWidth  = ptVideoDevice->iWidth;
    ptVideoBuf->tPixelDatas.iHeight = ptVideoDevice->iHeight;
    ptVideoBuf->tPixelDatas.iBpp    = (ptVideoDevice->iPixelFormat == V4L2_PIX_FMT_YUYV) ? 16 : \
                                        (ptVideoDevice->iPixelFormat == V4L2_PIX_FMT_MJPEG) ? 0 :  \
                                        (ptVideoDevice->iPixelFormat == V4L2_PIX_FMT_RGB565)? 16 : \
                                          0;
    ptVideoBuf->tPixelDatas.iLineBytes    = ptVideoDevice->iWidth * ptVideoBuf->tPixelDatas.iBpp / 8;
    ptVideoBuf->tPixelDatas.iTotalBytes   = iRet;
    ptVideoBuf->tPixelDatas.aucPixelDatas = ptVideoDevice->pucVideoBuf[0];    
    
    return 0;
}

/**********************************************************************
 * 函数名称： V4l2PutFrameForReadWrite
 * 功能描述： 空函数
 * 输入参数： 
 * 输出参数： 无
 * 返 回 值： 1 - 可以支持, 0 - 无法支持
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
static int V4l2PutFrameForReadWrite(PT_VideoDevice ptVideoDevice)
{
    return 0;
}


/**********************************************************************
 * 函数名称： V4l2InitDevice
 * 功能描述： 根据传入的设备名字初始化"V4l2设备"
 * 输入参数： PT_VideoDevice - 一个结构体,内含视频设备的参数及操作函数
 * 输出参数： 无
 * 参考文件：luvcview-v4l2uvc.c
 * 返 回 值： 0 - 成功, -1 - 失败
 * open
 * VIDIOC_QUERYCAP 确定它是否视频捕捉设备,支持哪种接口(streaming/read,write)
 * VIDIOC_ENUM_FMT 查询支持哪种格式
 * VIDIOC_S_FMT    设置摄像头使用哪种格式
 * VIDIOC_REQBUFS  申请buffer
 对于 streaming:
 * VIDIOC_QUERYBUF 确定每一个buffer的信息 并且 mmap
 * VIDIOC_QBUF     放入队列
 * VIDIOC_STREAMON 启动设备
 * poll            等待有数据
 * VIDIOC_DQBUF    从队列中取出
 * 处理....
 * VIDIOC_QBUF     放入队列
 * ....
 对于read,write:
    read
    处理....
    read
 * VIDIOC_STREAMOFF 停止设备
 *
 *
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
static int V4l2InitDevice (char *strDevName, PT_VideoDevice ptVideoDevice)
{
	int iFd;
	int iError;
	int i;

	struct v4l2_capability tV4l2Cap;
	struct v4l2_fmtdesc tFmtDesc;
	struct v4l2_format tV4l2Fmt;
	struct v4l2_requestbuffers tV4l2ReqBufs;
	struct v4l2_buffer tV4l2Buf;

	int iLcdWidth;
	int iLcdHeigt;
	int iLcdBpp;
	
	iFd = open(strDevName, O_RDWR);
	if (iFd < 0)
	{
		DBG_PRINTF("can't open %s\n", strDevName);
		return -1;
	}
	ptVideoDevice->iFd = iFd;

	memset(&tV4l2Cap, 0, sizeof(struct v4l2_capability));
	iError = ioctl(iFd, VIDIOC_QUERYCAP, &tV4l2Cap);
	if (iError)
	{
		DBG_PRINTF("Error opening device %s: unable to query device.\n", strDevName);
		goto err_exit;
	}

	if (!(tV4l2Cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		DBG_PRINTF(" %s: is not a video capture device\n", strDevName);
		goto err_exit;
	}

	if (tV4l2Cap.capabilities & V4L2_CAP_STREAMING)
	{
		DBG_PRINTF(" %s: is support streaming i/o\n", strDevName);
	}

	if (tV4l2Cap.capabilities & V4L2_CAP_READWRITE)
	{
		DBG_PRINTF(" %s: is support read i/o\n", strDevName);
	}

	memset(&tFmtDesc, 0, sizeof(struct v4l2_fmtdesc));
	tFmtDesc.index = 0;
	tFmtDesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	while ((iError = ioctl(iFd, VIDIOC_ENUM_FMT, &tFmtDesc)) == 0)
	{
		if (isSupportThisFormat(tFmtDesc.pixelformat))
		{
			ptVideoDevice->iPixelFormat = tFmtDesc.pixelformat;
			break;
		}
		tFmtDesc.index++;
	}

	if (!ptVideoDevice->iPixelFormat)
	{
		DBG_PRINTF("can not support those formats of this device\n");
        goto err_exit;  
	}

	/* set format in */
	GetDispResolution(&iLcdWidth, &iLcdHeigt, &iLcdBpp);
	memset(&tV4l2Fmt, 0, sizeof(struct v4l2_format));
	tV4l2Fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	tV4l2Fmt.fmt.pix.pixelformat = ptVideoDevice->iPixelFormat;
	tV4l2Fmt.fmt.pix.width = iLcdWidth;
	tV4l2Fmt.fmt.pix.height = iLcdHeigt;
	tV4l2Fmt.fmt.pix.field = V4L2_FIELD_ANY;

	/* 如果驱动程序发现无法支持某些参数(比如分辨率),
     * 它会调整这些参数, 并且返回给应用程序
     */
    iError = ioctl(iFd, VIDIOC_S_FMT, &tV4l2Fmt);
	if (iError)
	{
		DBG_PRINTF("Unable to set format\n");
        goto err_exit;		
	}
	/* 再次获得分辨率的参数，这才是最终确定的参数*/
	ptVideoDevice->iWidth  = tV4l2Fmt.fmt.pix.width;
    ptVideoDevice->iHeight = tV4l2Fmt.fmt.pix.height;
	ptVideoDevice->iType = tV4l2Fmt.type;

	/* request buffers这里仅仅是请求buf */
	memset(&tV4l2ReqBufs, 0, sizeof(struct v4l2_requestbuffers));
	tV4l2ReqBufs.count = NB_BUFFER;
	tV4l2ReqBufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	tV4l2ReqBufs.memory = V4L2_MEMORY_MMAP;

	iError = ioctl(iFd, VIDIOC_REQBUFS, &tV4l2ReqBufs);
	if (iError)
	{
		DBG_PRINTF("Unable to allocate buffers.\n");
        goto err_exit;
	}
	/* 再次获得所分配buf的个数，系统不一定按要求分配，可能小*/
	ptVideoDevice->iVideoBufCnt = tV4l2ReqBufs.count;

	/* 如果是streaming申请的buf，然后mmap映射到内存空间中，并且放入队列*/
	if (tV4l2Cap.capabilities & V4L2_CAP_STREAMING)
	{
		/* map the buffers */
		for (i = 0; i < ptVideoDevice->iVideoBufCnt; i++)
		{
			memset(&tV4l2Buf, 0, sizeof(struct v4l2_buffer));
			tV4l2Buf.index = i;
			tV4l2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			tV4l2Buf.memory = V4L2_MEMORY_MMAP;
			iError = ioctl(iFd, VIDIOC_QUERYBUF, &tV4l2Buf);
			if (iError)
			{
				DBG_PRINTF("Unable to query buffer.\n");
        	    goto err_exit;
			}
			ptVideoDevice->iVideoBufMaxLen = tV4l2Buf.length;
			/*将我们打开的文件映射到内存中，*/
			ptVideoDevice->pucVideoBuf[i] = mmap(0, tV4l2Buf.length,
											PROT_READ, MAP_SHARED, iFd,
											tV4l2Buf.m.offset);
			if (ptVideoDevice->pucVideoBuf[i] == MAP_FAILED) 
            {
        	    DBG_PRINTF("Unable to map buffer\n");
        	    goto err_exit;
        	}
		}	
		 /* Queue the buffers. */
		for (i = 0; i < ptVideoDevice->iVideoBufCnt; i++)
		{
			memset(&tV4l2Buf, 0 ,sizeof(struct v4l2_buffer));
			tV4l2Buf.index = i;
			tV4l2Buf.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        	tV4l2Buf.memory = V4L2_MEMORY_MMAP;
			iError = ioctl(iFd, VIDIOC_QBUF, &tV4l2Buf);
        	if (iError)
            {
        	    DBG_PRINTF("Unable to queue buffer.\n");
        	    goto err_exit;
        	}
		}
		
	}
	/* 如果是read/write则让他们的读取函数去完成格式的读写，再malloc一段用户空间的内存*/
	else if (tV4l2Cap.capabilities & V4L2_CAP_READWRITE)
	{
		g_tV4l2VideoOpr.GetFrame = V4l2GetFrameForReadWrite;
        g_tV4l2VideoOpr.PutFrame = V4l2PutFrameForReadWrite;
		/* read(fd, buf, size) */
        ptVideoDevice->iVideoBufCnt  = 1;
        /* 在这个程序所能支持的格式里, 一个象素最多只需要4字节 */
		ptVideoDevice->iVideoBufMaxLen = ptVideoDevice->iWidth * ptVideoDevice->iHeight * 4;
		ptVideoDevice->pucVideoBuf[0] = malloc(ptVideoDevice->iVideoBufMaxLen);
	}

	ptVideoDevice->ptOpr = &g_tV4l2VideoOpr;
	return 0;
		
err_exit:
	close(iFd);
	return -1;	
}

/**********************************************************************
 * 函数名称： V4l2ExitDevice
 * 功能描述： 退出V4L2设备，就是将初始化中mmap所映射的空间释放munmap
 * 输入参数： PT_VideoDevice
 * 输出参数： 无
 * 返 回 值： 0 - 成功, -1 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
static int V4l2ExitDevice (PT_VideoDevice ptVideoDevice)
{
	int i;
	for (i = 0 ; i < ptVideoDevice->iVideoBufCnt; i++)
	{
		if (ptVideoDevice->pucVideoBuf[i])
		{
			munmap(ptVideoDevice->pucVideoBuf[i], ptVideoDevice->iVideoBufMaxLen);
			ptVideoDevice->pucVideoBuf[i] =NULL;
		}
	}
	close(ptVideoDevice->iFd);
	return 0;
}

/**********************************************************************
 * 函数名称： V4l2GetFrameForStreaming
 * 功能描述： 首先根据PT_VideoDevice的iFD调用poll函数，但有数据时，
 *			  取出队列，将PT_VideoDevice中的数据放到PT_VideoBuf中
 * 输入参数： PT_VideoDevice，PT_VideoBuf
 * 输出参数： 无
 * 返 回 值： 0 - 成功, -1 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
static int V4l2GetFrameForStreaming (PT_VideoDevice ptVideoDevice, PT_VideoBuf ptVideoBuf)
{
	struct pollfd tFds[1];
	int iError;
	struct v4l2_buffer tV4l2Buf;
	
	/* poll */
	tFds[0].fd = ptVideoDevice->iFd;
	tFds[0].events =POLLIN;

	iError = poll(tFds, 1, -1);
	if (iError <= 0)
	{
		DBG_PRINTF("poll error!\n");
        return -1;
	}
	/* VIDIOC_DQBUF 取出队列*/
	memset(&tV4l2Buf, 0, sizeof(struct v4l2_buffer));
	tV4l2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tV4l2Buf.memory = V4L2_MEMORY_MMAP;
	iError = ioctl(ptVideoDevice->iFd, VIDIOC_DQBUF, &tV4l2Buf);
	if (iError < 0)
	{
		DBG_PRINTF("Unable to dequeue buffer.\n");
    	return -1;
	}
	ptVideoDevice->iVideoBufCurIndex = tV4l2Buf.index;

	ptVideoBuf->iPixelFormat        = ptVideoDevice->iPixelFormat;
    ptVideoBuf->tPixelDatas.iWidth  = ptVideoDevice->iWidth;
    ptVideoBuf->tPixelDatas.iHeight = ptVideoDevice->iHeight;
    ptVideoBuf->tPixelDatas.iBpp    = (ptVideoDevice->iPixelFormat == V4L2_PIX_FMT_YUYV) ? 16 : \
                                        (ptVideoDevice->iPixelFormat == V4L2_PIX_FMT_MJPEG) ? 0 :  \
                                        (ptVideoDevice->iPixelFormat == V4L2_PIX_FMT_RGB565) ? 16 :  \
                                        0;
    ptVideoBuf->tPixelDatas.iLineBytes    = ptVideoDevice->iWidth * ptVideoBuf->tPixelDatas.iBpp / 8;
    ptVideoBuf->tPixelDatas.iTotalBytes   = tV4l2Buf.bytesused;
    ptVideoBuf->tPixelDatas.aucPixelDatas = ptVideoDevice->pucVideoBuf[tV4l2Buf.index];    
    return 0;
}

/**********************************************************************
 * 函数名称： V4l2PutFrameForStreaming
 * 功能描述： 将PT_VideoDevice的iVideoBufCurIndex，所对应的v4l2_buf
 *			  重新放到队列
 * 输入参数： PT_VideoDevice
 * 输出参数： 无
 * 返 回 值： 0 - 成功, -1 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
static int V4l2PutFrameForStreaming (PT_VideoDevice ptVideoDevice)
{
	/* VIDIOC_QBUF */
	struct v4l2_buffer tV4l2Buf;
	int iError;

	memset(&tV4l2Buf, 0, sizeof(struct v4l2_buffer));
	tV4l2Buf.index = ptVideoDevice->iVideoBufCurIndex;
	tV4l2Buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	tV4l2Buf.memory = V4L2_MEMORY_MMAP;
	iError = ioctl(ptVideoDevice->iFd, VIDIOC_QBUF, &tV4l2Buf);
	if (iError) 
    {
	    DBG_PRINTF("Unable to queue buffer.\n");
	    return -1;
	}
    return 0;		
}

/**********************************************************************
 * 函数名称： V4l2StartDevice
 * 功能描述： 根据PT_VideoDevice的iFd和iType(设置格式后获得)启动传输
 * 输入参数： PT_VideoDevice
 * 输出参数： 无
 * 返 回 值： 0 - 成功, -1 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
static int V4l2StartDevice (PT_VideoDevice ptVideoDevice)
{
	int iError;
	iError = ioctl(ptVideoDevice->iFd, VIDIOC_STREAMON, ptVideoDevice->iType);
	if (iError) 
    {
    	DBG_PRINTF("Unable to start capture.\n");
    	return -1;
    }
    return 0;
}

/**********************************************************************
 * 函数名称： V4l2StopDevice
 * 功能描述： 根据PT_VideoDevice的iFd和iType(设置格式后获得)关闭传输
 * 输入参数： PT_VideoDevice
 * 输出参数： 无
 * 返 回 值： 0 - 成功, -1 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
static int V4l2StopDevice (PT_VideoDevice ptVideoDevice)
{
	int iError;
	iError = ioctl(ptVideoDevice->iFd, VIDIOC_STREAMOFF, ptVideoDevice->iType);
	if (iError) 
    {
    	DBG_PRINTF("Unable to start capture.\n");
    	return -1;
    }
    return 0;
}

/**********************************************************************
 * 函数名称： V4l2StopDevice
 * 功能描述： 返回PT_VideoDevice中已经设置好的iPixelFormat
 * 输入参数： PT_VideoDevice
 * 输出参数： 无
 * 返 回 值： int - 成功, 其他 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
static int V4l2GetFormat(PT_VideoDevice ptVideoDevice)
{
	return ptVideoDevice->iPixelFormat;
}



static T_VideoOpr g_tV4l2VideoOpr = {
	.name        = "v4l2",
	.InitDevice  = V4l2InitDevice,
	.ExitDevice  = V4l2ExitDevice,
	.GetFrame    = V4l2GetFrameForStreaming,
	.GetFormat   = V4l2GetFormat,
	.PutFrame    = V4l2PutFrameForStreaming,
	.StartDevice = V4l2StartDevice,
	.StopDevice  = V4l2StopDevice,
};


/**********************************************************************
 * 函数名称： V4l2Init
 * 功能描述： 注册"V4l2设备"
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0 - 成功, -1 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
int V4l2Init(void)
{
	return RegisterVideoOpr(&g_tV4l2VideoOpr);
}



