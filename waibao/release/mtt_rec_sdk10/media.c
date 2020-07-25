#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include <assert.h>
#include <semaphore.h>

#include "sample_comm.h"
#include "mp4.h"
#include "media.h"

#define VDEC_CHN_NUM 1
#define VI_CNT       2

typedef struct
{
	//媒体信息
	HI_BOOL bInit;
    VB_CONFIG_S stVbConf;
    SIZE_S stSize;
    VO_LAYER VoLayer;
    HI_U32 VpssGrpNum;

    SAMPLE_VI_CONFIG_S stViConfig;
    SAMPLE_VDEC_ATTR astSampleVdec[VDEC_MAX_CHN_NUM];
    VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_CHN_NUM];
    SAMPLE_VO_CONFIG_S stVoConfig;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    HI_BOOL abChnEnable[VPSS_MAX_CHN_NUM];
    
    //编码线程信息
    pthread_t stVencPid;
    
    //采集线程信息
    pthread_t stVinPid;
    HI_BOOL bVinThreadRun;    
} media_info_t;

static media_info_t media_info;

static SAMPLE_VENC_GETSTREAM_PARA_S gs_stPara;

static sem_t sem_snap[2];

#define BYTE_ALIGN(a,b)    ((( a ) / b) * b)

HI_S32 Media_SetIRSaturation(HI_VOID)
{
	HI_S32 s32Ret;
	ISP_SATURATION_ATTR_S stSatAttr;
	
	stSatAttr.enOpType = OP_TYPE_MANUAL;
	stSatAttr.stManual.u8Saturation = 0;

	s32Ret = HI_MPI_ISP_SetSaturationAttr(2, &stSatAttr);
	if(s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("HI_MPI_ISP_SetSaturationAttr set is faile!\n");
		return -1;
	}
	
	return s32Ret;
}

