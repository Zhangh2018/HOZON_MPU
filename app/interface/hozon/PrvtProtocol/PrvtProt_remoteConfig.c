/******************************************************
�ļ�����	PrvtProt_remoteCfg.c

������	��ҵ˽��Э�飨�㽭���ڣ�	
Data			Vasion			author
2018/1/10		V1.0			liujian
*******************************************************/

/*******************************************************
description�� include the header file
*******************************************************/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/times.h>
#include <sys/time.h>
#include "timer.h"
#include <sys/prctl.h>

#include <sys/types.h>
#include <sysexits.h>	/* for EX_* exit codes */
#include <assert.h>	/* for assert(3) */
#include "constr_TYPE.h"
#include "asn_codecs.h"
#include "asn_application.h"
#include "asn_internal.h"	/* for _ASN_DEFAULT_STACK_MAX */
#include "XcallReqinfo.h"
#include "Bodyinfo.h"
#include "per_encoder.h"
#include "per_decoder.h"
#include "dev_api.h"
#include "init.h"
#include "ble.h"
#include "log.h"
#include "list.h"
#include "../../support/protocol.h"
#include "../../../base/uds/server/uds_did.h"
#include "../sockproxy/sockproxy_txdata.h"
#include "cfg_api.h"
#include "hozon_SP_api.h"
#include "hozon_PP_api.h"
#include "shell_api.h"
#include "PrvtProt_shell.h"
#include "PrvtProt_queue.h"
#include "PrvtProt_EcDc.h"
#include "PrvtProt_cfg.h"
#include "PrvtProt.h"
#include "PrvtProt_remoteConfig.h"

/*******************************************************
description�� global variable definitions
*******************************************************/



/*******************************************************
description�� static variable definitions
*******************************************************/
typedef struct
{
	PrvtProt_pack_Header_t	Header;
	PrvtProt_DisptrBody_t	DisBody;
}__attribute__((packed))  PrvtProt_rmtCfg_pack_t; /**/

typedef struct
{
	PrvtProt_rmtCfg_pack_t 	pack;
	PrvtProt_rmtCfgSt_t	 	state;
}__attribute__((packed))  PrvtProt_rmtCfg_t; /*�ṹ��*/

static PrvtProt_pack_t 			PP_rmtCfg_Pack;
static PrvtProt_rmtCfg_t		PP_rmtCfg;
static PrvtProt_App_rmtCfg_t 	AppData_rmtCfg;

//static PrvtProt_TxInform_t rmtCfg_TxInform;
/*******************************************************
description�� function declaration
*******************************************************/
/*Global function declaration*/

/*Static function declaration*/
static int PP_rmtCfg_do_checksock(PrvtProt_task_t *task);
static int PP_rmtCfg_do_rcvMsg(PrvtProt_task_t *task,PrvtProt_rmtCfg_t *rmtCfg);
static void PP_rmtCfg_RxMsgHandle(PrvtProt_task_t *task,PrvtProt_rmtCfg_t *rmtCfg,PrvtProt_pack_t* rxPack,int len);
static int PP_rmtCfg_do_wait(PrvtProt_task_t *task);
static int PP_rmtCfg_do_checkConfig(PrvtProt_task_t *task,PrvtProt_rmtCfg_t *rmtCfg);
static int PP_rmtCfg_checkRequest(PrvtProt_task_t *task,PrvtProt_rmtCfg_t *rmtCfg);
static int PP_rmtCfg_getRequest(PrvtProt_task_t *task,PrvtProt_rmtCfg_t *rmtCfg);
static int PP_rmtCfg_CfgEndRequest(PrvtProt_task_t *task,PrvtProt_rmtCfg_t *rmtCfg);
static int PP_rmtCfg_ReadCfgResp(PrvtProt_task_t *task,PrvtProt_rmtCfg_t *rmtCfg,PrvtProt_DisptrBody_t *MsgDataBody);
static void PP_rmtCfg_reset(PrvtProt_rmtCfg_t *rmtCfg);
static int PP_rmtCfg_ConnResp(PrvtProt_task_t *task,PrvtProt_rmtCfg_t *rmtCfg,PrvtProt_DisptrBody_t *MsgDataBody);

static void PP_rmtCfg_send_cb(void * para);
static void PP_rmtCfg_HexToStr(uint8_t *pbDest, uint8_t *pbSrc, int nLen);
static void getPP_rmtCfg_localConfig(void);
/******************************************************
description�� function code
******************************************************/
/******************************************************
*��������PP_rmtCfg_init

*��  �Σ�void

*����ֵ��void

*��  ������ʼ��

*��  ע��
******************************************************/
void PP_rmtCfg_init(void)
{
	memset(&PP_rmtCfg,0 , sizeof(PrvtProt_rmtCfg_t));
	memset(&AppData_rmtCfg,0 , sizeof(PrvtProt_App_rmtCfg_t));

	memcpy(AppData_rmtCfg.checkReq.mcuSw,DID_F1B0_SW_UPGRADE_VER,strlen(DID_F1B0_SW_UPGRADE_VER));
	AppData_rmtCfg.checkReq.mcuSwlen = strlen(DID_F1B0_SW_UPGRADE_VER);
	memcpy(AppData_rmtCfg.checkReq.mpuSw,DID_F1B0_SW_UPGRADE_VER,strlen(DID_F1B0_SW_UPGRADE_VER));
	AppData_rmtCfg.checkReq.mpuSwlen = strlen(DID_F1B0_SW_UPGRADE_VER);

	memcpy(AppData_rmtCfg.checkReq.btMacAddr,"000000000000",strlen("000000000000"));
	AppData_rmtCfg.checkReq.btMacAddrlen = strlen("000000000000");
	memcpy(AppData_rmtCfg.checkReq.configSw,"00000",strlen("00000"));
	AppData_rmtCfg.checkReq.configSwlen = strlen("00000");
	memcpy(AppData_rmtCfg.checkReq.cfgVersion,"00000000000000000000000000000000",strlen("00000000000000000000000000000000"));
	AppData_rmtCfg.checkReq.cfgVersionlen = strlen("00000000000000000000000000000000");

	//读取本地配置
	getPP_rmtCfg_localConfig();

	PP_rmtCfg.state.avtivecheckflag = 0;
	PP_rmtCfg.state.iccidValid = 0;
	PP_rmtCfg.state.CfgSt = PP_RMTCFG_CFG_IDLE;

	#if 1
	unsigned char wifienable = 1;
	cfg_set_para(CFG_ITEM_WIFI_SET,&wifienable,1);
	#endif

}

/*
* 读取本地配置信息
*/
static void getPP_rmtCfg_localConfig(void)
{
	unsigned int len;
	int res;
	len = 1;
	res = cfg_get_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_ST,&AppData_rmtCfg.ReadResp.cfgsuccess,&len);
	if((res==0) && (AppData_rmtCfg.ReadResp.cfgsuccess == 1))
	{
		len = 33;
		cfg_get_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_VER,AppData_rmtCfg.ReadResp.cfgVersion,&len);
		memcpy(AppData_rmtCfg.checkReq.cfgVersion,AppData_rmtCfg.ReadResp.cfgVersion,33);
		AppData_rmtCfg.ReadResp.cfgVersionlen = 32;
		AppData_rmtCfg.checkReq.cfgVersionlen = 32;
		len = 256;
		cfg_get_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_APN1,&AppData_rmtCfg.ReadResp.APN1,&len);
		cfg_get_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_APN2,&AppData_rmtCfg.ReadResp.APN2,&len);
		cfg_get_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_FICM,&AppData_rmtCfg.ReadResp.FICM,&len);
		cfg_get_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_COMM,&AppData_rmtCfg.ReadResp.COMMON,&len);
		cfg_get_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_EXT,&AppData_rmtCfg.ReadResp.EXTEND,&len);
	}
	else
	{
		memcpy(AppData_rmtCfg.ReadResp.cfgVersion,AppData_rmtCfg.checkReq.cfgVersion,32);
		log_o(LOG_HOZON,"AppData_rmtCfg.ReadResp.cfgsuccess = %d",AppData_rmtCfg.ReadResp.cfgsuccess);
		AppData_rmtCfg.ReadResp.cfgVersionlen = 32;
		AppData_rmtCfg.ReadResp.APN1.apn1ConfigValid = 0;
		AppData_rmtCfg.ReadResp.APN2.apn2ConfigValid = 0;
		AppData_rmtCfg.ReadResp.COMMON.commonConfigValid = 0;
		AppData_rmtCfg.ReadResp.EXTEND.extendConfigValid= 0;
		AppData_rmtCfg.ReadResp.FICM.ficmConfigValid = 0;
	}
}

/******************************************************
*��������PP_rmtCfg_mainfunction

*��  �Σ�void

*����ֵ��void

*��  ������������

*��  ע��
******************************************************/
int PP_rmtCfg_mainfunction(void *task)
{
	int res;

	if(!dev_get_KL15_signal())
	{
		PP_rmtCfg.state.avtivecheckflag = 0;
		PP_rmtCfg.state.CfgSt = PP_RMTCFG_CFG_IDLE;
		return 0;
	}

	res = 		PP_rmtCfg_do_checksock((PrvtProt_task_t*)task) || \
				PP_rmtCfg_do_rcvMsg((PrvtProt_task_t*)task,&PP_rmtCfg) || \
				PP_rmtCfg_do_wait((PrvtProt_task_t*)task) || \
				PP_rmtCfg_do_checkConfig((PrvtProt_task_t*)task,&PP_rmtCfg);
	return res;
}

/******************************************************
*��������PP_rmtCfg_do_checksock

*��  �Σ�void

*����ֵ��void

*��  �������socket����

*��  ע��
******************************************************/
static int PP_rmtCfg_do_checksock(PrvtProt_task_t *task)
{
	if(1 == sockproxy_socketState())//socket open
	{

		return 0;
	}

	PP_rmtCfg_reset(&PP_rmtCfg);
	return -1;
}

