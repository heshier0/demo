#ifndef INC_MEDIA_H
#define INC_MEDIA_H

/**
 * @brief           ý�崦��ģ������
 * @s32Width        Ŀ��¼��ֱ��ʿ�
 * @s32Height       Ŀ��¼��ֱ��ʸ�
 * @s32FrameRate    ֡��,Ĭ��Ϊ25  
 * @s32Bitrate      ����(�����и��ݷֱ���ѡ���������,�ο�comm/sample_comm_venc.c����ֱ���) 
 * @s32Rotate       ��ת�Ƕ�(0, 90, 180, 270);          
 * @return          ����NULL ָ��
 */
int MEDIA_Start(int s32Width, int s32Height, int s32FrameRate, int s32Bitrate, int s32Rotate);

/**
 * @brief           ý�崦��ģ��ֹͣ
 * @return          ��
 */
void MEDIA_Stop(void);

/**
 * @brief           ����һ��jpegץ��
 * @ch              Ҫץ�ĵ�ͨ��(0 ��ɫ, 1 ����)
 * @return          �ɹ���ʧ��
 */
int MEDIA_Snap(int ch);

#endif
