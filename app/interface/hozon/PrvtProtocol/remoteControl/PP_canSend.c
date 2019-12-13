/**********************************************
			合众远控can报文发送
	ID ：0x3D2   TBOX主动认证请求
	ID ：0x440   远程控制命令报文
	ID : 0x445  远程控制命令报文
	ID : 0x526  里程同步报文
**********************************************/

#include <stdio.h>
#include "can_api.h"
#include "scom_msg_def.h"
#include "../../../../base/scom/scom_tl.h"
#include "log.h"
#include "PPrmtCtrl_cfg.h"
#include "scom_api.h"
#include <pthread.h>
#include "PP_canSend.h"

static PP_can_msg_info_t canmsg_3D2;
static uint64_t old_ID440_data = 0;
static uint64_t new_ID440_data = 0;

static uint64_t old_ID445_data = 0;
static uint64_t new_ID445_data = 0;

//static uint64_t ID526_data;
static uint8_t can_data[8] = {0};

static uint8_t virtual_on_flag;

static uint8_t sync_flag_440 = 1;  //上电起来同步一次
static uint8_t sync_flag_445 = 1;  //上电起来同步一次

static pthread_mutex_t sync_mutex_440 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t sync_mutex_445 = PTHREAD_MUTEX_INITIALIZER;

extern unsigned char GetPP_CertDL_CertValid(void);
int PP_canSend_init(void)
{
	memset(&canmsg_3D2,0,sizeof(PP_can_msg_info_t));
	canmsg_3D2.typ = PP_CAN_TYP_EVENT;  //事件报文
	canmsg_3D2.len =8;
	canmsg_3D2.id = CAN_ID_3D2;
	canmsg_3D2.port = 1;
	canmsg_3D2.period = 100;  //100ms
	canmsg_3D2.times_event = 1;
	//初始化445报文数据
	old_ID445_data |= (uint64_t)1 << 46;
	old_ID445_data |= (uint64_t)1 << 54;
	new_ID445_data = old_ID445_data;
	return 0;
}

/*************************************************
	MPU发送虚拟on线唤醒MCU
**************************************************/
int PP_send_virtual_on_to_mcu(unsigned char on)
{
    int len = 0;
    unsigned char buf[64];
	
    if (on > 1)
    {
        log_e(LOG_HOZON, "par error");
    }

    buf[len++] = on;
	virtual_on_flag = on;
    if (scom_tl_send_frame(SCOM_MPU_MCU_VIRTUAL_ON, SCOM_TL_SINGLE_FRAME, 0, buf, len))
    {
        log_e(LOG_HOZON, "Fail to send msg to MCU");
        return -2;
    }
    return 0;
}

/***********************************************
PP_send_event_info_to_mcu 用于发送事件性报文3D2
************************************************/
int PP_send_event_info_to_mcu(PP_can_msg_info_t *caninfo)
{
    int len = 0;

    if (NULL == caninfo)
    {
        log_e(LOG_HOZON, "caninfo pointer is NULL");
        return -1;
    }
    if (caninfo->typ > PP_CAN_TYP_MAX || caninfo->len > 8 || caninfo->port > 4)
    {
        log_e(LOG_HOZON, "caninfo parameter error");
        return -1;
    }
    unsigned char buf[64];
    buf[len++] = caninfo->typ;
    memcpy(buf + len, &caninfo->id, sizeof(caninfo->id));
    len += sizeof(caninfo->id);
    buf[len++] = caninfo->port;
    buf[len++] = caninfo->len;
    memcpy(buf + len, caninfo->data, sizeof(caninfo->data));
    len += sizeof(caninfo->data);
    buf[len++] = caninfo->times_event;
    memcpy(buf + len, &caninfo->period, sizeof(caninfo->period));
    len += sizeof(caninfo->period);
	log_o(LOG_HOZON,"3D2 is sending");
	if (scom_tl_send_frame(SCOM_TL_CMD_CTRL, SCOM_TL_SINGLE_FRAME, 0, buf, len))
	{
	   log_e(LOG_HOZON, "Fail to send msg to MCU");
	   return -2;
	}
    return 0;
}

