#ifndef __REMOTE_DIAG_H__
#define __REMOTE_DIAG_H__
#include "mid_def.h"
#include "remote_diag_api.h"
#include "tcom_api.h"


#define REMOTE_DIAG_MSG_LEN 1024
#define DIAG_REQUEST_LEN 200
#define DIAG_REQUEST_SINGEL_MSG_LEN 500 /*����Զ�������Ϣ����*/
#define DIAG_REQUEST_MSG_LEN 50000        /*�ܵ�Զ�������Ϣ����*/

#define DIAG_RESPONSE_LEN 1000

#define MAX_REMOTE_REQUEST 31

#define REMOTE_DIAG_MSG_TIMEOUT_INTERVAL 3000

#define NVTSP_TIMEOUT_INTERVAL 4500 /*ms*/


#define DEFAULT_SESSION_CMD "1001" /* �л�Ĭ�ϻỰ���� */


#define EXTEND_SESSION_CMD "1003" /* �л���չ�Ự���� */
#define SECURITY_LEVEL1_REQUEST_SEED "2701"  /* ��ȫ���ʵȼ�1�������� */
#define SECURITY_LEVEL2_REQUEST_SEED "2703"  /* ��ȫ���ʵȼ�2�������� */
#define SECURITY_LEVEL3_REQUEST_SEED "2709"  /* ��ȫ���ʵȼ�3�������� */

#define DIAG_REQUEST_ID_ROW   0
#define DIAG_RESPONSE_ID_ROW  1





typedef enum
{
	REMOTE_DIAG_ALL = 0,//
	REMOTE_DIAG_VCU,//
    REMOTE_DIAG_BMS,//
	REMOTE_DIAG_MCUp,
	REMOTE_DIAG_OBCp,
	REMOTE_DIAG_FLR,
	REMOTE_DIAG_FLC,
	REMOTE_DIAG_APA,
	REMOTE_DIAG_ESCPluse,
	REMOTE_DIAG_EPS,
	REMOTE_DIAG_EHB,
	REMOTE_DIAG_BDCM,
	REMOTE_DIAG_GW,
	REMOTE_DIAG_LSA,
	REMOTE_DIAG_CLM,
	REMOTE_DIAG_PTC,
	REMOTE_DIAG_EACP,
	REMOTE_DIAG_EGSM,
	REMOTE_DIAG_ALM,
	REMOTE_DIAG_WPC,
	REMOTE_DIAG_IHU,
	REMOTE_DIAG_ICU,
	REMOTE_DIAG_IRS,
	REMOTE_DIAG_DVR,
	REMOTE_DIAG_TAP,
	REMOTE_DIAG_MFCP,
	REMOTE_DIAG_TBOX,
	REMOTE_DIAG_ACU,
	REMOTE_DIAG_PLG,
	REMOTE_DIAG_ECU_NUM,
} REMOTE_DIAG_ECUTYPE;






/* ����Զ���������ִ��״̬ */
typedef enum
{
    REMOTE_DIAG_CMD_NOT_START = 0,//δ��ʼ
    REMOTE_DIAG_CMD_OPENING,/* �ѷ�����������ȴ���Ӧ*/
    REMOTE_DIAG_CMD_OPENED,/* ���óɹ�����δ����UDS�������*/
    REMOTE_DIAG_CMD_SESSION_CONTROLLING,/* �Ự���� �л��Ự��*/
    REMOTE_DIAG_CMD_SESSION_CONTROLLED, /* �Ự���������� */
    REMOTE_DIAG_CMD_SECURITY_SEED_REQUESTING,/* ��ȫ��������������*/
    REMOTE_DIAG_CMD_SECURITY_SEED_REQUESTED,/* ��ȫ�������������ӣ������յ�����*/
    REMEOT_DIAG_CMD_SECURITY_KEY_SENDING,/* ��ȫ���ʣ�����key��*/
    REMOTE_DIAG_CMD_SECURITY_KEY_SENDED,/* ��ȫ�����ѽ��� */
    REMOTE_DIAG_CMD_DIAGNOSING,/* �ѷ���������󣬵ȴ������Ӧ*/
    REMOTE_DIAG_CMD_DIAGNOSED,/* �ѽ��������Ӧ����Ͻ���*/
    REMOTE_DIAG_CMD_ClOSING,/* �ѷ��͹ر����󣬵ȴ���Ӧ*/
    REMOTE_DIAG_CMD_FINISHED,/* �ѽ��չرգ����� */
    REMOTE_DIAG_CMD_NUM,/* Զ���������״̬�� */
} REMOTE_DIAG_CMD_STATE_TYPE;


