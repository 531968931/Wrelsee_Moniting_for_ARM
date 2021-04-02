#include <convert_manager.h>
#include <pic_operation.h>
#include <stdlib.h>
#include "color.h"

extern void initLut(void);

/**********************************************************************
 * 函数名称： isSupportyuv2rgb
 * 功能描述： 判断是否支持这种格式的转换
 * 输入参数： int iPixelFormatIn, int iPixelFormatOut
 * 输出参数： 无
 * 返 回 值： 1 - 支持, 0 - 不支持
 * 修改日期 	   版本号	 修改人		  修改内容
 * -----------------------------------------------
 * 2021/04/01		 V1.0	  安龙		  创建
 ***********************************************************************/
static int isSupportyuv2rgb (int iPixelFormatIn, int iPixelFormatOut)
{
	if (iPixelFormatIn != V4L2_PIX_FMT_YUYV)
		return 0;
	if ((iPixelFormatOut != V4L2_PIX_FMT_RGB565) && (iPixelFormatOut != V4L2_PIX_FMT_RGB32))
    {
        return 0;
    }
    return 1;
}

/**********************************************************************
 * 函数名称： Pyuv422torgb565-参考luvcview 中utils.c及color.c
 * 功能描述： 将Yuv数据转换成RGB565数据，放在output_ptr
 * 输入参数： unsigned char * input_ptr, unsigned char * output_ptr, 分辨率
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他 - 失败
 * 修改日期 	   版本号	 修改人		  修改内容
 * -----------------------------------------------
 * 2021/04/01		 V1.0	  安龙		  创建
 ***********************************************************************/
static unsigned int Pyuv422torgb565(unsigned char *pucInput, unsigned char *pucOutput, unsigned int iWidth, unsigned int iHeight)
{
	unsigned int i, size;
	unsigned char Y, Y1, U, V;
	unsigned char *pucBufIn = pucInput;
	unsigned char *pucBufOut = pucOutput;
	unsigned int iRed, iGreen, iBule, iColor;
	
	size = iWidth * iHeight /2;
	for (i = size; i > 0; i--)
	{
		Y = pucBufIn[0] ;
		U = pucBufIn[1] ;
		Y1 = pucBufIn[2];
		V = pucBufIn[3];
		pucBufIn += 4;
		/* 一个Yuv包含两个RGB565，先取1次 */
		iRed = R_FROMYV(Y,V);
		iGreen = G_FROMYUV(Y,U,V); //b
		iBule = B_FROMYU(Y,U); //v

		/* 把r,g,b三色构造为rgb565的16位值 */
        iRed = iRed >> 3;
        iGreen = iGreen >> 2;
        iBule = iBule >> 3;
        iColor = (iRed << 11) | (iGreen << 5) | iBule;
        *pucBufOut++ = iColor & 0xff;
        *pucBufOut++ = (iColor >> 8) & 0xff;

		/* 再取1次 */
		iRed = R_FROMYV(Y1,V);
		iGreen = G_FROMYUV(Y1,U,V); //b
		iBule = B_FROMYU(Y1,U); //v

		/* 把r,g,b三色构造为rgb565的16位值 */
        iRed = iRed >> 3;
        iGreen = iGreen >> 2;
        iBule = iBule >> 3;
        iColor = (iRed << 11) | (iGreen << 5) | iBule;
        *pucBufOut++ = iColor & 0xff;
        *pucBufOut++ = (iColor >> 8) & 0xff;			
	}
	return 0;
}

/**********************************************************************
 * 函数名称： Pyuv422torgb32-参考luvcview 中utils.c及color.c
 * 功能描述： 将Yuv数据转换成RGB32数据，放在output_ptr
 * 输入参数： unsigned char * input_ptr, unsigned char * output_ptr, 分辨率
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他 - 失败
 * 修改日期 	   版本号	 修改人		  修改内容
 * -----------------------------------------------
 * 2021/04/01		 V1.0	  安龙		  创建
 ***********************************************************************/
static unsigned int Pyuv422torgb32(unsigned char *pucInput, unsigned char *pucOutput, unsigned int iWidth, unsigned int iHeight)
{
	unsigned int i, size;
	unsigned char Y, Y1, U, V;
	unsigned char *pucBufIn = pucInput;
	unsigned int *pucBufOut = (unsigned int *)pucOutput;
	unsigned int iRed, iGreen, iBule, iColor;
	
	size = iWidth * iHeight /2;
	for (i = size; i > 0; i--)
	{
		Y = pucBufIn[0] ;
		U = pucBufIn[1] ;
		Y1 = pucBufIn[2];
		V = pucBufIn[3];
		pucBufIn += 4;
		/* 一个Yuv包含两个RGB32，先取1次 */
		iRed = R_FROMYV(Y,V);
		iGreen = G_FROMYUV(Y,U,V); //b
		iBule = B_FROMYU(Y,U); //v

		/* 把r,g,b三色构造为rgb888的24位值 */
        iColor = (iRed << 16) | (iGreen << 8) | iBule;
        *pucBufOut++ = iColor;

		/* 再取1次 */
		iRed = R_FROMYV(Y1,V);
		iGreen = G_FROMYUV(Y1,U,V); //b
		iBule = B_FROMYU(Y1,U); //v

		/* 把r,g,b三色构造为rgb888的24位值 */
        iColor = (iRed << 16) | (iGreen << 8) | iBule;
        *pucBufOut++ = iColor;		
	}
	return 0;
}

