/******************************************************
�ļ�����	PrvtProt_signFltr.h

������	��ҵ˽��Э�飨�㽭���ڣ�	

Data			  Vasion			author
2019/04/16		   V1.0			    liujian
*******************************************************/
#ifndef		_PRVTPROT_SIGNFLTR_H
#define		_PRVTPROT_SIGNFLTR_H

/*******************************************************
description�� macro definitions
*******************************************************/
/**********�꿪�ض���*********/


/**********�곣������*********/
#define PPSIGNFLTR_FLTRTIME   30U/*�˲�ʱ������*/


#define PPSIGNFLTR_ON		1
#define PPSIGNFLTR_OFF		1

/***********�꺯��***********/


/*******************************************************
description�� struct definitions
*******************************************************/



/*******************************************************
description�� typedef definitions
*******************************************************/
/*****struct definitions*****/
typedef struct
{
	uint8_t e_u_St;/*��/��״̬*/
	uint8_t e_u_OnFltrCnt;/*��/�ؿ���״̬�˲���ʱ��*/
	uint8_t e_u_OffFltrCnt;/*��/�عر�״̬�˲���ʱ��*/
}PPsignFltrStruct;

/******enum definitions******/
typedef enum
{
	bdcmAuthSt,	  /*BDCM��֤״̬*/
	PPSIGNFLTR_NUM	  /*��������*/
}PPsignFltrEnum;

/******enum definitions******/

/******union definitions*****/

/*******************************************************
description�� variable External declaration
*******************************************************/

/*******************************************************
description�� function External declaration
*******************************************************/
extern void InitPPsignFltr_Parameter(void);
extern void TskPPsignFltr_MainFunction(void);
extern uint8_t GetPPsignFltr_u_PeriSt(uint8_t LePPsignFltr_u_Index);
#endif 
