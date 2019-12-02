#include "remote_diag.h"
#include "log.h"
#include "uds_define.h"
#include "tcom_api.h"
#include "uds.h"
#include "can_api.h"

typedef int (*remote_request_proc_func)(remote_diag_request_t *request,
                                        remote_diag_response_t *response);

extern timer_t remote_diag_request_timeout;
extern UDS_T    uds_client;
remote_diag_precondition_sig_id precondition_sig_id;

remote_diag_precondition_sig_id *get_remote_diag_precondition_sig_id(void)
{
    return (&precondition_sig_id);
}
/* Զ�����ʱ�����յ�TBOX�����������Ӧ��Ϣ����ִ�д˻ص����� */
static int remote_diag_set_client_callback(UDS_T *uds, int msg_id, int can_id, uint8_t *data,
        int len)
{
    switch (msg_id)
    {
        case MSG_ID_UDS_C_ACK:
            send_msg_to_remote_diag(MPU_MID_UDS, REMOTE_DIAG_SET_MCU_RESPONSE, data,
                                    len);/* ��Զ����Ϸ�������MCU UDS ACK��Ӧ*/
            break;

        case MSG_ID_UDS_IND:
            send_msg_to_remote_diag(MPU_MID_UDS, REMOTE_DIAG_MCU_RESPONSE, data, len);/*  MCU ���ص������Ӧ */
            break;

        default:
            break;
    }

    return 0;
}

/*����Զ����������MCU����T-Box UDSģ�飬�����������쳣��������ķ���������remote_request_proc �����д���*/
static int remote_diag_request_process(REMOTE_DIAG_CMD_STATE_TYPE pre_state,
                                       remote_request_proc_func remote_request_proc)
{
    int ret = 0;
    remote_diag_request_t *request = get_current_diag_cmd_request();
    remote_diag_response_t *response = get_current_diag_cmd_response();

    /* ǰ��״̬������ */
    if (get_current_diag_cmd_state() == pre_state)
    {
        ret = remote_request_proc(request, response);
    }
    else if (get_current_diag_cmd_state() >= (pre_state + 1))
    {
        ret = 0;
    }
    else
    {
        ret = 1;
    }

    return ret;
}

/*�������� MCU UDS client�˲��� */
int remote_diag_request_set_client(remote_diag_request_t *request, remote_diag_response_t *response)
{
    int ret = 0;

    response->request_id = request->request_id;
    response->response_id = request->response_id;

    if (1 == is_remote_diag_tbox())
    {
        remote_diag_cmd_state_update();/* REMOTE_DIAG_CMD_OPENING */
        remote_diag_cmd_state_update();/* REMOTE_DIAG_CMD_OPENED */
        ret = 0;/* ��������������������� */
    }
    else
    {
        /* ���� �������� ��MCU */
        uds_set_client_ex(request->port, request->request_id, request->request_id,
                          request->response_id, remote_diag_set_client_callback);

        /* ���µ�ǰ�������״̬Ϊ REMOTE_DIAG_CMD_OPENING */
        remote_diag_cmd_state_update();


        ret = 1;
    }

    return ret;
}


static int remote_diag_request_session_control(remote_diag_request_t *request,
        remote_diag_response_t *response)
{
    int ret = 0;
    unsigned char request_tmp[DIAG_REQUEST_LEN];
    memset(request_tmp, 0x00, DIAG_REQUEST_LEN);

    /* ����������Ҫ��չ�Ự*/
    if ((SESSION_TYPE_EXTENDED == request->session))
    {
        StrToHex(request_tmp, (unsigned char *)EXTEND_SESSION_CMD, 2);/* ��ȡ�л���չ�Ự���� */
        remote_diag_send_request(request_tmp, 2);/*�����������*/
        remote_diag_cmd_state_update();/* ���µ�ǰ���״̬Ϊ REMOTE_DIAG_CMD_SESSION_CONTROLLING */
        ret = 1;
    }
    else/*����Ҫ�л��Ự��������switch������ִ�к������� */
    {
        /* ֱ�Ӹ��»Ự����Ϊ��� */
        remote_diag_cmd_state_update();/* ���µ�ǰ���״̬Ϊ REMOTE_DIAG_CMD_SESSION_CONTROLLING */
        remote_diag_cmd_state_update();/* ���µ�ǰ���״̬Ϊ REMOTE_DIAG_CMD_SESSION_CONTROLLED */
        ret = 0;
    }

    return ret;
}

