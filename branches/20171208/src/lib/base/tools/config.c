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
** 函数： tCalcOneFieldSize( )                                              **
** 类型： int                                                               **
** 入参：                                                                   ** 
**      char *pcType ( 要计算的结构体某个域的描述字符串 )                   **
**      int  *piOffset ( 当前结构体的偏移量 )                               **
**      int  *piStructKey ( 当前结构体中最大的补齐值 )                      **
** 出参:                                                                    **
**      int  *piOffset ( 累加该域后结构体的偏移量 )                         **
** 返回值： 成功 - 0,  失败 - -1                                            **
** 功能：根据结构体域描述串计算结构体偏移量                                 **
*****************************************************************************/
int tCalcOneFieldSize( char *pcType, int *piOffset, int *piStructKey )
{
    int     iCnt = 0;
    void    *pcPtr;


    switch ( *pcType )
    {
    /* 字符串类型无须考虑偏移量直结填充 */
    case 'c':
        *piOffset += atoi( pcType + 1 ) ? atoi( pcType + 1) : 1;

        if ( piStructKey != NULL && *piStructKey < CHAR_OFF )
            *piStructKey = CHAR_OFF;
        break;

    /* float类型判断偏移量是否可被4整除不能则需要重定位偏移值 */
    case 'f':
        if ( *piOffset % FLOAT_OFF != 0 )
            *piOffset = ( *piOffset / FLOAT_OFF + 1 ) * FLOAT_OFF;

        iCnt = atoi( pcType + 1 ) ? atoi( pcType + 1) : 1;
        *piOffset += iCnt * sizeof( float );

        if ( piStructKey != NULL && *piStructKey < FLOAT_OFF )
            *piStructKey = FLOAT_OFF;
        break;

    /* double类型 */
    case 'd':
        if ( *piOffset % DOUBLE_OFF != 0 )
            *piOffset = ( *piOffset / DOUBLE_OFF + 1 ) * DOUBLE_OFF;

        iCnt = atoi( pcType + 1 ) ? atoi( pcType + 1) : 1;
        *piOffset += iCnt * sizeof( double );

        if ( piStructKey != NULL && *piStructKey < DOUBLE_OFF )
            *piStructKey = DOUBLE_OFF;
        break;

    /* int类型 */
    case 'i':
        if ( *piOffset % INT_OFF != 0 )
            *piOffset = ( *piOffset / INT_OFF + 1 ) * INT_OFF;

        iCnt = atoi( pcType + 1 ) ? atoi( pcType + 1) : 1;
        *piOffset += iCnt * sizeof( int );

        if ( piStructKey != NULL && *piStructKey < INT_OFF )
            *piStructKey = INT_OFF;
        break;

    /* short类型 */
    case 's':
        if ( *piOffset % SHORT_OFF != 0 )
            *piOffset = ( *piOffset / SHORT_OFF + 1 ) * SHORT_OFF;

        iCnt = atoi( pcType + 1 ) ? atoi( pcType + 1) : 1;
        *piOffset += iCnt * sizeof( short );

        if ( piStructKey != NULL && *piStructKey < SHORT_OFF )
            *piStructKey = SHORT_OFF;
        break;

    /* long类型 */
    case 'l':
        if ( *piOffset % LONG_OFF != 0 )
            *piOffset = ( *piOffset / LONG_OFF + 1 ) * LONG_OFF;

        iCnt = atoi( pcType + 1 ) ? atoi( pcType + 1) : 1;
        *piOffset += iCnt * sizeof( long );

        if ( piStructKey != NULL && *piStructKey < LONG_OFF )
            *piStructKey = LONG_OFF;
        break;

    /* 指针类型 */
    case 'p':
        if ( *piOffset % LONG_OFF != 0 )
            *piOffset = ( *piOffset / LONG_OFF + 1 ) * LONG_OFF;

        iCnt = atoi( pcType + 1 ) ? atoi( pcType + 1) : 1;
        *piOffset += iCnt * sizeof( pcPtr );

        if ( piStructKey != NULL && *piStructKey < LONG_OFF )
            *piStructKey = LONG_OFF;
        break;
    default:
        fprintf( stderr, "不支持的数据类型[%s]!\n", pcType );
        return( -1 );
    }

    return( 0 );
}
/*****************************************************************************
** 函数： tGetRecord( )                                                     **
** 类型： int                                                               **
** 入参:                                                                    **
**      FILE *pstFp ( 配置文件指针 )                                        **
**      char *pcItem ( 配置项名称 )                                         **
**      char *pcRecord ( 存放配置信息的字符串指针 )                         **
** 出参:                                                                    **
**      char *pcRecord ( 存放配置信息的字符串指针 )                         **
** 返回值： 成功 - 0,  失败 - -1                                            **
** 功能：从配置文件当前指针位置读取配置项的配置值                           **
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
/*      tErrLog( DEBUG, "读取的配置记录为[%s]", sTmp ); */
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
** 函数:  tStr2Struct( )                                                    **
** 类型： int                                                               **
** 入参：                                                                   **
**      char *pcType ( 要处理的字符串类型 )                                 **
**      char *pcStr ( 要处理的字符串指针 )                                  **
**      int  *piOffset ( 目前结构存储区的偏移量地址 )                       **
**      void *pvAddr ( 目前结构存储区的首地址 )                             **
**      int  *piStructKey ( 目前结构存储区的首地址 )                        **
** 出参:                                                                    **
**      int  *piOffset ( 目前结构存储区的偏移量地址 )                       **
** 返回值： 成功 - 0,  失败 - -1                                            **
** 功能：将字符串按目标类型进行转换并写入结构地址空间                       **
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
        /* 字符串类型无须考虑偏移量直结填充 */
    case 'c':
        //  TODO:
        //  tStr2Struct( "c3", "00", 3, pvMemPtr, 1 )
        //  tStr2Struct( "c3", "成功", 22, pvMemPtr, 1 )
        //  tStr2Struct( "c3", "96", 25, pvMemPtr, 1 )
        //  tStr2Struct( "c3", "失败", 44, pvMemPtr, 1 )
        //  tStr2Struct( "c3", "98", 47, pvMemPtr, 1 )
        //  tStr2Struct( "c3", "系统繁忙", 66, pvMemPtr, 1 )
        iLen = strlen( pcStr );
        memcpy( pcTmp + *piOffset, pcStr, iLen );
        pcTmp[*piOffset + iLen] = '\0';

        //  TODO:
        //printf("\t%s\n",&pcTmp[*piOffset]);
        *piOffset+= atoi( pcType + 1 ) ? atoi( pcType + 1) : 1;

        if ( piStructKey != NULL && *piStructKey < CHAR_OFF )
            *piStructKey = CHAR_OFF;
        break;

        /* float类型判断偏移量是否可被4整除不能则需要重定位偏移值 */
    case 'f':
        if ( *piOffset % FLOAT_OFF != 0 )
            *piOffset = ( *piOffset / FLOAT_OFF + 1 ) * FLOAT_OFF;

        fTmp = ( float )atof( pcStr );
        memcpy( pcTmp + *piOffset, &fTmp, sizeof( float ) );
        *piOffset+= sizeof( float );

        if ( piStructKey != NULL && *piStructKey < FLOAT_OFF )
            *piStructKey = FLOAT_OFF;
        break;

        /* double类型 */
    case 'd':
        if ( *piOffset % DOUBLE_OFF != 0 )
            *piOffset = ( *piOffset / DOUBLE_OFF + 1 ) * DOUBLE_OFF;

        dTmp = atof( pcStr );
        memcpy( pcTmp + *piOffset, &dTmp, sizeof( double ) );
        *piOffset+= sizeof( double );

        if ( piStructKey != NULL && *piStructKey < DOUBLE_OFF )
            *piStructKey = DOUBLE_OFF;
        break;

        /* int类型 */
    case 'i':
        if ( *piOffset % INT_OFF != 0 )
            *piOffset = ( *piOffset / INT_OFF + 1 ) * INT_OFF;

        iTmp = atoi( pcStr );
        memcpy( pcTmp + *piOffset, &iTmp, sizeof( int ) );
        *piOffset+= sizeof( int );

        if ( piStructKey != NULL && *piStructKey < INT_OFF )
            *piStructKey = INT_OFF;
        break;

        /* short类型 */
    case 's':
        if ( *piOffset % SHORT_OFF != 0 )
            *piOffset = ( *piOffset / SHORT_OFF + 1 ) * SHORT_OFF;

        nTmp = ( short )( atoi( pcStr ) );
        memcpy( pcTmp + *piOffset, &nTmp, sizeof( short ) );
        *piOffset+= sizeof( short );

        if ( piStructKey != NULL && *piStructKey < SHORT_OFF )
            *piStructKey = SHORT_OFF;
        break;

        /* long类型 */
    case 'l':
        if ( *piOffset % LONG_OFF != 0 )
            *piOffset = ( *piOffset / LONG_OFF + 1 ) * LONG_OFF;

        lTmp = atol( pcStr );
        memcpy( pcTmp + *piOffset, &lTmp, sizeof( long ) );
        *piOffset+= sizeof( long );

        if ( piStructKey != NULL && *piStructKey < LONG_OFF )
            *piStructKey = LONG_OFF;
        break;

        /* 指针类型 */
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
        fprintf( stderr, "不支持的数据类型[%s]!\n", pcType );
        return( -1 );
    }
    return( 0 );
}

