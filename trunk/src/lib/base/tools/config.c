/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#define MAX_MALLOC_NUM      128
#define MAX_CONF_REC_LEN    128
#define MAX_SON_STRUCT_DES  256
#define MAX_FIELD_NAME      128

static void *g_pvMallocPtr[MAX_MALLOC_NUM];
static int  g_iMallocNum = 0;

#define CHAR_OFF    1
#define SHORT_OFF   2
#define INT_OFF     4
#define LONG_OFF    8
#define DOUBLE_OFF  8
#define FLOAT_OFF   4

/*****************************************************************************
** ������ tCalcOneFieldSize( )                                              **
** ���ͣ� int                                                               **
** ��Σ�                                                                   ** 
**      char *pcType ( Ҫ����Ľṹ��ĳ����������ַ��� )                   **
**      int  *piOffset ( ��ǰ�ṹ���ƫ���� )                               **
**      int  *piStructKey ( ��ǰ�ṹ�������Ĳ���ֵ )                      **
** ����:                                                                    **
**      int  *piOffset ( �ۼӸ����ṹ���ƫ���� )                         **
** ����ֵ�� �ɹ� - 0,  ʧ�� - -1                                            **
** ���ܣ����ݽṹ��������������ṹ��ƫ����                                 **
*****************************************************************************/
int tCalcOneFieldSize( char *pcType, int *piOffset, int *piStructKey )
{
    int     iCnt = 0;
    void    *pcPtr;


    switch ( *pcType )
    {
    /* �ַ����������뿼��ƫ����ֱ����� */
    case 'c':
        *piOffset += atoi( pcType + 1 ) ? atoi( pcType + 1) : 1;

        if ( piStructKey != NULL && *piStructKey < CHAR_OFF )
            *piStructKey = CHAR_OFF;
        break;

    /* float�����ж�ƫ�����Ƿ�ɱ�4������������Ҫ�ض�λƫ��ֵ */
    case 'f':
        if ( *piOffset % FLOAT_OFF != 0 )
            *piOffset = ( *piOffset / FLOAT_OFF + 1 ) * FLOAT_OFF;

        iCnt = atoi( pcType + 1 ) ? atoi( pcType + 1) : 1;
        *piOffset += iCnt * sizeof( float );

        if ( piStructKey != NULL && *piStructKey < FLOAT_OFF )
            *piStructKey = FLOAT_OFF;
        break;

    /* double���� */
    case 'd':
        if ( *piOffset % DOUBLE_OFF != 0 )
            *piOffset = ( *piOffset / DOUBLE_OFF + 1 ) * DOUBLE_OFF;

        iCnt = atoi( pcType + 1 ) ? atoi( pcType + 1) : 1;
        *piOffset += iCnt * sizeof( double );

        if ( piStructKey != NULL && *piStructKey < DOUBLE_OFF )
            *piStructKey = DOUBLE_OFF;
        break;

    /* int���� */
    case 'i':
        if ( *piOffset % INT_OFF != 0 )
            *piOffset = ( *piOffset / INT_OFF + 1 ) * INT_OFF;

        iCnt = atoi( pcType + 1 ) ? atoi( pcType + 1) : 1;
        *piOffset += iCnt * sizeof( int );

        if ( piStructKey != NULL && *piStructKey < INT_OFF )
            *piStructKey = INT_OFF;
        break;

    /* short���� */
    case 's':
        if ( *piOffset % SHORT_OFF != 0 )
            *piOffset = ( *piOffset / SHORT_OFF + 1 ) * SHORT_OFF;

        iCnt = atoi( pcType + 1 ) ? atoi( pcType + 1) : 1;
        *piOffset += iCnt * sizeof( short );

        if ( piStructKey != NULL && *piStructKey < SHORT_OFF )
            *piStructKey = SHORT_OFF;
        break;

    /* long���� */
    case 'l':
        if ( *piOffset % LONG_OFF != 0 )
            *piOffset = ( *piOffset / LONG_OFF + 1 ) * LONG_OFF;

        iCnt = atoi( pcType + 1 ) ? atoi( pcType + 1) : 1;
        *piOffset += iCnt * sizeof( long );

        if ( piStructKey != NULL && *piStructKey < LONG_OFF )
            *piStructKey = LONG_OFF;
        break;

    /* ָ������ */
    case 'p':
        if ( *piOffset % LONG_OFF != 0 )
            *piOffset = ( *piOffset / LONG_OFF + 1 ) * LONG_OFF;

        iCnt = atoi( pcType + 1 ) ? atoi( pcType + 1) : 1;
        *piOffset += iCnt * sizeof( pcPtr );

        if ( piStructKey != NULL && *piStructKey < LONG_OFF )
            *piStructKey = LONG_OFF;
        break;
    default:
        fprintf( stderr, "��֧�ֵ���������[%s]!\n", pcType );
        return( -1 );
    }

    return( 0 );
}
/*****************************************************************************
** ������ tGetRecord( )                                                     **
** ���ͣ� int                                                               **
** ���:                                                                    **
**      FILE *pstFp ( �����ļ�ָ�� )                                        **
**      char *pcItem ( ���������� )                                         **
**      char *pcRecord ( ���������Ϣ���ַ���ָ�� )                         **
** ����:                                                                    **
**      char *pcRecord ( ���������Ϣ���ַ���ָ�� )                         **
** ����ֵ�� �ɹ� - 0,  ʧ�� - -1                                            **
** ���ܣ��������ļ���ǰָ��λ�ö�ȡ�����������ֵ                           **
*****************************************************************************/
int     tGetRecord( FILE *pstFp, char *pcItem, char *pcRecord )
{
    char    sTmp[MAX_CONF_REC_LEN];
    char    sTmp1[81];

    while ( !feof( pstFp ) )
    {
        fgets( sTmp, MAX_CONF_REC_LEN, pstFp );

        if ( sTmp[0] == '#' || sTmp[0] == '\n' || sTmp[0] == '\r' || 
                                sTmp[0] == '\t' || sTmp[0] == ' ' )
            continue;

        if ( sTmp[0] == '[' )
        {
            return( -1 );
        }
    
        tTrim( sTmp );
/*      tErrLog( DEBUG, "��ȡ�����ü�¼Ϊ[%s]", sTmp ); */
        sscanf( sTmp, "%s", sTmp1 );
        if ( memcmp( sTmp1, pcItem, strlen( sTmp1 ) ) )
            return( -1 );
        strcpy( pcRecord, &sTmp[strlen( sTmp1 )] );
        tTrim( pcRecord );

        return( 0 );
    }

    return( -1 );
}