/* ack���� */
static int remtoe_diag_set_ack(REMOTE_DIAG_CMD_RESULT_TYPE error_result_type, char *remote_diag_msg,
                               unsigned int mlen)
{
    int ret = 0;

    /*�϶���Ӧ����*/
    if (remote_diag_msg[0] != SID_NegativeResponse) /*�϶���Ӧ����*/
    {
        remote_diag_cmd_state_update();

        if (REMOTE_DIAG_CMD_FINISHED == get_current_diag_cmd_state())
        {
            ret = remote_diag_cmd_set_finish(REMOTE_DIAG_CMD_RESULT_OK);
        }
        else if (REMOTE_DIAG_CMD_DIAGNOSED == get_current_diag_cmd_state())
        {
            remote_diag_cmd_store_diag_response(remote_diag_msg, mlen);
        }
        else
        {
            ret = 0;
        }
    }
    else/* ����Ӧ���� */
    {
        if (remote_diag_msg[2] == 0x78) /*78�ӳ�*/
        {
            ret = 1;/* ������ִ�� */
            remote_diag_update_timer(2000);
        }
        else
        {
            ret = remote_diag_cmd_set_finish(error_result_type);/* ���õ�ǰ������� */
        }
    }

    return ret;
}

/* ackǰ���������� */
static int remtoe_diag_ack_process(REMOTE_DIAG_CMD_RESULT_TYPE error_result_type,
                                   REMOTE_DIAG_CMD_STATE_TYPE pre_state,
                                   unsigned int msgid, char *remote_diag_msg, unsigned int mlen)
{
    int ret = 0;

    if (get_current_diag_cmd_state() == pre_state)
    {
        if (((0 == is_remote_diag_tbox())  && (REMOTE_DIAG_MCU_RESPONSE == msgid))
            || ((1 == is_remote_diag_tbox()) && (REMOTE_DIAG_UDS_RESPONSE == msgid)))
        {
            remtoe_diag_set_ack(error_result_type, remote_diag_msg, mlen);
        }
        else
        {
            ret = 1;
        }
    }
    else if (get_current_diag_cmd_state() >= (pre_state + 1))
    {
        ret = 0;
    }
    else
    {
        ret = 1;
    }

    return ret;
}

static int remote_diag_request_seed(remote_diag_request_t *request,
                                    remote_diag_response_t *response)
{
    int ret = 0;
    unsigned char request_tmp[DIAG_REQUEST_LEN];
    memset(request_tmp, 0x00, DIAG_REQUEST_LEN);

    if (SecurityAccess_LEVEL1 == request->level) /*��Ҫ��ȫ���ʵȼ�1*/
    {
        StrToHex(request_tmp, (unsigned char *)SECURITY_LEVEL1_REQUEST_SEED,
                 strlen(SECURITY_LEVEL1_REQUEST_SEED) / 2); /* ��ȫ������������ */
        remote_diag_send_request(request_tmp, 2);/*�����������*/
        remote_diag_cmd_state_update();/* ���µ�ǰ���״̬Ϊ REMOTE_DIAG_CMD_SESSION_CONTROLLING */
        ret = 1;
    }
    else if (SecurityAccess_LEVEL2 == request->level) /*��Ҫ��ȫ���ʵȼ�2*/
    {
        StrToHex(request_tmp, (unsigned char *)SECURITY_LEVEL2_REQUEST_SEED,
                 strlen(SECURITY_LEVEL2_REQUEST_SEED) / 2); /* ��ȡ�л���չ�Ự���� */
        remote_diag_send_request(request_tmp, 2);/*�����������*/
        remote_diag_cmd_state_update();/* ���µ�ǰ���״̬Ϊ REMOTE_DIAG_CMD_SESSION_CONTROLLING */
        ret = 1;
    }
    else/* û�а�ȫ������Ҫ */
    {
        /* ֱ�ӽ�����ȫ�����߼� */
        remote_diag_cmd_state_update();
        remote_diag_cmd_state_update();
        remote_diag_cmd_state_update();
        remote_diag_cmd_state_update();
        ret = 0;
    }

    return ret;
}