/**********************************************************************
 * 函数名称： yuv2rgbConvert
 * 功能描述： 将Yuv数据转换成RGB16/32数据，放在ptVideoBufOut
 * 输入参数： PT_VideoBuf ptVideoBufIn, PT_VideoBuf ptVideoBufOut
 * 输出参数： 无
 * 返 回 值： 0 - 成功, -1 - 失败
 * 修改日期 	   版本号	 修改人		  修改内容
 * -----------------------------------------------
 * 2021/04/01		 V1.0	  安龙		  创建
 ***********************************************************************/
static int yuv2rgbConvert (PT_VideoBuf ptVideoBufIn, PT_VideoBuf ptVideoBufOut)
{
	PT_PixelDatas ptPixelDatasIn = &ptVideoBufIn->tPixelDatas;
	PT_PixelDatas ptPixelDatasOut = &ptVideoBufOut->tPixelDatas;
	
	ptPixelDatasOut->iWidth = ptPixelDatasIn->iWidth;
	ptPixelDatasOut->iHeight = ptPixelDatasIn->iHeight;
	
	if (ptVideoBufOut->iPixelFormat == V4L2_PIX_FMT_RGB565)	
	{
		ptPixelDatasOut->iBpp = 16;
		ptPixelDatasOut->iLineBytes = ptPixelDatasOut->iWidth * ptPixelDatasOut->iBpp / 8;
		ptPixelDatasOut->iTotalBytes = ptPixelDatasOut->iLineBytes * ptPixelDatasOut->iHeight;

		if (!ptPixelDatasOut->aucPixelDatas)
		{
			ptPixelDatasOut->aucPixelDatas = malloc(ptPixelDatasOut->iTotalBytes);
		}
		Pyuv422torgb565(ptPixelDatasIn->aucPixelDatas, ptPixelDatasOut->aucPixelDatas, ptPixelDatasOut->iWidth, ptPixelDatasOut->iHeight);
        return 0;	
	}
	else if (ptVideoBufOut->iPixelFormat == V4L2_PIX_FMT_RGB32)
	{
		
		ptPixelDatasOut->iBpp = 32;
		ptPixelDatasOut->iLineBytes = ptPixelDatasOut->iWidth * ptPixelDatasOut->iBpp / 8;
		ptPixelDatasOut->iTotalBytes = ptPixelDatasOut->iLineBytes * ptPixelDatasOut->iHeight;

		if (!ptPixelDatasOut->aucPixelDatas)
		{
			ptPixelDatasOut->aucPixelDatas = malloc(ptPixelDatasOut->iTotalBytes);
		}
		
		Pyuv422torgb32(ptPixelDatasIn->aucPixelDatas, ptPixelDatasOut->aucPixelDatas, ptPixelDatasOut->iWidth, ptPixelDatasOut->iHeight);
        return 0;		
	}
	return -1;
}

/**********************************************************************
 * 函数名称： yuv2rgbConvertExit
 * 功能描述： 退出转换，释放free转换过程中由malloc分配的用户内存
 * 输入参数： PT_VideoBuf
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他 - 失败
 * 修改日期 	   版本号	 修改人		  修改内容
 * -----------------------------------------------
 * 2021/04/01		 V1.0	  安龙		  创建
 ***********************************************************************/
static int yuv2rgbConvertExit (PT_VideoBuf ptVideoBufOut)
{
	if (ptVideoBufOut->tPixelDatas.aucPixelDatas)
	{
		free(ptVideoBufOut->tPixelDatas.aucPixelDatas);
		ptVideoBufOut->tPixelDatas.aucPixelDatas = NULL;
	}
	return 0;
}

static T_VideoConvert g_Yuv2RgbVideoConvert = {
	.name        = "yuv2rgb",
	.isSupport   = isSupportyuv2rgb,
	.Convert     = yuv2rgbConvert,
	.ConvertExit = yuv2rgbConvertExit,

};
	
/**********************************************************************
 * 函数名称： Yuv2RgbInit
 * 功能描述： 注册"Yuv2Rgb函数",并且初始化我们参考文件中的所用的函数initLut
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0 - 成功, -1 - 失败
 * 修改日期 	   版本号	 修改人		  修改内容
 * -----------------------------------------------
 * 2021/04/01		 V1.0	  安龙		  创建
 ***********************************************************************/
int Yuv2RgbInit(void)
{
	initLut();
	return RegisterVideoConvert(&g_Yuv2RgbVideoConvert);
}