static HI_S32 MediaInfo_Init(media_info_t * info, SIZE_S stSize, HI_S32 s32Rotate)
{
	HI_S32 s32Ret;
	HI_U32             u32BlkSize;
	PIC_SIZE_E         enPicSize;
	VI_DEV ViDev[2]   = {0, 1};
    VI_PIPE ViPipe[4] = {0, 1, 2, 3};
	
	if (!info)
		return HI_FAILURE;
		
	if (info->bInit)
		return HI_SUCCESS;	
		
	memset(info, 0, sizeof(media_info_t));	

    info->VpssGrpNum    = VI_CNT;

    /************************************************
    step:  init SYS, init common VB(for VPSS and VO)
    *************************************************/
    /*config vi*/
    SAMPLE_COMM_VI_GetSensorInfo(&info->stViConfig);

    info->stViConfig.s32WorkingViNum = VI_CNT;
    info->stViConfig.as32WorkingViId[0] = 0;
    info->stViConfig.as32WorkingViId[1] = 1;
    info->stViConfig.astViInfo[0].stSnsInfo.MipiDev         = ViDev[0];
    info->stViConfig.astViInfo[0].stSnsInfo.s32BusId        = 0;
    info->stViConfig.astViInfo[0].stSnsInfo.s32SnsId        = 0;
    info->stViConfig.astViInfo[0].stSnsInfo.enSnsType		= SENSOR0_TYPE;
    info->stViConfig.astViInfo[0].stDevInfo.ViDev           = ViDev[0];
    info->stViConfig.astViInfo[0].stDevInfo.enWDRMode       = WDR_MODE_NONE;
    info->stViConfig.astViInfo[0].stPipeInfo.enMastPipeMode = VI_OFFLINE_VPSS_OFFLINE;
    info->stViConfig.astViInfo[0].stPipeInfo.bIspBypass 	= HI_FALSE;
    info->stViConfig.astViInfo[0].stPipeInfo.aPipe[0]       = ViPipe[0];
    info->stViConfig.astViInfo[0].stPipeInfo.aPipe[1]       = -1;
    info->stViConfig.astViInfo[0].stPipeInfo.aPipe[2]       = -1;
    info->stViConfig.astViInfo[0].stPipeInfo.aPipe[3]       = -1;
    info->stViConfig.astViInfo[0].stChnInfo.ViChn           = 0;
    info->stViConfig.astViInfo[0].stChnInfo.enPixFormat     = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    info->stViConfig.astViInfo[0].stChnInfo.enDynamicRange  = DYNAMIC_RANGE_SDR8;
    info->stViConfig.astViInfo[0].stChnInfo.enVideoFormat   = VIDEO_FORMAT_LINEAR;
    info->stViConfig.astViInfo[0].stChnInfo.enCompressMode  = COMPRESS_MODE_NONE;

    info->stViConfig.astViInfo[1].stSnsInfo.MipiDev         = ViDev[1];
    info->stViConfig.astViInfo[1].stSnsInfo.s32BusId        = 1;
    info->stViConfig.astViInfo[1].stSnsInfo.s32SnsId        = 1;
    info->stViConfig.astViInfo[1].stDevInfo.ViDev           = ViDev[1];
    info->stViConfig.astViInfo[1].stSnsInfo.enSnsType		= SENSOR1_TYPE;
    info->stViConfig.astViInfo[1].stDevInfo.enWDRMode       = WDR_MODE_NONE;
    info->stViConfig.astViInfo[1].stPipeInfo.enMastPipeMode = VI_OFFLINE_VPSS_OFFLINE;
    info->stViConfig.astViInfo[1].stPipeInfo.bIspBypass 	= HI_FALSE;
    info->stViConfig.astViInfo[1].stPipeInfo.aPipe[0]       = ViPipe[2];
    info->stViConfig.astViInfo[1].stPipeInfo.aPipe[1]       = -1;
    info->stViConfig.astViInfo[1].stPipeInfo.aPipe[2]       = -1;
    info->stViConfig.astViInfo[1].stPipeInfo.aPipe[3]       = -1;
    info->stViConfig.astViInfo[1].stChnInfo.ViChn           = 0;
    info->stViConfig.astViInfo[1].stChnInfo.enPixFormat     = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    info->stViConfig.astViInfo[1].stChnInfo.enDynamicRange  = DYNAMIC_RANGE_SDR8;
    info->stViConfig.astViInfo[1].stChnInfo.enVideoFormat   = VIDEO_FORMAT_LINEAR;
    info->stViConfig.astViInfo[1].stChnInfo.enCompressMode  = COMPRESS_MODE_NONE;

    /*get picture size*/
    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(info->stViConfig.astViInfo[0].stSnsInfo.enSnsType, &enPicSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("get picture size by sensor failed!\n");
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &info->stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("get picture size failed!\n");
        return s32Ret;
    }

    info->stVbConf.u32MaxPoolCnt              = 2;
    u32BlkSize = COMMON_GetPicBufferSize(info->stSize.u32Width, info->stSize.u32Height, SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_8, COMPRESS_MODE_SEG, DEFAULT_ALIGN);
    info->stVbConf.astCommPool[0].u64BlkSize  = u32BlkSize;
    info->stVbConf.astCommPool[0].u32BlkCnt   = 8;

    u32BlkSize = VI_GetRawBufferSize(info->stSize.u32Width, info->stSize.u32Height, PIXEL_FORMAT_RGB_BAYER_16BPP, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
    info->stVbConf.astCommPool[1].u64BlkSize  = u32BlkSize;
    info->stVbConf.astCommPool[1].u32BlkCnt   = 8;
    
    s32Ret = SAMPLE_COMM_SYS_Init(&info->stVbConf);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("init sys fail for %#x!\n", s32Ret);
        goto error_exit;
    }

    /************************************************
    step4:  VPSS
    *************************************************/
    info->stVpssGrpAttr.u32MaxW = 1920;
    info->stVpssGrpAttr.u32MaxH = 1920;
    info->stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
    info->stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
    info->stVpssGrpAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
    info->stVpssGrpAttr.enPixelFormat  = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    info->stVpssGrpAttr.bNrEn   = HI_TRUE;

    info->abChnEnable[1] = HI_TRUE;
    info->astVpssChnAttr[1].u32Width                    = stSize.u32Width;;
    info->astVpssChnAttr[1].u32Height                   = stSize.u32Height;
    info->astVpssChnAttr[1].enChnMode                   = VPSS_CHN_MODE_USER;
    info->astVpssChnAttr[1].enCompressMode              = COMPRESS_MODE_NONE;
    info->astVpssChnAttr[1].enDynamicRange              = DYNAMIC_RANGE_SDR8;
    info->astVpssChnAttr[1].enPixelFormat               = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    info->astVpssChnAttr[1].stFrameRate.s32SrcFrameRate = -1;
    info->astVpssChnAttr[1].stFrameRate.s32DstFrameRate = -1;
    info->astVpssChnAttr[1].u32Depth                    = 0;
    info->astVpssChnAttr[1].bMirror                     = HI_FALSE;
    info->astVpssChnAttr[1].bFlip                       = HI_FALSE;
    info->astVpssChnAttr[1].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    info->astVpssChnAttr[1].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    
    info->abChnEnable[2] = HI_TRUE;
    info->astVpssChnAttr[2].u32Width                    = stSize.u32Width;
    info->astVpssChnAttr[2].u32Height                   = stSize.u32Height;
    info->astVpssChnAttr[2].enChnMode                   = VPSS_CHN_MODE_USER;
    info->astVpssChnAttr[2].enCompressMode              = COMPRESS_MODE_NONE;
    info->astVpssChnAttr[2].enDynamicRange              = DYNAMIC_RANGE_SDR8;
    info->astVpssChnAttr[2].enPixelFormat               = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    info->astVpssChnAttr[2].stFrameRate.s32SrcFrameRate = -1;
    info->astVpssChnAttr[2].stFrameRate.s32DstFrameRate = -1;
    info->astVpssChnAttr[2].u32Depth                    = 1;
    info->astVpssChnAttr[2].bMirror                     = HI_FALSE;
    info->astVpssChnAttr[2].bFlip                       = HI_FALSE;
    info->astVpssChnAttr[2].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    info->astVpssChnAttr[2].enVideoFormat               = VIDEO_FORMAT_LINEAR;    


    /************************************************
    step5:  VO
    *************************************************/
    SAMPLE_COMM_VO_GetDefConfig(&info->stVoConfig);
    info->stVoConfig.enDstDynamicRange = DYNAMIC_RANGE_SDR8;
	info->stVoConfig.enVoIntfType = VO_INTF_MIPI;
	info->stVoConfig.enPicSize = PIC_1080P;    
	info->stVoConfig.enVoMode = VO_MODE_2MUX;
    info->VoLayer = info->stVoConfig.VoDev;
    
    info->bInit = HI_TRUE;
    
    return HI_SUCCESS;	
    
error_exit:
	return HI_FAILURE;    
}

