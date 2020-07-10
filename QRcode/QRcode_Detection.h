#ifndef __QRCODE_DETECTION_H__
#define __QRCODE_DETECTION_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>


int QRcode_Detection(const char* buffer, int inwidth, int inheight, void** output);


#ifdef __cplusplus
}
#endif

#endif //__QRCODE_DETECTION_H__