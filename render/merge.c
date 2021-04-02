#include <pic_operation.h>
#include <string.h>


/**********************************************************************
 * 函数名称： PicMerge
 * 功能描述： 把小图片合并入大图片里
 * 输入参数： iX,iY      - 小图片合并入大图片的某个区域, iX/iY确定这个区域的左上角座标
 *            ptSmallPic - 内含小图片的象素数据
 *            ptBigPic   - 内含大图片的象素数据
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2021/04/01		 V1.0	  安龙		  创建
 ***********************************************************************/

 int PicMerge(int iX, int iY, PT_PixelDatas ptSmallPic, PT_PixelDatas ptBigPic)
{
	int i;
	unsigned char *pucSrc;
	unsigned char *pucDst;

	if ((ptSmallPic->iWidth > ptBigPic->iWidth)  ||
		(ptSmallPic->iHeight > ptBigPic->iHeight) ||
		(ptSmallPic->iBpp != ptBigPic->iBpp))
	{
		return -1;
	}
	pucSrc = ptSmallPic->aucPixelDatas;		
	pucDst = ptBigPic->aucPixelDatas + iY * ptBigPic->iLineBytes + iX * ptBigPic->iBpp / 8;
	for (i = 0; i < ptSmallPic->iHeight; i ++)
	{
		memcpy(pucDst, pucSrc, ptSmallPic->iLineBytes);
		pucSrc += ptSmallPic->iLineBytes;
		pucDst += ptBigPic->iLineBytes;
	}
	return 0;
}