extern HI_S32 SAMPLE_COMM_VENC_SaveStream(FILE* pFd, VENC_STREAM_S* pstStream);

static HI_S32 SnapSave(HI_S32 ch, VENC_STREAM_S * pstStream)
{
	HI_S32 s32Ret;
    char acFile[FILE_NAME_LEN]    = {0};
    FILE* pFile;
    
	struct tm * tm;
	time_t now = time(0);
	
	tm = localtime(&now);
	
	snprintf(acFile, 128, "SNAP_CH%02d-%04d%02d%02d-%02d%02d%02d.jpg", ch - 2,
	         tm->tm_year + 1900,
	         tm->tm_mon + 1,
	         tm->tm_mday,
	         tm->tm_hour,
	         tm->tm_min,
	         tm->tm_sec);
	         
    pFile = fopen(acFile, "wb");
    if (pFile == NULL)
    {
        SAMPLE_PRT("open file err\n");

        return HI_FAILURE;
    }
    
    s32Ret = SAMPLE_COMM_VENC_SaveStream(pFile, pstStream);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("save snap picture failed!\n");
    }	
    
    fclose(pFile);
    
    return s32Ret;
}


/**
 * @brief           编码线程,此处处理编码数据并送mp4录像
 * @p               线程处理用户参数
 * @return          返回NULL 指针
 */
