#ifndef		_PP_CAMERACTRL_H
#define		_PP_CAMERACTRL_H

typedef struct
{
	uint8_t req;
	long reqType;
	uint8_t CtrlSt;
	uint64_t period;
	uint8_t waitSt;
	uint64_t waittime;
	long expTime;
	char style;//��ʽ��tsp-1��2-����
}__attribute__((packed))  PP_rmtCameraCtrlSt_t; /*remote control�ṹ��*/


extern void PP_CameraCtrl_init(void);
extern int PP_CameraCtr_mainfunction(void *task);
extern void  PP_CameraCtrl_SetCtrlReq(unsigned char req,uint16_t reqType);
extern void PP_CameraCtrl_ClearStatus(void);
extern int PP_CameraCtrl_end(void);
extern int PP_CameraCtrl_start(void);
extern int SetPP_CameraCtrl_Request(char ctrlstyle,void *appdatarmtCtrl,void *disptrBody);
#endif




