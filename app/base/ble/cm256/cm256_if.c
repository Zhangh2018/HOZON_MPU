/****************************************************************
 file:          cm256_if.c
 description:   
 date:          2019/5/15
 author:        liuquanfu
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#include "app_ble.h"
#include "app_utils.h"
#include "app_disc.h"
#include "app_mgt.h"
#include "app_dm.h"
#include "app_ble_client.h"
#include "app_ble_server.h"
#include "app_ble_test_server.h"
#include "app_ble_test_client.h"

#include "ql_cm256sm_ble_server.h"
#include "ql_cm256sm_ble_sleep.h"
#include "ql_oe.h"
#include "app_manager.h"
#include "cm256_if.h"
#include "log.h"
#include "ble.h"
#include "ring_buffer.h"




#ifndef NULL
#define NULL (void *)0
#endif

typedef struct
{
    unsigned char aucDevName[256];
    unsigned char aucDevMac[6];
    unsigned char ucNameLen;
	unsigned char ucMacLen;
}BT_DEVINFO_TYPE;

BT_DEVINFO_TYPE Tc35661BtInfo;

volatile NF_BLE_MSG				g_NfBleMsg;
int								ulBleStatus;
static pthread_mutex_t			cm256Statemutex;
volatile int					bt_enable_flag = 0;
extern volatile int 			bt_notify_timer_flag;
extern volatile int 			bt_indicate_timer_flag;
static unsigned char       		aucTxbuff[TX_BUFFER_SIZE];
static struct ring_buffer  		stTxRb;
static unsigned char       		aucRxbuff[RX_BUFFER_SIZE];
static struct ring_buffer  		stRxRb;
BT_DEVINFO_TYPE                 ble_info;

BLE_CTL						    stBleCtl;

/******************************************************************************
* Function Name  : Cm256_SetName
* Description    :  set ble NAME
* Input          :  
* Return         : NONE
******************************************************************************/
void Cm256_SetName(void)
{
}

/******************************************************************************
* Function Name  : Cm256_Init
* Description    :  init
* Input          :  
* Return         : NONE
******************************************************************************/

BOOLEAN app_ble_test_mgt_callback(tBSA_MGT_EVT event, tBSA_MGT_MSG *p_data)
{
    switch(event)
    {
    case BSA_MGT_STATUS_EVT:
        APP_DEBUG0("BSA_MGT_STATUS_EVT");
        if (p_data->status.enable)
        {
            APP_DEBUG0("Bluetooth restarted => re-initialize the application");
            app_ble_start();
        }
        break;

    case BSA_MGT_DISCONNECT_EVT:
        APP_DEBUG1("BSA_MGT_DISCONNECT_EVT reason:%d", p_data->disconnect.reason);
        /* Connection with the Server lost => Just exit the application */
        exit(-1);
        break;

    default:
        break;
    }
    return FALSE;
}
extern tBSA_SEC_IO_CAP g_sp_caps ;

/******************************************************************************
* Function Name  : Cm256_Init
* Description    :  init
* Input          :  
* Return         : NONE
******************************************************************************/
int cm256_init(void)
{
   int status;

   	memset(&aucTxbuff, 0, sizeof(aucTxbuff));
	rb_init(&stTxRb, aucTxbuff, sizeof(aucTxbuff));
	log_i(LOG_BLE, "rb_unused_len(&stTxRb)  = %d\r\n",rb_unused_len(&stTxRb) );
	log_i(LOG_BLE, "stTxRb.size)  = %d\r\n", stTxRb.size);
	log_i(LOG_BLE, "stTxRb.in  = %d\r\n", stTxRb.in);
	log_i(LOG_BLE, "stTxRb.out  = %d\r\n", stTxRb.out);
	
	memset(&aucRxbuff, 0, sizeof(aucRxbuff));
	rb_init(&stRxRb, aucRxbuff, sizeof(aucRxbuff));
	log_i(LOG_BLE, "rb_unused_len(&stRxRb)  = %d\r\n",rb_unused_len(&stRxRb) );
	log_i(LOG_BLE, "stRxRb.size)  = %d\r\n", stRxRb.size);
	log_i(LOG_BLE, "stRxRb.in  = %d\r\n", stRxRb.in);
	log_i(LOG_BLE, "stRxRb.out  = %d\r\n", stRxRb.out);

	memset(&stBleCtl, 0, sizeof(BLE_CTL));	
		
    /* Initialize BLE application */
    status = app_ble_init();
    if (status < 0)
    {
        APP_ERROR0("Couldn't Initialize BLE app, exiting");
        return -1;
    }

    /* Open connection to BSA Server */
    app_mgt_init();
	
    if (app_mgr_config())
    {
       // APP_ERROR0("Couldn't configure successfully, exiting");
       // return -1;
    }
			
    if(app_mgt_open(NULL, app_ble_test_mgt_callback) < 0)
    {
        APP_ERROR0("Unable to connect to server");
        return -1;
    }
    app_mgr_sec_set_sp_cap(BTA_IO_CAP_OUT);

    if(0 != strlen((const char *)ble_info.aucDevName))
    {
    	status = app_dm_set_local_bt_name(ble_info.aucDevName);
    }
	else
	{
		status = app_dm_set_local_bt_name((unsigned char*)"HZ00000000000000000");
	}
	
	if ( 0 != status)
	{
		return status;
	}

	/* Start BLE application */
    status = app_ble_start();
    if (status < 0)
    {
        APP_ERROR0("Couldn't Start BLE app, exiting");
        return -1;
    }

	return 0;
}