/******************************************************
*��������PP_rmtCfg_do_rcvMsg

*��  �Σ�void

*����ֵ��void

*��  �����������ݺ���

*��  ע��
******************************************************/
static int PP_rmtCfg_do_rcvMsg(PrvtProt_task_t *task,PrvtProt_rmtCfg_t *rmtCfg)
{	
	int rlen = 0;
	PrvtProt_pack_t rcv_pack;
	memset(&rcv_pack,0 , sizeof(PrvtProt_pack_t));
	if ((rlen = RdPP_queue(PP_REMOTE_CFG,rcv_pack.Header.sign,sizeof(PrvtProt_pack_t))) <= 0)
    {
		return 0;
	}
	
	log_i(LOG_HOZON, "receive remote config message");
	protocol_dump(LOG_HOZON, "remote_config_message", rcv_pack.Header.sign, rlen, 0);
	if((rcv_pack.Header.sign[0] != 0x2A) || (rcv_pack.Header.sign[1] != 0x2A) || \
			(rlen <= 18))
	{
		return 0;
	}
	
	if(rlen > (18 + PP_MSG_DATA_LEN))
	{
		return 0;
	}
	PP_rmtCfg_RxMsgHandle(task,rmtCfg,&rcv_pack,rlen);

	return 0;
}

/******************************************************
*��������PP_rmtCfg_RxMsgHandle

*��  �Σ�void

*����ֵ��void

*��  �����������ݴ���

*��  ע��
******************************************************/
static void PP_rmtCfg_RxMsgHandle(PrvtProt_task_t *task,PrvtProt_rmtCfg_t *rmtCfg,PrvtProt_pack_t* rxPack,int len)
{
	int aid;
	int i;
	int idlenode;
	if(PP_OPERATETYPE_NGTP != rxPack->Header.opera)
	{
		log_e(LOG_HOZON, "unknow package");
		return;
	}

	PrvtProt_DisptrBody_t MsgDataBody;
	if(0 != PrvtPro_decodeMsgData(rxPack->msgdata,(len - 18),&MsgDataBody,&AppData_rmtCfg))
	{
		log_e(LOG_HOZON, "decode error\r\n");
		return;
	}

	aid = (MsgDataBody.aID[0] - 0x30)*100 +  (MsgDataBody.aID[1] - 0x30)*10 + \
			  (MsgDataBody.aID[2] - 0x30);
	if(PP_AID_RMTCFG != aid)
	{
		log_e(LOG_HOZON, "aid unmatch");
		return;
	}
	switch(MsgDataBody.mID)
	{
		case PP_MID_CHECK_CFG_RESP://check remote config response
		{
			if(rmtCfg->state.waitSt == PP_RMTCFG_CHECK_WAIT_RESP)
			{
				if(1 == AppData_rmtCfg.checkResp.needUpdate)
				{
					rmtCfg->state.needUpdata = 1;
				}
				AppData_rmtCfg.checkResp.needUpdate = 0;
				rmtCfg->state.waitSt = 0;
				log_o(LOG_HOZON, "\r\ncheck config req ok\r\n");
			}
		}
		break;
		case PP_MID_GET_CFG_RESP://get remote config response
		{
			if(rmtCfg->state.waitSt == PP_RMTCFG_GET_WAIT_RESP)
			{
				rmtCfg->state.waitSt = 0;

				log_o(LOG_HOZON, "\r\nget config req ok\r\n");
			}
		}
		break;
		case PP_MID_CONN_CFG_REQ:
		{
			rmtCfg->state.req 	= 0;
			rmtCfg->state.reqCnt 	= 0;
			rmtCfg->state.waitSt  = PP_RMTCFG_WAIT_IDLE;
			rmtCfg->state.CfgSt 	= PP_RMTCFG_CFG_IDLE;
			rmtCfg->state.cfgAccept = 1;
			rmtCfg->state.eventid = MsgDataBody.eventId;
			if(0 == PP_rmtCfg_ConnResp(task,rmtCfg,&MsgDataBody))
			{
				idlenode = PP_getIdleNode();
				PP_TxInform[idlenode].aid = PP_AID_RMTCFG;
				PP_TxInform[idlenode].mid = PP_MID_CONN_CFG_RESP;
				PP_TxInform[idlenode].pakgtype = PP_TXPAKG_SIGTIME;
				PP_TxInform[idlenode].eventtime = tm_get_time();
				PP_TxInform[idlenode].idleflag = 1;
				PP_TxInform[idlenode].description = "resp of cfg req";
				SP_data_write(PP_rmtCfg_Pack.Header.sign,PP_rmtCfg_Pack.totallen, \
						PP_rmtCfg_send_cb,&PP_TxInform[idlenode]);
			}
		}
		break;
		case PP_MID_READ_CFG_REQ:
		{
			//读取本地配置
			getPP_rmtCfg_localConfig();
			memset(AppData_rmtCfg.ReadResp.readreq,0,PP_RMTCFG_SETID_MAX);
			for(i = 0; i < AppData_rmtCfg.ReadReq.SettingIdlen;i++)
			{
				AppData_rmtCfg.ReadResp.readreq[AppData_rmtCfg.ReadReq.SettingId[i] -1] = 1;
			}
			rmtCfg->state.eventid = MsgDataBody.eventId;
			if(0 == PP_rmtCfg_ReadCfgResp(task,rmtCfg,&MsgDataBody))
			{
				idlenode = PP_getIdleNode();
				PP_TxInform[idlenode].aid = PP_AID_RMTCFG;
				PP_TxInform[idlenode].mid = PP_MID_READ_CFG_RESP;
				PP_TxInform[idlenode].pakgtype = PP_TXPAKG_SIGTIME;
				PP_TxInform[idlenode].eventtime = tm_get_time();
				PP_TxInform[idlenode].idleflag = 1;
				PP_TxInform[idlenode].description = "resp of cfg read";
				SP_data_write(PP_rmtCfg_Pack.Header.sign,PP_rmtCfg_Pack.totallen, \
						PP_rmtCfg_send_cb,&PP_TxInform[idlenode]);
			}

			for(i = 0; i < PP_RMTCFG_SETID_MAX;i++)
			{
				AppData_rmtCfg.ReadReq.SettingId[i] = 0;
			}
		}
		break;
		default:
		break;
	}
}

/******************************************************
*��������PP_rmtCfg_do_wait

*��  �Σ�void

*����ֵ��void

*��  ��������Ƿ����¼��ȴ�Ӧ��

*��  ע��
******************************************************/
static int PP_rmtCfg_do_wait(PrvtProt_task_t *task)
{
	if(!PP_rmtCfg.state.waitSt)
	{
		return 0;
	}

	if(PP_rmtCfg.state.waitSt == PP_RMTCFG_CHECK_WAIT_RESP)
	{
		if((tm_get_time() - PP_rmtCfg.state.waittime) > PP_RMTCFG_WAIT_TIMEOUT)
		{
			log_e(LOG_HOZON, "check cfg response time out");
			PP_rmtCfg.state.reqCnt = 0;
			PP_rmtCfg.state.req = 0;
			PP_rmtCfg.state.waitSt = PP_RMTCFG_WAIT_IDLE;
			PP_rmtCfg.state.CfgSt = PP_RMTCFG_CFG_IDLE;
		}
	}
	else if(PP_rmtCfg.state.waitSt == PP_RMTCFG_GET_WAIT_RESP)
	{
		 if((tm_get_time() - PP_rmtCfg.state.waittime) > PP_RMTCFG_WAIT_TIMEOUT)
		 {
			 log_e(LOG_HOZON, "get cfg response time out");
			 PP_rmtCfg.state.reqCnt = 0;
			 PP_rmtCfg.state.req = 0;
			 PP_rmtCfg.state.waitSt = PP_RMTCFG_WAIT_IDLE;
			 PP_rmtCfg.state.CfgSt = PP_RMTCFG_CFG_IDLE;
		 }
	}
	else if(PP_rmtCfg.state.waitSt == PP_RMTCFG_END_WAIT_SENDRESP)
	{
		 if((tm_get_time() - PP_rmtCfg.state.waittime) > PP_RMTCFG_WAIT_TIMEOUT)
		 {
			 PP_rmtCfg.state.reqCnt = 0;
			 PP_rmtCfg.state.req = 0;
			 PP_rmtCfg.state.waitSt = PP_RMTCFG_WAIT_IDLE;
			 PP_rmtCfg.state.CfgSt = PP_RMTCFG_CFG_IDLE;
		 }
	}
	return -1;
}

