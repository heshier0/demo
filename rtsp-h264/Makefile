INCLUDE=-I./include -I/mnt/g/workspace/plugins/hisi/include
LIBS=-L/mnt/g/workspace/plugins/hisi/lib -lmpi -l_hiae -lisp -l_hiawb -lhi_cipher -lVoiceEngine \
     -lupvqe -ldnvqe -live -lmd -lsecurec -l_hidrc -l_hildci -l_hidehaze -lhdmi -lsns_imx327 -lsns_imx327_2l \
     -lpthread -lm -ldl

all:rtsp-h264

rtsp-h264:
	arm-himix200-linux-gcc -o rtsp-h264 -Dhi3516dv300 -DSENSOR0_TYPE=SONY_IMX327_2L_MIPI_2M_30FPS_12BIT -DSENSOR1_TYPE=SONY_IMX327_2L_MIPI_2M_30FPS_12BIT -DHI_RELEASE -DHI_XXXX -DISP_V2 -DHI_ACODEC_TYPE_INNER -mcpu=arm926ej-s -mno-unaligned-access -fno-aggressive-loop-optimizations -ffunction-sections -fdata-sections main.c ringfifo.c rtputils.c rtspservice.c rtsputils.c loadbmp.c sample_comm_audio.c sample_comm_isp.c sample_comm_sys.c  sample_comm_venc.c sample_comm_vi.c sample_comm_vo.c sample_comm_vpss.c sample_venc.c $(INCLUDE) $(LIBS)
	
clean:
	rm -rfv rtsp-h264
