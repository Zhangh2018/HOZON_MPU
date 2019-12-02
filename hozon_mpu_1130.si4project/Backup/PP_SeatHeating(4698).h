

/******************************************************
文件名：PP_SeatHeating.h

描述：	车门锁控制

Data			  Vasion			author
2019/05/18		   V1.0			    liujian
*******************************************************/
#ifndef		_PP_SEATHEATING_H
#define		_PP_SEATHEATING_H
/*******************************************************
description： include the header file
*******************************************************/

/*******************************************************
description： macro definitions
*******************************************************/
/**********宏开关定义*********/

/**********宏常量定义*********/
#define PP_SEATHEATING_IDLE   		0
#define PP_SEATHEATING_REQSTART  	1
#define PP_SEATHEATING_RESPWAIT   	2
#define PP_SEATHEATING_END    		3
/***********宏函数***********/

typedef struct
{
	uint8_t req;
	long reqType;
	uint8_t CtrlSt;
	uint64_t period;
	uint8_t waitSt;
	uint64_t waittime;
	long expTime;
	uint8_t failtype;
	char style;
}__attribute__((packed))  PP_rmtseatheatingSt_t;

typedef enum 
{
	PP_seatheating_drviver = 0,
	PP_seatheating_passenger ,
	PP_seatheating_max,
}PP_RMTCTRL_SEATHEATTYLE;

extern void PP_seatheating_init(void);

extern int PP_seatheating_mainfunction(void *task);

extern uint8_t PP_seatheating_start(void) ;

extern uint8_t PP_seatheating_end(void);

extern int SetPP_seatheating_Request(char ctrlstyle,void *appdatarmtCtrl,void *disptrBody);

extern void  PP_seatheating_SetCtrlReq(uint32_t reqType,unsigned char level);

extern void PP_seatheating_ClearStatus(void);

extern unsigned char GetPP_SeatCtrl_Sleep(void);

extern void PP_SeatCtrl_SeatStMonitor(void *task);

extern uint8_t PP_seatheating_cmdoff(void);

#endif

