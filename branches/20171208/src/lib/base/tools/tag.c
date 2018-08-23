/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Gets next tlv object separated into tag, length and value*/
short getNextTLVObject(unsigned short *tag, short *length, unsigned char *value, unsigned char *buffer)
{
	unsigned char *ptr;
	unsigned char tagByte1, tagByte2;
	unsigned char dataByte;
	short numLengthBytes;
	short dataLength;
	short i;
	short numTagBytes;
	short bytesRead;

	/* Get the tag*/

	ptr = (unsigned char *)buffer;   

	tagByte1 = *ptr;
	/* ?0x00 ????? */
	
	if ( tagByte1 == (unsigned char)0x00 )
	{
		return 1;
	}

	if ((tagByte1 & 0x1F) == 0x1F)                                        /* Bit pattern must be 10011111 or greater  */ 
	{
		ptr++;
		tagByte2 = *ptr;
		*tag = (short) ((tagByte1 << 8) + tagByte2);
		numTagBytes = 2;
	}
	else
	{
		*tag = (short) tagByte1;
		numTagBytes = 1;
	}

	/*  Get the data  */ 
	ptr++;
	dataByte = *ptr;
	if (dataByte & 128)                                                  /*   If last bit is set   */ 
	{
		dataLength = 0;
		numLengthBytes = (short) dataByte & 127;                         /*   b7 - b1 represent the number of subsequent length bytes   */ 
		ptr++;
		for (i = 0; i < numLengthBytes; i++)
		{
			dataLength = (dataLength << 8) + (short) *ptr;
			ptr++;
		}
		numLengthBytes++;
	}
	else                                                                  /* Length field consists of 1 byte max value of 127   */ 
	{
		numLengthBytes = 1;
		dataLength = (short) *ptr;
		ptr++;
	}

	*length = dataLength;

	/*  ptr should now be pointing at the data  */ 
	for (i = 0; i < dataLength; i++)
	{
		value[i] = *ptr;
		ptr++;
	}

	bytesRead = numTagBytes + numLengthBytes + dataLength;

	return (bytesRead);
}
/*
 			strcpy( sData, "01669f26088276b6e950bf483f9f2701809f101307010103a00010010a0100000001003e8892079f37045efff0e89f3602002b950580000460009a031201159c01639f02060000000100005f2a02015682027c009f1a0201569f03060000000000009f3303e058209f34030203009f3501149f1e0836393232343020208408a0000003330101019f090200209f41040000346157136214190100000006954d21122200000000042f000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000" );
			tAsc2Bcd( caBcd, sData, strlen(sData), LEFT_ALIGN );
			tFindTag( 0x9F10, sTag, &sTagLen, caBcd, strlen(sData)/2 );
 */

int tFindTag( unsigned short tag, unsigned char *value, short *length, unsigned char *buffer, short bufLen)
{
	unsigned char *ptr, buf[1024], cSubBuf[1024] = { 0 };
	unsigned short nextTag;
	short len = 0, shSubLen = 0;
	short bytesRead, shSubOff = 0;

	ptr = buffer;                                                  

	bytesRead = 0;
	do
	{
		bytesRead += getNextTLVObject(&nextTag, &len, buf, ptr + bytesRead);
		if (nextTag == tag)
		{
			memcpy(value, buf, len);
			*length = len;
			//????????
			return (int)bytesRead;
		}
	} while (bytesRead < bufLen);

	return 0;
}


void tFindTagValue(unsigned char *buffer, short bufLen, unsigned short tags[], short *length)
{
	unsigned char *ptr, tagByte1, tagByte2;
	short len = 0;
	short bytesRead, total, numTagBytes;

	ptr = buffer;                                                  

	total = 0;
	bytesRead = 0;
	do
	{
		tagByte1 = *ptr;
		if ((tagByte1 & 0x1F) == 0x1F)
		{
			ptr++;
			tagByte2 = *ptr;
			tags[total] = (short) ((tagByte1 << 8) + tagByte2);
			numTagBytes = 2;
		}
		else
		{
			tags[total] = (short) tagByte1;
			numTagBytes = 1;
		}
		bytesRead += numTagBytes+1;
		total++;
		ptr += 2;
	} while (bytesRead < bufLen);

	*length = total;
}