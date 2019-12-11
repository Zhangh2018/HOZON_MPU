/******************************************************
�ļ�����	PrvtProt_remoteConfig.h

������	��ҵ˽��Э�飨�㽭���ڣ�	

Data			  Vasion			author
2019/04/16		   V1.0			    liujian
*******************************************************/
#ifndef		_PRVTPROT_REMOTE_CFG_H
#define		_PRVTPROT_REMOTE_CFG_H
/*******************************************************
description�� include the header file
*******************************************************/

/*******************************************************
description�� macro definitions
*******************************************************/
/**********�꿪�ض���*********/

/**********�곣������*********/
#define RMTCFG_DELAY_TIME 		1500//
//#define PP_XCALL_ACK_WAIT 	0x01//Ӧ��ɹ�
//#define PP_XCALL_ACK_SUCCESS 	0x02//Ӧ��ɹ�

/***********�꺯��***********/

/*******************************************************
description�� struct definitions
*******************************************************/

/*******************************************************
description�� typedef definitions
*******************************************************/
/******enum definitions******/

typedef enum
{
	PP_RMTCFG_CFG_IDLE = 0,//
	PP_CHECK_CFG_REQ,//���Զ������
	PP_CHECK_CFG_RESP,//
	PP_GET_CFG_REQ,//
	PP_GET_CFG_RESP,//
	PP_RMTCFG_CFG_END,//
} PP_RMTCFG_STATE_MACHINE;//״̬��

typedef enum
{
	PP_RMTCFG_WAIT_IDLE = 0,//
	PP_RMTCFG_CHECK_WAIT_RESP,//
	PP_RMTCFG_GET_WAIT_RESP,//
	PP_RMTCFG_END_WAIT_SENDRESP,//end cfg send response
} PP_RMTCFG_WAIT_STATE;//�ȴ�״̬

typedef enum
{
	PP_RMTCFG_FICM = 0,//
	PP_RMTCFG_APN1,//
	PP_RMTCFG_APN2,//
	PP_RMTCFG_COMMON,//
	PP_RMTCFG_EXTEND,//
	PP_RMTCFG_SETID_MAX//
} PP_RMTCFG_READ_SETID;//read cfg settingID

/*****struct definitions*****/
typedef struct
{
	uint8_t req;
	uint8_t reqCnt;
	uint8_t CfgSt;
	uint64_t period;
	uint8_t waitSt;
	uint64_t waittime;
	uint8_t iccidValid;
	uint64_t delaytime;
	uint8_t  avtivecheckflag;

	/* remote config */
	uint8_t needUpdata;//�����Ƿ���Ҫ����
	uint8_t newCfgVersion[33];
	uint8_t cfgAccept;//�����Ƿ����
	uint8_t cfgsuccess;//�����Ƿ�ɹ�
	uint8_t getRespResult;
	uint8_t readreq[PP_RMTCFG_SETID_MAX];
	long 	eventid;
	long	expTime;
	char	apn1tspaddrchangedflag;
	char	apn1certaddrchangeflag;
	char	tspSMSchangeflag;
}__attribute__((packed))  PrvtProt_rmtCfgSt_t; /*remote config�ṹ��*/

/***********************************
			remote config
***********************************/
typedef struct
{
	uint8_t mcuSw[11];
	uint8_t mpuSw[11];
	uint8_t vehicleVin[18];
	uint8_t iccID[21];
	uint8_t btMacAddr[13];//����mac��ַ
	uint8_t configSw[6];
	uint8_t cfgVersion[33];
	uint8_t mcuSwlen;
	uint8_t mpuSwlen;
	uint8_t vehicleVinlen;
	uint8_t iccIDlen;
	uint8_t btMacAddrlen;
	uint8_t configSwlen;
	uint8_t cfgVersionlen;
}__attribute__((packed)) App_rmtCfg_checkReq_t;
typedef struct
{
	int needUpdate;
	uint8_t cfgVersion[33];
	uint8_t cfgVersionlen;
}__attribute__((packed)) App_rmtCfg_checkResp_t;
typedef struct
{
	uint8_t cfgVersion[33];
	uint8_t cfgVersionlen;
}__attribute__((packed)) App_rmtCfg_getReq_t;

typedef struct
{
	uint8_t token[33];
	uint8_t userID[33];
	char directConnEnable;
	uint8_t address[33];
	uint8_t port[7];
	uint8_t tokenlen;
	uint8_t userIDlen;
	uint8_t addresslen;
	uint8_t portlen;
	uint8_t ficmConfigValid;
}__attribute__((packed))  App_rmtCfg_FICM_t;

typedef struct
{
	uint8_t tspAddr[33];
	uint8_t tspUser[17];
	uint8_t tspPass[17];
	uint8_t tspIP[16];
	uint8_t tspSms[33];
	uint8_t tspPort[7];
	uint8_t certAddress[33];
	uint8_t certPort[7];
	uint8_t tspAddrlen;
	uint8_t tspUserlen;
	uint8_t tspPasslen;
	uint8_t tspIPlen;
	uint8_t tspSmslen;
	uint8_t tspPortlen;
	uint8_t certAddresslen;
	uint8_t certPortlen;
	uint8_t apn1ConfigValid;
}__attribute__((packed)) App_rmtCfg_APN1_t;

