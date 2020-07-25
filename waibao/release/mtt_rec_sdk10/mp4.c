#include <stdio.h>
#include <time.h>
#include <libavformat/avformat.h>

#include "sample_comm.h"
#include "mp4.h"

#define USING_PTS
#define STREAM_FRAME_RATE 25

typedef struct
{
	AVFormatContext* g_OutFmt_Ctx;	//ÿ��ͨ����AVFormatContext
	int vi;	                        //��Ƶ��������
	int ai;	                        //��Ƶ��������
	HI_BOOL b_First_IDR_Find;	        //��һ֡��I֡��־
	long int VptsInc;	                //������Ƶ֡��������
	long int AptsInc;	                //��Ƶ֡����
	HI_U64 Audio_PTS;	                //��ƵPTS
	HI_U64 Video_PTS;	                //��ƵPTS
	HI_U64 Afirst;	                //���ļ���һ֡��Ƶ��־
	HI_U64 Vfirst;	                //��Ƶ��һ֡��־
	long int moov_pos;	            //moov��pos��δʹ��
	int moov_flags;	                //moovǰ�ñ�־��δʹ��
	int file_flags;	
	char filename[128];	            //�ļ���
} ffmpegCtx_t;

ffmpegCtx_t ffmpegCtx[MTT_VENC_MAX_CHN_NUM];


static ffmpegCtx_t * GetVencChnCtx(int VeChn)
{
	return (ffmpegCtx_t * )&ffmpegCtx[VeChn];
}

static int HI_PDT_Add_Stream(VENC_CHN VeChn)
{
    AVOutputFormat *pOutFmt = NULL;	//���ڻ�ȡAVFormatContext->Format
    AVCodecParameters *vAVCodecPar=NULL;	//���������AVStream->CodecPar

    AVStream *vAVStream = NULL;	//����ָ���½�����Ƶ��
	AVCodec *vcodec = NULL;	    //����ָ����Ƶ������

	ffmpegCtx_t * fc = GetVencChnCtx(VeChn);
	
	pOutFmt = fc->g_OutFmt_Ctx->oformat;	//���Format
	
    vcodec = avcodec_find_encoder(pOutFmt->video_codec);	//������Ƶ��������Ĭ�Ͼ���H264��
    if (NULL == vcodec)
    {
        printf("Muxing:could not find video encoder H264\n");
        return -1;
    }
 	
	//������Ƶ��������Ϣ��H264������AVFormatContext���½���Ƶ��ͨ��
    vAVStream = avformat_new_stream(fc->g_OutFmt_Ctx, vcodec);	
    if (NULL == vAVStream)
    {
       printf("Muxing:could not allocate vcodec stream \n");
       return -1;
    }
   
    //���½�����Ƶ��һ��ID��0
    vAVStream->id = fc->g_OutFmt_Ctx->nb_streams - 1;	//nb_streams�ǵ�ǰAVFormatContext������������
    
	fc->vi = vAVStream->index;	//��ȡ��Ƶ����������
	
    vAVCodecPar = vAVStream->codecpar;	//
	if(vcodec->type == AVMEDIA_TYPE_VIDEO)	//����������Ƶ������
    {
    	//����Ƶ���Ĳ�������
		vAVCodecPar->codec_type = AVMEDIA_TYPE_VIDEO;	
        vAVCodecPar->codec_id = AV_CODEC_ID_H264;
        vAVCodecPar->bit_rate = 2000;	//kbps��������Ҫ
        vAVCodecPar->width = 1280;	//����
        vAVCodecPar->height = 720;
        vAVStream->time_base = (AVRational){1, STREAM_FRAME_RATE};	//ʱ���
        vAVCodecPar->format = AV_PIX_FMT_YUV420P;
    }

    return HI_SUCCESS;
}

static void generate_file_name(VENC_CHN VeChn)
{
	struct tm * tm;
	time_t now = time(0);
	ffmpegCtx_t * fc = GetVencChnCtx(VeChn);
	
	tm = localtime(&now);
	
	snprintf(fc->filename, 128, "ch%02d-%04d%02d%02d-%02d%02d%02d.mp4", VeChn,
	         tm->tm_year + 1900,
	         tm->tm_mon + 1,
	         tm->tm_mday,
	         tm->tm_hour,
	         tm->tm_min,
	         tm->tm_sec);
	
}


