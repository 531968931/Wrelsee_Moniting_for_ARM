#ifndef _VIDEO_MANAGER_H
#define _VIDEO_MANAGER_H

#include <config.h>
#include <pic_operation.h>
#include <linux/videodev2.h>

#define NB_BUFFER 4
struct VideoDevice;
struct VideoOpr;
typedef struct VideoDevice T_VideoDevice, *PT_VideoDevice;
typedef struct VideoOpr T_VideoOpr, *PT_VideoOpr;
typedef struct VideoBuf T_VideoBuf, *PT_VideoBuf;

struct VideoDevice{
	int iFd;
	int iPixelFormat;
	int iType;
	int iWidth;
	int iHeight;
	int iVideoBufCnt;
	int iVideoBufMaxLen;
	int iVideoBufCurIndex;
	unsigned char *pucVideoBuf[NB_BUFFER];
	PT_VideoOpr ptOpr;
};

struct VideoBuf {
	T_PixelDatas tPixelDatas;
	int iPixelFormat;
};

struct VideoOpr {
	char *name;
	int (*InitDevice) (char *strDevName, PT_VideoDevice ptVideoDevice);
	int (*ExitDevice) (PT_VideoDevice ptVideoDevice);
	int (*GetFrame) (PT_VideoDevice ptVideoDevice, PT_VideoBuf ptVideoBuf);
	int (*GetFormat) (PT_VideoDevice ptVideoDevice);
	int (*PutFrame) (PT_VideoDevice ptVideoDevice);
	int (*StartDevice) (PT_VideoDevice ptVideoDevice);
	int (*StopDevice) (PT_VideoDevice ptVideoDevice);
	struct VideoOpr *ptNext;
};

/**********************************************************************
 * 函数名称： VideoDeviceInit
 * 功能描述： 根据传进来的名字初始化视频设备
 * 输入参数： PT_VideoDevice - 一个结构体,内含视频设备的参数及操作函数
 * 			  name-所要注册的设备的名字
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01	     V1.0	  安龙	      创建
 ***********************************************************************/
int VideoDeviceInit(char *strDevName, PT_VideoDevice ptVideoDevice);
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
int V4l2Init(void);
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
int RegisterVideoOpr(PT_VideoOpr ptVideoOpr);
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
void ShowVideoOpr(void);
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
PT_VideoOpr GetVideoOpr(char *pcName);
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
int VideoInit(void);



#endif