/********************************************************
PP_send_cycle_info_to_mcu 用于发送周期性报文 440 
********************************************************/
int PP_send_cycle_ID440_to_mcu(uint8_t *dt)
{
	int len = 0;
	unsigned char buf[64];
	memcpy(buf + len, dt, 8*sizeof(uint8_t));
    len += 8*sizeof(uint8_t);
	scom_tl_send_frame(SCOM_MPU_MCU_0x440, SCOM_TL_SINGLE_FRAME, 0, buf, len);
    return 0;
}

/********************************************************
PP_send_cycle_info_to_mcu 用于发送周期性报文 445 
********************************************************/
int PP_send_cycle_ID445_to_mcu(uint8_t *dt)
{
	int len = 0;
	unsigned char buf[64];
	memcpy(buf + len, dt, 8*sizeof(uint8_t));
    len += 8*sizeof(uint8_t);
	scom_tl_send_frame(SCOM_MPU_MCU_0x445, SCOM_TL_SINGLE_FRAME, 0, buf, len);

    return 0;
}
/********************************************************
PP_send_cycle_info_to_mcu 用于发送周期性报文 526
********************************************************/
int PP_send_cycle_ID526_to_mcu(uint8_t *dt)
{
	int len = 0;
	unsigned char buf[64];
	memcpy(buf + len, dt, 8*sizeof(uint8_t));
    len += 8*sizeof(uint8_t);
	if (scom_tl_send_frame(SOCM_MCU_MPU_0x526, SCOM_TL_SINGLE_FRAME, 0, buf, len))
	{
	   log_e(LOG_HOZON, "Fail to send msg to MCU");
	   return -2;
	}
    return 0;
}
void PP_can_unpack(uint64_t data,uint8_t *dt)
{
	int i;
	memset(dt,0,8*sizeof(uint8_t));
	for(i=7;i>=0;i--)
	{
		dt[i] = (uint8_t) (data >> (i*8));
	}

}
/***************************************************************************
函数名：PP_canSend_setbit       功能：将8个字节中某一位置位
id    ：报文ID
bit   ：起始的bit位
bitl  ：占几个bit
data  ：具体的数据
*dt   ：此参数用于发送3D2报文的发送，其余ID的包文，此参数填NULL
****************************************************************************/
void PP_canSend_setbit(unsigned int id,uint8_t bit,uint8_t bitl,uint8_t data,uint8_t *dt)
{
	if(id == CAN_ID_440)
	{
		new_ID440_data &= ~((uint64_t)((1<<bitl)-1) << (bit-bitl+1)) ; //再移位
		new_ID440_data |= (uint64_t)data << (bit-bitl+1);      //置位
		if(new_ID440_data != old_ID440_data)
		{
			pthread_mutex_lock(&sync_mutex_440);
			sync_flag_440 = 1;
			pthread_mutex_unlock(&sync_mutex_440);
			old_ID440_data = new_ID440_data;
		}
	}
	else if(id == CAN_ID_445)
	{
		new_ID445_data &=  ~((uint64_t)((1<<bitl)-1) << (bit-bitl+1)) ; //再移位
		new_ID445_data |= (uint64_t)data << (bit-bitl+1);      //置位
		if(new_ID445_data != old_ID445_data)
		{
			pthread_mutex_lock(&sync_mutex_445);
			sync_flag_445 = 1;
			pthread_mutex_unlock(&sync_mutex_445);
			old_ID445_data = new_ID445_data;
		}
	}
	else if(id == CAN_ID_526)
	{
		memset(can_data,0,8);
		can_data[0] = dt[0];
		can_data[1] = dt[1];
		can_data[2] = dt[2];
		PP_send_cycle_ID526_to_mcu(can_data);
	}
	else
	{
		int i;
		if(dt == NULL)
		{
			memset(canmsg_3D2.data,0,8*sizeof(uint8_t));
			log_o(LOG_HOZON,"3D2 is packing");
		}
		else
		{
			for(i=0;i<8;i++)
			{
				canmsg_3D2.data[i] = dt[i];
			}
		}
		PP_send_event_info_to_mcu(&canmsg_3D2);
	}
}
/***************************************************
		广播440 445报文
****************************************************/
void PP_can_send_cycle(void)
{
	int i = 0;
	if(1 == scom_dev_openSt())
	{
		if(PP_rmtCtrl_cfg_RmtStartSt() == 0)  //防止空调开启之后，高压电不是TBOX下的，导致一些状态不能恢复
		{
			PP_canSend_setbit(CAN_ID_445,1,1,0,NULL);//无效
			PP_canSend_setbit(CAN_ID_445,17,1,0,NULL);//autocmd 清零
		}
		if(GetPP_CertDL_CertValid() == 1) //TBOX与HU之间的证书有效
		{
			PP_canSend_setbit(CAN_ID_440,23,1,1,NULL);//证书有效
		}
		
		if(sync_flag_440 == 1)
		{
			pthread_mutex_lock(&sync_mutex_440);
			for(i = 0; i< 3 ;i++)
			{
				PP_can_unpack(old_ID440_data,can_data);
				PP_send_cycle_ID440_to_mcu(can_data);
				usleep(10);	
			}
			sync_flag_440 = 0;
			pthread_mutex_unlock(&sync_mutex_440);
		}
		
		if(sync_flag_445 == 1)
		{
			pthread_mutex_lock(&sync_mutex_445);
			for(i = 0; i< 3 ;i++)
			{
				PP_can_unpack(old_ID445_data,can_data);
				PP_send_cycle_ID445_to_mcu(can_data);
				usleep(10);
			}
			sync_flag_445 = 0;
			pthread_mutex_unlock(&sync_mutex_445);
		}
		
	}
}
/**************************************
			唤醒MCU
***************************************/
void PP_can_mcu_awaken(void)
{
	int i;
	for(i=0;i<10;i++)
	{
		PP_send_virtual_on_to_mcu(1);
	}
	 log_o(LOG_HOZON,
  			"############### send virtual on to mcu:1 #################");
}

