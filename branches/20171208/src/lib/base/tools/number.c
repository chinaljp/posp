/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*****************************************************************************
** 函数: tMake( )                                 							**
** 类型: void 	                                        					**
** 入参:																	**
**		double  *pdVar      ( 计算四舍五入的变量 )	     					**
**     	int     iBitNum   ( 小数点保留位数 )             					**
**     	char    cMakeFlag ( 四舍五入的标志,u 表示四舍五入, d 表示全舍 )		**
** 出参:  																	**
** 返回值： 返回四舍五入的数值 	 	               							**
** 功能：   根据结构体域描述串及用于计算的表达式,计算出数值					**
*****************************************************************************/
void	tMake( double *pdVar, int iBitNum, char cMakeFlag )
{
	int		i;
	int		iTmpBit;
	double	dTmpBit;
	char	sTmpBit[20];

	sprintf( sTmpBit, "1" );

	for( i = 0; i < iBitNum; i ++ )
	{
		strcat( sTmpBit, "0" );
	}

	iTmpBit = atoi( sTmpBit );
	dTmpBit = atof( sTmpBit );

	if ( cMakeFlag == 'u' )
	{
		if ( *pdVar > 0.00 )
			*pdVar = (long)((*pdVar * iTmpBit) + 0.5000001) / dTmpBit;
		else
			*pdVar = (long)((*pdVar * iTmpBit) - 0.5000001) / dTmpBit;
	}
	else if ( cMakeFlag == 'd' )
		*pdVar = (long)(*pdVar * iTmpBit) / dTmpBit;

	return;
}
