#include <config.h>
#include <convert_manager.h>
#include <string.h>


static PT_VideoConvert g_ptVideoConvertHead = NULL;


/**********************************************************************
 * 函数名称： RegisterVideoConvert
 * 功能描述： 注册"视频格式转化操作函数", 把所能支持的格式转化放入链表
 * 输入参数： PT_VideoConvert - 一个结构体,内含视频设备的操作函数
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
int RegisterVideoConvert(PT_VideoConvert ptVideoConvert)
{
	PT_VideoConvert ptTemp;

	if (!g_ptVideoConvertHead)
	{
		g_ptVideoConvertHead = ptVideoConvert;
		ptVideoConvert->ptNext = NULL;
	}
	else
	{
		ptTemp = g_ptVideoConvertHead;
		while(ptTemp->ptNext)
		{
			ptTemp = ptTemp->ptNext;
		}
		ptTemp->ptNext = ptVideoConvert;
		ptVideoConvert->ptNext = NULL;
	}
	return 0;
}

/**********************************************************************
 * 函数名称： ShowVideoConvert
 * 功能描述： 显示本程序能支持的"视频格式转化操作"
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
void ShowVideoConvert(void)
{
	int i = 0;
	PT_VideoConvert ptTmp = g_ptVideoConvertHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}	
}
/**********************************************************************
 * 函数名称： GetVideoConvertForFormats
 * 功能描述： 根据输入输出格式的要求，返回适合的视频格式转化操作
 * 输入参数： int iPixelFormatIn, int iPixelFormatOut
 * 输出参数： 无
 * 返 回 值： NULL   - 失败,没有指定的模块, 
 *            非NULL - 视频格式转化操作的结构体指针
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
PT_VideoConvert GetVideoConvertForFormats(int iPixelFormatIn, int iPixelFormatOut)
{
	
	PT_VideoConvert ptTmp = g_ptVideoConvertHead;
	
	while (ptTmp)
	{
		if (ptTmp->isSupport(iPixelFormatIn, iPixelFormatOut))
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}

/**********************************************************************
 * 函数名称： VideoConvertInit
 * 功能描述： 注册"VideoConvert"
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0 - 成功, -1 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
int VideoConvertInit(void)
{
	int iError;
	iError = Yuv2RgbInit();
	iError |= Mjpeg2RgbInit();
	iError |= Rgb2RgbInit();
	
	return iError;	
}