HI_VOID* MTT_VENC_GetVencStreamProc(HI_VOID* p)
{
    HI_S32 i;
    HI_S32 s32ChnTotal;
    VENC_CHN_ATTR_S stVencChnAttr;
    SAMPLE_VENC_GETSTREAM_PARA_S* pstPara;
    HI_S32 maxfd = 0;
    struct timeval TimeoutVal;
    fd_set read_fds;
    HI_U32 u32PictureCnt[VENC_MAX_CHN_NUM]={0};
    HI_S32 VencFd[VENC_MAX_CHN_NUM];
    VENC_CHN_STATUS_S stStat;
    VENC_STREAM_S stStream;
    HI_S32 s32Ret;
    VENC_CHN VencChn;
    VENC_STREAM_BUF_INFO_S stStreamBufInfo[VENC_MAX_CHN_NUM];

    prctl(PR_SET_NAME, "GetVencStream", 0,0,0);

    pstPara = (SAMPLE_VENC_GETSTREAM_PARA_S*)p;
    s32ChnTotal = pstPara->s32Cnt;
    /******************************************
     step 1:  check & prepare save-file & venc-fd
    ******************************************/
    if (s32ChnTotal >= VENC_MAX_CHN_NUM)
    {
        SAMPLE_PRT("input count invaild\n");
        return NULL;
    }
    for (i = 0; i < s32ChnTotal; i++)
    {
        /* decide the stream file name, and open file to save stream */
        VencChn = pstPara->VeChn[i];
        s32Ret = HI_MPI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VENC_GetChnAttr chn[%d] failed with %#x!\n", \
                       VencChn, s32Ret);
            return NULL;
        }
        
        /* Set Venc Fd. */
        VencFd[i] = HI_MPI_VENC_GetFd(i);
        if (VencFd[i] < 0)
        {
            SAMPLE_PRT("HI_MPI_VENC_GetFd failed with %#x!\n",
                       VencFd[i]);
            return NULL;
        }
        if (maxfd <= VencFd[i])
        {
            maxfd = VencFd[i];
        }

        s32Ret = HI_MPI_VENC_GetStreamBufInfo (i, &stStreamBufInfo[i]);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VENC_GetStreamBufInfo failed with %#x!\n", s32Ret);
            return (void *)HI_FAILURE;
        }
    }
    
    HI_PDT_Init();

    /******************************************
     step 2:  Start to get streams of each channel.
    ******************************************/
    while (HI_TRUE == pstPara->bThreadStart)
    {
        FD_ZERO(&read_fds);
        for (i = 0; i < s32ChnTotal; i++)
        {
            FD_SET(VencFd[i], &read_fds);
        }

        TimeoutVal.tv_sec  = 2;
        TimeoutVal.tv_usec = 0;
        s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            SAMPLE_PRT("select failed!\n");
            break;
        }
        else if (s32Ret == 0)
        {
            SAMPLE_PRT("get venc stream time out, exit thread\n");
            continue;
        }
        else
        {
            for (i = 0; i < s32ChnTotal; i++)
            {
                if (FD_ISSET(VencFd[i], &read_fds))
                {
                    /*******************************************************
                     step 2.1 : query how many packs in one-frame stream.
                    *******************************************************/
                    memset(&stStream, 0, sizeof(stStream));

                    s32Ret = HI_MPI_VENC_QueryStatus(i, &stStat);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("HI_MPI_VENC_QueryStatus chn[%d] failed with %#x!\n", i, s32Ret);
                        break;
                    }

                    /*******************************************************
                    step 2.2 :suggest to check both u32CurPacks and u32LeftStreamFrames at the same time,for example:
                     if(0 == stStat.u32CurPacks || 0 == stStat.u32LeftStreamFrames)
                     {
                        SAMPLE_PRT("NOTE: Current  frame is NULL!\n");
                        continue;
                     }
                    *******************************************************/
                    if(0 == stStat.u32CurPacks)
                    {
                          SAMPLE_PRT("NOTE: Current  frame is NULL!\n");
                          continue;
                    }
                    /*******************************************************
                     step 2.3 : malloc corresponding number of pack nodes.
                    *******************************************************/
                    stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                    if (NULL == stStream.pstPack)
                    {
                        SAMPLE_PRT("malloc stream pack failed!\n");
                        break;
                    }

                    /*******************************************************
                     step 2.4 : call mpi to get one-frame stream
                    *******************************************************/
                    stStream.u32PackCount = stStat.u32CurPacks;
                    s32Ret = HI_MPI_VENC_GetStream(i, &stStream, HI_TRUE);
                    if (HI_SUCCESS != s32Ret)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        SAMPLE_PRT("HI_MPI_VENC_GetStream failed with %#x!\n", \
                                   s32Ret);
                        break;
                    }
                    
                    if (i == 0 || i == 1)
                    {	
                    	//264保存成mp4
                        HI_PDT_WriteVideo(i,  &stStream);
                    }     
                    else if (i == 2 || i == 3)
                    {
                    	//jpeg存文件
                    	SnapSave(i, &stStream);
                    	sem_post(&sem_snap[i - 2]);
                    }	 

                    if (HI_SUCCESS != s32Ret)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        SAMPLE_PRT("save stream failed!\n");
                        break;
                    }
                    /*******************************************************
                     step 2.6 : release stream
                     *******************************************************/
                    s32Ret = HI_MPI_VENC_ReleaseStream(i, &stStream);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("HI_MPI_VENC_ReleaseStream failed!\n");
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        break;
                    }

                    /*******************************************************
                     step 2.7 : free pack nodes
                    *******************************************************/
                    free(stStream.pstPack);
                    stStream.pstPack = NULL;
                    u32PictureCnt[i]++;
                }
            }
        }
    }
    
    HI_PDT_Exit();

    return NULL;
}

