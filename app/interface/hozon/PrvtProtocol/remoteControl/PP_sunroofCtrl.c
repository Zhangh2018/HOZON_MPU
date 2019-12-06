/******************************************************
文件名：	PP_autodoorCtrl.c

描述：	企业私有协议（浙江合众）
Data			Vasion			author
2018/1/10		V1.0			liujian
*******************************************************/

/*******************************************************
description： include the header file
*******************************************************/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include  <errno.h>
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
#include "Bodyinfo.h"
#include "per_encoder.h"
#include "per_decoder.h"

#include "init.h"
#include "log.h"
#include "list.h"
#include "ble.h"

#include "../../support/protocol.h"
#include "gb32960_api.h"
#include "hozon_SP_api.h"
#include "hozon_PP_api.h"
#include "shell_api.h"
#include "../PrvtProt_shell.h"
#include "../PrvtProt_EcDc.h"
#include "../PrvtProt.h"
#include "../PrvtProt_cfg.h"
#include "PP_rmtCtrl.h"
#include "../../../gb32960/gb32960.h"
#include "PP_canSend.h"
#include "../PrvtProt_SigParse.h"
#include "PPrmtCtrl_cfg.h"
#include "../PrvtProt_lock.h"

#include "PP_sunroofCtrl.h"

#define PP_SUNROOFOPEN  1
#define PP_SUNROOFCLOSE 2
#define PP_SUNROOFUPWARP   3
#define PP_SUNROOFSTOP     4
typedef struct
{
	PrvtProt_pack_Header_t	Header;
	PrvtProt_DisptrBody_t	DisBody;
}__attribute__((packed))  PP_rmtsunroofCtrl_pack_t; /**/

typedef struct
{
	PP_rmtsunroofCtrl_pack_t 	pack;
	PP_rmtsunroofCtrlSt_t		state;
	uint8_t              success_flag;
}__attribute__((packed))  PrvtProt_rmtsunroofCtrl_t; /*结构体*/

static PrvtProt_rmtsunroofCtrl_t PP_rmtsunroofCtrl;
static unsigned long long PP_Respwaittime = 0;