/*****************************************************************************
** 函数： tChkSonStruNum( )                                                 **
** 类型： int                                                               **
** 入参:                                                                    **  
**      FILE *pstFp ( 配置文件指针 )                                        **
**      char *pcItem ( 存放配置信息的字符串指针 )                           **
**      int  iSonNum ( 子结构数目 )                                         **
** 出参:  无                                                                **
** 返回值： 成功 - 0,  失败 - -1                                            **
** 功能：从当前指针位置检查配置文件某个配置组下子结构数是否大于子结构配置数 **
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
** 函数： tCalcStructSize( )                                                **
** 类型： int                                                               **
** 入参:                                                                    **
**      char *pcStructDes ( 结构体的域描述字符串 )                          **
**      int  *piStructSize ( 结构体的大小 )                                 **
** 出参:                                                                    **
**      int  *piStructSize ( 结构体的大小 )                                 **
** 返回值： 成功 - 返回0,  失败 - -1                                        **
** 功能：根据结构体域描述串计算出结构体大小                                 **
*****************************************************************************/
int tCalcStructSize( char *pcStructDes, int *piStructSize )
{
    int     iCnt = 0, iOffset = 0, iStructKey = 0;
    char    *pcTmpPtr, sOneDes[MAX_SON_STRUCT_DES];


    if ( !pcStructDes[0] )
    {
        fprintf( stderr, "结构体描述串为空, 不能计算结构体大小.\n" );
        return ( -1 );
    }

    for ( pcTmpPtr = pcStructDes, iStructKey = 0; pcTmpPtr != NULL; )
    {
        memset( sOneDes, 0x00, sizeof( sOneDes ) );
        tGetOneField( &pcTmpPtr, ',', sOneDes );
        tTrim( sOneDes );

        if ( sOneDes[0] < 'a' || sOneDes[0] > 'z' )
        {
            fprintf( stderr, "非法的结构体描述串.\n" );
            return ( -1 );
        }

        /* 将记录值进行转换并写入结构体内存 */
        tCalcOneFieldSize( sOneDes, &iOffset, &iStructKey );
    }
    if ( iOffset % iStructKey )
        iOffset = ( iOffset / iStructKey + 1 ) * iStructKey;

    *piStructSize = iOffset;

    return( 0 );
}
/*****************************************************************************
** 函数：tGetConfig( )                                                      **
** 类型： int                                                               **
** 入参:                                                                    **
**      char *pcFielName ( 配置文件名 )                                     **
**      char *pcStructDes ( 与配置组对应的结构体描述字串 )                  **
**      char *pcStruName ( 与配置组对应的结构体字段名称字串 )               **
**      void *pvStructAddr ( 与配置组对应的结构体的指针 )                   **
**      char *pcGroup ( 配置文件中的配置组名 )                              **
** 出参：                                                                   **
**      void *pvStructAddr ( 与配置组对应的结构体的指针 )                   **
** 返回值： 0 - 成功, 非零 - 未找到                                         **
** 功能：从配置文件中读取指定组的全部配置信息到对应的结构体中               **
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
        fprintf( stderr, "打开文件%s失败, errno[%d].\n", pcFileName, errno );
        return( -1 );
    }

    if ( !( iLen = strlen( pcGroup ) ) )
    {
        fprintf( stderr, "配置组名称为空!\n" );
        return( -1 );
    }

    /* 确定配置组所在位置 */
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
        fprintf( stderr, "配置文件%s中没有配置组[%s]的信息.\n", pcFileName, pcGroup );
        return( -1 );
    }

    pcTmp1 = pcStructDes;
    pcTmp2 = pcStruName;

    /* 对结构体描述串和结构体字段名称串循环处理  */
    while ( pcTmp1 != NULL && pcTmp2 != NULL )
    {
        /* 截取结构体描述串和结构体字段名称串中的一个字段的描述信息及名称 */
        memset( sOneDes, 0, sizeof( sOneDes ) );
        memset( sFldName, 0, sizeof( sFldName ) );
        tCutOneField( &pcTmp1, sOneDes );
        tCutOneField( &pcTmp2, sFldName );

        /* 无子结构的情况 */
        if ( sOneDes[0] != '(' )
        {
            /* 从配置文件中顺序读取一条有效记录 */
            if ( tGetRecord( pstFp, sFldName, sTmp1 ) < 0 )
            {
                fprintf( stderr, "配置文件%s中的[%s]配置组没有[%s]配置项.\n", 
                                                pcFileName, pcGroup, sFldName );
                return( -1 );
            }

            /* 将记录值进行转换并写入结构体内存 */
            if ( tStr2Struct( sOneDes, sTmp1, &iOffset, pvStructAddr, 
                                                                NULL ) < 0 )
            {
                fprintf( stderr, "配置文件%s中的[%s]配置组的[%s]配置项值写入配置结构体失败!\n", pcFileName, pcGroup, sFldName );
                return( -1 );
            }
            continue;
        }

        /* 有子结构的情况 */
        tTrim( sOneDes );
        sOneDes[0] = ' ';
        sOneDes[strlen(sOneDes) - 1] = ' ';
        strcpy( sSonStruDes, sOneDes );

        /* 拆分子结构描述串 */
        iSonFields = tSeperate( sOneDes, pcWord, 16, ',' );

        /* 提取配置文件中子结构个数  */
        iSonLoops = atoi( sTmp1 );

        /* 根据子结构体描述串计算出子结构体大小 */
        if ( tCalcStructSize( sSonStruDes, &iSonStruSize ) < 0 )
        {
            fprintf( stderr, "根据结构体描述串，计算结构体大小失败.\n" );
            return( -1 );
        }

        /* 根据配置文件中定义的子结构数,分配子结构体数组空间 */
        /* 用全程变量记录 malloc 取得的内存指针, 用于日后释放之用 */
        if ( g_iMallocNum >= MAX_MALLOC_NUM )
        {
            fprintf( stderr, "动态分配内存指针数组已满!\n" );
            return( -1 );
        }
        pvMemPtr = ( void * )malloc( iSonStruSize * iSonLoops );
        g_pvMallocPtr[g_iMallocNum++] = pvMemPtr;
        memset( pvMemPtr, 0x00, iSonStruSize * iSonLoops );
        if ( tStr2Struct( "p", ( char * )( pvMemPtr ), &iOffset, 
                                                    pvStructAddr, NULL ) < 0 )
        {
            fprintf( stderr, "配置文件%s中的[%s]配置组的配置值写入配置结构体失败!\n", pcFileName, pcGroup );
            return( -1 );
        }
        if ( iSonLoops == 0 )
            continue;
    
        /*  检查配置文件中子结构配置数目是否大于子结构配置信息数    */
        if ( tChkSonStruNum( pstFp, sFldName, iSonLoops ) < 0 )
        {
            fprintf( stderr, "配置文件%s的[%s]配置组中[%s]子结构数大于子结构配置数目.\n", pcFileName, pcGroup, sFldName );
            return( -1 );
        }

        /* 读入配置文件中的子结构信息,并写入子结构体    */
        for ( i = 0; i < iSonLoops; i++ )
        {
            /* 从配置文件中顺序读取一条有效记录 */
            sprintf( sTmp, "%s%d", sFldName, i );
            if ( tGetRecord( pstFp, sTmp, sTmp1 ) < 0 )
            {
                fprintf( stderr, "配置文件%s中的[%s]配置组没有[%s]配置项.\n",
                                                pcFileName, pcGroup, sTmp );
                return( -1 );
            }
            //  TODO:
            //printf("i=%d\tsTmp=[%s]\tsTmp1=[%s]\n", i, sTmp, sTmp1);


            /* 拆分二维记录各子域 */
            //if ( tSeperate( sTmp1, pcWord1, 16, '|' ) != iSonFields )
            if ( tSeperate( sTmp1, pcWord1, 16, '|' ) != iSonFields )
            {
                fprintf( stderr, "配置文件%s中[%s]配置组下的被拆分的配置项[%s]子域数目不正确, 子域数目应为[%d].\n", pcFileName, pcGroup, sTmp, iSonFields );
                return( -1 );
            }
            //printf("\tiSonFields=%d\tpcWord1[%d] = [%s]\n", iSonFields, i, pcWord1[i]);

            iStructKey = 0;

            /* 滤掉各子域值字符串前后的空隔并依次写入堆内存 */
            for ( j = 0; j < iSonFields; j++ )
            {
                tTrim( pcWord1[j] );
                //tStr2Struct( "c3", "00", 3, pvMemPtr, 1 )
                if ( tStr2Struct( pcWord[j], pcWord1[j], 
                                    &iOffset1, pvMemPtr, &iStructKey ) < 0 )
                {
                    fprintf( stderr, "配置文件%s中的[%s]配置组的[%s]配置项值写入配置结构体失败!\n", pcFileName, pcGroup, sTmp );
                    return( -1 );
                }
                //  TODO:
                //printf("\t[%d,%d]\tpcWord[%d] = [%s]\tpcWord1[%d] = [%s]\n", iOffset1, iStructKey, j, pcWord[j], j, pcWord1[j]);
            }

            if ( iOffset1 % iStructKey )
                iOffset1 = ( iOffset1 / iStructKey + 1 ) * iStructKey;
        }

        /* 子结构偏移量重置 */
        iOffset1 = 0;
    }
    return( 0 );
}


/*****************************************************************************
** 函数：tFreeConfig( )                                                     **
** 类型： void                                                              **
** 入参：                                                                   **
**      void *g_pvMallocPtr[] ( 保存堆内存指针的数组 )                      **
**      int g_iMallocNum ( 堆内存申请次数 )                                 **
** 出参： 无                                                                **
** 返回值： 无                                                              **
** 功能：释放读取配置信息时自动申请的堆内存                                 **
*****************************************************************************/
void    tFreeConfig( )
{
    int     i;

    for ( i = 0; i < g_iMallocNum; i++ )
        free( g_pvMallocPtr[i] );

    /* 申请堆内存总数置 0 */
    g_iMallocNum = 0;
}