/******************************************************************************
* funciton : start get venc stream process thread
******************************************************************************/
HI_S32 MTT_VENC_StartGetStream(VENC_CHN VeChn[],HI_S32 s32Cnt)
{
    HI_U32 i;
    media_info_t * info = &media_info;
    
    gs_stPara.bThreadStart = HI_TRUE;
    gs_stPara.s32Cnt = s32Cnt;
    for(i=0; i<s32Cnt; i++)
    {
        gs_stPara.VeChn[i] = VeChn[i];
    }
    return pthread_create(&info->stVencPid, 0, MTT_VENC_GetVencStreamProc, (HI_VOID*)&gs_stPara);
}

/******************************************************************************
* funciton : stop get venc stream process.
******************************************************************************/
HI_S32 MTT_VENC_StopGetStream(void)
{
	media_info_t * info = &media_info;
	
    if (HI_TRUE == gs_stPara.bThreadStart)
    {
        gs_stPara.bThreadStart = HI_FALSE;
        pthread_join(info->stVencPid, 0);
    }
    return HI_SUCCESS;
}

static HI_S32 SNAP_Init(void)
{
	sem_init(&sem_snap[0], 0, 0);
	sem_init(&sem_snap[1], 0, 0);
	
	return 0;
}

static HI_VOID SNAP_Exit(void)
{
    sem_destroy(&sem_snap[1]);
    sem_destroy(&sem_snap[0]);
}


/**
 * @brief           启动一次jpeg抓拍
 * @ch              要抓拍的通道(0 彩色, 1 红外)
 * @return          成功或失败
 */
HI_S32 MEDIA_Snap(int ch)
{
	HI_S32 s32Ret;
    VENC_RECV_PIC_PARAM_S  stRecvParam;
    
    if (ch < 0 || ch > 1)
    {
    	printf("invalid snap chn %d, need be 0 or 1\n", ch);
    	return -1;
    }

    /******************************************
     step 2:  Start Recv Venc Pictures
    ******************************************/
    stRecvParam.s32RecvPicNum = 1;
    s32Ret = HI_MPI_VENC_StartRecvFrame(2 + ch, &stRecvParam);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_StartRecvPic faild with%#x!\n", s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = sem_wait(&sem_snap[ch]);
    if (s32Ret == -1) 
    {
        if (errno == ETIMEDOUT)
            printf("sem_timedwait() timed out\n");
        else
            perror("sem_timedwait");
    }
    
    s32Ret = HI_MPI_VENC_StopRecvFrame(2 + ch);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VENC_StopRecvPic failed with %#x!\n",  s32Ret);
        return HI_FAILURE;
    }
        
	return 0;
}