/******************************************************
*��������PP_rmtCfg_do_checkConfig

*��  �Σ�

*����ֵ��

*��  �����������

*��  ע��
******************************************************/
static int PP_rmtCfg_do_checkConfig(PrvtProt_task_t *task,PrvtProt_rmtCfg_t *rmtCfg)
{
	unsigned int len;
	int idlenode;

	if(0 == PP_rmtCfg.state.avtivecheckflag)
	{//上电主动查询配置
		PP_rmtCfg.state.req  = 1;
		PP_rmtCfg.state.reqCnt = 0;
		PP_rmtCfg.state.period = tm_get_time();
		PP_rmtCfg.state.delaytime = tm_get_time();
		PP_rmtCfg.state.avtivecheckflag = 1;
	}

	AppData_rmtCfg.checkReq.iccIDlen = 20;
	if(!AppData_rmtCfg.checkReq.iccID[0])
	{//get iccid
		(void)PrvtProtCfg_get_iccid((char *)(AppData_rmtCfg.checkReq.iccID));
	}
	else
	{
		PP_rmtCfg.state.iccidValid = 1;
	}

	switch(rmtCfg->state.CfgSt)
	{
		case PP_RMTCFG_CFG_IDLE:
		{
			if(((1 == PP_rmtCfg.state.iccidValid) && ((tm_get_time() - PP_rmtCfg.state.delaytime) >= 15000)) || \
					((tm_get_time() - PP_rmtCfg.state.delaytime) >= 30000))
			{
				if((1 == rmtCfg->state.req) && (rmtCfg->state.reqCnt < PP_RETRANSMIT_TIMES))
				{
					if((tm_get_time() - rmtCfg->state.period) >= RMTCFG_DELAY_TIME)
					{
						log_o(LOG_HOZON, "start request remote config\r\n");
						rmtCfg->state.CfgSt = PP_CHECK_CFG_REQ;
						rmtCfg->state.waitSt = PP_RMTCFG_WAIT_IDLE;
						rmtCfg->state.reqCnt++;
					}
				}
				else
				{
					rmtCfg->state.reqCnt = 0;
					rmtCfg->state.req = 0;
				}
			}
		}
		break;
		case PP_CHECK_CFG_REQ:
		{
			len = 18;
			cfg_get_user_para(CFG_ITEM_GB32960_VIN,AppData_rmtCfg.checkReq.vehicleVin,&len);//vin
			AppData_rmtCfg.checkReq.vehicleVinlen = 17;

			len = 33;
			cfg_get_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_VER,AppData_rmtCfg.checkReq.cfgVersion,&len);
			AppData_rmtCfg.checkReq.cfgVersionlen = 32;

			unsigned char Mac[32];
			BleGetMac(Mac);
			PP_rmtCfg_HexToStr(AppData_rmtCfg.checkReq.btMacAddr,Mac,6);
			AppData_rmtCfg.checkReq.btMacAddrlen = 12;
			if(0 == PP_rmtCfg_checkRequest(task,rmtCfg))
			{
				idlenode = PP_getIdleNode();
				PP_TxInform[idlenode].aid = PP_AID_RMTCFG;
				PP_TxInform[idlenode].mid = PP_MID_CHECK_CFG_REQ;
				PP_TxInform[idlenode].pakgtype = PP_TXPAKG_SIGTIME;
				PP_TxInform[idlenode].eventtime = tm_get_time();
				PP_TxInform[idlenode].idleflag = 1;
				PP_TxInform[idlenode].description = "check cfg req";
				SP_data_write(PP_rmtCfg_Pack.Header.sign,PP_rmtCfg_Pack.totallen, \
						PP_rmtCfg_send_cb,&PP_TxInform[idlenode]);
			}

			rmtCfg->state.waitSt 	= PP_RMTCFG_CHECK_WAIT_RESP;
			rmtCfg->state.CfgSt 	= PP_CHECK_CFG_RESP;
			rmtCfg->state.waittime 	= tm_get_time();
		}
		break;
		case PP_CHECK_CFG_RESP:
		{
			if(rmtCfg->state.needUpdata == 1)
			{
				rmtCfg->state.needUpdata = 0;
				rmtCfg->state.CfgSt = PP_GET_CFG_REQ;
			}
			else
			{
				rmtCfg->state.req = 0;
				rmtCfg->state.reqCnt = 0;
				rmtCfg->state.waitSt = PP_RMTCFG_WAIT_IDLE;
				rmtCfg->state.CfgSt = PP_RMTCFG_CFG_IDLE;
				log_o(LOG_HOZON, "noneed updata\n");
			}
		}
		break;
		case PP_GET_CFG_REQ:
		{
			memset(&(AppData_rmtCfg.getResp),0,sizeof(App_rmtCfg_getResp_t));
			if(0 == PP_rmtCfg_getRequest(task,rmtCfg))
			{
				idlenode = PP_getIdleNode();
				PP_TxInform[idlenode].aid = PP_AID_RMTCFG;
				PP_TxInform[idlenode].mid = PP_MID_GET_CFG_REQ;
				PP_TxInform[idlenode].pakgtype = PP_TXPAKG_SIGTIME;
				PP_TxInform[idlenode].eventtime = tm_get_time();
				PP_TxInform[idlenode].idleflag = 1;
				PP_TxInform[idlenode].description = "get cfg req";
				SP_data_write(PP_rmtCfg_Pack.Header.sign,PP_rmtCfg_Pack.totallen, \
						PP_rmtCfg_send_cb,&PP_TxInform[idlenode]);
			}

			rmtCfg->state.waitSt 	= PP_RMTCFG_GET_WAIT_RESP;
			rmtCfg->state.CfgSt 	= PP_GET_CFG_RESP;
			rmtCfg->state.waittime 	= tm_get_time();
		}
		break;
		case PP_GET_CFG_RESP:
		{

			rmtCfg->state.CfgSt = PP_RMTCFG_CFG_END;
		}
		break;
		case PP_RMTCFG_CFG_END://��������
		{
			if(AppData_rmtCfg.getResp.result == 1)
			{
				unsigned int len;
				len = 256;
				cfg_get_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_APN1,&AppData_rmtCfg.ReadResp.APN1,&len);
				cfg_get_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_APN2,&AppData_rmtCfg.ReadResp.APN2,&len);
				cfg_get_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_FICM,&AppData_rmtCfg.ReadResp.FICM,&len);
				cfg_get_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_COMM,&AppData_rmtCfg.ReadResp.COMMON,&len);
				cfg_get_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_EXT,&AppData_rmtCfg.ReadResp.EXTEND,&len);

				AppData_rmtCfg.getResp.result = 0;
				memcpy(AppData_rmtCfg.ReadResp.cfgVersion,AppData_rmtCfg.checkResp.cfgVersion, \
														  AppData_rmtCfg.checkResp.cfgVersionlen);
				AppData_rmtCfg.ReadResp.cfgVersionlen = AppData_rmtCfg.checkResp.cfgVersionlen;
				if(1 == AppData_rmtCfg.getResp.FICM.ficmConfigValid)
				{
					memcpy(&(AppData_rmtCfg.ReadResp.FICM),&(AppData_rmtCfg.getResp.FICM),sizeof(App_rmtCfg_FICM_t));
				}

				if(1 == AppData_rmtCfg.getResp.APN1.apn1ConfigValid)
				{
					if((0 != strcmp((const char*)AppData_rmtCfg.ReadResp.APN1.tspAddr,(const char*)AppData_rmtCfg.getResp.APN1.tspAddr)) || \
						(0 != strcmp((const char*)AppData_rmtCfg.ReadResp.APN1.tspPort,(const char*)AppData_rmtCfg.getResp.APN1.tspPort)))
					{
						rmtCfg->state.apn1tspaddrchangedflag = 1;
					}

					if((0 != strcmp((const char*)AppData_rmtCfg.ReadResp.APN1.certAddress,(const char*)AppData_rmtCfg.getResp.APN1.certAddress)) || \
						(0 != strcmp((const char*)AppData_rmtCfg.ReadResp.APN1.certPort,(const char*)AppData_rmtCfg.getResp.APN1.certPort)))
					{
						rmtCfg->state.apn1certaddrchangeflag = 1;
					}

					if(0 != strcmp((const char*)AppData_rmtCfg.ReadResp.APN1.tspSms, \
									(const char*)AppData_rmtCfg.getResp.APN1.tspSms))
					{
						rmtCfg->state.tspSMSchangeflag = 1;
					}

					memcpy(&(AppData_rmtCfg.ReadResp.APN1),&(AppData_rmtCfg.getResp.APN1),sizeof(App_rmtCfg_APN1_t));
				}

				if(1 == AppData_rmtCfg.getResp.APN2.apn2ConfigValid)
				{
					memcpy(&(AppData_rmtCfg.ReadResp.APN2),&(AppData_rmtCfg.getResp.APN2),sizeof(App_rmtCfg_APN2_t));
				}
		
				if(1 == AppData_rmtCfg.getResp.COMMON.commonConfigValid)
				{
					memcpy(&(AppData_rmtCfg.ReadResp.COMMON),&(AppData_rmtCfg.getResp.COMMON),sizeof(App_rmtCfg_COMMON_t));
				}

				if(1 == AppData_rmtCfg.getResp.EXTEND.extendConfigValid)
				{
					memcpy(&(AppData_rmtCfg.ReadResp.EXTEND),&(AppData_rmtCfg.getResp.EXTEND),sizeof(App_rmtCfg_EXTEND_t));
				}

				rmtCfg->state.cfgsuccess = 1;
			}

			if(0 == PP_rmtCfg_CfgEndRequest(task,rmtCfg))
			{
				idlenode = PP_getIdleNode();
				PP_TxInform[idlenode].aid = PP_AID_RMTCFG;
				PP_TxInform[idlenode].mid = PP_MID_CFG_END;
				PP_TxInform[idlenode].pakgtype = PP_TXPAKG_SIGTIME;
				PP_TxInform[idlenode].eventtime = tm_get_time();
				PP_TxInform[idlenode].idleflag = 1;
				PP_TxInform[idlenode].description = "cfg end";
				SP_data_write(PP_rmtCfg_Pack.Header.sign,PP_rmtCfg_Pack.totallen, \
						PP_rmtCfg_send_cb,&PP_TxInform[idlenode]);
			}

			rmtCfg->state.waitSt 	= PP_RMTCFG_END_WAIT_SENDRESP;
			rmtCfg->state.waittime 	= tm_get_time();
		}
		break;
		default:
		break;
	}

	return 0;
}

/******************************************************
*��������PP_rmtCfg_checkRequest

*��  �Σ�

*����ֵ��

*��  ����check remote config request

*��  ע��
******************************************************/
static int PP_rmtCfg_checkRequest(PrvtProt_task_t *task,PrvtProt_rmtCfg_t *rmtCfg)
{
	int msgdatalen;
	int res = 0;
	/*header*/
	memcpy(rmtCfg->pack.Header.sign,"**",2);
	rmtCfg->pack.Header.commtype.Byte = 0xe1;
	rmtCfg->pack.Header.opera = 0x02;
	rmtCfg->pack.Header.ver.Byte = task->version;
	rmtCfg->pack.Header.nonce  = PrvtPro_BSEndianReverse((uint32_t)task->nonce);
	rmtCfg->pack.Header.tboxid = PrvtPro_BSEndianReverse((uint32_t)task->tboxid);
	memcpy(&PP_rmtCfg_Pack, &rmtCfg->pack.Header, sizeof(PrvtProt_pack_Header_t));
	/*body*/
	memcpy(rmtCfg->pack.DisBody.aID,"100",3);
	rmtCfg->pack.DisBody.mID = PP_MID_CHECK_CFG_REQ;
	rmtCfg->pack.DisBody.eventId = 0;//主动上行数据，eventid为0
	rmtCfg->pack.DisBody.eventTime = PrvtPro_getTimestamp();
	rmtCfg->pack.DisBody.expTime   = PP_rmtCfg.state.expTime;
	rmtCfg->pack.DisBody.ulMsgCnt++;	/* OPTIONAL */
	rmtCfg->pack.DisBody.appDataProVer = 256;
	rmtCfg->pack.DisBody.testFlag = 1;
	/*appdata*/

	if(0 != PrvtPro_msgPackageEncoding(ECDC_RMTCFG_CHECK_REQ,PP_rmtCfg_Pack.msgdata,&msgdatalen,\
									   &rmtCfg->pack.DisBody,&AppData_rmtCfg))
	{
		log_e(LOG_HOZON, "uper error");
		return -1;
	}

	PP_rmtCfg_Pack.totallen = 18 + msgdatalen;
	PP_rmtCfg_Pack.Header.msglen = PrvtPro_BSEndianReverse((long)(18 + msgdatalen));

	return res;
}

