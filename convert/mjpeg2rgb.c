

#include <convert_manager.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <jpeglib.h>


typedef struct MyErrorMgr
{
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
}T_MyErrorMgr, *PT_MyErrorMgr;

extern void jpeg_mem_src_tj(j_decompress_ptr, unsigned char *, unsigned long);


/**********************************************************************
 * 函数名称： isSupportmjpeg2rgb
 * 功能描述： 判断是否支持这种格式的转换
 * 输入参数： int iPixelFormatIn, int iPixelFormatOut
 * 输出参数： 无
 * 返 回 值： 1 - 支持, 0 - 不支持
 * 修改日期 	   版本号	 修改人		  修改内容
 * -----------------------------------------------
 * 2021/04/01		 V1.0	  安龙		  创建
 ***********************************************************************/
static int isSupportmjpeg2rgb (int iPixelFormatIn, int iPixelFormatOut)
{
	if (iPixelFormatIn != V4L2_PIX_FMT_MJPEG)
		return 0;
	if ((iPixelFormatOut != V4L2_PIX_FMT_RGB565) && (iPixelFormatOut != V4L2_PIX_FMT_RGB32))
    {
        return 0;
    }
    return 1;
}

/**********************************************************************
 * 函数名称： MyErrorExit
 * 功能描述： 自定义的libjpeg库出错处理函数
 *            默认的错误处理函数是让程序退出
 *            参考libjpeg里的bmp.c编写了这个错误处理函数
 * 输入参数： ptCInfo - libjpeg库抽象出来的通用结构体
 * 输出参数： 无
 * 返 回 值： 无
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01		 V1.0	  安龙		  创建
 ***********************************************************************/
static void MyErrorExit(j_common_ptr ptCInfo)
{
    static char errStr[JMSG_LENGTH_MAX];
    
	PT_MyErrorMgr ptMyErr = (PT_MyErrorMgr)ptCInfo->err;

    /* Create the message */
    (*ptCInfo->err->format_message) (ptCInfo, errStr);
    DBG_PRINTF("%s\n", errStr);

	longjmp(ptMyErr->setjmp_buffer, 1);
}

/**********************************************************************
 * 函数名称： CovertOneLine
 * 功能描述： 把已经从JPG文件取出的一行象素数据,转换为能在显示设备上使用的格式
 * 输入参数： iWidth      - 宽度,即多少个象素
 *            iSrcBpp     - 已经从JPG文件取出的一行象素数据里面,一个象素用多少位来表示
 *            iDstBpp     - 显示设备上一个象素用多少位来表示
 *            pudSrcDatas - 已经从JPG文件取出的一行象素数所存储的位置
 *            pudDstDatas - 转换所得数据存储的位置
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01		 V1.0	  安龙		  创建
 ***********************************************************************/
static int CovertOneLine(int iWidth, int iSrcBpp, int iDstBpp, unsigned char *pudSrcDatas, unsigned char *pudDstDatas)
{
	unsigned int dwRed;
	unsigned int dwGreen;
	unsigned int dwBlue;
	unsigned int dwColor;
	
	unsigned short *pwDstDatas16bpp = (unsigned short *)pudDstDatas;
	unsigned int   *pwDstDatas32bpp = (unsigned int *)pudDstDatas;

	int i;
	int pos = 0;;

	if (iSrcBpp != 24)
	{
		DBG_PRINTF("this source bpp: %d is wrong\n", iSrcBpp);
		return -1;
	}

	if (iDstBpp == 24)
	{
		memcpy(pudDstDatas, pudSrcDatas, iWidth*3);
	}
	else
	{
		for (i = 0; i < iWidth; i ++)
		{
			dwRed  = pudSrcDatas[pos++];
			dwGreen = pudSrcDatas[pos++];
			dwBlue = pudSrcDatas[pos++];
			if (iDstBpp == 32)
			{
				dwColor = (dwRed << 16) | (dwGreen << 8) | dwBlue;
				*pwDstDatas32bpp = dwColor;
				pwDstDatas32bpp++;
			}
			else if (iDstBpp == 16)
			{
				/* 565 */
				dwRed   = dwRed >> 3;
				dwGreen = dwGreen >> 2;
				dwBlue  = dwBlue >> 3;
				dwColor = (dwRed << 11) | (dwGreen << 5) | (dwBlue);
				*pwDstDatas16bpp = dwColor;
				pwDstDatas16bpp++;
			}
			else
			{	
				DBG_PRINTF("this destination bpp: %d is wrong\n", iDstBpp);
				return -1;
			}
		}
	}
	return 0;
}

/**********************************************************************
 * 函数名称： mjpeg2rgbConvert-
 *			  MJPEG : 实质上每一帧数据都是一个完整的JPEG文件
 * 			  通过调用libjpeg来进行转换
 * 功能描述： 将mjpeg数据转换成RGB16/32数据，放在ptVideoBufOut
 * 输入参数： PT_VideoBuf ptVideoBufIn, PT_VideoBuf ptVideoBufOut
 * 输出参数： 无
 * 返 回 值： 0 - 成功, -1 - 失败
 * 修改日期 	   版本号	 修改人		  修改内容
 * -----------------------------------------------
 * 2021/04/01		 V1.0	  安龙		  创建
 ***********************************************************************/