/******************************************************************************
* Function Name  : cm256_enpower
* Description    :  
* Input          :   
* Return         : 0 indicates success,others indicates failed
******************************************************************************/
int cm256_enpower(unsigned char ucEn)
{
	return YT_OK;
}
/******************************************************************************
* Function Name  : cm256_get_uuid
* Description    :   
* Input          :   
* Return         : 0 indicates success,others indicates failed
******************************************************************************/
int cm256_get_uuid(unsigned     short *usUuid, unsigned char ucUuidCnt)
{
	if (ucUuidCnt > BLE_UUID_CNT)
	{
		return YT_ERR;
	}
	return YT_OK;
}


/******************************************************************************
* Function Name  : cm256_get_rssi
* Description    :  open cm256
* Input          :  
* Return         : 0 indicates success,others indicates failed
******************************************************************************/
int cm256_get_rssi(unsigned short * usRssi, unsigned char ucLen)
{
	return YT_OK;
}

/******************************************************************************
* Function Name  : Cm256_Open
* Description    :  open cm256
* Input          :  
* Return         : NONE
******************************************************************************/
int cm256_open(void)
{
	tBSA_DM_GET_CONFIG bt_config;
	int iRet = -1;
	
    iRet = app_ble_test_server_start();
	if (-1 == iRet)
	{
		return iRet;
	}

    //GKI_delay(3000);
    //Ql_Autosleep_Enable(1);
	iRet = api_get_bt_config(&bt_config);

	if(0 == iRet)
	{
		memset(ble_info.aucDevMac, 0, sizeof(ble_info.aucDevMac));
		memcpy(ble_info.aucDevMac, (const void *)bt_config.bd_addr, 6);
		ble_info.ucMacLen = 6;
	}
	else
	{
		memset(ble_info.aucDevMac, 0, sizeof(ble_info.aucDevMac));
		ble_info.ucMacLen = 0;
	}

    ql_ble_sleep_init();
	return 0;
}

/******************************************************************************
* Function Name  : Cm256_Close
* Description    :  close cm256
* Input          :  
* Return         : NONE
******************************************************************************/
void Cm256_Close(void)
{
	/* Exit BLE mode */
	app_ble_exit();

	/* Close BSA Connection before exiting (to release resources) */
	app_mgt_close();
}
/******************************************************************************
* Function Name  : Cm256_Process
* Description    :  close cm256
* Input          :  
* Return         : NONE
******************************************************************************/
void Cm256_Process(unsigned char ucCycle)
{
}

/******************************************************************************
* Function Name  : cm256_close
* Description    :  close  
* Input          :  
* Return         : 0 indicates success,others indicates failed
******************************************************************************/
int cm256_close(void)
{
	int iRet = YT_ERR;
	return iRet;
}