/******************************************************
*��������PP_rmtCfg_getRequest

*��  �Σ�

*����ֵ��

*��  ����get remote config request

*��  ע��
******************************************************/
static int PP_rmtCfg_getRequest(PrvtProt_task_t *task,PrvtProt_rmtCfg_t *rmtCfg)
{
	int msgdatalen;
	int res = 0;
	/*header*/
	memcpy(rmtCfg->pack.Header.sign,"**",2);
	rmtCfg->pack.Header.commtype.Byte = 0xe1;
	rmtCfg->pack.Header.opera = 0x02;
	rmtCfg->pack.Header.ver.Byte = task->version;
	rmtCfg->pack.Header.nonce  = PrvtPro_BSEndianReverse((uint32_t)task->nonce);
	rmtCfg->pack.Header.tboxid = PrvtPro_BSEndianReverse((uint32_t)task->tboxid);
	memcpy(&PP_rmtCfg_Pack, &rmtCfg->pack.Header, sizeof(PrvtProt_pack_Header_t));
	/*body*/
	memcpy(rmtCfg->pack.DisBody.aID,"100",3);
	rmtCfg->pack.DisBody.mID = PP_MID_GET_CFG_REQ;
	rmtCfg->pack.DisBody.eventId = 0;//主动上行数据，eventid为0
	rmtCfg->pack.DisBody.eventTime = PrvtPro_getTimestamp();
	rmtCfg->pack.DisBody.expTime   = PP_rmtCfg.state.expTime;
	rmtCfg->pack.DisBody.ulMsgCnt++;	/* OPTIONAL */
	rmtCfg->pack.DisBody.appDataProVer = 256;
	rmtCfg->pack.DisBody.testFlag = 1;
	/*appdata*/
	memcpy(AppData_rmtCfg.getReq.cfgVersion,AppData_rmtCfg.checkResp.cfgVersion,AppData_rmtCfg.checkResp.cfgVersionlen);
	AppData_rmtCfg.getReq.cfgVersionlen = AppData_rmtCfg.checkResp.cfgVersionlen;

	if(0 != PrvtPro_msgPackageEncoding(ECDC_RMTCFG_GET_REQ,PP_rmtCfg_Pack.msgdata,&msgdatalen,\
									   &rmtCfg->pack.DisBody,&AppData_rmtCfg))
	{
		log_e(LOG_HOZON, "uper error");
		return -1;
	}

	PP_rmtCfg_Pack.totallen = 18 + msgdatalen;
	PP_rmtCfg_Pack.Header.msglen = PrvtPro_BSEndianReverse((long)(18 + msgdatalen));

	return res;
}

/******************************************************
*��������PP_rmtCfg_CfgEndRequest

*��  �Σ�

*����ֵ��

*��  ����remote config end request

*��  ע��
******************************************************/
static int PP_rmtCfg_CfgEndRequest(PrvtProt_task_t *task,PrvtProt_rmtCfg_t *rmtCfg)
{
	int msgdatalen;
	int res = 0;
	/*header*/
	memcpy(rmtCfg->pack.Header.sign,"**",2);
	rmtCfg->pack.Header.commtype.Byte = 0xe1;
	rmtCfg->pack.Header.opera = 0x02;
	rmtCfg->pack.Header.ver.Byte = task->version;
	rmtCfg->pack.Header.nonce  = PrvtPro_BSEndianReverse((uint32_t)task->nonce);
	rmtCfg->pack.Header.tboxid = PrvtPro_BSEndianReverse((uint32_t)task->tboxid);
	memcpy(&PP_rmtCfg_Pack, &rmtCfg->pack.Header, sizeof(PrvtProt_pack_Header_t));
	/*body*/
	memcpy(rmtCfg->pack.DisBody.aID,"100",3);
	rmtCfg->pack.DisBody.mID = PP_MID_CFG_END;
	rmtCfg->pack.DisBody.eventId = 0;//主动上行数据，eventid为0
	rmtCfg->pack.DisBody.eventTime = PrvtPro_getTimestamp();
	rmtCfg->pack.DisBody.expTime   = PP_rmtCfg.state.expTime;
	rmtCfg->pack.DisBody.ulMsgCnt++;	/* OPTIONAL */
	rmtCfg->pack.DisBody.appDataProVer = 256;
	rmtCfg->pack.DisBody.testFlag = 1;
	/*appdata*/
	AppData_rmtCfg.EndReq.configSuccess = rmtCfg->state.cfgsuccess;
	memcpy(AppData_rmtCfg.EndReq.mcuSw,AppData_rmtCfg.checkReq.mcuSw,AppData_rmtCfg.checkReq.mcuSwlen);
	AppData_rmtCfg.EndReq.mcuSwlen = AppData_rmtCfg.checkReq.mcuSwlen;
	memcpy(AppData_rmtCfg.EndReq.mpuSw,AppData_rmtCfg.checkReq.mpuSw,AppData_rmtCfg.checkReq.mpuSwlen);
	AppData_rmtCfg.EndReq.mpuSwlen = AppData_rmtCfg.checkReq.mpuSwlen;
	memcpy(AppData_rmtCfg.EndReq.configSw,AppData_rmtCfg.checkReq.configSw,AppData_rmtCfg.checkReq.configSwlen);
	AppData_rmtCfg.EndReq.configSwlen = AppData_rmtCfg.checkReq.configSwlen;
	memcpy(AppData_rmtCfg.EndReq.cfgVersion,AppData_rmtCfg.checkReq.cfgVersion,AppData_rmtCfg.checkReq.cfgVersionlen);
	AppData_rmtCfg.EndReq.cfgVersionlen = AppData_rmtCfg.checkReq.cfgVersionlen;

	if(0 != PrvtPro_msgPackageEncoding(ECDC_RMTCFG_END_REQ,PP_rmtCfg_Pack.msgdata,&msgdatalen,\
									   &rmtCfg->pack.DisBody,&AppData_rmtCfg))
	{
		log_e(LOG_HOZON, "uper error");
		return -1;
	}

	PP_rmtCfg_Pack.totallen = 18 + msgdatalen;
	PP_rmtCfg_Pack.Header.msglen = PrvtPro_BSEndianReverse((long)(18 + msgdatalen));

	return res;
}

/******************************************************
*��������PP_rmtCfg_ConnResp

*��  �Σ�

*����ֵ��

*��  ���� response of remote config request

*��  ע��
******************************************************/
static int PP_rmtCfg_ConnResp(PrvtProt_task_t *task,PrvtProt_rmtCfg_t *rmtCfg,PrvtProt_DisptrBody_t *MsgDataBody)
{
	int msgdatalen;
	int res = 0;
	/*header*/
	memcpy(rmtCfg->pack.Header.sign,"**",2);
	rmtCfg->pack.Header.commtype.Byte = 0xe1;
	rmtCfg->pack.Header.opera = 0x02;
	rmtCfg->pack.Header.ver.Byte = task->version;
	rmtCfg->pack.Header.nonce  = PrvtPro_BSEndianReverse((uint32_t)task->nonce);
	rmtCfg->pack.Header.tboxid = PrvtPro_BSEndianReverse((uint32_t)task->tboxid);
	memcpy(&PP_rmtCfg_Pack, &rmtCfg->pack.Header, sizeof(PrvtProt_pack_Header_t));
	/*body*/
	memcpy(rmtCfg->pack.DisBody.aID,"100",3);
	rmtCfg->pack.DisBody.eventId = MsgDataBody->eventId;
	rmtCfg->pack.DisBody.mID = PP_MID_CONN_CFG_RESP;
	rmtCfg->pack.DisBody.eventTime = PrvtPro_getTimestamp();
	rmtCfg->pack.DisBody.expTime   = MsgDataBody->expTime;
	rmtCfg->pack.DisBody.ulMsgCnt++;	/* OPTIONAL */
	rmtCfg->pack.DisBody.appDataProVer = 256;
	rmtCfg->pack.DisBody.testFlag = 1;

	/*appdata*/
	AppData_rmtCfg.connResp.configAccepted = 0;
	if(1 == rmtCfg->state.cfgAccept)
	{
		AppData_rmtCfg.connResp.configAccepted = 1;
	}

	if(0 != PrvtPro_msgPackageEncoding(ECDC_RMTCFG_CONN_RESP,PP_rmtCfg_Pack.msgdata,&msgdatalen,\
									   &rmtCfg->pack.DisBody,&AppData_rmtCfg))
	{
		log_e(LOG_HOZON, "uper error");
		return -1;
	}

	PP_rmtCfg_Pack.totallen = 18 + msgdatalen;
	PP_rmtCfg_Pack.Header.msglen = PrvtPro_BSEndianReverse((long)(18 + msgdatalen));

	return res;
}