/**
 * @brief           采集线程,可以把此处的帧数据送给算法处理
 * @p               线程处理用户参数
 * @return          返回NULL 指针
 */
static HI_VOID* MEDIA_VIN_GetFrameProc(HI_VOID* p)
{
	HI_S32 s32Cnt;
    HI_S32 s32Ret;
	VPSS_GRP VpssGrp[2] = {0, 1};
	VIDEO_FRAME_INFO_S  stColorFrame;
	VIDEO_FRAME_INFO_S  stGrayFrame;
	HI_S32 s32MilliSec = 20000;
	
	media_info_t * info;
		
	s32Cnt = 0;
	info = (media_info_t *)p;
    
    while (HI_TRUE == info->bVinThreadRun)
    {
        s32Ret = HI_MPI_VPSS_GetChnFrame(VpssGrp[0], 2, &stColorFrame, s32MilliSec);
		if (HI_SUCCESS != s32Ret)
		{
		    SAMPLE_PRT("vpss get frame failed, ret:0x%08x\n", s32Ret);
		    continue;
		}
		
        s32Ret = HI_MPI_VPSS_GetChnFrame(VpssGrp[1], 2, &stGrayFrame, s32MilliSec);
		if (HI_SUCCESS != s32Ret)
		{
		    SAMPLE_PRT("vpss get frame failed, ret:0x%08x\n", s32Ret);
		    break;
		}
       
	    s32Ret = HI_MPI_VPSS_ReleaseChnFrame(VpssGrp[0], 2, &stColorFrame);
		if ( HI_SUCCESS != s32Ret )
		{
		    SAMPLE_PRT("vpss release frame failed, ret:0x%08x\n", s32Ret);
		    break;
		}  
		
	    s32Ret = HI_MPI_VPSS_ReleaseChnFrame(VpssGrp[1], 2, &stGrayFrame);
		if ( HI_SUCCESS != s32Ret )
		{
		    SAMPLE_PRT("vpss release frame failed, ret:0x%08x\n", s32Ret);
		    break;
		}  
	
		s32Cnt++;     
    }
    
    return NULL;
}

ROTATION_E GetRotateSize(int s32Rotate)
{
	switch (s32Rotate)
	{
		case 90:
			return ROTATION_90;
			
		case 180:
			return ROTATION_180;
			
		case 270:
			return ROTATION_270;
			
		default:
		case 0:
			return ROTATION_0;	
			
	}
	
	return ROTATION_0;
}

/**
 * @brief           媒体处理模块启动
 * @s32Width        目标录像分辨率宽
 * @s32Height       目标录像分辨率高
 * @s32FrameRate    帧率,默认为25  
 * @s32Bitrate      码率(请自行根据分辨率选择合适码率,参考comm/sample_comm_venc.c相近分辨率)           
 * @return          返回NULL 指针
 */