/*****************************************************************************
** ����:  tStr2Struct( )                                                    **
** ���ͣ� int                                                               **
** ��Σ�                                                                   **
**      char *pcType ( Ҫ������ַ������� )                                 **
**      char *pcStr ( Ҫ������ַ���ָ�� )                                  **
**      int  *piOffset ( Ŀǰ�ṹ�洢����ƫ������ַ )                       **
**      void *pvAddr ( Ŀǰ�ṹ�洢�����׵�ַ )                             **
**      int  *piStructKey ( Ŀǰ�ṹ�洢�����׵�ַ )                        **
** ����:                                                                    **
**      int  *piOffset ( Ŀǰ�ṹ�洢����ƫ������ַ )                       **
** ����ֵ�� �ɹ� - 0,  ʧ�� - -1                                            **
** ���ܣ����ַ�����Ŀ�����ͽ���ת����д��ṹ��ַ�ռ�                       **
*****************************************************************************/
static int  tStr2Struct( char *pcType, char *pcStr, int *piOffset, void *pvAddr, int *piStructKey )
{
    int     iTmp, iLen;
    float   fTmp;
    double  dTmp;
    short   nTmp;
    long    lTmp;
    char    *pcTmp;

    pcTmp = ( char * )pvAddr;

    switch ( *pcType )
    {
        /* �ַ����������뿼��ƫ����ֱ����� */
    case 'c':
        //  TODO:
        //  tStr2Struct( "c3", "00", 3, pvMemPtr, 1 )
        //  tStr2Struct( "c3", "�ɹ�", 22, pvMemPtr, 1 )
        //  tStr2Struct( "c3", "96", 25, pvMemPtr, 1 )
        //  tStr2Struct( "c3", "ʧ��", 44, pvMemPtr, 1 )
        //  tStr2Struct( "c3", "98", 47, pvMemPtr, 1 )
        //  tStr2Struct( "c3", "ϵͳ��æ", 66, pvMemPtr, 1 )
        iLen = strlen( pcStr );
        memcpy( pcTmp + *piOffset, pcStr, iLen );
        pcTmp[*piOffset + iLen] = '\0';

        //  TODO:
        //printf("\t%s\n",&pcTmp[*piOffset]);
        *piOffset+= atoi( pcType + 1 ) ? atoi( pcType + 1) : 1;

        if ( piStructKey != NULL && *piStructKey < CHAR_OFF )
            *piStructKey = CHAR_OFF;
        break;

        /* float�����ж�ƫ�����Ƿ�ɱ�4������������Ҫ�ض�λƫ��ֵ */
    case 'f':
        if ( *piOffset % FLOAT_OFF != 0 )
            *piOffset = ( *piOffset / FLOAT_OFF + 1 ) * FLOAT_OFF;

        fTmp = ( float )atof( pcStr );
        memcpy( pcTmp + *piOffset, &fTmp, sizeof( float ) );
        *piOffset+= sizeof( float );

        if ( piStructKey != NULL && *piStructKey < FLOAT_OFF )
            *piStructKey = FLOAT_OFF;
        break;

        /* double���� */
    case 'd':
        if ( *piOffset % DOUBLE_OFF != 0 )
            *piOffset = ( *piOffset / DOUBLE_OFF + 1 ) * DOUBLE_OFF;

        dTmp = atof( pcStr );
        memcpy( pcTmp + *piOffset, &dTmp, sizeof( double ) );
        *piOffset+= sizeof( double );

        if ( piStructKey != NULL && *piStructKey < DOUBLE_OFF )
            *piStructKey = DOUBLE_OFF;
        break;

        /* int���� */
    case 'i':
        if ( *piOffset % INT_OFF != 0 )
            *piOffset = ( *piOffset / INT_OFF + 1 ) * INT_OFF;

        iTmp = atoi( pcStr );
        memcpy( pcTmp + *piOffset, &iTmp, sizeof( int ) );
        *piOffset+= sizeof( int );

        if ( piStructKey != NULL && *piStructKey < INT_OFF )
            *piStructKey = INT_OFF;
        break;

        /* short���� */
    case 's':
        if ( *piOffset % SHORT_OFF != 0 )
            *piOffset = ( *piOffset / SHORT_OFF + 1 ) * SHORT_OFF;

        nTmp = ( short )( atoi( pcStr ) );
        memcpy( pcTmp + *piOffset, &nTmp, sizeof( short ) );
        *piOffset+= sizeof( short );

        if ( piStructKey != NULL && *piStructKey < SHORT_OFF )
            *piStructKey = SHORT_OFF;
        break;

        /* long���� */
    case 'l':
        if ( *piOffset % LONG_OFF != 0 )
            *piOffset = ( *piOffset / LONG_OFF + 1 ) * LONG_OFF;

        lTmp = atol( pcStr );
        memcpy( pcTmp + *piOffset, &lTmp, sizeof( long ) );
        *piOffset+= sizeof( long );

        if ( piStructKey != NULL && *piStructKey < LONG_OFF )
            *piStructKey = LONG_OFF;
        break;

        /* ָ������ */
    case 'p':
        //  tStr2Struct( "p", ( char * )( pvMemPtr ), &iOffset, pvStructAddr, NULL )
        // tStr2Struct( char *pcType, char *pcStr, int *piOffset, void *pvAddr, int *piStructKey )
        if ( *piOffset % LONG_OFF != 0 )
            *piOffset = ( *piOffset / LONG_OFF + 1 ) * LONG_OFF;

        memcpy( pcTmp + *piOffset, &pcStr, sizeof( pcTmp ) );
        *piOffset+= sizeof( pcTmp );

        if ( piStructKey != NULL && *piStructKey < LONG_OFF )
            *piStructKey = LONG_OFF;
        break;
    default:
        fprintf( stderr, "��֧�ֵ���������[%s]!\n", pcType );
        return( -1 );
    }
    return( 0 );
}

