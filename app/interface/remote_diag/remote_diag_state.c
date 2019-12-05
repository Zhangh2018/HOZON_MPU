#include "remote_diag.h"
#include "log.h"
#include "uds_define.h"

#define UDS_P2_SERVER_T (50)

static remote_diag_state_t remote_diag_state;
extern timer_t remote_diag_request_timeout;

const unsigned int remote_diag_state_timeout[REMOTE_DIAG_CMD_NUM] = 
{
    0xffffffff,                                     //REMOTE_DIAG_CMD_NOT_START = 0,//δ��ʼ
    UDS_P2_SERVER_T,                                //REMOTE_DIAG_CMD_OPENING,/* �ѷ�����������ȴ���Ӧ*/
    UDS_P2_SERVER_T,                                //REMOTE_DIAG_CMD_OPENED,/* ���óɹ�����δ����UDS�������*/
    UDS_P2_SERVER_T,                                //REMOTE_DIAG_CMD_SESSION_CONTROLLING,/* �Ự���� �л��Ự��*/
    UDS_P2_SERVER_T,                                //REMOTE_DIAG_CMD_SESSION_CONTROLLED, /* �Ự���������� */
    UDS_P2_SERVER_T,                                //REMOTE_DIAG_CMD_SECURITY_SEED_REQUESTING,/* ��ȫ��������������*/
    UDS_P2_SERVER_T,                                //REMOTE_DIAG_CMD_SECURITY_SEED_REQUESTED,/* ��ȫ�������������ӣ������յ�����*/
    UDS_P2_SERVER_T,                                //REMEOT_DIAG_CMD_SECURITY_KEY_SENDING,/* ��ȫ���ʣ�����key��*/
    UDS_P2_SERVER_T,                                //REMOTE_DIAG_CMD_SECURITY_KEY_SENDED,/* ��ȫ�����ѽ��� */
    REMOTE_DIAG_MSG_TIMEOUT_INTERVAL,               //REMOTE_DIAG_CMD_DIAGNOSING,/* �ѷ���������󣬵ȴ������Ӧ*/
    UDS_P2_SERVER_T,                                //REMOTE_DIAG_CMD_DIAGNOSED,/* �ѽ��������Ӧ����Ͻ���*/
    UDS_P2_SERVER_T,                                //REMOTE_DIAG_CMD_ClOSING,/* �ѷ��͹ر����󣬵ȴ���Ӧ*/
    0xffffffff                                      //REMOTE_DIAG_CMD_FINISHED,/* �ѽ��չرգ����� */
};

extern const unsigned int REMOTE_DIAG_CAN_ID[REMOTE_DIAG_ECU_NUM][2];

void remote_diag_update_timer(unsigned int interval)
{
    if (get_current_diag_cmd_state() > REMOTE_DIAG_CMD_NOT_START)
    {
        tm_stop(remote_diag_request_timeout);
    }

    if (get_current_diag_cmd_state() < REMOTE_DIAG_CMD_FINISHED)
    {
        tm_start(remote_diag_request_timeout, interval, TIMER_TIMEOUT_REL_ONCE);
    }
}


/* ��ǰԶ���������״̬����
    0 ���³ɹ�
    1 ����ʧ��*/
int remote_diag_cmd_state_update(void)
{
    int ret = 0;

    if (remote_diag_state.current_cmd_state < REMOTE_DIAG_CMD_FINISHED)
    {

        remote_diag_state.current_cmd_state++;
        remote_diag_update_timer(remote_diag_state_timeout[get_current_diag_cmd_state()]);
        ret = 0;
    }
    else
    {
        ret = 1;
    }

    return ret;
}

/*�洢�����Ϣ*/
int remote_diag_cmd_store_diag_response(char *remote_diag_msg, unsigned int mlen)
{
    remote_diag_response_t *response = get_current_diag_cmd_response();
    memcpy(response->diag_response, remote_diag_msg, mlen);
    response->diag_response_len = mlen;

    return 0;
}

/* ������ǰ��������� */
int remote_diag_cmd_set_finish(REMOTE_DIAG_CMD_RESULT_TYPE ret_type)
{
    if (remote_diag_state.remote_diag_response_arr.remote_diag_response_size
        < remote_diag_state.remote_diag_request_arr.remote_diag_request_size)
    {
        remote_diag_state.remote_diag_response_arr.remote_diag_response_size++;
    }

    remote_diag_response_t *response = get_current_diag_cmd_response();
    response->result_type = ret_type;

    while (remote_diag_cmd_state_update() == 0); /* ���õ�ǰִ������״̬Ϊ�ѽ��� */

    return 0;
}