int MEDIA_Start(int s32Width, int s32Height, int s32FrameRate, int s32BitRate, int s32Rotate)
{
	HI_S32 s32Ret = HI_SUCCESS;
	ROTATION_E enRotate;
	VI_CHN ViChn = 0;
    VPSS_GRP VpssGrp[2] = {0, 1};
    VENC_CHN VencChn[4] = {0, 1, 2, 3};
    VI_PIPE ViPipe[4] 	= {0, 1, 2, 3};
    SIZE_S          stSize;
    VENC_GOP_MODE_E enGopMode = VENC_GOPMODE_NORMALP;;
    VENC_GOP_ATTR_S stGopAttr;
    HI_S32          s32Gop = 25;
    SAMPLE_RC_E     enRcMode = SAMPLE_RC_CBR;
    HI_BOOL         bRcnRefShareBuf = HI_TRUE;
     
    media_info_t * info;
    
    enRotate = GetRotateSize(s32Rotate);
    
    info = &media_info;
    
    SNAP_Init();
    
    stSize.u32Width  = s32Width;
    stSize.u32Height = s32Height;
  
    s32Ret = MediaInfo_Init(info, stSize, s32Rotate);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("MEDIA info init for %#x!\n", s32Ret);
        return HI_FAILURE;
    }	
    
    /*start vi*/
    s32Ret = SAMPLE_COMM_VI_StartVi(&info->stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed.s32Ret:0x%x !\n", s32Ret);
        goto EXIT;
    }    

    s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp[0], info->abChnEnable, &info->stVpssGrpAttr, info->astVpssChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
        goto EXIT1;
    }

    s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp[1], info->abChnEnable, &info->stVpssGrpAttr, info->astVpssChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
        goto EXIT2;
    }

    /*vi bind vpss*/
    s32Ret = SAMPLE_COMM_VI_Bind_VPSS(ViPipe[0], ViChn, VpssGrp[0]);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
        goto EXIT3;
    }
	
    s32Ret = SAMPLE_COMM_VI_Bind_VPSS(ViPipe[2], ViChn, VpssGrp[1]);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
        goto EXIT4;
    }	
  
    s32Ret = SAMPLE_COMM_VENC_GetGopAttr(enGopMode,&stGopAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Venc Get GopAttr for %#x!\n", s32Ret);
        goto EXIT5;
    }

    /***encode h.264 **/
    s32Ret = SAMPLE_COMM_VENC_Start(VencChn[0], PT_H264, &stSize, enRcMode, s32FrameRate, s32BitRate, s32Gop, 0, bRcnRefShareBuf, &stGopAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Venc Start failed for %#x!\n", s32Ret);
        goto EXIT5;
    }

    s32Ret = SAMPLE_COMM_VENC_Start(VencChn[1], PT_H264, &stSize, enRcMode, s32FrameRate, s32BitRate, s32Gop, 0, bRcnRefShareBuf, &stGopAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Venc Start failed for %#x!\n", s32Ret);
        goto EXIT6;
    }
    
    s32Ret = SAMPLE_COMM_VENC_SnapStart(VencChn[2], &stSize, 0);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Venc Start failed for %#x!\n", s32Ret);
        goto EXIT7;
    }

    s32Ret = SAMPLE_COMM_VENC_SnapStart(VencChn[3], &stSize, 0);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Venc Start failed for %#x!\n", s32Ret);
        goto EXIT8;
    } 

    s32Ret = SAMPLE_COMM_VPSS_Bind_VENC(VpssGrp[0], VPSS_CHN2, VencChn[0]);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Venc bind Vpss failed for %#x!\n", s32Ret);
        goto EXIT9;
    }    
    
    s32Ret = SAMPLE_COMM_VPSS_Bind_VENC(VpssGrp[1], VPSS_CHN2, VencChn[1]);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Venc bind Vpss failed for %#x!\n", s32Ret);
        goto EXIT10;
    }  
    
    s32Ret = SAMPLE_COMM_VPSS_Bind_VENC(VpssGrp[0], VPSS_CHN2, VencChn[2]);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Venc bind Vpss failed for %#x!\n", s32Ret);
        goto EXIT11;
    }    
    
    s32Ret = SAMPLE_COMM_VPSS_Bind_VENC(VpssGrp[1], VPSS_CHN2, VencChn[3]);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Venc bind Vpss failed for %#x!\n", s32Ret);
        goto EXIT12;
    }   
    
    if (enRotate)
    {
        s32Ret = HI_MPI_VI_SetChnRotation(ViPipe[0], ViChn, enRotate);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VI_SetChnRotation failed with %d\n", s32Ret);
            goto EXIT13;
        }
        
        s32Ret = HI_MPI_VI_SetChnRotation(ViPipe[2], ViChn, enRotate);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VI_SetChnRotation failed with %d\n", s32Ret);
            goto EXIT13;
        } 
    }       
    
    s32Ret = MTT_VENC_StartGetStream(VencChn, 4);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto EXIT13;
    }        
    
    //用于把红外相机设成黑白的
    //Media_SetIRSaturation();  
    
    info->bVinThreadRun = HI_TRUE;
	pthread_create(&info->stVinPid, 0, MEDIA_VIN_GetFrameProc, (HI_VOID*)info);     
   
    return 0;