static int remote_diag_request_send_diag(remote_diag_request_t *request,
        remote_diag_response_t *response)
{
    int ret = 0;
    /*�����������*/
    remote_diag_send_request(request->diag_request, request->diag_request_len);

    /* �����������״̬ */
    remote_diag_cmd_state_update();
    ret = 1;
    return ret;
}

static int remote_diag_request_closing(remote_diag_request_t *request,
        remote_diag_response_t *response)
{
    int ret = 0;
    unsigned char request_tmp[DIAG_REQUEST_LEN];
    memset(request_tmp, 0x00, DIAG_REQUEST_LEN);

    StrToHex(request_tmp, (unsigned char *)DEFAULT_SESSION_CMD, 2);/* ��ȡ�л�Ĭ�ϻỰ���� */
    remote_diag_send_request(request_tmp, 2);/*�����������*/
    remote_diag_cmd_state_update();/* ���µ�ǰ���״̬Ϊ REMOTE_DIAG_CMD_ClOSING */
    ret = 1;

    return ret;
}


/*0:��������������� �� ����
  1:��������������� δ ����*/
static int remote_diag_request_set_client_process(void)
{
    int ret = 0;
    ret = remote_diag_request_process(REMOTE_DIAG_CMD_NOT_START, remote_diag_request_set_client);
    return ret;
}

/*
    0:��������յ����� MCU ��Ӧ ״̬
    1:δ������յ����� MCU ��Ӧ ״̬
*/
static int remote_diag_recieve_set_client_ack_process(unsigned int msgid)
{
    int ret = 0;

    if (get_current_diag_cmd_state() == REMOTE_DIAG_CMD_OPENING)
    {
        /* ���յ�����MCU UDS��Ӧ */
        if (REMOTE_DIAG_SET_MCU_RESPONSE == msgid)
        {
            remote_diag_cmd_state_update();/* ���µ�ǰ�������״̬Ϊ REMOTE_DIAG_CMD_OPENED */
            ret = 0;
        }
        else
        {
            ret = 1;
        }
    }
    else if (get_current_diag_cmd_state() >= REMOTE_DIAG_CMD_OPENED)
    {
        ret = 0;
    }
    else
    {
        ret = 1;
    }

    return ret;
}

/*0:������
  1:δ����*/
static int remote_diag_request_session_control_process(void)
{
    int ret = 0;
    ret = remote_diag_request_process(REMOTE_DIAG_CMD_OPENED, remote_diag_request_session_control);
    return ret;
}

/*0:������
  1:δ����*/
static int remtoe_diag_recieve_session_control_ack_process(unsigned int msgid,
        char *remote_diag_msg, unsigned int mlen)
{
    int ret = 0;
    ret = remtoe_diag_ack_process(REMOTE_DIAG_CMD_RESULT_SESSION_CONTROL_FAILED,
                                  REMOTE_DIAG_CMD_SESSION_CONTROLLING, msgid, remote_diag_msg, mlen);
    return ret;
}

/* Զ����ϣ���������
   0:������
   1:δ����*/
static int remote_diag_request_seed_process(void)
{
    int ret = 0;
    ret = remote_diag_request_process(REMOTE_DIAG_CMD_SESSION_CONTROLLED, remote_diag_request_seed);
    return ret;
}

/* Զ����ϣ�����������Ӧ����
   0:������
   1:δ����*/
static int remote_diag_recieve_request_seed_ack_process(unsigned int msgid, char *remote_diag_msg,
        unsigned int mlen)
{
    int ret = 0;
    ret = remtoe_diag_ack_process(REMOTE_DIAG_CMD_RESULT_SECURITY_ACCESS_FAILED,
                                  REMOTE_DIAG_CMD_SECURITY_SEED_REQUESTING,
                                  msgid, remote_diag_msg, mlen);
    return ret;
}

/*0:�ѷ���key
  1:δ����key*/
