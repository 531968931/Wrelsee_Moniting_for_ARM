#include <config.h>
#include <video_manager.h>
#include <string.h>


static PT_VideoOpr g_ptVideoOprHead = NULL;

/**********************************************************************
 * 函数名称： RegisterVideoOpr
 * 功能描述： 注册"视频设备的操作函数", 把所能支持的视频设备的操作函数放入
 * 			  链表
 * 输入参数： PT_VideoOpr - 一个结构体,内含视频设备的操作函数
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
int RegisterVideoOpr(PT_VideoOpr ptVideoOpr)
{	
	PT_VideoOpr ptTemp;

	if (!g_ptVideoOprHead)
	{
		g_ptVideoOprHead = ptVideoOpr;
		ptVideoOpr->ptNext = NULL;
	}
	else
	{
		ptTemp = g_ptVideoOprHead;
		while(ptTemp->ptNext)
		{
			ptTemp = ptTemp->ptNext;
		}
		ptTemp->ptNext = ptVideoOpr;
		ptVideoOpr->ptNext = NULL;
	}
	return 0;
}

/**********************************************************************
 * 函数名称： ShowVideoOpr
 * 功能描述： 显示本程序能支持的"视频设备"
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
void ShowVideoOpr(void)
{
	int i = 0;
	PT_VideoOpr ptTmp = g_ptVideoOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

/**********************************************************************
 * 函数名称： GetVideoOpr
 * 功能描述： 根据名字取出指定的"视频设备"
 * 输入参数： pcName - 名字
 * 输出参数： 无
 * 返 回 值： NULL   - 失败,没有指定的模块, 
 *            非NULL - PT_VideoOpr结构体指针
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
PT_VideoOpr GetVideoOpr(char *pcName)
{
	PT_VideoOpr ptTmp = g_ptVideoOprHead;
	
	while (ptTmp)
	{
		if (strcmp(ptTmp->name, pcName) == 0)
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}


/**********************************************************************
 * 函数名称： VideoDeviceInit
 * 功能描述： 注册"视频设备", 把所能支持的视频设备放入链表
 * 输入参数： PT_VideoDevice - 一个结构体,内含视频设备的参数及操作函数
 * 			  name-所要注册的设备的名字
 * 输出参数： 无
 * 返 回 值： 0 - 成功, -1 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
int VideoDeviceInit(char *strDevName, PT_VideoDevice ptVideoDevice)
{
	int iError;
	PT_VideoOpr ptTmp = g_ptVideoOprHead;
	
	while (ptTmp)
	{
		iError = ptTmp->InitDevice(strDevName, ptVideoDevice);
		if (!iError)
		{
			return 0;
		}
		ptTmp = ptTmp->ptNext;
	}
	return -1;
}


/**********************************************************************
 * 函数名称： VideoInit
 * 功能描述： 注册显示设备
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
int VideoInit(void)
{
	int iError;
	iError = V4l2Init();
	return iError;	
}