void PP_sunroofctrl_init(void)
{
	memset(&PP_rmtsunroofCtrl,0,sizeof(PrvtProt_rmtsunroofCtrl_t));
	memcpy(PP_rmtsunroofCtrl.pack.Header.sign,"**",2);
	PP_rmtsunroofCtrl.pack.Header.ver.Byte = 0x30;
	PP_rmtsunroofCtrl.pack.Header.commtype.Byte = 0xe1;
	PP_rmtsunroofCtrl.pack.Header.opera = 0x02;
	PP_rmtsunroofCtrl.pack.Header.tboxid = 27;
	memcpy(PP_rmtsunroofCtrl.pack.DisBody.aID,"110",3);
	PP_rmtsunroofCtrl.pack.DisBody.eventId = PP_AID_RMTCTRL + PP_MID_RMTCTRL_RESP;
	PP_rmtsunroofCtrl.pack.DisBody.appDataProVer = 256;
	PP_rmtsunroofCtrl.pack.DisBody.testFlag = 1;
}
int PP_sunroofctrl_mainfunction(void *task)
{
	int res = 0;
	switch(PP_rmtsunroofCtrl.state.CtrlSt)
	{
		case PP_SUNROOFCTRL_IDLE:
		{
			if(PP_rmtsunroofCtrl.state.req == 1)
			{
				PP_rmtsunroofCtrl.success_flag = 0;
				PP_rmtsunroofCtrl.state.CtrlSt = PP_SUNROOFCTRL_REQSTART;
				if(PP_rmtsunroofCtrl.state.style == RMTCTRL_TSP)//tsp
				{
					PP_rmtCtrl_Stpara_t rmtCtrl_Stpara;
					rmtCtrl_Stpara.rvcReqStatus = 1;  //开始执行
					rmtCtrl_Stpara.rvcFailureType = 0;
					rmtCtrl_Stpara.expTime = PP_rmtsunroofCtrl.state.expTime;
					rmtCtrl_Stpara.reqType =PP_rmtsunroofCtrl.state.reqType;
					rmtCtrl_Stpara.eventid = PP_rmtsunroofCtrl.pack.DisBody.eventId;
					rmtCtrl_Stpara.Resptype = PP_RMTCTRL_RVCSTATUSRESP;
					res = PP_rmtCtrl_StInformTsp(&rmtCtrl_Stpara);
				}
				else//蓝牙
				{
				}
				PP_rmtsunroofCtrl.state.req = 0;	
			}
		}
		break;
		case PP_SUNROOFCTRL_REQSTART:
		{
			if(PP_rmtsunroofCtrl.state.sunroofcmd == PP_SUNROOFOPEN) //天窗打开
			{
				PP_can_send_data(PP_CAN_SUNROOF,CAN_SUNROOFOPEN,0);
			}
			else if(PP_rmtsunroofCtrl.state.sunroofcmd == PP_SUNROOFCLOSE)//天窗关闭
			{
				PP_can_send_data(PP_CAN_SUNSHADE,CAN_SUNSHADECLOSE,0);  //遮阳帘关闭
			}
			else if(PP_rmtsunroofCtrl.state.sunroofcmd == PP_SUNROOFUPWARP)//天窗翘起
			{
				PP_can_send_data(PP_CAN_SUNROOF,CAN_SUNROOFUP,0);
			}
			else  //天窗停止
			{
				PP_can_send_data(PP_CAN_SUNROOF,CAN_SUNROOFSTOP,0);
			}
			PP_rmtsunroofCtrl.state.CtrlSt = PP_SUNROOFCTRL_RESPWAIT;
			PP_Respwaittime = tm_get_time();
		}
		break;
		case PP_SUNROOFCTRL_RESPWAIT://执行等待车控响应
		{
			if((tm_get_time() - PP_Respwaittime) > 320)
			{
				PP_can_send_data(PP_CAN_SUNSHADE,CAN_SUNSHADECLEAN,0);
				PP_can_send_data(PP_CAN_SUNROOF,CAN_SUNROOFCLEAN,0);
				if(PP_rmtsunroofCtrl.state.sunroofcmd == PP_SUNROOFOPEN) //天窗打开结果
				{
					if((tm_get_time() - PP_Respwaittime) < 35000)
					{
						if(PP_rmtCtrl_cfg_sunroofSt() == 4) //状态为4，天窗打开ok
						{
							log_o(LOG_HOZON,"PP_SUNROOFCTRL_OPEN SUCCESS");
							//PP_can_send_data(PP_CAN_SUNROOF,CAN_SUNROOFCLEAN,0);
							PP_rmtsunroofCtrl.success_flag = 1;
							PP_rmtsunroofCtrl.state.CtrlSt = PP_SUNROOFCTRL_END;
						}
					}
					else//响应超时
					{
						log_o(LOG_HOZON,"BDM response timed out");
						PP_rmtsunroofCtrl.state.failtype = PP_RMTCTRL_TIMEOUTFAIL;
						PP_can_send_data(PP_CAN_SUNROOF,CAN_SUNROOFCLEAN,0);
						PP_rmtsunroofCtrl.success_flag = 0;
						PP_rmtsunroofCtrl.state.CtrlSt = PP_SUNROOFCTRL_END;
					}
				}
				else if(PP_rmtsunroofCtrl.state.sunroofcmd == PP_SUNROOFCLOSE)//天窗关闭结果
				{
					if((tm_get_time() - PP_Respwaittime) < 35000)
					{
						if(PP_rmtCtrl_cfg_sunroofSt() == 2) //
						{
							log_o(LOG_HOZON,"PP_SUNROOFCTRL_CLOSE SUCCESS");
							//PP_can_send_data(PP_CAN_SUNSHADE,CAN_SUNSHADECLEAN,0);  //遮阳帘
							PP_rmtsunroofCtrl.success_flag = 1;
							PP_rmtsunroofCtrl.state.CtrlSt = PP_SUNROOFCTRL_END;
						}
					}
					else//响应超时
					{
						log_o(LOG_HOZON,"BDM response timed out");
						PP_rmtsunroofCtrl.state.failtype = PP_RMTCTRL_TIMEOUTFAIL;
						PP_can_send_data(PP_CAN_SUNSHADE,CAN_SUNSHADECLEAN,0);  //遮阳帘
						PP_rmtsunroofCtrl.success_flag = 0;
						PP_rmtsunroofCtrl.state.CtrlSt = PP_SUNROOFCTRL_END;
					}
				}
				else if(PP_rmtsunroofCtrl.state.sunroofcmd == PP_SUNROOFUPWARP)//天窗翘起结果
				{
					if((tm_get_time() - PP_Respwaittime) < 35000)
					{
						if(PP_rmtCtrl_cfg_sunroofSt() == 0) //
						{
							log_o(LOG_HOZON,"PP_SUNROOFCTRL_UPWARP SUCCESS");
							//PP_can_send_data(PP_CAN_SUNROOF,CAN_SUNROOFCLEAN,0);
							PP_rmtsunroofCtrl.success_flag = 1;
							PP_rmtsunroofCtrl.state.CtrlSt = PP_SUNROOFCTRL_END;
						}
					}
					else//响应超时
					{
						log_o(LOG_HOZON,"BDM response timed out");
						PP_rmtsunroofCtrl.state.failtype = PP_RMTCTRL_TIMEOUTFAIL;
						PP_can_send_data(PP_CAN_SUNROOF,CAN_SUNROOFCLEAN,0);
						PP_rmtsunroofCtrl.success_flag = 0;
						PP_rmtsunroofCtrl.state.CtrlSt = PP_SUNROOFCTRL_END;
					}
				}
				else
				{
				}	
			}
		}
		break;
		case PP_SUNROOFCTRL_END:
		{
			PP_rmtCtrl_Stpara_t rmtCtrl_Stpara;
			memset(&rmtCtrl_Stpara,0,sizeof(PP_rmtCtrl_Stpara_t));
			if(PP_rmtsunroofCtrl.state.style == RMTCTRL_TSP)//tsp
			{
				rmtCtrl_Stpara.reqType =PP_rmtsunroofCtrl.state.reqType;
				rmtCtrl_Stpara.eventid = PP_rmtsunroofCtrl.pack.DisBody.eventId;
				rmtCtrl_Stpara.expTime = PP_rmtsunroofCtrl.state.expTime;
				rmtCtrl_Stpara.Resptype = PP_RMTCTRL_RVCSTATUSRESP;
				rmtCtrl_Stpara.rvcFailureType  = PP_rmtsunroofCtrl.state.failtype;
				if(1 == PP_rmtsunroofCtrl.success_flag)
				{
					rmtCtrl_Stpara.rvcReqStatus = 2;  //执行完成
					rmtCtrl_Stpara.rvcFailureType = 0;
				}
				else
				{
					rmtCtrl_Stpara.rvcReqStatus = 3;  //执行失败
				}
				res = PP_rmtCtrl_StInformTsp(&rmtCtrl_Stpara);
				
			}
			else if(PP_rmtsunroofCtrl.state.style ==  RMTCTRL_BLUETOOTH) //蓝牙
			{
				PP_rmtCtrl_inform_tb(BT_PANORAMIC_SUNROOF_RESP,PP_rmtsunroofCtrl.state.sunroofcmd,PP_rmtsunroofCtrl.success_flag);
			}
			clearPP_lock_odcmtxlock(PP_LOCK_VEHICTRL_SUNROOF);//释放锁
			PP_rmtsunroofCtrl.state.CtrlSt = PP_SUNROOFCTRL_IDLE;
		}
		break;
		default:
		break;
	}
	return res;
}