//MP4������������ʼ����д�ļ�ͷ��
int HI_PDT_CreateMp4(VENC_CHN VeChn)
{
    int ret = 0; 
    ffmpegCtx_t * fc = GetVencChnCtx(VeChn);
    
    AVOutputFormat *pOutFmt = NULL;	//���Formatָ��
    #ifdef FFMPEG_MUXING	
    AVCodec *audio_codec;
    #endif
    
	generate_file_name(VeChn);

    avformat_alloc_output_context2(&(fc->g_OutFmt_Ctx), NULL, NULL, fc->filename);//��ʼ�������Ƶ������AVFormatContext��
	if (NULL == fc->g_OutFmt_Ctx)	//ʧ�ܴ���
    {	
        printf("Muxing:Could not deduce output format from file extension: using mp4. \n");//�����־
        avformat_alloc_output_context2(&(fc->g_OutFmt_Ctx), NULL, "mp4", fc->filename);
		if (NULL == fc->g_OutFmt_Ctx)
    	{
    		printf("Muxing:avformat_alloc_output_context2 failed\n");
        	return -1;
    	}
    }

    pOutFmt = fc->g_OutFmt_Ctx->oformat;		//��ȡ���Formatָ��
    
    if (pOutFmt->video_codec == AV_CODEC_ID_NONE)	//�����Ƶ������
    {
        printf("Muxing:add_video_stream ID failed\n"); 
		goto exit_outFmt_failed;
	}
	
	if (pOutFmt->audio_codec == AV_CODEC_ID_NONE)	//�����Ƶ������
    {
        printf("Muxing:add_audio_stream ID failed\n"); 
		goto exit_outFmt_failed;
	}
	
    if (!(pOutFmt->flags & AVFMT_NOFILE))	//Ӧ�����ж��ļ�IO�Ƿ��
    {
        ret = avio_open(&(fc->g_OutFmt_Ctx->pb), fc->filename, AVIO_FLAG_WRITE);	//��������mp4�ļ�
        if (ret < 0)
        {
        	printf("chan:%d filename:%s\n",VeChn, fc->filename);
            printf("Muxing:could not create video file\n");
            goto exit_avio_open_failed;
        }
    }
    
	//��ʼ��һЩ����
	fc->Video_PTS = 0;	
	fc->Audio_PTS = 0;
	fc->Vfirst = 0;
	fc->Afirst = 0;
	fc->vi = -1;
	fc->ai = -1;
	fc->b_First_IDR_Find = 0;
	return HI_SUCCESS;
	
//������
exit_avio_open_failed:	
	if (fc->g_OutFmt_Ctx && !(fc->g_OutFmt_Ctx->flags & AVFMT_NOFILE))
		avio_close(fc->g_OutFmt_Ctx->pb);
		
exit_outFmt_failed:
	if(NULL != fc->g_OutFmt_Ctx)
		avformat_free_context(fc->g_OutFmt_Ctx);
	return -1;
}

void HI_PDT_CloseMp4(VENC_CHN VeChn)
{
	int ret;
	ffmpegCtx_t * fc = GetVencChnCtx(VeChn);
	
    if (fc->g_OutFmt_Ctx)
    {
       ret = av_write_trailer(fc->g_OutFmt_Ctx);	//д�ļ�β
       if(ret < 0)
       {
       		printf("av_write_trailer faild\n");
       }
    }
    if (fc->g_OutFmt_Ctx && !(fc->g_OutFmt_Ctx->oformat->flags & AVFMT_NOFILE)) //�ļ�״̬���
	{
		ret = avio_close(fc->g_OutFmt_Ctx->pb);	//�ر��ļ�
		if(ret < 0)
        {
       		printf("avio_close faild\n");
        }
	}
	if (fc->g_OutFmt_Ctx)
    {
        avformat_free_context(fc->g_OutFmt_Ctx);	//�ͷŽṹ��
        fc->g_OutFmt_Ctx = NULL;
    }
    //�����ر�־
	fc->vi = -1;
	fc->ai = -1;		
	fc->VptsInc=0;
	fc->AptsInc=0;
	fc->Afirst=0;
	fc->Vfirst=0;
	fc->b_First_IDR_Find = 0;
}