/* ����Զ���������ִ�н�� */
typedef enum
{
    REMOTE_DIAG_CMD_RESULT_OK,/*����ȷ����*/
    REMOTE_DIAG_CMD_RESULT_SESSION_CONTROL_FAILED,/* �Ự����ʧ�� */
    REMOTE_DIAG_CMD_RESULT_SECURITY_ACCESS_FAILED,/* ��ȫ����ʧ�� */
    REMOTE_DIAG_CMD_RESULT_NEGATIVE,/*���յ�����Ӧ*/
    REMOTE_DIAG_CMD_RESULT_CLOSING_FAILED,/*�ر�ʧ��*/
    REMOTE_DIAG_CMD_RESULT_TIMEOUT,/* ��Ӧ��ʱ */
} REMOTE_DIAG_CMD_RESULT_TYPE;

/* һ��Զ����Ͻ�� */
typedef enum
{
    REMOTE_DIAG_REQUEST_RESULT_OK = 0x11,/* ����ִ�гɹ� */
    REMOTE_DIAG_REQUEST_RESULT_UNKNOWN_MISTAKE = 0x12,/* δ֪����ԭ�� */
    REMOTE_DIAG_REQUEST_RESULT_SYSTEM_BUSY = 0x21,/* ��ϵͳæ��δִ�� */
    REMOTE_DIAG_REQUEST_RESULT_LOW_BATTERY = 0x22,/* �����������㣬 δִ�� */
    REMOTE_DIAG_REQUEST_RESULT_CHARGING = 0x23,/* ��������У� δִ�� */
    REMOTE_DIAG_REQUEST_RESULT_DRIVING = 0x24,/* ������ʻ�У� δִ�� */
    REMOTE_DIAG_REQUEST_RESULT_ENGINE_IDLE = 0x25,/* �򷢶���δ������ δִ�� */
    REMOTE_DIAG_REQUEST_RESULT_ENGINE_RUNNING = 0x25,/* �򷢶��������У� δִ�� */
    REMOTE_DIAG_REQUEST_RESULT_TIMEOUT = 0x27,/* ����ִ�г�ʱ */
    REMOTE_DIAG_REQUEST_RESULT_VEHICLE_GEAR_ERROR = 0x28,/* ������λ����δִ�� */
    REMOTE_DIAG_REQUEST_RESULT_POWER_STALL_ERROR = 0x29,/* ���Դ��λ����δִ�� */
    REMOTE_DIAG_REQUEST_RESULT_CMD_PARAMETER_ERROR = 0x30,/*����������� */
    REMOTE_DIAG_REQUEST_RESULT_UPSTREAM_COMMAND = 0xFF,/*����������� */
} REMOTE_DIAG_REQUEST_RESULT_TYPE;

/* Զ���������ִ��״̬ */
typedef enum
{
    REMOTE_DIAG_IDLE = 0,//δ��ʼ
    REMOTE_DIAG_EXECUTING,//ִ����
    REMOTE_DIAG_FINISHED,//ִ����ɣ����ѽ�Զ����Ͻ�����ͳ�ȥ
} REMOTE_DIAG_STATE_TYPE;




/* Զ������߳���Ϣ���� */
typedef enum
{
    REMOTE_DIAG_REQUEST = MPU_MID_REMOTE_DIAG,//ƽ̨���͵�Զ���������
    REMOTE_DIAG_UDS_RESPONSE,/* Tbox UDS ��Զ�������Ӧ */
    REMOTE_DIAG_SET_MCU_RESPONSE,/* ����MCU UDS��������Ӧ */
    REMOTE_DIAG_MCU_RESPONSE,/* MCU���ص�����ECU����Ӧ */
    REMOTE_DIAG_REQUEST_TIMEOUT,/* �����ѷ�������Ӧ��ʱ*/
} REMOTE_DIAG_MSG_TYPE;

typedef struct
{
    int vehicle_speed_sig_id;
    int vehicle_gear_sig_id;
    int power_stall_sig_id;
    int charge_state_sig_id;
} remote_diag_precondition_sig_id;

typedef struct
{
    unsigned int baud;
    unsigned int port;
    unsigned int request_id;
    unsigned int response_id;
    REMOTE_DIAG_ECUTYPE ecu_type;
    unsigned int diag_request_len;
    unsigned char level;
    unsigned char session;
    unsigned char diag_request[DIAG_REQUEST_LEN];/*UDS msg*/
} remote_diag_request_t;