typedef struct
{
	uint8_t apn2Address[33];
	uint8_t apn2User[17];
	uint8_t apn2Pass[17];
	uint8_t apn2Addresslen;
	uint8_t apn2Userlen;
	uint8_t apn2Passlen;
	uint8_t apn2ConfigValid;
}__attribute__((packed)) App_rmtCfg_APN2_t;

typedef struct
{
	char actived;
	char rcEnabled;
	char svtEnabled;
	char vsEnabled;
	char iCallEnabled;
	char bCallEnabled;
	char eCallEnabled;
	char dcEnabled;
	char dtcEnabled;
	char journeysEnabled;
	char onlineInfEnabled;
	char rChargeEnabled;
	char btKeyEntryEnabled;
	char carEmpowerEnabled;
	char eventReportEnabled;
	char carAlarmEnabled;
	int heartbeatTimeout;
	int dormancyHeartbeatTimeout;
	int infoCollectCycle;
	int regularUpCycle;
	uint8_t commonConfigValid;
}__attribute__((packed)) App_rmtCfg_COMMON_t;

typedef struct
{
	uint8_t ecallNO[17];
	uint8_t bcallNO[17];
	uint8_t ccNO[17];
	uint8_t ecallNOlen;
	uint8_t bcallNOlen;
	uint8_t ccNOlen;
	uint8_t extendConfigValid;
}__attribute__((packed)) App_rmtCfg_EXTEND_t;

typedef struct
{
	int result;
	App_rmtCfg_FICM_t 	FICM;
	App_rmtCfg_APN1_t 	APN1;
	App_rmtCfg_APN2_t 	APN2;
	App_rmtCfg_COMMON_t COMMON;
	App_rmtCfg_EXTEND_t EXTEND;
}__attribute__((packed)) App_rmtCfg_getResp_t;

typedef struct
{
	int configSuccess;
	uint8_t mcuSw[11];
	uint8_t mpuSw[11];
	uint8_t configSw[6];
	uint8_t cfgVersion[33];
	uint8_t mcuSwlen;
	uint8_t mpuSwlen;
	uint8_t configSwlen;
	uint8_t cfgVersionlen;
}__attribute__((packed)) App_rmtCfg_EndCfgReq_t;

typedef struct
{
	int configAccepted;
}App_rmtCfg_CfgconnResp_t;

typedef struct
{
	long SettingId[PP_RMTCFG_SETID_MAX];
	uint8_t SettingIdlen;
}__attribute__((packed)) App_rmtCfg_CfgReadReq_t;

typedef struct
{
	char result;
	uint8_t cfgsuccess;
	uint8_t cfgVersion[33];
	uint8_t cfgVersionlen;
	uint8_t readreq[PP_RMTCFG_SETID_MAX];
	App_rmtCfg_FICM_t 	FICM;
	App_rmtCfg_APN1_t 	APN1;
	App_rmtCfg_APN2_t 	APN2;
	App_rmtCfg_COMMON_t COMMON;
	App_rmtCfg_EXTEND_t EXTEND;
}__attribute__((packed)) App_rmtCfg_CfgReadResp_t;

typedef struct
{
	/* check config request */
	App_rmtCfg_checkReq_t checkReq;

	/* check config response */
	App_rmtCfg_checkResp_t checkResp;

	/* get config req */
	App_rmtCfg_getReq_t getReq;

	/* get config response */
	App_rmtCfg_getResp_t getResp;

	/* end config req */
	App_rmtCfg_EndCfgReq_t EndReq;

	/* config  conn req */
	App_rmtCfg_CfgconnResp_t connResp;

	/* read config req */
	App_rmtCfg_CfgReadReq_t	ReadReq;
	/* read config resp */
	App_rmtCfg_CfgReadResp_t ReadResp;
}__attribute__((packed)) PrvtProt_App_rmtCfg_t;
/******union definitions*****/

/*******************************************************
description�� variable External declaration
*******************************************************/

/*******************************************************
description�� function External declaration
*******************************************************/
extern void PP_rmtCfg_init(void);
extern int PP_rmtCfg_mainfunction(void *task);
extern void PP_rmtCfg_SetCfgReq(unsigned char req);
extern void PP_rmtCfg_ShowCfgPara(void);
extern void PP_rmtCfg_setCfgEnable(unsigned char obj,unsigned char enable);
extern void PP_rmtCfg_setCfgapn1(unsigned char obj,const void *ddata1,const void *data2);
extern void PP_rmtCfg_setCfgficm(unsigned char obj,const void *data);
extern int getPP_rmtCfg_heartbeatTimeout(void);
#endif 