static int remote_diag_request_send_key_process(unsigned int msgid, char *remote_diag_msg,
        unsigned int mlen)
{
    int ret = 0;
    char request_tmp[DIAG_REQUEST_LEN];

    if (get_current_diag_cmd_state() == REMOTE_DIAG_CMD_SECURITY_SEED_REQUESTED)
    {
        request_tmp[0] = remote_diag_msg[0] - 0x40;
        request_tmp[1] = remote_diag_msg[1] + 0x01;

        remote_seed_To_Key(get_current_diag_cmd_ecu_type(), &(remote_diag_msg[2]), &(request_tmp[2]),
                           get_current_diag_cmd_security_level());
        remote_diag_send_request((unsigned char *)request_tmp, 6);/*�����������*/
        remote_diag_cmd_state_update();/* ���µ�ǰ�������״̬Ϊ REMEOT_DIAG_CMD_SECURITY_KEY_SENDING */
        ret = 1;
    }
    else if (get_current_diag_cmd_state() >= REMEOT_DIAG_CMD_SECURITY_KEY_SENDING)
    {
        ret = 0;/*�������״̬*/
    }
    else
    {
        ret = 1;
    }

    return ret;
}

/*
    0:�������״̬
    1:�������״̬*/
static int remote_diag_recieve_send_key_ack_process(unsigned int msgid, char *remote_diag_msg,
        unsigned int mlen)
{

    int ret = 0;
    ret = remtoe_diag_ack_process(REMOTE_DIAG_CMD_RESULT_SECURITY_ACCESS_FAILED,
                                  REMEOT_DIAG_CMD_SECURITY_KEY_SENDING, msgid, remote_diag_msg, mlen);
    return ret;
}

/*0:�ѷ����������
  1:δ�����������*/
static int remote_diag_request_send_diag_process(void)
{
    int ret = 0;
    ret = remote_diag_request_process(REMOTE_DIAG_CMD_SECURITY_KEY_SENDED,
                                      remote_diag_request_send_diag);
    return ret;
}

/*0:�ѷ����������
  1:δ�����������*/
static int remote_diag_request_closing_process(void)
{
    int ret = 0;
    ret = remote_diag_request_process(REMOTE_DIAG_CMD_DIAGNOSED,
                                      remote_diag_request_closing);
    return ret;
}


/* ����UDS ���ص����Tbox����Ӧ */
static int remote_diag_recieve_request_diag_process(unsigned int msgid, char *remote_diag_msg,
        unsigned int mlen)
{
    int ret = 0;
    ret = remtoe_diag_ack_process(REMOTE_DIAG_CMD_RESULT_NEGATIVE, REMOTE_DIAG_CMD_DIAGNOSING, msgid,
                                  remote_diag_msg, mlen);
    return ret;
}

/* �����л�Ĭ�ϻỰ������Ӧ */
static int remote_diag_recieve_closing_process(unsigned int msgid, char *remote_diag_msg,
        unsigned int mlen)
{
    int ret = 0;
    ret = remtoe_diag_ack_process(REMOTE_DIAG_CMD_RESULT_CLOSING_FAILED, REMOTE_DIAG_CMD_ClOSING, msgid,
                                  remote_diag_msg, mlen);
    return ret;
}


/*
    0:���������������������
    1:���������������
*/
static int remote_diag_single_cmd_start(void)
{
    int ret = 0;

    ret = remote_diag_request_set_client_process()
          || remote_diag_request_session_control_process()
          || remote_diag_request_seed_process()
          || remote_diag_request_send_diag_process();

    return (ret ? 0 : 1);

}

/* Զ����Ͻ������� */
static int remote_diag_finish_process(void)
{
    int ret = 0;

    /* ������������ִ����� */
    if (0 == is_remote_diag_need())
    {
        ret = update_remote_diag_state();/* �������״̬ */

        if (ret == 0)
        {
            uds_set_client_ex(0, 0, 0, 0, NULL);
            remote_diag_excuted_result_output();/* �����Ͻ���������*/
        }
    }
    else
    {
        ret = remote_diag_single_cmd_start();
    }

    return ret;
}

/*
    Զ����ϴ����� */
