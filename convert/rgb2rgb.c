#include <convert_manager.h>
#include <stdlib.h>
#include <string.h>


/**********************************************************************
 * 函数名称： isSupportmrgb2rgb
 * 功能描述： 判断是否支持这种格式的转换
 * 输入参数： int iPixelFormatIn, int iPixelFormatOut
 * 输出参数： 无
 * 返 回 值： 1 - 支持, 0 - 不支持
 * 修改日期 	   版本号	 修改人		  修改内容
 * -----------------------------------------------
 * 2021/04/01		 V1.0	  安龙		  创建
 ***********************************************************************/
static int isSupportrgb2rgb (int iPixelFormatIn, int iPixelFormatOut)
{
	if (iPixelFormatIn != V4L2_PIX_FMT_RGB565)
		return 0;
	if ((iPixelFormatOut != V4L2_PIX_FMT_RGB565) && (iPixelFormatOut != V4L2_PIX_FMT_RGB32))
    {
        return 0;
    }
    return 1;
}


/**********************************************************************
 * 函数名称： rgb2rgbConvert-		  
 * 功能描述： 将rgb数据转换成RGB16/32数据，放在ptVideoBufOut
 * 输入参数： PT_VideoBuf ptVideoBufIn, PT_VideoBuf ptVideoBufOut
 * 输出参数： 无
 * 返 回 值： 0 - 成功, -1 - 失败
 * 修改日期 	   版本号	 修改人		  修改内容
 * -----------------------------------------------
 * 2021/04/01		 V1.0	  安龙		  创建
 ***********************************************************************/
static int rgb2rgbConvert (PT_VideoBuf ptVideoBufIn, PT_VideoBuf ptVideoBufOut)
{
	 PT_PixelDatas ptPixelDatasIn  = &ptVideoBufIn->tPixelDatas;
    PT_PixelDatas ptPixelDatasOut = &ptVideoBufOut->tPixelDatas;

    int x, y;
    int r, g, b;
    int color;
    unsigned short *pwSrc = (unsigned short *)ptPixelDatasIn->aucPixelDatas;
    unsigned int *pdwDest;

    if (ptVideoBufIn->iPixelFormat != V4L2_PIX_FMT_RGB565)
    {
        return -1;
    }

    if (ptVideoBufOut->iPixelFormat == V4L2_PIX_FMT_RGB565)
    {
        ptPixelDatasOut->iWidth  = ptPixelDatasIn->iWidth;
        ptPixelDatasOut->iHeight = ptPixelDatasIn->iHeight;
        ptPixelDatasOut->iBpp    = 16;
        ptPixelDatasOut->iLineBytes  = ptPixelDatasOut->iWidth * ptPixelDatasOut->iBpp / 8;
        ptPixelDatasOut->iTotalBytes = ptPixelDatasOut->iLineBytes * ptPixelDatasOut->iHeight;
        if (!ptPixelDatasOut->aucPixelDatas)
        {
            ptPixelDatasOut->aucPixelDatas = malloc(ptPixelDatasOut->iTotalBytes);
        }
        
        memcpy(ptPixelDatasOut->aucPixelDatas, ptPixelDatasIn->aucPixelDatas, ptPixelDatasOut->iTotalBytes);
        return 0;
    }
    else if (ptVideoBufOut->iPixelFormat == V4L2_PIX_FMT_RGB32)
    {
        ptPixelDatasOut->iWidth  = ptPixelDatasIn->iWidth;
        ptPixelDatasOut->iHeight = ptPixelDatasIn->iHeight;
        ptPixelDatasOut->iBpp    = 32;
        ptPixelDatasOut->iLineBytes  = ptPixelDatasOut->iWidth * ptPixelDatasOut->iBpp / 8;
        ptPixelDatasOut->iTotalBytes = ptPixelDatasOut->iLineBytes * ptPixelDatasOut->iHeight;
        if (!ptPixelDatasOut->aucPixelDatas)
        {
            ptPixelDatasOut->aucPixelDatas = malloc(ptPixelDatasOut->iTotalBytes);
        }

        pdwDest = (unsigned int *)ptPixelDatasOut->aucPixelDatas;
        
        for (y = 0; y < ptPixelDatasOut->iHeight; y++)
        {
            for (x = 0; x < ptPixelDatasOut->iWidth; x++)
            {
                color = *pwSrc++;
                /* 从RGB565格式的数据中提取出R,G,B */
                r = color >> 11;
                g = (color >> 5) & (0x3f);
                b = color & 0x1f;

                /* 把r,g,b转为0x00RRGGBB的32位数据 */
                color = ((r << 3) << 16) | ((g << 2) << 8) | (b << 3);

                *pdwDest = color;
                pdwDest++;
            }
        }
        return 0;
    }

    return -1;
}

/**********************************************************************
 * 函数名称： rgb2rgbConvertExit
 * 功能描述： 退出转换，释放free转换过程中由malloc分配的用户内存
 * 输入参数： PT_VideoBuf
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他 - 失败
 * 修改日期 	   版本号	 修改人		  修改内容
 * -----------------------------------------------
 * 2021/04/01		 V1.0	  安龙		  创建
 ***********************************************************************/
static int rgb2rgbConvertExit (PT_VideoBuf ptVideoBufOut)
{
	if (ptVideoBufOut->tPixelDatas.aucPixelDatas)
	{
		free(ptVideoBufOut->tPixelDatas.aucPixelDatas);
		ptVideoBufOut->tPixelDatas.aucPixelDatas = NULL;
	}
	return 0;
}

static T_VideoConvert g_Rgb2RgbVideoConvert = {
	.name        = "mrgb2rgb",
	.isSupport   = isSupportrgb2rgb,
	.Convert     = rgb2rgbConvert,
	.ConvertExit = rgb2rgbConvertExit,

};
	
/**********************************************************************
 * 函数名称： mrgb2RgbInit
 * 功能描述： 注册"mrgb2Rgb函数",并且初始化我们参考文件中的所用的函数initLut
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0 - 成功, -1 - 失败
 * 修改日期 	   版本号	 修改人		  修改内容
 * -----------------------------------------------
 * 2021/04/01		 V1.0	  安龙		  创建
 ***********************************************************************/
int Rgb2RgbInit(void)
{
	return RegisterVideoConvert(&g_Rgb2RgbVideoConvert);
}