typedef struct
{
    remote_diag_request_t remote_diag_request[REMOTE_DIAG_ECU_NUM];
    int remote_diag_request_size;
} remote_diag_request_arr_t;

typedef struct
{
    unsigned int request_id;
    unsigned int response_id;
    unsigned char diag_response[DIAG_RESPONSE_LEN];/*UDS msg*/
    unsigned int diag_response_len;
    REMOTE_DIAG_CMD_RESULT_TYPE result_type;
} remote_diag_response_t;

typedef struct
{
    remote_diag_response_t remote_diag_response[REMOTE_DIAG_ECU_NUM];
    int remote_diag_response_size;
} remote_diag_response_arr_t;


typedef struct
{
    union
    {
        char msg[REMOTE_DIAG_MSG_LEN];
    };
} remote_diag_msg_t;

typedef enum
{
    REMOTE_DIAG_OK = 0,
    REMOTE_DIAG_ERROR_PAR_INVALID,//��������
    REMOTE_DIAG_ERROR_WAKEUP_TMO, //���ѳ�����ʱ
    REMOTE_DIAG_ERROR_SECUR_TMO,  //��ȫ��֤��ʱ
    REMOTE_DIAG_ERROR_INSIDE,     //�ڲ�������ͨ�Ŵ���
    REMOTE_DIAG_ERROR_CONDITION,  //����������
    REMOTE_DIAG_ERROR_EX_FAIL,    //ִ��ʧ��
} remote_diag_error_t;

typedef struct
{
    /* Զ��������� */
    remote_diag_request_arr_t remote_diag_request_arr;

    /* Զ�������Ӧ */
    remote_diag_response_arr_t remote_diag_response_arr;

    /* ��ǰ���������ţ����ڽ�����Ӧ���� */
    int current_cmd_no;

    /* ��ǰ�������״̬ */
    REMOTE_DIAG_CMD_STATE_TYPE current_cmd_state;

    /* ��ǰԶ�����ģ��״̬ */
    REMOTE_DIAG_STATE_TYPE remote_diag_state;
} remote_diag_state_t;

int parsing_remote_diag_msg(char *remote_diag_msg, TCOM_MSG_HEADER msg,
                            remote_diag_request_arr_t *remote_diag_request_arr);
int remote_diag_excuted_result_output(void);
int update_remote_diag_cmd_no(void);
int is_remote_diag_need(void);
int update_remote_diag_state(void);
REMOTE_DIAG_STATE_TYPE get_remote_diag_state(void);
remote_diag_response_arr_t *get_remote_diag_response(void);
remote_diag_response_t *get_current_diag_cmd_response(void);
REMOTE_DIAG_ECUTYPE get_current_diag_cmd_ecu_type(void);
unsigned char get_current_diag_cmd_security_level(void);

remote_diag_request_t *get_current_diag_cmd_request(void);
REMOTE_DIAG_CMD_STATE_TYPE get_current_diag_cmd_state(void);
int remote_diag_cmd_set_finish(REMOTE_DIAG_CMD_RESULT_TYPE ret_type);
int remote_diag_cmd_store_diag_response(char *remote_diag_msg, unsigned int mlen);

int remote_diag_cmd_state_update(void);
void StrToHex(unsigned char *pbDest, unsigned char *pbSrc, int nLen);

int is_remote_diag_tbox(void);
int remote_diag_single_cmd_process(unsigned int msgid, char *remote_diag_msg, unsigned int mlen);
int remote_diag_timeout_process(char *remote_diag_msg, unsigned int mlen);
void init_remote_diag_state(void);
int remote_diag_process_start(char *remote_diag_msg, TCOM_MSG_HEADER msg);
remote_diag_request_arr_t *get_remote_diag_request(void);
remote_diag_state_t *get_remote_diag_state_t(void);
int remote_diag_send_request(unsigned char *msg, unsigned int len);
void remote_diag_process(unsigned int msgid, char *remote_diag_msg, unsigned int mlen);

uint32_t remote_seed_To_Key(REMOTE_DIAG_ECUTYPE ecu_type, char       *c_seed, char *c_key,
                            unsigned char securitylevel);
void remote_diag_update_timer(unsigned int interval);
int remote_diag_precondition_check(void);
remote_diag_precondition_sig_id *get_remote_diag_precondition_sig_id(void);


#endif //__REMOTE_DIAG_H__