/**************************************
			休眠MCU
***************************************/
void PP_can_mcu_sleep(void)
{
	PP_send_virtual_on_to_mcu(0);
	 log_o(LOG_HOZON,
  			"############### send virtual on to mcu:0 #################");
}

/******************************************************
		远程控制报文发送
*******************************************************/
void PP_can_send_data(int type,uint8_t data,uint8_t para)
{
	switch(type)
	{
		case PP_CAN_DOORLOCK:  //门锁
			PP_canSend_setbit(CAN_ID_440,17,2,data,NULL);
			break;
		case PP_CAN_SUNROOF:   //天窗
			PP_canSend_setbit(CAN_ID_440,47,3,data,NULL);  //天窗
			break;
		case PP_CAN_SUNSHADE:   //遮阳帘
			PP_canSend_setbit(CAN_ID_440,33,2,data,NULL); //遮阳帘
			break;
		case PP_CAN_AUTODOOR:  //尾门
			PP_canSend_setbit(CAN_ID_440,19,2,data,NULL);
			break;
		case PP_CAN_SEARCH:   //寻车
			PP_canSend_setbit(CAN_ID_440,17,2,data,NULL);
			break;
		case PP_CAN_ENGINE:  //高压电控制
			PP_canSend_setbit(CAN_ID_440,1,2,data,NULL);
			break;
		case PP_CAN_ACCTRL://空调控制
			{
				switch(data)
				{
					case CAN_OPENACC:
						PP_canSend_setbit(CAN_ID_445,1,1,1,NULL);//有效
						PP_canSend_setbit(CAN_ID_445,17,1,1,NULL);//开启
						break;
					case CAN_OPNEACCFIAL:
						PP_canSend_setbit(CAN_ID_445,1,1,0,NULL);//无效
						PP_canSend_setbit(CAN_ID_445,17,1,0,NULL);//autocmd 清零
						break;
					case CAN_SETACCTEP:
						PP_canSend_setbit(CAN_ID_445,47,6,para,NULL); //设置主驾温度
						PP_canSend_setbit(CAN_ID_445,55,6,para,NULL); //设置副驾温度
						break;
					case CAN_CLOSEACC:
						PP_canSend_setbit(CAN_ID_445,14,1,1,NULL);//关闭
						PP_canSend_setbit(CAN_ID_445,17,1,0,NULL);//autocmd 清零
						break;
					case CAN_CLOSEACCCLEAN:
						PP_canSend_setbit(CAN_ID_445,14,1,0,NULL);//清掉关闭
						PP_canSend_setbit(CAN_ID_445,1,1,0,NULL);//无效
						break;
					case CAN_ACCMD_INVAILD:
						PP_canSend_setbit(CAN_ID_445,1,1,0,NULL);//无效
						break;
					default:
						break;
				}	
			}
			break;
		case PP_CAN_CHAGER:  //充电控制
			switch(data)
			{
				case CAN_STARTCHAGER:
					PP_canSend_setbit(CAN_ID_440,4,1,0,NULL);
					PP_canSend_setbit(CAN_ID_440,3,2,2,NULL);
				break;
				case CAN_STOPCHAGER:
					PP_canSend_setbit(CAN_ID_440,3,2,1,NULL);
					break;
				case CAN_SETAPPOINT:
					PP_canSend_setbit(CAN_ID_440,4,1,1,NULL);
					break;
				case CAN_CANCELAPPOINT:
					PP_canSend_setbit(CAN_ID_440,4,1,0,NULL);
					break;
				case CAN_CLEANCHARGE:
					PP_canSend_setbit(CAN_ID_440,3,2,0,NULL);
					break;
				default:
					break;	
			}
			break;
		case PP_CAN_FORBID:   //禁止启动
			PP_canSend_setbit(CAN_ID_440,31,2,data,NULL);
			break;
		case PP_CAN_SEATHEAT: //座椅加热
			PP_canSend_setbit(CAN_ID_445,para,3,data,NULL);
			break;
		case PP_CAN_OTAREQ:   //OTA请求
			PP_canSend_setbit(CAN_ID_440,39,2,data,NULL);
			break;
		case PP_CAN_CERTIFICATE: //证书有效性
			PP_canSend_setbit(CAN_ID_440,23,1,data,NULL);
			break;
		case PP_CAN_BLUESTART:  //蓝牙一键启动
			PP_canSend_setbit(CAN_ID_440,40,1,data,NULL);	
			break;
		case PP_CAN_SOS:       //SOS触发信号
			PP_canSend_setbit(CAN_ID_440,41,1,data,NULL);	
			break;
		case PP_CAN_HV:       //OTA下高压电请求
			PP_canSend_setbit(CAN_ID_440,42,1,data,NULL);	
			break;
		default:
			break;

	}
}

/*****************************************************
		远程控制认证报文发送
*****************************************************/
void PP_can_send_identificat(uint8_t type,uint8_t *dt)
{
	switch(type)
	{
		case PP_CAN_RANDOM:
			PP_canSend_setbit(CAN_ID_3D2,0,0,0,dt);
			break;
		case PP_CAN_XTEAENCIPHER:
			PP_canSend_setbit(CAN_ID_3D2,0,0,0,dt);
			break;
		default:
			break;	
	}
}

/*****************************************************
		TBOX 同步里程
*****************************************************/
void PP_can_send_mileage(uint8_t *dt)
{
	PP_canSend_setbit(CAN_ID_526,0,0,0,dt);	
}
uint8_t PP_get_virtual_flag()
{
	uint8_t st;
	if(virtual_on_flag == 0)
		st = 1;
	else
		st = 0;
	return st;
}