void remote_diag_process(unsigned int msgid, char *remote_diag_msg, unsigned int mlen)
{
    if (REMOTE_DIAG_EXECUTING == get_remote_diag_state())
    {
        if(remote_diag_single_cmd_process(msgid, remote_diag_msg, mlen) == 0) /* ��ǰ�����ѽ��� */
        {
            /* ���µ���һ��������� */
            if (update_remote_diag_cmd_no() == 1)
            {
                /* ���µ���һ������ʧ�ܣ����н������� */
                /* �ж��Ƿ�����Զ���������ѽ��������������������� */
                remote_diag_finish_process();
            }
            else
            {
                remote_diag_single_cmd_start();
            }
        }
    }
    else
    {
        log_o(LOG_REMOTE_DIAG, "remote diag state error:%d", get_remote_diag_state());
    }
}

/* ������������
   0����ǰִ�еĵ��������ѽ���
   1����ǰִ�еĵ�������δ����*/
int remote_diag_single_cmd_process(unsigned int msgid, char *remote_diag_msg, unsigned int mlen)
{
    int ret = 0;

    ret = remote_diag_request_set_client_process()
          || remote_diag_recieve_set_client_ack_process(msgid)
          || remote_diag_request_session_control_process()
          || remtoe_diag_recieve_session_control_ack_process(msgid, remote_diag_msg, mlen)
          || remote_diag_request_seed_process()
          || remote_diag_recieve_request_seed_ack_process(msgid, remote_diag_msg, mlen)
          || remote_diag_request_send_key_process(msgid, remote_diag_msg, mlen)
          || remote_diag_recieve_send_key_ack_process(msgid, remote_diag_msg, mlen)
          || remote_diag_request_send_diag_process()
          || remote_diag_recieve_request_diag_process(msgid, remote_diag_msg, mlen)
          || remote_diag_request_closing_process()
          || remote_diag_recieve_closing_process(msgid, remote_diag_msg, mlen);

    return ret;
}

/* ��ʱ������ */
int remote_diag_timeout_process(char *remote_diag_msg, unsigned int mlen)
{
    int ret = 0;

    if((get_current_diag_cmd_state() < REMOTE_DIAG_CMD_ClOSING)
     &&(get_current_diag_cmd_state() > REMOTE_DIAG_CMD_OPENING))
    {
        while (get_current_diag_cmd_state() < REMOTE_DIAG_CMD_DIAGNOSED) /* ���õ�ǰִ������״̬Ϊ��Ͻ��� */
        {
            remote_diag_cmd_state_update();
        }
        ret = remote_diag_request_closing_process();
        return ret;
    }
    
    /* ���µ�ǰ�������״̬Ϊ REMOTE_DIAG_CMD_FINISHED */
    remote_diag_cmd_set_finish(REMOTE_DIAG_CMD_RESULT_TIMEOUT);

    /* ���µ���һ��������� */
    if (update_remote_diag_cmd_no() == 1)
    {
        /* ���µ���һ������ʧ�ܣ����н������� */
        /* �ж��Ƿ�����Զ���������ѽ��������������������� */
        remote_diag_finish_process();
    }
    else
    {
        ret = remote_diag_single_cmd_start();
    }

    return ret;
}
/* ����Զ���������
   0���ѳɹ�����
   1��δ�ɹ�����*/
int remote_diag_process_start(char *remote_diag_msg, TCOM_MSG_HEADER msg)
{
    int ret = 0;

    switch (get_remote_diag_state())
    {
        case REMOTE_DIAG_IDLE:
        case REMOTE_DIAG_FINISHED:
            /* ����Զ�������Ϣ */
            ret = parsing_remote_diag_msg(remote_diag_msg, msg,
                                          &(get_remote_diag_state_t()->remote_diag_request_arr));

            /* ������Ϣʧ�� */
            if (ret != 0)
            {
                break;
            }

            /* ��ʼ��Զ�����״̬ */
            init_remote_diag_state();

            /* ��Զ��������󣬸���Զ�����״̬*/
            if (is_remote_diag_need())
            {

                ret = update_remote_diag_state();

                if (ret == 0)
                {
                    ret = remote_diag_single_cmd_start();
                }
            }

            break;

        case REMOTE_DIAG_EXECUTING:
            {
                /*ϵͳæδִ��*/
                log_e(LOG_REMOTE_DIAG, "Remote diagnosis is in progress!");
                break;
                ret = 1;
            }

        default:
            ret = 1;
            break;
    }

    return ret;
}