uint8_t PP_sunroofctrl_start(void) 
{
	if(PP_rmtsunroofCtrl.state.req == 1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t PP_sunroofctrl_end(void)
{
	if((PP_rmtsunroofCtrl.state.CtrlSt == PP_SUNROOFCTRL_IDLE) && \
			(PP_rmtsunroofCtrl.state.req == 0))
	{
		
		return 1;
	}
	else
	{
		return 0;
	}
}

int SetPP_sunroofctrl_Request(char ctrlstyle,void *appdatarmtCtrl,void *disptrBody)
{
	int mtxlockst = 0;
	mtxlockst = setPP_lock_odcmtxlock(PP_LOCK_VEHICTRL_SUNROOF);
	if(PP_LOCK_OK == mtxlockst)
	{
		switch(ctrlstyle)
		{
			case RMTCTRL_TSP:
			{
				PrvtProt_App_rmtCtrl_t *appdatarmtCtrl_ptr = (PrvtProt_App_rmtCtrl_t *)appdatarmtCtrl;
				PrvtProt_DisptrBody_t *  disptrBody_ptr= (PrvtProt_DisptrBody_t *)disptrBody;
				PP_rmtsunroofCtrl.state.reqType = appdatarmtCtrl_ptr->CtrlReq.rvcReqType;
				PP_rmtsunroofCtrl.state.expTime = disptrBody_ptr->expTime;
				if(PP_rmtsunroofCtrl.state.reqType == PP_RMTCTRL_PNRSUNROOFOPEN) //天窗打开
				{
					PP_rmtsunroofCtrl.state.sunroofcmd = PP_SUNROOFOPEN;
					log_i(LOG_HOZON,"TSP remote sunroof open control");
				}
				else if(PP_rmtsunroofCtrl.state.reqType == PP_RMTCTRL_PNRSUNROOFCLOSE)//天窗关闭
				{
					PP_rmtsunroofCtrl.state.sunroofcmd = PP_SUNROOFCLOSE;
					log_i(LOG_HOZON,"TSP remote sunroof close control");
				}
				else if(PP_rmtsunroofCtrl.state.reqType == PP_RMTCTRL_PNRSUNROOFUPWARP)//天窗翘起
				{
					PP_rmtsunroofCtrl.state.sunroofcmd = PP_SUNROOFUPWARP;
					log_i(LOG_HOZON,"TSP remote sunroof upwarp control");
				}
				else  //天窗停止
				{
					PP_rmtsunroofCtrl.state.sunroofcmd = PP_SUNROOFSTOP;
					log_i(LOG_HOZON,"TSP remote sunroof stop control");
				}
				PP_rmtsunroofCtrl.state.req = 1;
				PP_rmtsunroofCtrl.pack.DisBody.eventId = disptrBody_ptr->eventId;
				PP_rmtsunroofCtrl.state.style = RMTCTRL_TSP;
			}
			break;
			case RMTCTRL_BLUETOOTH:
			{
				 unsigned char cmd = *(unsigned char *)appdatarmtCtrl;
				 if(cmd == 1 )//蓝牙开天窗
				 {
				 	PP_rmtsunroofCtrl.state.sunroofcmd = PP_SUNROOFOPEN;
					log_i(LOG_HOZON,"bluetooth sunroof open control");
				 }
				 else if (cmd == 2) //蓝牙关天窗
				 {
				 	PP_rmtsunroofCtrl.state.sunroofcmd = PP_SUNROOFCLOSE;
					log_i(LOG_HOZON,"bluetooth sunroof close control");
				 }
				 else if(cmd == 3) //天窗翘起
				 {
				 	PP_rmtsunroofCtrl.state.sunroofcmd = PP_SUNROOFUPWARP;
					log_i(LOG_HOZON,"bluetooth sunroof upwarp control");
				 }
				 else    //天窗停止
				 {
				 	PP_rmtsunroofCtrl.state.sunroofcmd = PP_SUNROOFSTOP;
					log_i(LOG_HOZON,"bluetooth sunroof stop control");
				 }
				 PP_rmtsunroofCtrl.state.req = 1;
				 PP_rmtsunroofCtrl.state.style = RMTCTRL_BLUETOOTH;	 
			}
			break;
			default:
			break;
		}
	}
	return mtxlockst;
}

void PP_sunroofctrl_ClearStatus(void)
{
	clearPP_lock_odcmtxlock(PP_LOCK_VEHICTRL_SUNROOF);//释放锁
	PP_rmtsunroofCtrl.state.req = 0;
}

void PP_sunroofctrl_SetCtrlReq(unsigned char req,uint16_t reqType)
{
	PP_rmtsunroofCtrl.state.reqType = (long)reqType;
	if(PP_rmtsunroofCtrl.state.reqType == PP_RMTCTRL_PNRSUNROOFOPEN) //天窗打开
	{
		PP_rmtsunroofCtrl.state.sunroofcmd = PP_SUNROOFOPEN;
	}
	else if(PP_rmtsunroofCtrl.state.reqType == PP_RMTCTRL_PNRSUNROOFCLOSE)//天窗关闭
	{
		PP_rmtsunroofCtrl.state.sunroofcmd = PP_SUNROOFCLOSE;
	}
	else if(PP_rmtsunroofCtrl.state.reqType == PP_RMTCTRL_PNRSUNROOFUPWARP)//天窗翘起
	{
		PP_rmtsunroofCtrl.state.sunroofcmd = PP_SUNROOFUPWARP;
	}
	else  //天窗停止
	{
		PP_rmtsunroofCtrl.state.sunroofcmd = PP_SUNROOFSTOP;
	}
	PP_rmtsunroofCtrl.state.req = 1;
	PP_rmtsunroofCtrl.state.style = RMTCTRL_TSP;

}