/*******************************************************************************
 **
 ** Function        app_ble_server_close
 **
 ** Description     This is the ble close connection
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int api_ble_server_close(void)
{
    tBSA_STATUS status;
    tBSA_BLE_SE_CLOSE ble_close_param;
    int server_num = 1;

    APP_INFO0("Select Server:");
    //app_ble_server_display();
    //server_num = app_get_choice("Select");

    if ((server_num < 0) ||
       (server_num >= BSA_BLE_SERVER_MAX) ||
       (ql_app_ble_cb.ble_server[server_num].enabled == FALSE))
    {
        APP_ERROR1("Wrong server number! = %d", server_num);
        return -1;
    }
	   
    status = BSA_BleSeCloseInit(&ble_close_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeCloseInit failed status = %d", status);
        return -1;
    }
   // ble_close_param.conn_id = ql_app_ble_cb.ble_server[server_num].conn_id;
   ble_close_param.conn_id = g_NfBleMsg.ulHandle;
    status = BSA_BleSeClose(&ble_close_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleSeClose failed status = %d", status);
        return -1;
    }

    return 0;
}

/******************************************************************************
* Function Name  : cm256_disconnect
* Description    : 
* Input          :  
* Return         : 0 indicates success,others indicates failed
******************************************************************************/
int cm256_disconnect(void)
{
  int iRet = YT_ERR;
  api_ble_server_close();
  return iRet;
}
/******************************************************************************
* Function Name  : cm256_get_state
* Description    :   
* Input          :  
* Return         : NONE
******************************************************************************/
int cm256_get_state(void)
{
    int iRet = YT_ERR;
   	pthread_mutex_lock(&cm256Statemutex);
	iRet = ulBleStatus;
    pthread_mutex_unlock(&cm256Statemutex);

	if (BLE_CONNECT == iRet)
	{
		return YT_OK;
	}
	else
	{
		return YT_ERR;
	}	
}
/******************************************************************************
* Function Name  : cm256_get_name
* Description    :   
* Input          :  
* Return         : 0 indicates success,others indicates failed
******************************************************************************/
int cm256_get_name(unsigned char *paucInBtName, unsigned char *pucInLen)
{
     if (strlen((char *)ble_info.aucDevName))
    {
    	memcpy(paucInBtName,(const void *)ble_info.aucDevName, strlen((char *)ble_info.aucDevName));
		*pucInLen = 6;
    }
	else
	{
		*pucInLen = 0;
		return YT_ERR;
	}
	
	return YT_OK;
}
/******************************************************************************
* Function Name  : cm256_set_name
* Description    :   
* Input          :  
* Return         : 0 indicates success,others indicates failed
******************************************************************************/
int cm256_set_name(unsigned char *paucInBtName, unsigned char *pucInLen)
{

	if ((strlen((const char *)paucInBtName)) > BLE_NAME_SIZE)
	{
		return YT_ERR;
	}

	if (*pucInLen == (strlen((const char *)paucInBtName)))
    {
    	memset(ble_info.aucDevName, 0 ,sizeof(ble_info.aucDevName));
    	memcpy(ble_info.aucDevName, paucInBtName, *pucInLen);
		log_e(LOG_BLE, "cm256_set_name = %s",ble_info.aucDevName);
    }
	else
	{
		return YT_ERR;
	}

	return YT_OK;
}
/******************************************************************************
* Function Name  : cm256_get_mac
* Description    :   
* Input          :  
* Return         : 0 indicates success,others indicates failed
******************************************************************************/
int cm256_get_mac(unsigned char *paucBtMac, unsigned char *pucLen)
{
   if (6 != ble_info.ucMacLen)
   {
		*pucLen = 0;
		return YT_ERR;
   }
   
  	log_i(LOG_BLE,"cm256_get_mac %02x:%02x:%02x:%02x:%02x:%02x",
             ble_info.aucDevMac[0], ble_info.aucDevMac[1],
             ble_info.aucDevMac[2], ble_info.aucDevMac[3],
             ble_info.aucDevMac[4], ble_info.aucDevMac[5]);

	memcpy(paucBtMac,(const void *)ble_info.aucDevMac, 6);
	*pucLen = 6;

	return YT_OK;
}
/******************************************************************************
* Function Name  : cm256_send
* Description    :   
* Input          :  
* Return         : 0 indicates success,others indicates failed
******************************************************************************/
int cm256_send(unsigned char *pucBuf, unsigned int *pulLen)
{
    log_i(LOG_BLE, "cm256_send");
	log_i(LOG_BLE, "*pulLen = %d",*pulLen);
	if (*pulLen <= 0)
	{
		return YT_ERR;
	}
	
	ble_send_notification(pucBuf, pulLen);
    return YT_OK;
}
/******************************************************************************
* Function Name  : cm256_recv
* Description    :   
* Input          :  
* Return         : 0 indicates success,others indicates failed
******************************************************************************/
int cm256_recv(unsigned char *pucBuf, unsigned int *pulLen)
{
	int iRet = YT_ERR;
	rb_clean(&stTxRb);
	
	iRet = rb_out(&stRxRb, pucBuf, *pulLen);
	if (YT_ERR == iRet)
	{
		//log_i(LOG_BLE, "Nf3303Recv==iRet  = %d\r\n", iRet);
	}
	else
	{
		*pulLen =  iRet;
	}
	return iRet;
}