/* ��ȡ��ǰԶ���������ִ��״̬ */
REMOTE_DIAG_CMD_STATE_TYPE get_current_diag_cmd_state(void)
{
    return remote_diag_state.current_cmd_state;
}

/*��ȡ��ǰ���� ECU ����*/
REMOTE_DIAG_ECUTYPE get_current_diag_cmd_ecu_type(void)
{

    return get_current_diag_cmd_request()->ecu_type;
}

/*��ȡ��ǰ���� ��ȫ�ȼ�*/
unsigned char get_current_diag_cmd_security_level(void)
{
    return get_current_diag_cmd_request()->level;
}


/* ��ȡ��ǰԶ�������������*/
remote_diag_request_t *get_current_diag_cmd_request(void)
{
    return &(remote_diag_state.remote_diag_request_arr.remote_diag_request[remote_diag_state.current_cmd_no]);
}

/* ��ȡ��ǰԶ����������� */
remote_diag_response_t *get_current_diag_cmd_response(void)
{
    return &(remote_diag_state.remote_diag_response_arr.remote_diag_response[remote_diag_state.current_cmd_no]);
}

remote_diag_response_arr_t *get_remote_diag_response(void)
{
    return &(remote_diag_state.remote_diag_response_arr);
}

remote_diag_request_arr_t *get_remote_diag_request(void)
{
    return &(remote_diag_state.remote_diag_request_arr);
}

remote_diag_state_t *get_remote_diag_state_t(void)
{
    return &(remote_diag_state);
}



/* ��ȡԶ�����ģ��״̬ */
REMOTE_DIAG_STATE_TYPE get_remote_diag_state(void)
{
    return remote_diag_state.remote_diag_state;
}

/* ����Զ�����ģ��״̬
    0 ���³ɹ�
    1 ����ʧ��*/
int update_remote_diag_state(void)
{
    int ret = 0;

    if (remote_diag_state.remote_diag_state < REMOTE_DIAG_FINISHED)
    {
        remote_diag_state.remote_diag_state++;
        ret = 0;
    }
    else
    {
        log_e(LOG_REMOTE_DIAG, "remote diag state update failed,current state:REMOTE_DIAG_FINISHED");
        ret = 1;
    }

    return ret;
}

/* �Ƿ���Զ���������
    1 ��
    0 û��*/
int is_remote_diag_need(void)
{
    int ret = 0;

    if (remote_diag_state.remote_diag_request_arr.remote_diag_request_size == 0)
    {
        ret = 0;
    }
    else if (remote_diag_state.remote_diag_response_arr.remote_diag_response_size
             < remote_diag_state.remote_diag_request_arr.remote_diag_request_size)
    {
        ret = 1;
    }
    else if (get_current_diag_cmd_state() < REMOTE_DIAG_CMD_FINISHED)
    {
        ret = 1;
    }
    else
    {
        ret = 0;
    }

    return ret;
}



/* ��ת����һ���������ִ�� */
int update_remote_diag_cmd_no(void)
{
    int ret = 0;

    if (remote_diag_state.current_cmd_no <
        (remote_diag_state.remote_diag_request_arr.remote_diag_request_size - 1))
    {
        remote_diag_state.current_cmd_no++;
        remote_diag_state.current_cmd_state = REMOTE_DIAG_CMD_NOT_START;
        ret = 0;
    }
    else
    {
        ret = 1;
    }

    return ret;
}

/* 1��Զ�����Tbox
   0���������ECU*/
int is_remote_diag_tbox(void)
{
    remote_diag_request_t *request = NULL;
    request = get_current_diag_cmd_request();

    if (request->request_id == REMOTE_DIAG_CAN_ID[REMOTE_DIAG_TBOX][DIAG_REQUEST_ID_ROW])
    {
        return 1;/* Զ�����Tbox*/
    }
    else
    {
        return 0;/* Զ���������ECU*/
    }
}

void init_remote_diag_state(void)
{
    remote_diag_state.current_cmd_no = 0;
    remote_diag_state.current_cmd_state = REMOTE_DIAG_CMD_NOT_START;
    memset(&(remote_diag_state.remote_diag_response_arr), 0x00, sizeof(remote_diag_response_arr_t));
    //memcpy(&(remote_diag_state.remote_diag_request_arr), &request_arr, sizeof(remote_diag_request_arr_t));
    remote_diag_state.remote_diag_state = REMOTE_DIAG_IDLE;
}






