/*
 * CRCUtil.h
 *
 *  Created on: 31 Jul 2019
 *      Author: Michael Neuweiler
 */

#ifndef CRCUTIL_H_
#define CRCUTIL_H_

#include <Arduino.h>
#include <string>

class CRCUtil
{
public:
    CRCUtil();

    static byte *getCRCByte(String command);
    static String getCRC(String command);
    static bool checkCRC(String resultValue);
    static String toHexString(String s);
    static int caluCRC(String pByte);

private:
    static char crc_tb[];
    static byte byteBuf[];
};

#endif /* CRCUTIL_H_ */
