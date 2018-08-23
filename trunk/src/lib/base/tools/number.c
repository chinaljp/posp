/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*****************************************************************************
** ����: tMake( )                                 							**
** ����: void 	                                        					**
** ���:																	**
**		double  *pdVar      ( ������������ı��� )	     					**
**     	int     iBitNum   ( С���㱣��λ�� )             					**
**     	char    cMakeFlag ( ��������ı�־,u ��ʾ��������, d ��ʾȫ�� )		**
** ����:  																	**
** ����ֵ�� ���������������ֵ 	 	               							**
** ���ܣ�   ���ݽṹ���������������ڼ���ı��ʽ,�������ֵ					**
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