static HI_S32 HI_ADD_SPS_PPS(VENC_CHN VeChn, uint8_t *buf, uint32_t size)
{
	HI_S32 ret;
	ffmpegCtx_t * fc = GetVencChnCtx(VeChn);
	
	ret = HI_PDT_Add_Stream(VeChn);	//����һ����������ӵ���ǰAVFormatContext��
	if(ret < 0)
	{
		printf("Muxing:HI_PDT_Add_Stream faild\n");
		goto Add_Stream_faild;
	}
	
	fc->g_OutFmt_Ctx->streams[fc->vi]->codecpar->extradata_size = size;
	fc->g_OutFmt_Ctx->streams[fc->vi]->codecpar->extradata = (uint8_t*)av_malloc(size + AV_INPUT_BUFFER_PADDING_SIZE);
	memcpy(fc->g_OutFmt_Ctx->streams[fc->vi]->codecpar->extradata, buf, size);	//д��SPS��PPS

	ret = avformat_write_header(fc->g_OutFmt_Ctx, NULL);		//д�ļ�ͷ
	
	if(ret < 0)
	{
		printf("Muxing:avformat_write_header faild\n");
		goto write_header_faild;
	}	
    return HI_SUCCESS;
    
write_header_faild:
	if (fc->g_OutFmt_Ctx && !(fc->g_OutFmt_Ctx->flags & AVFMT_NOFILE))
		avio_close(fc->g_OutFmt_Ctx->pb);
	
Add_Stream_faild:
    if(NULL != fc->g_OutFmt_Ctx)
		avformat_free_context(fc->g_OutFmt_Ctx);
	fc->vi = -1;
	fc->ai = -1;		//AeChn
	fc->VptsInc=0;
	fc->AptsInc=0;
	fc->b_First_IDR_Find = 0;	//sps��pps֡��־���
	return HI_FAILURE;
}