/*****************************************************************************
** ������ tChkSonStruNum( )                                                 **
** ���ͣ� int                                                               **
** ���:                                                                    **  
**      FILE *pstFp ( �����ļ�ָ�� )                                        **
**      char *pcItem ( ���������Ϣ���ַ���ָ�� )                           **
**      int  iSonNum ( �ӽṹ��Ŀ )                                         **
** ����:  ��                                                                **
** ����ֵ�� �ɹ� - 0,  ʧ�� - -1                                            **
** ���ܣ��ӵ�ǰָ��λ�ü�������ļ�ĳ�����������ӽṹ���Ƿ�����ӽṹ������ **
*****************************************************************************/
static int  tChkSonStruNum( FILE *pstFp, char *pcItem, int iSonNum )
{
    char    sTmp[MAX_CONF_REC_LEN];
    char    sTmp1[81];
    int     i = -1;
    long    lFpPos;


    lFpPos = ftell( pstFp );
    while ( fgets( sTmp, MAX_CONF_REC_LEN, pstFp ) )
    {
        if ( sTmp[0] == '#' || sTmp[0] == '\n' || sTmp[0] == '\r' || 
                                sTmp[0] == '\t' || sTmp[0] == ' ' )
            continue;

        if ( memcmp( sTmp, pcItem, strlen( pcItem ) ) )
            break;
        i++;
    }

    fseek( pstFp, lFpPos, SEEK_SET );
    if ( i >= iSonNum )
        return( -1 );

    return( 0 );
}