/******************************************************
*��������PP_rmtCfg_ReadCfgResp

*��  �Σ�

*����ֵ��

*��  ���� response of remote read config request

*��  ע��
******************************************************/
static int PP_rmtCfg_ReadCfgResp(PrvtProt_task_t *task,PrvtProt_rmtCfg_t *rmtCfg,PrvtProt_DisptrBody_t *MsgDataBody)
{
	int msgdatalen;
	int res = 0;
	/*header*/
	memcpy(rmtCfg->pack.Header.sign,"**",2);
	rmtCfg->pack.Header.commtype.Byte = 0xe1;
	rmtCfg->pack.Header.opera = 0x02;
	rmtCfg->pack.Header.ver.Byte = task->version;
	rmtCfg->pack.Header.nonce  = PrvtPro_BSEndianReverse((uint32_t)task->nonce);
	rmtCfg->pack.Header.tboxid = PrvtPro_BSEndianReverse((uint32_t)task->tboxid);
	memcpy(&PP_rmtCfg_Pack, &rmtCfg->pack.Header, sizeof(PrvtProt_pack_Header_t));
	/*body*/
	memcpy(rmtCfg->pack.DisBody.aID,"100",3);
	rmtCfg->pack.DisBody.mID = PP_MID_READ_CFG_RESP;
	rmtCfg->pack.DisBody.eventId = MsgDataBody->eventId;
	rmtCfg->pack.DisBody.eventTime = PrvtPro_getTimestamp();
	rmtCfg->pack.DisBody.expTime   = MsgDataBody->expTime;
	rmtCfg->pack.DisBody.ulMsgCnt++;	/* OPTIONAL */
	rmtCfg->pack.DisBody.appDataProVer = 256;
	rmtCfg->pack.DisBody.testFlag = 1;
	/*appdata*/
	AppData_rmtCfg.ReadResp.result = 1;

	if(0 != PrvtPro_msgPackageEncoding(ECDC_RMTCFG_READ_RESP,PP_rmtCfg_Pack.msgdata,&msgdatalen,\
									   &rmtCfg->pack.DisBody,&AppData_rmtCfg))
	{
		log_e(LOG_HOZON, "uper error");
		return -1;
	}

	PP_rmtCfg_Pack.totallen = 18 + msgdatalen;
	PP_rmtCfg_Pack.Header.msglen = PrvtPro_BSEndianReverse((long)(18 + msgdatalen));

	return res;
}

/******************************************************
*������:PP_rmtCfg_SetCfgReq

*��  �Σ�

*����ֵ��

*��  �������� ����

*��  ע��
******************************************************/
void PP_rmtCfg_SetCfgReq(unsigned char req)
{
	PP_rmtCfg.state.req  = req;
	PP_rmtCfg.state.reqCnt = 0;
	PP_rmtCfg.state.expTime = -1;
	PP_rmtCfg.state.period = tm_get_time();
}

/******************************************************
*������:PP_rmtCfg_setCfgEnable

*��  �Σ�

*����ֵ��

*��  �������� ����

*��  ע��
******************************************************/
void PP_rmtCfg_setCfgEnable(unsigned char obj,unsigned char enable)
{
	switch(obj)
	{
		case 1:////T服务开启使能
		{
			AppData_rmtCfg.ReadResp.COMMON.actived = enable;
		}
		break;
		case 2://远程控制使能
		{
			AppData_rmtCfg.ReadResp.COMMON.rcEnabled = enable;
		}
		break;
		case 3://被盗追踪开关
		{
			AppData_rmtCfg.ReadResp.COMMON.svtEnabled = enable;
		}
		break;
		case 4://车辆状态开关
		{
			AppData_rmtCfg.ReadResp.COMMON.vsEnabled = enable;
		}
		break;
		case 5://呼叫中心开关
		{
			AppData_rmtCfg.ReadResp.COMMON.iCallEnabled = enable;
		}
		break;
		case 6://道路救援开关
		{
			AppData_rmtCfg.ReadResp.COMMON.bCallEnabled = enable;
		}
		break;
		case 7://紧急救援开关
		{
			AppData_rmtCfg.ReadResp.COMMON.eCallEnabled = enable;
		}
		break;
		case 8://数据采集开关
		{
			AppData_rmtCfg.ReadResp.COMMON.dcEnabled = enable;
		}
		break;
		case 9://远程诊断开关
		{
			AppData_rmtCfg.ReadResp.COMMON.dtcEnabled = enable;
		}
		break;
		case 10://行程开关
		{
			AppData_rmtCfg.ReadResp.COMMON.journeysEnabled = enable;
		}
		break;
		case 11://在线资讯开关
		{
			AppData_rmtCfg.ReadResp.COMMON.onlineInfEnabled = enable;
		}
		break;
		case 12://远程充电开关
		{
			AppData_rmtCfg.ReadResp.COMMON.rChargeEnabled = enable;
		}
		break;
		case 13://蓝牙钥匙开关
		{
			AppData_rmtCfg.ReadResp.COMMON.btKeyEntryEnabled = enable;
		}
		break;	
		case 14://车辆授权服务开关
		{
			AppData_rmtCfg.ReadResp.COMMON.carEmpowerEnabled = enable;
		}
		break;
		case 15://事件上报服务开关
		{
			AppData_rmtCfg.ReadResp.COMMON.eventReportEnabled = enable;
		}
		break;
		case 16://车辆报警服务开关
		{
			AppData_rmtCfg.ReadResp.COMMON.carAlarmEnabled = enable;
		}
		break;	
		case 17://心跳超时时间(s)
		{
			AppData_rmtCfg.ReadResp.COMMON.heartbeatTimeout = enable;
		}
		break;
		case 18://休眠心跳超时时间(s)
		{
			AppData_rmtCfg.ReadResp.COMMON.dormancyHeartbeatTimeout = enable;
		}
		break;
		case 19://国标报文采集打 包周期(s)
		{
			AppData_rmtCfg.ReadResp.COMMON.infoCollectCycle = enable;
		}
		break;
		case 20://国标报文定时上 报周期(s)
		{
			AppData_rmtCfg.ReadResp.COMMON.regularUpCycle = enable;
		}
		break;
		default:
		break;
	}
	AppData_rmtCfg.ReadResp.cfgsuccess = 1;
	AppData_rmtCfg.ReadResp.COMMON.commonConfigValid = 1;
	(void)cfg_set_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_ST,&AppData_rmtCfg.ReadResp.cfgsuccess,1);
	(void)cfg_set_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_COMM,&AppData_rmtCfg.ReadResp.COMMON,256);
}

void PP_rmtCfg_setCfgapn1(unsigned char obj,const void *data1,const void *data2)
{
	switch(obj)
	{
		case 1:  //TSP APN Address
		{
			strcpy((char *)AppData_rmtCfg.ReadResp.APN1.tspAddr,(const char*)data1);
			AppData_rmtCfg.ReadResp.APN1.tspAddrlen = strlen((const char*)data1);
			strcpy((char *)AppData_rmtCfg.ReadResp.APN1.tspPort,(const char*)data2);
			setsockproxy_bdlAddrPort((char*)AppData_rmtCfg.ReadResp.APN1.tspAddr, \
				(char*)AppData_rmtCfg.ReadResp.APN1.tspPort);
		}
		break;
		case 2:
		{
			strcpy((char *)AppData_rmtCfg.ReadResp.APN1.tspUser,(const char*)data1);
		}
		break;
		case 3:
		{
			strcpy((char *)AppData_rmtCfg.ReadResp.APN1.tspPass,(const char*)data1);
		}
		break;
		case 4:
		{
			strcpy((char *)AppData_rmtCfg.ReadResp.APN1.tspIP,(const char*)data1);
			AppData_rmtCfg.ReadResp.APN1.tspIPlen = strlen((const char*)data1);
			strcpy((char *)AppData_rmtCfg.ReadResp.APN1.tspPort,(const char*)data2);
			setsockproxy_bdlAddrPort((char*)AppData_rmtCfg.ReadResp.APN1.tspIP, \
				(char*)AppData_rmtCfg.ReadResp.APN1.tspPort);
		}
		break;
		case 5:
		{
			strcpy((char *)AppData_rmtCfg.ReadResp.APN1.tspSms,(const char*)data1);
		}
		break;
		case 6:
		{
			strcpy((char *)AppData_rmtCfg.ReadResp.APN1.certAddress,(const char*)data1);
			AppData_rmtCfg.ReadResp.APN1.certAddresslen = strlen((const char*)data1);
			strcpy((char *)AppData_rmtCfg.ReadResp.APN1.certPort,(const char*)data2);
			setsockproxy_sgAddrPort((char*)AppData_rmtCfg.ReadResp.APN1.certAddress, \
				(char*)AppData_rmtCfg.ReadResp.APN1.certPort);
		}
		break;
		default:
		break;	
	}
	AppData_rmtCfg.ReadResp.cfgsuccess = 1;
	AppData_rmtCfg.ReadResp.APN1.apn1ConfigValid = 1;
	(void)cfg_set_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_ST,&AppData_rmtCfg.ReadResp.cfgsuccess,1);
	(void)cfg_set_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_APN1,&AppData_rmtCfg.ReadResp.APN1,256);
}

void PP_rmtCfg_setCfgficm(unsigned char obj,const void *data)
{
	switch(obj)
	{
	  	case 1:
	  	{
	  		strcpy((char *)AppData_rmtCfg.ReadResp.FICM.token,(const char*)data);
	  	}
	  	break;
	  	case 2:
	  	{
	  		strcpy((char *)AppData_rmtCfg.ReadResp.FICM.userID,(const char*)data);
	  	}
	  	break;
	  	case 3:
	  	{
	  		AppData_rmtCfg.ReadResp.FICM.directConnEnable = atoi(data);
	  	}
	  	break;
	  	case 4:
	  	{
	  		strcpy((char *)AppData_rmtCfg.ReadResp.FICM.address,(const char*)data);
	  	}
	  	break;
	  	case 5:
	  	{
	  		strcpy((char *)AppData_rmtCfg.ReadResp.FICM.port,(const char*)data);
	  	}
	  	break;
		default:
		break;
	}
	(void)cfg_set_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_FICM,&AppData_rmtCfg.ReadResp.FICM,256);
}

