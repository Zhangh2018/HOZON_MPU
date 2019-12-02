/******************************************************
�ļ�����	
������	����tsp�Խ�socket��·�Ľ������Ͽ�����/�����ݴ���	
Data			Vasion			author
2019/4/17		V1.0			liujian
*******************************************************/

/*******************************************************
description�� include the header file
*******************************************************/
#include "sockproxy_rxdata.h"

/*******************************************************
description�� function declaration
*******************************************************/
/*Global function declaration*/

/*Static function declaration*/
static sockProxyObj_t SPObj[SP_MAX];

/*******************************************************
description�� global variable definitions
*******************************************************/

/*******************************************************
description�� static variable definitions
*******************************************************/
static void ClrSockproxyData_Queue(void);

/******************************************************
description�� function code
******************************************************/

/******************************************************
*��������SockproxyData_Init
*��  �Σ�
*����ֵ��
*��  ����
*��  ע��
******************************************************/
void SockproxyData_Init(void)
{
	ClrSockproxyData_Queue();//�建�����
}


/******************************************************
*��������WrSockproxyData_Queue
*��  �Σ�
*����ֵ��
*��  ����д���ݵ����ݶ���
*��  ע��
******************************************************/
int WrSockproxyData_Queue(unsigned char  obj,unsigned char* data,int len)
{
	int Lng;
	
	if(len > SP_DATA_LNG) return -1;
	
	for(Lng = 0U;Lng< len;Lng++)
	{
		SPObj[obj].SPCache[SPObj[obj].HeadLabel].data[Lng] = data[Lng];
	}
	SPObj[obj].SPCache[SPObj[obj].HeadLabel].len = len;
	SPObj[obj].SPCache[SPObj[obj].HeadLabel].NonEmptyFlg = 1U;/*�÷ǿձ�־*/
	SPObj[obj].HeadLabel++;
	if(SP_QUEUE_LNG == SPObj[obj].HeadLabel)
	{
		SPObj[obj].HeadLabel = 0U;
	}
	
	return 0;
}


/******************************************************
*��������RdSockproxyData_Queue
*��  �Σ�
*����ֵ��
*��  ������ȡ����
*��  ע��
******************************************************/
int RdSockproxyData_Queue(unsigned char  obj,unsigned char* data,int len)
{	
	int Lng;
	if(0U == SPObj[obj].SPCache[SPObj[obj].TialLabel].NonEmptyFlg) 
	{
		return 0;//������
	}
		
	if(SPObj[obj].SPCache[SPObj[obj].TialLabel].len > len) 
	{
		return -1;//�������
	}

	for(Lng = 0U;Lng < SPObj[obj].SPCache[SPObj[obj].TialLabel].len;Lng++)
	{
		data[Lng] =SPObj[obj].SPCache[SPObj[obj].TialLabel].data[Lng];
	}
	SPObj[obj].SPCache[SPObj[obj].TialLabel].NonEmptyFlg = 0U;	/*��ǿձ�־*/
		
	SPObj[obj].TialLabel++;
	if(SP_QUEUE_LNG == SPObj[obj].TialLabel)
	{
		SPObj[obj].TialLabel = 0U;
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
static void ClrSockproxyData_Queue(void) 
{
	unsigned char  i,j;

	for(i = 0U;i < SP_MAX;i++)
	{
		for(j = 0U;j < SP_QUEUE_LNG;j++)
		{
			SPObj[i].SPCache[j].NonEmptyFlg = 0U;
		}
		SPObj[i].HeadLabel = 0U;
		SPObj[i].TialLabel = 0U;
	}
}