EXIT13:
    SAMPLE_COMM_VPSS_UnBind_VENC(VpssGrp[1], VPSS_CHN2, VencChn[3]);
EXIT12:
    SAMPLE_COMM_VPSS_UnBind_VENC(VpssGrp[0], VPSS_CHN2, VencChn[2]);        
EXIT11:
    SAMPLE_COMM_VPSS_UnBind_VENC(VpssGrp[1], VPSS_CHN2, VencChn[1]);
EXIT10:
    SAMPLE_COMM_VPSS_UnBind_VENC(VpssGrp[0], VPSS_CHN2, VencChn[0]);
EXIT9:
    SAMPLE_COMM_VENC_SnapStop(VencChn[3]);
EXIT8:
    SAMPLE_COMM_VENC_SnapStop(VencChn[2]);
EXIT7:
    SAMPLE_COMM_VENC_Stop(VencChn[1]);
EXIT6:
    SAMPLE_COMM_VENC_Stop(VencChn[0]);  
EXIT5:
    SAMPLE_COMM_VI_UnBind_VPSS(ViPipe[2], ViChn, VpssGrp[1]);
EXIT4:
    SAMPLE_COMM_VI_UnBind_VPSS(ViPipe[0], ViChn, VpssGrp[0]);      
EXIT3:
    SAMPLE_COMM_VPSS_Stop(VpssGrp[1], info->abChnEnable);
EXIT2:
    SAMPLE_COMM_VPSS_Stop(VpssGrp[0], info->abChnEnable);
EXIT1:
    SAMPLE_COMM_VI_StopVi(&info->stViConfig);
    
EXIT:
    SAMPLE_COMM_SYS_Exit();
    
    info->bInit = HI_FALSE;

    return s32Ret;
}

/**
 * @brief           媒体处理模块停止
 * @return          无
 */
HI_VOID MEDIA_Stop(HI_VOID)
{
	VI_CHN ViChn = 0;
    VPSS_GRP VpssGrp[2] = {0, 1};
    VENC_CHN VencChn[4] = {0, 1, 2, 3};
    VI_PIPE ViPipe[4] 	= {0, 1, 2, 3};
    media_info_t * info;
    
    info = &media_info;	
    
    if (HI_TRUE == info->bVinThreadRun)
    {
        info->bVinThreadRun = HI_FALSE;
        pthread_join(info->stVinPid, 0);
    }    
 
    MTT_VENC_StopGetStream();
    
    SAMPLE_COMM_VPSS_UnBind_VENC(VpssGrp[1], VPSS_CHN2, VencChn[3]);
    SAMPLE_COMM_VPSS_UnBind_VENC(VpssGrp[0], VPSS_CHN2, VencChn[2]);        
    SAMPLE_COMM_VPSS_UnBind_VENC(VpssGrp[1], VPSS_CHN2, VencChn[1]);
    SAMPLE_COMM_VPSS_UnBind_VENC(VpssGrp[0], VPSS_CHN2, VencChn[0]);
    SAMPLE_COMM_VENC_SnapStop(VencChn[3]);
    SAMPLE_COMM_VENC_SnapStop(VencChn[2]);
    SAMPLE_COMM_VENC_Stop(VencChn[1]);
    SAMPLE_COMM_VENC_Stop(VencChn[0]);  
    SAMPLE_COMM_VI_UnBind_VPSS(ViPipe[2], ViChn, VpssGrp[1]);
    SAMPLE_COMM_VI_UnBind_VPSS(ViPipe[0], ViChn, VpssGrp[0]);      
    SAMPLE_COMM_VPSS_Stop(VpssGrp[1], info->abChnEnable);
    SAMPLE_COMM_VPSS_Stop(VpssGrp[0], info->abChnEnable);
    SAMPLE_COMM_VI_StopVi(&info->stViConfig);
    SAMPLE_COMM_SYS_Exit();  
    
    SNAP_Exit();
    
    info->bInit = HI_FALSE;
}