/******************************************************
*������:PP_rmtCfg_SetmcuSw

*��  �Σ�

*����ֵ��

*��  ����

*��  ע��
******************************************************/
void PP_rmtCfg_SetmcuSw(const char *mcuSw)
{
	memset(AppData_rmtCfg.checkReq.mcuSw,0 , 11);
	memcpy(AppData_rmtCfg.checkReq.mcuSw,mcuSw,strlen(mcuSw));
	AppData_rmtCfg.checkReq.mcuSwlen = strlen(mcuSw);
	if (cfg_set_user_para(CFG_ITEM_HOZON_TSP_MCUSW, AppData_rmtCfg.checkReq.mcuSw, sizeof(AppData_rmtCfg.checkReq.mcuSw)))
	{
		log_e(LOG_HOZON, "save mcuSw failed");
	}
}

/******************************************************
*������:PP_rmtCfg_SetmpuSw

*��  �Σ�

*����ֵ��

*��  ����

*��  ע��
******************************************************/
void PP_rmtCfg_SetmpuSw(const char *mpuSw)
{
	memset(AppData_rmtCfg.checkReq.mpuSw,0 , 11);
	memcpy(AppData_rmtCfg.checkReq.mpuSw,mpuSw,strlen(mpuSw));
	AppData_rmtCfg.checkReq.mpuSwlen = strlen(mpuSw);
	if (cfg_set_user_para(CFG_ITEM_HOZON_TSP_MPUSW, AppData_rmtCfg.checkReq.mpuSw, sizeof(AppData_rmtCfg.checkReq.mpuSw)))
	{
		log_e(LOG_HOZON, "save mpuSw failed");
	}
}

/******************************************************
*������:PP_rmtCfg_Seticcid

*��  �Σ�

*����ֵ��

*��  ����

*��  ע��������
******************************************************/
void PP_rmtCfg_Seticcid(const char *iccid)
{
	memcpy(AppData_rmtCfg.checkReq.iccID,iccid,strlen(iccid));
	AppData_rmtCfg.checkReq.iccIDlen = strlen(iccid);
}

/******************************************************
*������:PP_rmtCfg_ShowCfgPara

*��  �Σ�

*����ֵ��

*��  ����

*��  ע��������
******************************************************/
void PP_rmtCfg_ShowCfgPara(void)
{
	unsigned int len;
	len =18;
	cfg_get_user_para(CFG_ITEM_GB32960_VIN,AppData_rmtCfg.checkReq.vehicleVin,&len);
	len = 33;
	cfg_get_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_VER,AppData_rmtCfg.checkReq.cfgVersion,&len);
	log_o(LOG_HOZON, "/******************************/");
	log_o(LOG_HOZON, "       remote  cfg parameter    ");
	log_o(LOG_HOZON, "/******************************/");
	log_o(LOG_HOZON, "vehicleVin = %s",AppData_rmtCfg.checkReq.vehicleVin);
	log_o(LOG_HOZON, "mcuSw = %s",AppData_rmtCfg.checkReq.mcuSw);
	log_o(LOG_HOZON, "mpuSw = %s",AppData_rmtCfg.checkReq.mpuSw);
	log_o(LOG_HOZON, "ICCID = %s",AppData_rmtCfg.checkReq.iccID);

	char cfgversion[256] = {0};
	getPP_rmtCfg_cfgVersion(cfgversion);
	log_o(LOG_HOZON, "cfgVersion = %s",cfgversion);
	log_o(LOG_HOZON, "configSw = %s",AppData_rmtCfg.checkReq.configSw);
	unsigned char Mac[32];
	BleGetMac(Mac);
	PP_rmtCfg_HexToStr(AppData_rmtCfg.checkReq.btMacAddr,Mac,6);
	log_o(LOG_HOZON, "btMacAddr = %s",AppData_rmtCfg.checkReq.btMacAddr);

	log_o(LOG_HOZON, "AppData_rmtCfg.ReadResp.cfgsuccess = %d",AppData_rmtCfg.ReadResp.cfgsuccess);
	log_o(LOG_HOZON, "\n/* FICM info */");
	log_o(LOG_HOZON, "ficmConfigValid = %d",AppData_rmtCfg.ReadResp.FICM.ficmConfigValid);
	log_o(LOG_HOZON, "FICM.token = %s",AppData_rmtCfg.ReadResp.FICM.token);
	log_o(LOG_HOZON, "FICM.userID = %s",AppData_rmtCfg.ReadResp.FICM.userID);
	log_o(LOG_HOZON, "FICM.directConnEnable = %d",AppData_rmtCfg.ReadResp.FICM.directConnEnable);
	log_o(LOG_HOZON, "FICM.address = %s",AppData_rmtCfg.ReadResp.FICM.address);
	log_o(LOG_HOZON, "FICM.port = %s",AppData_rmtCfg.ReadResp.FICM.port);
	log_o(LOG_HOZON, "FICM info length = %d\n",sizeof(App_rmtCfg_FICM_t));

	log_o(LOG_HOZON, "\n/* APN1 info */");
	log_o(LOG_HOZON, "apn1ConfigValid = %d",AppData_rmtCfg.ReadResp.APN1.apn1ConfigValid);
	log_o(LOG_HOZON, "APN1.tspAddr = %s",AppData_rmtCfg.ReadResp.APN1.tspAddr);
	log_o(LOG_HOZON, "APN1.tspUser = %s",AppData_rmtCfg.ReadResp.APN1.tspUser);
	log_o(LOG_HOZON, "APN1.tspPass = %s",AppData_rmtCfg.ReadResp.APN1.tspPass);
	log_o(LOG_HOZON, "APN1.tspIP = %s",AppData_rmtCfg.ReadResp.APN1.tspIP);
	log_o(LOG_HOZON, "APN1.tspSms = %s",AppData_rmtCfg.ReadResp.APN1.tspSms);
	log_o(LOG_HOZON, "APN1.tspPort = %s",AppData_rmtCfg.ReadResp.APN1.tspPort);
	log_o(LOG_HOZON, "APN1.certAddress = %s",AppData_rmtCfg.ReadResp.APN1.certAddress);
	log_o(LOG_HOZON, "APN1.certPort = %s",AppData_rmtCfg.ReadResp.APN1.certPort);
	log_o(LOG_HOZON, "APN1 info length = %d\n",sizeof(App_rmtCfg_APN1_t));

	log_o(LOG_HOZON, "\n/* APN2 info */");
	log_o(LOG_HOZON, "apn2ConfigValid = %d",AppData_rmtCfg.ReadResp.APN2.apn2ConfigValid);
	log_o(LOG_HOZON, "APN2.apn2Address = %s",AppData_rmtCfg.ReadResp.APN2.apn2Address);
	log_o(LOG_HOZON, "APN2.apn2User = %s",AppData_rmtCfg.ReadResp.APN2.apn2User);
	log_o(LOG_HOZON, "APN2.apn2Pass = %s",AppData_rmtCfg.ReadResp.APN2.apn2Pass);
	log_o(LOG_HOZON, "APN2 info length = %d\n",sizeof(App_rmtCfg_APN2_t));

	log_o(LOG_HOZON, "\n/* COMMON info */");
	len = 256;
    cfg_get_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_COMM,&AppData_rmtCfg.ReadResp.COMMON,&len);
	log_o(LOG_HOZON, "commonConfigValid = %d",AppData_rmtCfg.ReadResp.COMMON.commonConfigValid);
	log_o(LOG_HOZON, "COMMON.actived = %d",AppData_rmtCfg.ReadResp.COMMON.actived);
	log_o(LOG_HOZON, "COMMON.rcEnabled = %d",AppData_rmtCfg.ReadResp.COMMON.rcEnabled);
	log_o(LOG_HOZON, "COMMON.svtEnabled = %d",AppData_rmtCfg.ReadResp.COMMON.svtEnabled);
	log_o(LOG_HOZON, "COMMON.vsEnabled = %d",AppData_rmtCfg.ReadResp.COMMON.vsEnabled);
	log_o(LOG_HOZON, "COMMON.iCallEnabled = %d",AppData_rmtCfg.ReadResp.COMMON.iCallEnabled);
	log_o(LOG_HOZON, "COMMON.bCallEnabled = %d",AppData_rmtCfg.ReadResp.COMMON.bCallEnabled);
	log_o(LOG_HOZON, "COMMON.eCallEnabled = %d",AppData_rmtCfg.ReadResp.COMMON.eCallEnabled);
	log_o(LOG_HOZON, "COMMON.dcEnabled = %d",AppData_rmtCfg.ReadResp.COMMON.dcEnabled);
	log_o(LOG_HOZON, "COMMON.dtcEnabled = %d",AppData_rmtCfg.ReadResp.COMMON.dtcEnabled);
	log_o(LOG_HOZON, "COMMON.journeysEnabled = %d",AppData_rmtCfg.ReadResp.COMMON.journeysEnabled);
	log_o(LOG_HOZON, "COMMON.onlineInfEnabled = %d",AppData_rmtCfg.ReadResp.COMMON.onlineInfEnabled);
	log_o(LOG_HOZON, "COMMON.rChargeEnabled = %d",AppData_rmtCfg.ReadResp.COMMON.rChargeEnabled);
	log_o(LOG_HOZON, "COMMON.btKeyEntryEnabled = %d",AppData_rmtCfg.ReadResp.COMMON.btKeyEntryEnabled);
	log_o(LOG_HOZON, "COMMON.carEmpowerEnabled  = %d",AppData_rmtCfg.ReadResp.COMMON.carEmpowerEnabled);
	log_o(LOG_HOZON, "COMMON.eventReportEnabled  = %d",AppData_rmtCfg.ReadResp.COMMON.eventReportEnabled);
	log_o(LOG_HOZON, "COMMON.carAlarmEnabled  = %d",AppData_rmtCfg.ReadResp.COMMON.carAlarmEnabled);
	log_o(LOG_HOZON, "COMMON.heartbeatTimeout  = %d",AppData_rmtCfg.ReadResp.COMMON.heartbeatTimeout);
	log_o(LOG_HOZON, "COMMON.dormancyHeartbeatTimeout  = %d",AppData_rmtCfg.ReadResp.COMMON.dormancyHeartbeatTimeout);
	log_o(LOG_HOZON, "COMMON.infoCollectCycle  = %d",AppData_rmtCfg.ReadResp.COMMON.infoCollectCycle);
	log_o(LOG_HOZON, "COMMON.regularUpCycle  = %d",AppData_rmtCfg.ReadResp.COMMON.regularUpCycle);
	log_o(LOG_HOZON, "COMMON info length = %d\n",sizeof(App_rmtCfg_COMMON_t));

	log_o(LOG_HOZON, "\n/* EXTEND info */");
	log_o(LOG_HOZON, "extendConfigValid = %d",AppData_rmtCfg.ReadResp.EXTEND.extendConfigValid);
	log_o(LOG_HOZON, "EXTEND.ecallNO = %s",AppData_rmtCfg.ReadResp.EXTEND.ecallNO);
	log_o(LOG_HOZON, "EXTEND.bcallNO = %s",AppData_rmtCfg.ReadResp.EXTEND.bcallNO);
	log_o(LOG_HOZON, "EXTEND.ccNO = %s",AppData_rmtCfg.ReadResp.EXTEND.ccNO);
	log_o(LOG_HOZON, "EXTEND info length = %d\n",sizeof(App_rmtCfg_EXTEND_t));
}