HI_S32 HI_PDT_WriteVideo(VENC_CHN VeChn, VENC_STREAM_S *pstStream)
{
	HI_U32 i=0;	//
	HI_U8* pPackVirtAddr = NULL;	//�����׵�ַ
	HI_U32 u32PackLen = 0;	//��������
    int ret = 0;
    AVStream *Vpst = NULL; //��Ƶ��ָ��
    AVPacket pkt;	//����Ƶ���ṹ�壬��������Ǻ�˼�İ������֮����������д������
    uint8_t sps_buf[32];	//
    uint8_t pps_buf[32];
    uint8_t sps_pps_buf[64];	
    HI_U32 pps_len=0;
    HI_U32 sps_len=0;
    ffmpegCtx_t * fc = GetVencChnCtx(VeChn);
    
	if(NULL == pstStream)	//��������Ч�ж�
	{
		return HI_SUCCESS;
	}
	//u32PackCount�Ǻ�˼�м�¼�������ṹ����������������һ�㺬I֡����4������P֡1��
    for (i = 0 ; i < pstStream->u32PackCount; i++)	
    {
    	//�Ӻ�˼�������л�ȡ���ݵ�ַ������
        pPackVirtAddr = pstStream->pstPack[i].pu8Addr + pstStream->pstPack[i].u32Offset;	
        u32PackLen = pstStream->pstPack[i].u32Len -pstStream->pstPack[i].u32Offset;	
		av_init_packet(&pkt);	//��ʼ��AVpack����
		pkt.flags=AV_PKT_FLAG_KEY;	//Ĭ���ǹؼ�֡���ز��ؼ�����û����
		switch(pstStream->pstPack[i].DataType.enH264EType)
		{
			case H264E_NALU_SPS:	//����������SPS
				pkt.flags =   0;	//���ǹؼ�֡
				if(fc->b_First_IDR_Find == 2)	//������ǵ�һ��SPS֡
				{
					continue;	//����������
					//��ֻҪ�½��ļ�֮��ĵ�һ��SPS PPS��Ϣ�����涼��һ���ģ�ֻҪ��һ������
				}
				else //����ǵ�һ��SPS֡
				{
					sps_len = u32PackLen;
					memcpy(sps_buf, pPackVirtAddr, sps_len);
					if(fc->b_First_IDR_Find == 1)	//���PPS֡�Ѿ��յ�
					{
						memcpy(sps_pps_buf, sps_buf, sps_len);	//����sps
						memcpy(sps_pps_buf+sps_len, pps_buf, pps_len);	//����pps
						//ȥ�����Ƶ������SPS PPS��Ϣ���ⲽ֮��ſ�ʼд����Ƶ֡
						ret = HI_ADD_SPS_PPS(VeChn, sps_pps_buf, sps_len+pps_len);
						if(ret < 0)
							return HI_FAILURE;
					}
					fc->b_First_IDR_Find++;
				}
				continue; //����
				//break; 
			case H264E_NALU_PPS:
				pkt.flags = 0;	//���ǹؼ�֡
				if(fc->b_First_IDR_Find == 2)	//������ǵ�һ��PPS֡
				{
					continue;
				}
				else //�ǵ�һ��PPS֡
				{
					pps_len = u32PackLen;
					memcpy(pps_buf, pPackVirtAddr, pps_len);	//����
					if(fc->b_First_IDR_Find ==1)	//���SPS֡�Ѿ��յ�
					{
						memcpy(sps_pps_buf, sps_buf, sps_len);
						memcpy(sps_pps_buf + sps_len, pps_buf, pps_len);
						//�����SPS���ﻥ�⣬ֻ��һ����ִ�У���Ҫ�ǿ�SPS��PPS��˭���ں���
						ret = HI_ADD_SPS_PPS(VeChn, sps_pps_buf, sps_len+pps_len); 
						if(ret < 0)
							return HI_FAILURE;
					}
					fc->b_First_IDR_Find++;
				}
				continue;
			case H264E_NALU_SEI:	//��ǿ֡
				continue;	//��ϡ�����֡
			case H264E_NALU_PSLICE:		//P֡
			case H264E_NALU_IDRSLICE:	//I֡
				if(fc->b_First_IDR_Find !=2)	//�������ļ���û���յ���sps��pps֡
				{
					continue;	//��������������֡
				}
				break;
			default:
				break;
		}
		
		if(fc->vi < 0)	//�������ţ����g_OutFmt_Ctx���滹û���½���Ƶ����Ҳ����˵��û�յ�I֡
		{
			printf("vi less than 0 \n");
			return HI_SUCCESS;
		}

		if(fc->Vfirst==0)	//������ļ��ĵ�һ֡��Ƶ
		{
			fc->Vfirst=1;	
			#ifdef USING_SEQ	//ʹ��֡��ż���PTS
			fc->Video_PTS = pstStream->u32Seq; //��¼��ʼ���
			#endif
			#ifdef USING_PTS	//ֱ��ʹ�ú�˼��PTS
			fc->Video_PTS = pstStream->pstPack[i].u64PTS;	//��¼��ʼʱ���
			#endif
		}
		
		Vpst = fc->g_OutFmt_Ctx->streams[fc->vi];	//���������Ż�ȡ��Ƶ����ַ
	    pkt.stream_index = Vpst->index;	//��Ƶ���������� ���� ��������������ţ���ʾ�����������Ƶ��
		
		//���£�ʱ���ת����PTS����Ҫ���漰����Ƶͬ������
		#if 0	//ԭ�����ģ�������
			pkt.pts = av_rescale_q_rnd((fc->VptsInc++), Vpst->codec->time_base,Vpst->time_base,(enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
			pkt.dts = av_rescale_q_rnd(pkt.pts, Vpst->time_base,Vpst->time_base,(enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		#endif
		#if 1	//���õ�
			#ifdef USING_SEQ	
				//��ԭ������࣬�����м䶪֡�����²�ͬ�������������������
				pkt.pts = av_rescale_q_rnd(pstStream->u32Seq - fc->Video_PTS, (AVRational){1, STREAM_FRAME_RATE},Vpst->time_base,(enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
				pkt.dts = pkt.pts;	//ֻ��I��P֡����Ⱦ�����
			#endif
			#ifdef USING_PTS	
				//��˼��PTS��us��λ�����Խ���ʵ�����1000000usת��90000HzƵ�ʵ�ʱ��
				pkt.pts = pkt.dts =(int64_t)((pstStream->pstPack[i].u64PTS - fc->Video_PTS) *0.09+0.5);
			#endif
		#endif
		//һ��25֡��һ֡40ms������д0Ҳ�У�ffmpeg�ڲ������ˣ�
		//����˵���½�����ʱ�򣬸���codepar֡�ʲ�����ffmpeg�ǿ��Լ����
		pkt.duration = 40;	
		pkt.duration = av_rescale_q(pkt.duration, Vpst->time_base, Vpst->time_base);
		pkt.pos = -1;	//Ĭ��
		
		//����Ҫ������Ҫ��AVpack��
		pkt.data = pPackVirtAddr ;	//������Ƶ����NAUL
   		pkt.size = u32PackLen;	//��Ƶ���ݳ���
		//��AVpack��д��mp4
		ret = av_interleaved_write_frame(fc->g_OutFmt_Ctx, &pkt);
		if (ret < 0)
		{
		    printf("Muxing:cannot write video frame\n");
		    return HI_FAILURE;
		}
    }
	return HI_SUCCESS;
}

int HI_PDT_Init(void)
{
	int i;
	
	for (i = 0; i < MTT_VENC_MAX_CHN_NUM; i++)
	{
		memset(&ffmpegCtx[i], 0, sizeof(ffmpegCtx_t));
		HI_PDT_CreateMp4(i);
	}
	
	return 0;
}

int HI_PDT_Exit(void)
{
	int i;
	
	for (i = 0; i < MTT_VENC_MAX_CHN_NUM; i++)
	{
		HI_PDT_CloseMp4(i);
	}
		
	return 0;
}
