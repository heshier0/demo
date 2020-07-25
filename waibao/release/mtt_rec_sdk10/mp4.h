#ifndef INC_MP4_H
#define INC_MP4_H

#define MTT_VENC_MAX_CHN_NUM 2

int HI_PDT_Init(void);
int HI_PDT_Exit(void);

int HI_PDT_CreateMp4(VENC_CHN VeChn);
void HI_PDT_CloseMp4(VENC_CHN VeChn);
HI_S32 HI_PDT_WriteVideo(VENC_CHN VeChn, VENC_STREAM_S *pstStream);

#endif

