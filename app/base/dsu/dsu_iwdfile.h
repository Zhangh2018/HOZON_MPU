/**
 * @Title: dsu_iwdfile.h
 * @author yuzhimin
 * @date Nov 21, 2017
 * @version V1.0
 */
#ifndef __DSU_IWDFILE_H__
#define __DSU_IWDFILE_H__

#include "dsu_comm.h"

#define IWD_FAULT_CH_MAX                256
#define IWD_LOGCAN_UNDEFINE             ((short)0xFEFE)

enum
{
    IWD_CONTINUE_REC = 0,
    IWD_REWIND_TMP_FILE,
    IWD_CLOSE_IWD_FILE,
};

/* iwd status structure */
typedef struct
{
    unsigned char   enable;
    /*
    stat: 0: ֹͣ��¼(tmp��iwd), ��������tmp,
          1: �ƶ�tmpдָ�뵽iwdͷ��β��,
          2: ��¼��iwd
    */
    unsigned char   stat;
    RTCTIME         cantag;
    short           countdown;      // ��¼�ĵ���ʱʱ��S
    short           logtime;        // �����ļ� ����ǰ��ʱ�䵥λS
    int             fault_flag;     // ���dbc��E01,E03�ı��λ
    long            offset;         // iwdͷ��Ϣ��β���ļ�ͷ��ƫ��
    pthread_mutex_t mutex;          // mutex for stat;
} IWD_ATTR;

typedef struct 
{
    unsigned int    uptime;
    unsigned int    miscUptime;
    unsigned char   type;
    unsigned short  dlc          :4;
    unsigned short  port         :4;
    unsigned short  canFD        :1;
    unsigned short  BRS          :1;
    unsigned short  ESI          :1;
    unsigned char   canFDchannel :1;  
} __attribute__((packed)) IWD_MSG;



typedef struct _iwd_fault_info_t
{
    int siglist[IWD_FAULT_CH_MAX];
    int cnt;
    struct _iwd_fault_info_t *next;
} IWD_FAULT_INFO_T;



int iwd_file_init(void);
int iwd_file_append(unsigned char *data, unsigned int len);
void iwd_attr_init(DSU_CFG_T *cfg);
void iwd_fault_ch_happen(int id, unsigned int uptime);
void iwd_fault_list_setting(IWD_FAULT_INFO_T *faultinf);
int iwd_fault_dbc_surfix(IWD_FAULT_INFO_T *faultinf, int sigid, const char *sfx);

#endif /*__IWDFILE_H__ */
