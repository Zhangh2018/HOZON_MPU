/******************************************************
�ļ�����	
������
Data			Vasion			author
2019/4/17		V1.0			liujian
*******************************************************/

/*******************************************************
description�� include the header file
*******************************************************/
#include "PrvtProt_queue.h"

/*******************************************************
description�� function declaration
*******************************************************/
/*Global function declaration*/

/*Static function declaration*/
static PPObj_t PPObj[PP_MAX];

/*******************************************************
description�� global variable definitions
*******************************************************/

/*******************************************************
description�� static variable definitions
*******************************************************/
static void ClrPP_queue(void);

/******************************************************
description�� function code
******************************************************/

/******************************************************
*��������PP_queue_Init
*��  �Σ�
*����ֵ��
*��  ����
*��  ע��
******************************************************/
void PP_queue_Init(void)
{
	ClrPP_queue();//�建�����
}


/******************************************************
*��������WrPP_queue
*��  �Σ�
*����ֵ��
*��  ����д���ݵ����ݶ���
*��  ע��
******************************************************/
int WrPP_queue(unsigned char  obj,unsigned char* data,int len)
{
	int Lng;
	
	if(len > PP_DATA_LNG) return -1;
	
	for(Lng = 0U;Lng< len;Lng++)
	{
		PPObj[obj].PPCache[PPObj[obj].HeadLabel].data[Lng] = data[Lng];
	}
	PPObj[obj].PPCache[PPObj[obj].HeadLabel].len = len;
	PPObj[obj].PPCache[PPObj[obj].HeadLabel].NonEmptyFlg = 1U;
	PPObj[obj].HeadLabel++;
	if(PP_QUEUE_LNG == PPObj[obj].HeadLabel)
	{
		PPObj[obj].HeadLabel = 0U;
	}
	
	return 0;
}


/******************************************************
*������:RdPP_queue
*��  �Σ�
*����ֵ��
*��  ������ȡ����
*��  ע��
******************************************************/
int RdPP_queue(unsigned char  obj,unsigned char* data,int len)
{	
	int Lng;
	if(0U == PPObj[obj].PPCache[PPObj[obj].TialLabel].NonEmptyFlg)
	{
		return 0;//
	}
		
	if(PPObj[obj].PPCache[PPObj[obj].TialLabel].len > len)
	{
		return -1;//
	}

	for(Lng = 0U;Lng < PPObj[obj].PPCache[PPObj[obj].TialLabel].len;Lng++)
	{
		data[Lng] =PPObj[obj].PPCache[PPObj[obj].TialLabel].data[Lng];
	}
	PPObj[obj].PPCache[PPObj[obj].TialLabel].NonEmptyFlg = 0U;
		
	PPObj[obj].TialLabel++;
	if(PP_QUEUE_LNG == PPObj[obj].TialLabel)
	{
		PPObj[obj].TialLabel = 0U;
	}
	return Lng;
}

/******************************************************
*��������ClrUnlockLogCache_Queue
*��  �Σ�
*����ֵ��
*��  ���������ݶ���
*��  ע��
******************************************************/
static void ClrPP_queue(void)
{
	unsigned char  i,j;

	for(i = 0U;i < PP_MAX;i++)
	{
		for(j = 0U;j < PP_QUEUE_LNG;j++)
		{
			PPObj[i].PPCache[j].NonEmptyFlg = 0U;
		}
		PPObj[i].HeadLabel = 0U;
		PPObj[i].TialLabel = 0U;
	}
}
