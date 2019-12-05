/******************************************************
�ļ�����	PrvtProt_signFltr.c

������	��ҵ˽��Э�飨�㽭���ڣ�	
Data			Vasion			author
2018/1/10		V1.0			liujian
*******************************************************/

/*******************************************************
description�� include the header file
*******************************************************/
#include "init.h"
#include "log.h"
#include "PrvtProt_SigParse.h"
#include "PrvtProt_signFltr.h"
/*******************************************************

description�� function declaration

*******************************************************/

/*Global function declaration*/



/*Static function declaration*/

static PPsignFltrStruct  SaPPsignFltr_h_Para[PPSIGNFLTR_NUM];/*���������ṹ��*/

/*******************************************************

description�� global variable definitions

*******************************************************/
const uint8_t CaPPsignFltr_u_Active[PPSIGNFLTR_NUM] = /*������Ч���ò��������ɼ����ݸ�����һ������Ϊ���迪��*/
{
	1U/*״̬--����*/
};

/*******************************************************

description�� static variable definitions

*******************************************************/
static void PPsignFltr_FltrDeal(void);
static uint8_t GetPPsignFltr_u_RTSt(uint8_t LeFltr_u_Index);
/******************************************************

description�� function code

******************************************************/
/******************************************************
*��������InitPPsignFltr_Parameter

*��  �Σ�

*����ֵ��

*��  ����

*��  ע��
******************************************************/
void InitPPsignFltr_Parameter(void)
{
	uint8_t LePPsignFltr_u_Index;
	for(LePPsignFltr_u_Index = 0U;LePPsignFltr_u_Index < PPSIGNFLTR_NUM;LePPsignFltr_u_Index++)
	{
		SaPPsignFltr_h_Para[LePPsignFltr_u_Index].e_u_St = 0;
		SaPPsignFltr_h_Para[LePPsignFltr_u_Index].e_u_OnFltrCnt = 0;
		SaPPsignFltr_h_Para[LePPsignFltr_u_Index].e_u_OffFltrCnt = 0;
	}
}


/******************************************************
*��������TskPPsignFltr_MainFunction

*��  �Σ�

*����ֵ��

*��  ������������

*��  ע����������5ms
******************************************************/
void TskPPsignFltr_MainFunction(void)
{
	PPsignFltr_FltrDeal();
}

/******************************************************
*��������PPsignFltr_FltrDeal

*��  �Σ�

*����ֵ��

*��  ����

*��  ע����������5ms
******************************************************/
static void PPsignFltr_FltrDeal(void)
{
	uint8_t LePPsignFltr_u_Index;
	uint8_t LeFltr_u_St;

	for(LePPsignFltr_u_Index = 0U;LePPsignFltr_u_Index < PPSIGNFLTR_NUM;LePPsignFltr_u_Index++)
	{
		LeFltr_u_St = GetPPsignFltr_u_RTSt(LePPsignFltr_u_Index);
		if(CaPPsignFltr_u_Active[LePPsignFltr_u_Index] == LeFltr_u_St)/*��/�ص�ǰʵ��״̬������*/
		{
			SaPPsignFltr_h_Para[LePPsignFltr_u_Index].e_u_OffFltrCnt =0U;
			SaPPsignFltr_h_Para[LePPsignFltr_u_Index].e_u_OnFltrCnt++;/*�˲���ʱ*/
			if(SaPPsignFltr_h_Para[LePPsignFltr_u_Index].e_u_OnFltrCnt >= PPSIGNFLTR_FLTRTIME)/*�����̰�����ʱ����������*/
			{
				SaPPsignFltr_h_Para[LePPsignFltr_u_Index].e_u_OnFltrCnt =0U;
				SaPPsignFltr_h_Para[LePPsignFltr_u_Index].e_u_St = 1;/*��״̬*/
			}
		}
		else/*��/�ص�ǰʵ��״̬���ر�*/
		{
			SaPPsignFltr_h_Para[LePPsignFltr_u_Index].e_u_OnFltrCnt =0U;
			SaPPsignFltr_h_Para[LePPsignFltr_u_Index].e_u_OffFltrCnt++;/*�˲���ʱ*/
			if(SaPPsignFltr_h_Para[LePPsignFltr_u_Index].e_u_OffFltrCnt >= PPSIGNFLTR_FLTRTIME)/*�����̰�����ʱ����������*/
			{
				SaPPsignFltr_h_Para[LePPsignFltr_u_Index].e_u_OffFltrCnt =0U;
				SaPPsignFltr_h_Para[LePPsignFltr_u_Index].e_u_St = 0;/*��״̬*/
			}
		}
	}
}

/******************************************************
*��������GetPPsignFltr_u_PeriSt

*��  �Σ�

*����ֵ��

*��  ������ȡ����״̬

*��  ע��
******************************************************/
uint8_t GetPPsignFltr_u_PeriSt(uint8_t LePPsignFltr_u_Index)
{
	return SaPPsignFltr_h_Para[LePPsignFltr_u_Index].e_u_St;
}



/******************************************************
*��������  GetPPsignFltr_u_RTSt

*��  �Σ�

*����ֵ��

*��  ����

*��  ע��
******************************************************/
static uint8_t GetPPsignFltr_u_RTSt(uint8_t LeFltr_u_Index)
{
	uint8_t Ret = 0U;

	switch(LeFltr_u_Index)
	{
		case bdcmAuthSt:
		{
			Ret = PrvtProt_SignParse_autheSt();
		}
		break;
		default:
		break;
	}

	return Ret;
}