#if 0
/******************************************************
*������:PP_rmtCfg_strcmp

*��  �Σ�

*����ֵ��

*��  �����ַ����Ƚ�

*��  ע��
******************************************************/
static int PP_rmtCfg_strcmp(unsigned char* str1,unsigned char* str2,int len)
{
	int i;
	for(i = 0;i < len;i++)
	{
		if(str1[i] != str2[i])
		{
			return -1;
		}
	}
	return 0;
}
#endif

/******************************************************
*������:PP_rmtCfg_reset

*��  �Σ�

*����ֵ��

*��  �����ַ����Ƚ�

*��  ע��
******************************************************/
static void PP_rmtCfg_reset(PrvtProt_rmtCfg_t *rmtCfg)
{
	rmtCfg->state.CfgSt = 0;
	rmtCfg->state.cfgAccept = 0;
	rmtCfg->state.cfgsuccess = 0;
	rmtCfg->state.needUpdata = 0;
	rmtCfg->state.req = 0;
	rmtCfg->state.period = 0;
	rmtCfg->state.reqCnt = 0;
	rmtCfg->state.waitSt = 0;
	rmtCfg->state.waittime = 0;
}

/******************************************************
*��������PP_rmtCfg_send_cb

*��  �Σ�

*����ֵ��

*��  ����

*��  ע��
******************************************************/
static void PP_rmtCfg_send_cb(void * para)
{
	PrvtProt_TxInform_t *TxInform_ptr = (PrvtProt_TxInform_t*)para;
	log_i(LOG_HOZON, "aid = %d",TxInform_ptr->aid);
	log_i(LOG_HOZON, "mid = %d",TxInform_ptr->mid);
	log_i(LOG_HOZON, "pakgtype = %d",TxInform_ptr->pakgtype);
	log_i(LOG_HOZON, "eventtime = %d",TxInform_ptr->eventtime);
	log_i(LOG_HOZON, "successflg = %d",TxInform_ptr->successflg);
	log_i(LOG_HOZON, "failresion = %d",TxInform_ptr->failresion);
	log_i(LOG_HOZON, "txfailtime = %d",TxInform_ptr->txfailtime);

	switch(TxInform_ptr->mid)
	{
		case PP_MID_CONN_CFG_RESP:
		{
			if(PP_TXPAKG_SUCCESS == TxInform_ptr->successflg)
			{
				PP_rmtCfg.state.req  = 1;
				PP_rmtCfg.state.reqCnt = 0;
				PP_rmtCfg.state.period = tm_get_time();
			}
		}
		break;
		case PP_MID_READ_CFG_RESP:
		{

		}
		break;
		case PP_MID_CHECK_CFG_REQ:
		{
			if(PP_TXPAKG_FAIL == TxInform_ptr->successflg)//������ʧ��
			{
				PP_rmtCfg.state.req = 0;
				PP_rmtCfg.state.reqCnt = 0;
				PP_rmtCfg.state.period = tm_get_time();
				PP_rmtCfg.state.CfgSt = PP_RMTCFG_CFG_IDLE;
				PP_rmtCfg.state.waitSt = PP_RMTCFG_WAIT_IDLE;
			}
		}
		break;
		case PP_MID_GET_CFG_REQ:
		{
			if(PP_TXPAKG_FAIL == TxInform_ptr->successflg)//������ʧ��
			{
				PP_rmtCfg.state.req = 0;
				PP_rmtCfg.state.reqCnt = 0;
				PP_rmtCfg.state.period = tm_get_time();
				PP_rmtCfg.state.waitSt = PP_RMTCFG_WAIT_IDLE;
				PP_rmtCfg.state.CfgSt = PP_RMTCFG_CFG_IDLE;
			}
		}
		break;
		case PP_MID_CFG_END:
		{
			if(PP_TXPAKG_SUCCESS == TxInform_ptr->successflg)//����ok
			{
				log_i(LOG_HOZON, "remote config take effect\r\n");
				if(PP_rmtCfg.state.cfgsuccess == 1)
				{
					PP_rmtCfg.state.cfgsuccess = 0;
					AppData_rmtCfg.ReadResp.cfgsuccess = 1;
					//配置参数写入flash
					(void)cfg_set_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_ST,&AppData_rmtCfg.ReadResp.cfgsuccess,1);
					(void)cfg_set_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_VER,AppData_rmtCfg.ReadResp.cfgVersion,33);
					(void)cfg_set_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_APN1,&AppData_rmtCfg.ReadResp.APN1,256);
					(void)cfg_set_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_APN2,&AppData_rmtCfg.ReadResp.APN2,256);
					(void)cfg_set_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_FICM,&AppData_rmtCfg.ReadResp.FICM,256);
					(void)cfg_set_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_COMM,&AppData_rmtCfg.ReadResp.COMMON,256);
					(void)cfg_set_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_EXT,&AppData_rmtCfg.ReadResp.EXTEND,256);
					memcpy(AppData_rmtCfg.checkReq.cfgVersion,AppData_rmtCfg.checkResp.cfgVersion,AppData_rmtCfg.checkResp.cfgVersionlen);
					AppData_rmtCfg.checkReq.cfgVersionlen = AppData_rmtCfg.checkResp.cfgVersionlen;
					PP_rmtCfg_settbox();
				}
			}
			PP_rmtCfg.state.waitSt  = PP_RMTCFG_WAIT_IDLE;
			PP_rmtCfg.state.CfgSt 	= PP_RMTCFG_CFG_IDLE;
			PP_rmtCfg.state.req 	= 0;
			PP_rmtCfg.state.reqCnt 	= 0;
		}
		break;
		default:
		break;
	}

	TxInform_ptr->idleflag = 0;
}

/*
* 获取iccid
*/
uint8_t PP_rmtCfg_getIccid(uint8_t* iccid)
{
	if(!AppData_rmtCfg.checkReq.iccID[0])
	{//get iccid
		(void)PrvtProtCfg_get_iccid((char *)(AppData_rmtCfg.checkReq.iccID));
	}
	else
	{
		PP_rmtCfg.state.iccidValid = 1;
	}

	if(PP_rmtCfg.state.iccidValid)
	{
		memcpy(iccid,AppData_rmtCfg.checkReq.iccID,21);
	}

	return PP_rmtCfg.state.iccidValid;
}

/*
* 下发配置
*/
void PP_rmtCfg_settbox(void)
{
	int ret;	
	//FICMConfigSettings
	if(AppData_rmtCfg.ReadResp.FICM.ficmConfigValid == 1)
	{
		if(PP_rmtCfg_is_empty(AppData_rmtCfg.ReadResp.FICM.token,33) == 1)  //国标Token
		{
			
		}
		if(PP_rmtCfg_is_empty(AppData_rmtCfg.ReadResp.FICM.userID,33) == 1) //国标用户
		{
			
		}
		if(PP_rmtCfg_is_empty(AppData_rmtCfg.ReadResp.FICM.address,33) == 1)   //国标地址
		{
			
		}
		if(PP_rmtCfg_is_empty(AppData_rmtCfg.ReadResp.FICM.port,7) == 1)      //国标端口
		{
		}
	}
	
	//APN1ConfigSettings APN1 
	if(PP_rmtCfg.state.apn1tspaddrchangedflag)
	{
		PP_rmtCfg.state.apn1tspaddrchangedflag = 0;
		setsockproxy_bdlAddrPort((char*)AppData_rmtCfg.ReadResp.APN1.tspAddr, \
				(char*)AppData_rmtCfg.ReadResp.APN1.tspPort);
	}

	if(PP_rmtCfg.state.apn1certaddrchangeflag)
	{
		PP_rmtCfg.state.apn1certaddrchangeflag = 0;
		setsockproxy_sgAddrPort((char*)AppData_rmtCfg.ReadResp.APN1.certAddress, \
				(char*)AppData_rmtCfg.ReadResp.APN1.certPort);
	}

	if(PP_rmtCfg.state.tspSMSchangeflag)
	{
		PP_rmtCfg.state.tspSMSchangeflag = 0;
		ret = cfg_set_para(CFG_ITEM_WHITE_LIST, (unsigned char *)AppData_rmtCfg.ReadResp.APN1.tspSms, 512);
		if (ret != 0)
		{
			log_e(LOG_HOZON,"set whitelist failed,ret=%d\r\n", ret);
		}
	}

	//APN2ConfigSettings APN2
	if(AppData_rmtCfg.ReadResp.APN2.apn2ConfigValid == 1)
	{
		if(PP_rmtCfg_is_empty(AppData_rmtCfg.ReadResp.APN2.apn2Address,33) == 1) //配置APN2地址
		{
		}
		if(PP_rmtCfg_is_empty(AppData_rmtCfg.ReadResp.APN2.apn2User,33) == 1) //配置APN2用户名
		{
		}
		if(PP_rmtCfg_is_empty(AppData_rmtCfg.ReadResp.APN2.apn2Pass,17) == 1)//配置APN2密码
		{
		}
	}

	//ExtendConfigSettings
	if(AppData_rmtCfg.ReadResp.EXTEND.extendConfigValid == 1)
	{
		unsigned char xcall[32];
		uint32_t len = 32;
		if(PP_rmtCfg_is_empty(AppData_rmtCfg.ReadResp.EXTEND.bcallNO,17) == 1)  //设置BCALL
		{
			cfg_get_para(CFG_ITEM_BCALL,xcall,&len);
			if(strncmp(( char *)AppData_rmtCfg.ReadResp.EXTEND.bcallNO,( char *)xcall,17) != 0)
			{
				memset(xcall,0,32);
				strncpy((char *)xcall,(char *)AppData_rmtCfg.ReadResp.EXTEND.bcallNO,17);
				(void)cfg_set_para(CFG_ITEM_BCALL,(void *)xcall,32);
			}
		}
		if(PP_rmtCfg_is_empty(AppData_rmtCfg.ReadResp.EXTEND.ecallNO,17) == 1) //设置ECALL
		{
			cfg_get_para(CFG_ITEM_ECALL,xcall,&len);
			if(strncmp(( char *)AppData_rmtCfg.ReadResp.EXTEND.ecallNO,(char *)xcall,17) != 0)
			{
				memset(xcall,0,32);
				strncpy((char *)xcall,(char *)AppData_rmtCfg.ReadResp.EXTEND.ecallNO,17);
				cfg_set_para(CFG_ITEM_ECALL,(void *)xcall,32);
			}
		}
		if(PP_rmtCfg_is_empty(AppData_rmtCfg.ReadResp.EXTEND.ccNO,17) == 1) //设置ICALL
		{
			cfg_get_para(CFG_ITEM_ICALL,xcall,&len);
			if(strncmp((char *)AppData_rmtCfg.ReadResp.EXTEND.ccNO,( char *)xcall,17) != 0)
			{
				memset(xcall,0,32);
				strncpy((char *)xcall,(char *)AppData_rmtCfg.ReadResp.EXTEND.ccNO,17);
				(void)cfg_set_para(CFG_ITEM_ICALL,(void *)xcall,32);
			}
		}
	}
	
}
uint8_t PP_rmtCfg_is_empty(uint8_t *dt,int len)
{
	int i;
	for(i=0;i<len;i++)
	{
		if(dt[i] != 0)
		{
			return 1;
		}
	}
	return 0;
}