/******************************************************************************
* Function Name  : cm256_apiget_recv
* Description    :   privte
* Input          :  
* Return         : 0 indicates success,others indicates failed
******************************************************************************/
int cm256_apiget_recv(unsigned char *pucInBuf, unsigned int *ulInLen)
{
    u32 ulTmpLen = *ulInLen;
	log_i(LOG_BLE, "Nf3303ApiGetRecv\r\n");
	
	log_i(LOG_BLE, "ulTmpLen = %d\r\n",ulTmpLen);
	ApiBLETraceBuf(pucInBuf, ulTmpLen);
	log_i(LOG_BLE, "rb_unused_len(&stTxRb)  = %d\r\n",rb_unused_len(&stRxRb) );
	log_i(LOG_BLE, "stTxRb.size)  = %d\r\n", stRxRb.size);
	log_i(LOG_BLE, "stTxRb.in  = %d\r\n", stRxRb.in);
	log_i(LOG_BLE, "stTxRb.out  = %d\r\n", stRxRb.out);

    if (rb_unused_len(&stRxRb) < ulTmpLen)
    {
        log_e(LOG_BLE, "too long data");
        return YT_ERR;
    }
	
    rb_in(&stRxRb, pucInBuf, ulTmpLen);
	
	BleSendMsg(BLE_MSG_RECV_TYPE, 1);	
	log_i(LOG_BLE, "Nf3303ApiGetRecv\r\n");
	return YT_OK;
}
/******************************************************************************
* Function Name  : cm256_apiset_send
* Description    :   privte
* Input          :  
* Return         : 0 indicates success,others indicates failed
******************************************************************************/
int cm256_apiset_send(unsigned char *pucOutBuf, unsigned int *ulOutLen)
{
	return YT_OK;
}
/******************************************************************************
* Function Name  : cm256_apiget_state
* Description    :   privte
* Input          :  
* Return         : 0 indicates success,others indicates failed
******************************************************************************/
int cm256_apiget_state(unsigned char *pucInRemot, int ulInState)
{

   	pthread_mutex_lock(&cm256Statemutex);

	if (BLE_CONNECT == ulBleStatus)
	{
		memcpy((char *)g_NfBleMsg.aucRemoteAddress, pucInRemot, 6);
		//DEBUG("Gatt Request read event :  [%02X:%02X:%02X:%02X:%02X:%02X]\r\n",
        //    g_NfBleMsg.aucRemoteAddress[0], g_NfBleMsg.aucRemoteAddress[1], g_NfBleMsg.aucRemoteAddress[2],
         //   g_NfBleMsg.aucRemoteAddress[3], g_NfBleMsg.aucRemoteAddress[4], g_NfBleMsg.aucRemoteAddress[5]);
	}
    pthread_mutex_unlock(&cm256Statemutex);
   
	if (BLE_CONNECT == ulBleStatus)
	{
		BleSendMsg(BLE_MSG_CONNECT, 1);
	}
	else
	{
		BleSendMsg(BLE_MSG_DISCONNECT, 1);
	}
	return YT_OK;
}
/******************************************************************************
* Function Name  : cm256_check_len
* Description    :    
* Input          :  
* Return         : 0 indicates success,others indicates failed
******************************************************************************/
int cm256_check_len(void)
{
	unsigned char  ucTmpBuff[UUID_SIZE] = {0};

	rb_used_len(&stRxRb);
	rb_get(&stRxRb, ucTmpBuff, 0, UUID_SIZE);
    return YT_OK;	
}