static int mjpeg2rgbConvert (PT_VideoBuf ptVideoBufIn, PT_VideoBuf ptVideoBufOut)
{
	//int iError;
	int iRowStride;
	unsigned char *pucLineBuffer = NULL;
	unsigned char *pucDest;
	PT_PixelDatas ptPixelDatas = &ptVideoBufOut->tPixelDatas; 

	T_MyErrorMgr tJerr;
	/*1.分配一个jpeg_decompress_struct结构体*/
	struct jpeg_decompress_struct tDInfo;
		
	/*2.初始化结构体*/
	tDInfo.err = jpeg_std_error(&tJerr.pub);
	tJerr.pub.error_exit     = MyErrorExit;

	if (setjmp(tJerr.setjmp_buffer))
	{
		/* 如果程序能运行到这里, 表示JPEG解码出错,我们释放已分配的资源 */
		jpeg_destroy_decompress(&tDInfo);
		if (pucLineBuffer)
		{
			free(pucLineBuffer);
		}
		if (ptPixelDatas->aucPixelDatas)
		{
			free(ptPixelDatas->aucPixelDatas);
		}
		return -1;		
	}
	jpeg_create_decompress(&tDInfo);
	/*3.指定源文件，或者指定内存地址*/
	jpeg_mem_src_tj(&tDInfo, ptVideoBufIn->tPixelDatas.aucPixelDatas, ptVideoBufIn->tPixelDatas.iTotalBytes);
	/*4.用jpeg_read_header获得jpeg信息*/
	jpeg_read_header(&tDInfo, TRUE);
	/*5.设置解压参数，放大缩小*/
	tDInfo.scale_num = tDInfo.scale_denom = 1;
	/*6.启动解压jpeg_start_decompress*/
	jpeg_start_decompress(&tDInfo);
	
	//一行的数据长度,及一些初始化数据
	iRowStride = tDInfo.output_width * tDInfo.output_components;
	pucLineBuffer = malloc(iRowStride);
	if (pucLineBuffer == NULL)
	{
		DBG_PRINTF("malloc failed\n");
		return -1;
	}

	ptPixelDatas->iWidth = tDInfo.output_width;
	ptPixelDatas->iHeight = tDInfo.output_height;
	ptPixelDatas->iLineBytes    = ptPixelDatas->iWidth * ptPixelDatas->iBpp / 8;
    ptPixelDatas->iTotalBytes   = ptPixelDatas->iHeight * ptPixelDatas->iLineBytes;

	if (ptPixelDatas->aucPixelDatas ==NULL)
	{
		ptPixelDatas->aucPixelDatas = malloc(ptPixelDatas->iTotalBytes);
	}
	
	pucDest = ptPixelDatas->aucPixelDatas;
	/*7.循环调用jpeg_read_scanlines来逐行读取数据*/
	while (tDInfo.output_scanline < tDInfo.output_height)
	{
		/* 得到一行数据,里面的颜色格式为0xRR, 0xGG, 0xBB */
		(void) jpeg_read_scanlines(&tDInfo, &pucLineBuffer, 1);

		CovertOneLine(ptPixelDatas->iWidth, 24, ptPixelDatas->iBpp, pucLineBuffer, pucDest);
		pucDest += ptPixelDatas->iLineBytes;
	}	
	/*8.结束解压：jpeg_finish_decompress
	 *  并释放jpeg_decompress_struct结构体*/
	free(pucLineBuffer);
	jpeg_finish_decompress(&tDInfo);
	jpeg_destroy_decompress(&tDInfo);	

	return 0;
}

/**********************************************************************
 * 函数名称： mjpeg2rgbConvertExit
 * 功能描述： 退出转换，释放free转换过程中由malloc分配的用户内存
 * 输入参数： PT_VideoBuf
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他 - 失败
 * 修改日期 	   版本号	 修改人		  修改内容
 * -----------------------------------------------
 * 2021/04/01		 V1.0	  安龙		  创建
 ***********************************************************************/
static int mjpeg2rgbConvertExit (PT_VideoBuf ptVideoBufOut)
{
	if (ptVideoBufOut->tPixelDatas.aucPixelDatas)
	{
		free(ptVideoBufOut->tPixelDatas.aucPixelDatas);
		ptVideoBufOut->tPixelDatas.aucPixelDatas = NULL;
	}
	return 0;
}

static T_VideoConvert g_Mjpeg2RgbVideoConvert = {
	.name        = "mjpeg2rgb",
	.isSupport   = isSupportmjpeg2rgb,
	.Convert     = mjpeg2rgbConvert,
	.ConvertExit = mjpeg2rgbConvertExit,

};
	
/**********************************************************************
 * 函数名称： mjpeg2RgbInit
 * 功能描述： 注册"mjpeg2Rgb函数",并且初始化我们参考文件中的所用的函数initLut
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0 - 成功, -1 - 失败
 * 修改日期 	   版本号	 修改人		  修改内容
 * -----------------------------------------------
 * 2021/04/01		 V1.0	  安龙		  创建
 ***********************************************************************/
int Mjpeg2RgbInit(void)
{
	return RegisterVideoConvert(&g_Mjpeg2RgbVideoConvert);
}