/*****************************************************************************
** ������ tCalcStructSize( )                                                **
** ���ͣ� int                                                               **
** ���:                                                                    **
**      char *pcStructDes ( �ṹ����������ַ��� )                          **
**      int  *piStructSize ( �ṹ��Ĵ�С )                                 **
** ����:                                                                    **
**      int  *piStructSize ( �ṹ��Ĵ�С )                                 **
** ����ֵ�� �ɹ� - ����0,  ʧ�� - -1                                        **
** ���ܣ����ݽṹ����������������ṹ���С                                 **
*****************************************************************************/
int tCalcStructSize( char *pcStructDes, int *piStructSize )
{
    int     iCnt = 0, iOffset = 0, iStructKey = 0;
    char    *pcTmpPtr, sOneDes[MAX_SON_STRUCT_DES];


    if ( !pcStructDes[0] )
    {
        fprintf( stderr, "�ṹ��������Ϊ��, ���ܼ���ṹ���С.\n" );
        return ( -1 );
    }

    for ( pcTmpPtr = pcStructDes, iStructKey = 0; pcTmpPtr != NULL; )
    {
        memset( sOneDes, 0x00, sizeof( sOneDes ) );
        tGetOneField( &pcTmpPtr, ',', sOneDes );
        tTrim( sOneDes );

        if ( sOneDes[0] < 'a' || sOneDes[0] > 'z' )
        {
            fprintf( stderr, "�Ƿ��Ľṹ��������.\n" );
            return ( -1 );
        }

        /* ����¼ֵ����ת����д��ṹ���ڴ� */
        tCalcOneFieldSize( sOneDes, &iOffset, &iStructKey );
    }
    if ( iOffset % iStructKey )
        iOffset = ( iOffset / iStructKey + 1 ) * iStructKey;

    *piStructSize = iOffset;

    return( 0 );
}
/*****************************************************************************
** ������tGetConfig( )                                                      **
** ���ͣ� int                                                               **
** ���:                                                                    **
**      char *pcFielName ( �����ļ��� )                                     **
**      char *pcStructDes ( ���������Ӧ�Ľṹ�������ִ� )                  **
**      char *pcStruName ( ���������Ӧ�Ľṹ���ֶ������ִ� )               **
**      void *pvStructAddr ( ���������Ӧ�Ľṹ���ָ�� )                   **
**      char *pcGroup ( �����ļ��е��������� )                              **
** ���Σ�                                                                   **
**      void *pvStructAddr ( ���������Ӧ�Ľṹ���ָ�� )                   **
** ����ֵ�� 0 - �ɹ�, ���� - δ�ҵ�                                         **
** ���ܣ��������ļ��ж�ȡָ�����ȫ��������Ϣ����Ӧ�Ľṹ����               **
*****************************************************************************/
int     tGetConfig( char *pcFileName, char *pcStructDes, char *pcStruName, void *pvStructAddr, char *pcGroup )
{
    FILE    *pstFp;
    int     i, j, iLen, iSonStruSize;
    int     iOffset = 0, iOffset1 = 0;
    int     iSonFields, iSonLoops, iStructKey;
    long    lFpPos;
    void    *pvMemPtr;
    char    *pcWord[16], *pcWord1[16], *pcTmp1, *pcTmp2;
    char    sTmp[MAX_CONF_REC_LEN], sTmp1[MAX_CONF_REC_LEN];
    char    sOneDes[MAX_SON_STRUCT_DES],sFldName[MAX_FIELD_NAME];
    char    sSonStruDes[256];


    pstFp = fopen( pcFileName, "r" );
    if ( pstFp == ( FILE * )NULL )
    {
        fprintf( stderr, "���ļ�%sʧ��, errno[%d].\n", pcFileName, errno );
        return( -1 );
    }

    if ( !( iLen = strlen( pcGroup ) ) )
    {
        fprintf( stderr, "����������Ϊ��!\n" );
        return( -1 );
    }

    /* ȷ������������λ�� */
    while ( !feof( pstFp ) )
    {
        fgets( sTmp, MAX_CONF_REC_LEN, pstFp );
        if ( sTmp[0] == '[' && memcmp( sTmp + 1, pcGroup, iLen ) == 0 
                                        && sTmp[1 + iLen] == ']' )
            break;
    }

    if ( feof( pstFp ) )
    {
        fclose( pstFp );
        fprintf( stderr, "�����ļ�%s��û��������[%s]����Ϣ.\n", pcFileName, pcGroup );
        return( -1 );
    }

    pcTmp1 = pcStructDes;
    pcTmp2 = pcStruName;

    /* �Խṹ���������ͽṹ���ֶ����ƴ�ѭ������  */
    while ( pcTmp1 != NULL && pcTmp2 != NULL )
    {
        /* ��ȡ�ṹ���������ͽṹ���ֶ����ƴ��е�һ���ֶε�������Ϣ������ */
        memset( sOneDes, 0, sizeof( sOneDes ) );
        memset( sFldName, 0, sizeof( sFldName ) );
        tCutOneField( &pcTmp1, sOneDes );
        tCutOneField( &pcTmp2, sFldName );

        /* ���ӽṹ����� */
        if ( sOneDes[0] != '(' )
        {
            /* �������ļ���˳���ȡһ����Ч��¼ */
            if ( tGetRecord( pstFp, sFldName, sTmp1 ) < 0 )
            {
                fprintf( stderr, "�����ļ�%s�е�[%s]������û��[%s]������.\n", 
                                                pcFileName, pcGroup, sFldName );
                return( -1 );
            }

            /* ����¼ֵ����ת����д��ṹ���ڴ� */
            if ( tStr2Struct( sOneDes, sTmp1, &iOffset, pvStructAddr, 
                                                                NULL ) < 0 )
            {
                fprintf( stderr, "�����ļ�%s�е�[%s]�������[%s]������ֵд�����ýṹ��ʧ��!\n", pcFileName, pcGroup, sFldName );
                return( -1 );
            }
            continue;
        }

        /* ���ӽṹ����� */
        tTrim( sOneDes );
        sOneDes[0] = ' ';
        sOneDes[strlen(sOneDes) - 1] = ' ';
        strcpy( sSonStruDes, sOneDes );

        /* ����ӽṹ������ */
        iSonFields = tSeperate( sOneDes, pcWord, 16, ',' );

        /* ��ȡ�����ļ����ӽṹ����  */
        iSonLoops = atoi( sTmp1 );

        /* �����ӽṹ��������������ӽṹ���С */
        if ( tCalcStructSize( sSonStruDes, &iSonStruSize ) < 0 )
        {
            fprintf( stderr, "���ݽṹ��������������ṹ���Сʧ��.\n" );
            return( -1 );
        }

        /* ���������ļ��ж�����ӽṹ��,�����ӽṹ������ռ� */
        /* ��ȫ�̱�����¼ malloc ȡ�õ��ڴ�ָ��, �����պ��ͷ�֮�� */
        if ( g_iMallocNum >= MAX_MALLOC_NUM )
        {
            fprintf( stderr, "��̬�����ڴ�ָ����������!\n" );
            return( -1 );
        }
        pvMemPtr = ( void * )malloc( iSonStruSize * iSonLoops );
        g_pvMallocPtr[g_iMallocNum++] = pvMemPtr;
        memset( pvMemPtr, 0x00, iSonStruSize * iSonLoops );
        if ( tStr2Struct( "p", ( char * )( pvMemPtr ), &iOffset, 
                                                    pvStructAddr, NULL ) < 0 )
        {
            fprintf( stderr, "�����ļ�%s�е�[%s]�����������ֵд�����ýṹ��ʧ��!\n", pcFileName, pcGroup );
            return( -1 );
        }
        if ( iSonLoops == 0 )
            continue;
    
        /*  ��������ļ����ӽṹ������Ŀ�Ƿ�����ӽṹ������Ϣ��    */
        if ( tChkSonStruNum( pstFp, sFldName, iSonLoops ) < 0 )
        {
            fprintf( stderr, "�����ļ�%s��[%s]��������[%s]�ӽṹ�������ӽṹ������Ŀ.\n", pcFileName, pcGroup, sFldName );
            return( -1 );
        }

        /* ���������ļ��е��ӽṹ��Ϣ,��д���ӽṹ��    */
        for ( i = 0; i < iSonLoops; i++ )
        {
            /* �������ļ���˳���ȡһ����Ч��¼ */
            sprintf( sTmp, "%s%d", sFldName, i );
            if ( tGetRecord( pstFp, sTmp, sTmp1 ) < 0 )
            {
                fprintf( stderr, "�����ļ�%s�е�[%s]������û��[%s]������.\n",
                                                pcFileName, pcGroup, sTmp );
                return( -1 );
            }
            //  TODO:
            //printf("i=%d\tsTmp=[%s]\tsTmp1=[%s]\n", i, sTmp, sTmp1);


            /* ��ֶ�ά��¼������ */
            //if ( tSeperate( sTmp1, pcWord1, 16, '|' ) != iSonFields )
            if ( tSeperate( sTmp1, pcWord1, 16, '|' ) != iSonFields )
            {
                fprintf( stderr, "�����ļ�%s��[%s]�������µı���ֵ�������[%s]������Ŀ����ȷ, ������ĿӦΪ[%d].\n", pcFileName, pcGroup, sTmp, iSonFields );
                return( -1 );
            }
            //printf("\tiSonFields=%d\tpcWord1[%d] = [%s]\n", iSonFields, i, pcWord1[i]);

            iStructKey = 0;

            /* �˵�������ֵ�ַ���ǰ��Ŀո�������д����ڴ� */
            for ( j = 0; j < iSonFields; j++ )
            {
                tTrim( pcWord1[j] );
                //tStr2Struct( "c3", "00", 3, pvMemPtr, 1 )
                if ( tStr2Struct( pcWord[j], pcWord1[j], 
                                    &iOffset1, pvMemPtr, &iStructKey ) < 0 )
                {
                    fprintf( stderr, "�����ļ�%s�е�[%s]�������[%s]������ֵд�����ýṹ��ʧ��!\n", pcFileName, pcGroup, sTmp );
                    return( -1 );
                }
                //  TODO:
                //printf("\t[%d,%d]\tpcWord[%d] = [%s]\tpcWord1[%d] = [%s]\n", iOffset1, iStructKey, j, pcWord[j], j, pcWord1[j]);
            }

            if ( iOffset1 % iStructKey )
                iOffset1 = ( iOffset1 / iStructKey + 1 ) * iStructKey;
        }

        /* �ӽṹƫ�������� */
        iOffset1 = 0;
    }
    return( 0 );
}


/*****************************************************************************
** ������tFreeConfig( )                                                     **
** ���ͣ� void                                                              **
** ��Σ�                                                                   **
**      void *g_pvMallocPtr[] ( ������ڴ�ָ������� )                      **
**      int g_iMallocNum ( ���ڴ�������� )                                 **
** ���Σ� ��                                                                **
** ����ֵ�� ��                                                              **
** ���ܣ��ͷŶ�ȡ������Ϣʱ�Զ�����Ķ��ڴ�                                 **
*****************************************************************************/
void    tFreeConfig( )
{
    int     i;

    for ( i = 0; i < g_iMallocNum; i++ )
        free( g_pvMallocPtr[i] );

    /* ������ڴ������� 0 */
    g_iMallocNum = 0;
}