//国标直连开关
uint8_t PP_rmtCfg_enable_directConnEnable(void)
{
	//return AppData_rmtCfg.ReadResp.FICM.directConnEnable;
	return 1;
}


uint8_t PP_rmtCfg_enable_remotecontorl(void)
{
	//return AppData_rmtCfg.ReadResp.COMMON.rcEnabled;
	return 1;
}

uint8_t PP_rmtCfg_enable_icall(void)
{
	//return AppData_rmtCfg.ReadResp.COMMON.iCallEnabled;
	return 1;
}

uint8_t PP_rmtCfg_enable_bcall(void)
{
	//return AppData_rmtCfg.ReadResp.COMMON.bCallEnabled;
	return 1;
}

uint8_t PP_rmtCfg_enable_ecall(void)
{
	//return AppData_rmtCfg.ReadResp.COMMON.eCallEnabled;
	return 1;
}

uint8_t PP_rmtCfg_enable_actived(void)
{
	//return AppData_rmtCfg.ReadResp.COMMON.actived;
	return 1;
}
uint8_t PP_rmtCfg_enable_dtcEnabled(void)
{
	//return AppData_rmtCfg.ReadResp.COMMON.dtcEnabled;
	return 1;
}

uint8_t PP_rmtCfg_enable_dcEnabled(void)
{
	//return AppData_rmtCfg.ReadResp.COMMON.dcEnabled;
	return 1;
}

uint8_t PP_rmtCfg_enable_rChargeEnabled(void)
{
	//return AppData_rmtCfg.ReadResp.COMMON.rChargeEnabled;
	return 1;
}
uint8_t PP_rmtCfg_enable_svtEnabled(void)
{
	//return AppData_rmtCfg.ReadResp.COMMON.svtEnabled;
	return 1;
}
uint8_t PP_rmtCfg_enable_vsEnabled(void)
{
	//return AppData_rmtCfg.ReadResp.COMMON.vsEnabled;
	return 1;
}
uint8_t PP_rmtCfg_enable_btKeyEntryEnabled(void)
{
	//return AppData_rmtCfg.ReadResp.COMMON.btKeyEntryEnabled;
	return 1;
}
uint8_t PP_rmtCfg_enable_journeysEnabled(void)
{
	//return AppData_rmtCfg.ReadResp.COMMON.journeysEnabled;
	return 1;
}

/*
* 获取心跳超时时间配置
*/
int getPP_rmtCfg_heartbeatTimeout(void)
{
	int hbtimeout = 0;
	if(1 == AppData_rmtCfg.ReadResp.COMMON.commonConfigValid)
	{
		hbtimeout = AppData_rmtCfg.ReadResp.COMMON.heartbeatTimeout;
	}
	
	return hbtimeout;
}

/*
* 获取tsp ip addr和port
*/
void getPP_rmtCfg_tspAddrPort(char* addr,int* port)
{
	unsigned int len;
	len = 256;
	cfg_get_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_APN1,&AppData_rmtCfg.ReadResp.APN1,&len);
	if(1 == AppData_rmtCfg.ReadResp.APN1.apn1ConfigValid)
	{
		memcpy(addr,(char*)AppData_rmtCfg.ReadResp.APN1.tspAddr, \
								AppData_rmtCfg.ReadResp.APN1.tspAddrlen);
		*port = atoi((const char*)AppData_rmtCfg.ReadResp.APN1.tspPort);
	}
#if 0
	log_i(LOG_HOZON,"AppData_rmtCfg.ReadResp.APN1.apn1ConfigValid = %d",AppData_rmtCfg.ReadResp.APN1.apn1ConfigValid);
	log_i(LOG_HOZON,"AppData_rmtCfg.ReadResp.APN1.tspAddr = %s",AppData_rmtCfg.ReadResp.APN1.tspAddr);
	log_i(LOG_HOZON,"AppData_rmtCfg.ReadResp.APN1.tspAddrlen = %d",AppData_rmtCfg.ReadResp.APN1.tspAddrlen);
	log_i(LOG_HOZON,"AppData_rmtCfg.ReadResp.APN1.tspPort = %s",AppData_rmtCfg.ReadResp.APN1.tspPort);
#endif
}

/*
* 获取cert ip addr和port
*/
void getPP_rmtCfg_certAddrPort(char* addr,int* port)
{
	unsigned int len;
	len = 256;
	cfg_get_user_para(CFG_ITEM_HOZON_TSP_RMTCFG_APN1,&AppData_rmtCfg.ReadResp.APN1,&len);
	if(1 == AppData_rmtCfg.ReadResp.APN1.apn1ConfigValid)
	{
		memcpy(addr,(char*)AppData_rmtCfg.ReadResp.APN1.certAddress, \
							AppData_rmtCfg.ReadResp.APN1.certAddresslen);
		*port = atoi((const char*)AppData_rmtCfg.ReadResp.APN1.certPort);
	}
#if 0
	log_i(LOG_HOZON,"AppData_rmtCfg.ReadResp.APN1.apn1ConfigValid = %d",AppData_rmtCfg.ReadResp.APN1.apn1ConfigValid);
	log_i(LOG_HOZON,"AppData_rmtCfg.ReadResp.APN1.certAddress = %s",AppData_rmtCfg.ReadResp.APN1.certAddress);
	log_i(LOG_HOZON,"AppData_rmtCfg.ReadResp.APN1.certAddresslen = %d",AppData_rmtCfg.ReadResp.APN1.certAddresslen);
	log_i(LOG_HOZON,"AppData_rmtCfg.ReadResp.APN1.certPort = %s",AppData_rmtCfg.ReadResp.APN1.certPort);
#endif
}

/*无符号长整形转字符型*/
int PP_rmtCfg_ultoa(unsigned long value, char *string, int radix)
{
	char tmp[33] = {0};
	char *tp = tmp;
	int i;

	if (radix > 36 || radix <= 1)
	{
		return -1;
	}

	if(value != 0)
	{
		while(value)
		{
			i = value % radix;
			value = value / radix;
			if (i < 10)
			*tp++ = i +'0';
			else
			*tp++ = i + 'a' - 10;
		}

		while(tp > tmp)
		{
			*string++ = *(--tp);
		}
	}
	else
	{
		*string++ = '0';
	}

	*string = 0;

	return 0;
}

/*
* 获取配置版本
*/
void getPP_rmtCfg_cfgVersion(char* ver)
{
	int i,j;
	uint32_t tempVal;
	char stringVal[33] = {0};
	char *ver_tp = ver;

	for(i = 0;i < sizeof(AppData_rmtCfg.checkReq.cfgVersion)/4;i++)
	{
		tempVal = 0;
		memset(stringVal,0,sizeof(stringVal));
		tempVal |= ((uint32_t)AppData_rmtCfg.checkReq.cfgVersion[4*i]) << 24;
		tempVal |= ((uint32_t)AppData_rmtCfg.checkReq.cfgVersion[4*i+1]) << 16;
		tempVal |= ((uint32_t)AppData_rmtCfg.checkReq.cfgVersion[4*i+2]) << 8;
		tempVal |= (uint32_t)AppData_rmtCfg.checkReq.cfgVersion[4*i+3];
		PP_rmtCfg_ultoa(tempVal,stringVal,10);

		for(j=0;j<strlen(stringVal);j++)
		{
			*ver_tp++ = stringVal[j];
		}
		
		if(i < 7)
		{
			*ver_tp++ = '.';
		}
	}

	*ver_tp = 0;
}

static void PP_rmtCfg_HexToStr(uint8_t *pbDest, uint8_t *pbSrc, int nLen)
{
	char ddl,ddh;
	int i;
	for (i=0; i<nLen; i++)
	{
		ddh = 48 + pbSrc[i] / 16;
		ddl = 48 + pbSrc[i] % 16;
		if (ddh > 57) ddh = ddh + 7;
		if (ddl > 57) ddl = ddl + 7;
		pbDest[i*2] = ddh;
		pbDest[i*2+1] = ddl;
	}

	pbDest[nLen*2] = 0;
}