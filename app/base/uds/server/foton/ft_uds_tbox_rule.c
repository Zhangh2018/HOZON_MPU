#include "dev_api.h"
#include "ft_uds_tbox_rule.h"
#include "com_app_def.h"
#include "can_api.h"
#include "uds_define.h"
#include "log.h"
#include "cfg_api.h"


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>


#define BASE_DBC_PATH "/usrdata/dbc"
#define CUSTOM_NAME "FOTON"

static char filename[256][256];
static int len = 0;

uint8_t *get_date_yymmdd(char *buf, uint8_t mode, uint8_t *hex2bcd)
{
    char _date[11] = {0};
    char ver[64] = {0};
    int i = 0, month = 0;
    char *ptr1;

    static const char *_month[] =
    {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    };

    if (mode == 0)
    {
        upg_get_mcu_run_ver((void *)&ver, sizeof(ver));
    }
    else if (mode == 1)
    {
        strncpy(ver, COM_APP_SYS_VER, sizeof(COM_APP_SYS_VER));
    }

    ptr1 = strchr(ver, '\\');

    if (ptr1 == NULL)
    {
        log_e(LOG_UDS, "ver format error");
        return NULL;
    }

    strncpy(_date, (strchr(ver, '\\') + 1), 11);

    memcpy(buf, _date + 7, 4);
    memcpy(buf + 6, _date + 4, 2);

    for (i = 0; i < 12; i++)
    {
        if (memcmp(_month[i], _date, 3) == 0)
        {
            month = i + 1;
            break;
        }
    }

    buf[4] = month / 10 % 10 + '0';
    buf[5] = month % 10 + '0';

    if (buf[6] == ' ')
    {
        buf[6] = '0';
    }

    hex2bcd[0] = ((buf[0] - '0') << 4) + (buf[1]) - '0';
    hex2bcd[1] = ((buf[2] - '0') << 4) + (buf[3]) - '0';
    hex2bcd[2] = ((buf[4] - '0') << 4) + (buf[5]) - '0';
    hex2bcd[3] = ((buf[6] - '0') << 4) + (buf[7]) - '0';

    return (uint8_t *)hex2bcd;
}

/*
char *get_uds_sw_ver(UDS_DID_VER* sw, char *data, uint8_t* hex2bcd)
{
    char mcu_ver[64] = {0};
    char mpu_ver[64] = {0};

    upg_get_mcu_run_ver((void *)&mcu_ver, sizeof(mcu_ver));
    upg_get_app_ver((void *)&mpu_ver, sizeof(mpu_ver));

    if (strlen(mcu_ver) == 0)
    {
        return NULL_PTR;
    }

    if (strchr((void *)mpu_ver, '[') == NULL || strchr((void *)mcu_ver, '[') == NULL)
    {
        return NULL_PTR;
    }

    memcpy((void *) & (sw->item), "H4", sizeof(sw->item));
    memcpy((void *) & (sw->hswid), "SW", sizeof(sw->hswid));
    sw->stage = 'A';

    memcpy((void *) & (sw->ver[0]), (strchr((void *)mpu_ver, '[') - 2), 2);
    memcpy((void *) & (sw->ver[2]), (strchr((void *)mcu_ver, '[') - 1), 1);

    get_date_yymmdd(data, 1, hex2bcd);

    memcpy((void *) & (sw->data), (void *)&data[2], 6);

    return (char *)sw;
}
*/

char *get_uds_sw_ver(UDS_DID_VER *sw)
{
    char ver[64]; 
    char ft_hw_ver[14];
    unsigned int length;
    
    memset(ver, 0, sizeof(ver));
    upg_get_mcu_run_ver((void *)ver, sizeof(ver));
    if (strlen(ver) == 0 || strchr((void *)ver, '[') == NULL)
    {
        log_e(LOG_UDS, "mcu ver not syns to mpu!");
        return NULL_PTR;
    } 
    memcpy((void *) & (sw->mcuver), (strchr((void *)ver, '[') - 3), 3);
    
    memset(ver, 0, sizeof(ver));
    upg_get_mcu_blt_ver((void *)ver, sizeof(ver));
    if (strlen(ver) == 0 || strchr((void *)ver, '[') == NULL)
    {
        log_e(LOG_UDS, "bootloader ver not syns to mpu!");
        memset(sw, 0, sizeof(UDS_DID_VER));
        return NULL_PTR;
    }
    memcpy((void *) & (sw->bver), (strchr((void *)ver, '[') - 2), 2);
    
    memset(ver, 0, sizeof(ver));
    upg_get_app_ver((void *)ver, sizeof(ver));
    if (strlen(ver) == 0 || strchr((void *)ver, '[') == NULL)
    {
        log_e(LOG_UDS, "mpu ver blank!");
        memset(sw, 0, sizeof(UDS_DID_VER));
        return NULL_PTR;
    }
    memcpy((void *) & (sw->mpuver[0]), (strchr((void *)ver, '[') - 3), 3);

    memset(ver, 0, sizeof(ver));   
    upg_get_fw_ver((void *)ver, sizeof(ver));
    if (strlen(ver) == 0)
    {
        log_e(LOG_UDS, "firmware blank!");
        memset(sw, 0, sizeof(UDS_DID_VER));
        return NULL_PTR;
    }
    memcpy((void *) & (sw->firmver[0]),  ver + strlen(ver) - 2, 2);

    length = 14;
    memset(ft_hw_ver, 0, sizeof(ft_hw_ver));
    cfg_get_para(CFG_ITEM_FT_UDS_HW, ft_hw_ver, &length);
    
    memcpy((void *) & (sw->item), ft_hw_ver, sizeof(sw->item));
   
    sw->hswid = 'S';
    sw->stage = 'A';

    return (char *)sw;   
}

int trave_dir(char *path, int depth)
{
    DIR *d;//����һ�����
    struct dirent *file;//readdir�����ķ���ֵ�ʹ��������ṹ����
    struct stat sb;

    if (!(d = opendir(path)))
    {
        printf("error opendir %s!!!\n", path);
        return -1;
    }

    while ((file = readdir(d)) != NULL)
    {
        //�ѵ�ǰĿ¼.����һ��Ŀ¼..�������ļ���ȥ����������ѭ������Ŀ¼
        if (strncmp(file->d_name, ".", 1) == 0)
        {
            continue;
        }

        strcpy(filename[len++], file->d_name);//������������ļ���

        //�жϸ��ļ��Ƿ���Ŀ¼�����Ƿ������������㣬�����Ҷ���ֻ����������Ŀ¼��̫��Ͳ����ˣ�ʡ���ѳ�̫���ļ�
        if (stat(file->d_name, &sb) >= 0 && S_ISDIR(sb.st_mode) && depth <= 3)
        {
            trave_dir(file->d_name, depth + 1);
        }
    }

    closedir(d);
    return 0;
}

/*
type:
    M4�Ῠ���綯��        0000 0000
    V1�Ῠ���綯��        0000 0001
    P202���綯��        0000 0010
    M4�п�ȼ�ϵ�أ�0000 0011
    K1���綯��          0000 0100
    P201���綯��        0000 0101
    P203���綯��        0000 0110
    U201���綯��        0000 0111
    M4�Ῠ��ͳ��         0000 1000
    V1�Ῠ��ͳ��         0000 1001
    P202��ͳ��         0000 1010
    M4�п���ͳ��         0000 1011
    K1��ͳ��           0000 1100
    P201��ͳ��         0000 1101
    P203��ͳ��         0000 1110
    U201��ͳ��         0000 1111
*/

/*
mode 0: shellָ���Ҫ�ж�����Ӧ��������DBC CAN2�����ʶԱȣ�
     1��UDS���ó������ͣ���Ҫ����Ӧ��������DBC CAN2�����ʶԱ�
*/
int dbc_set_by_type(unsigned char type, unsigned char mode)
{
    int i = 0;
    char ffpah[256];
    DBC_NAME_STR dbc_name;
    char temp[30];
    char *ptr1 = filename[0];
    char *ptr2 = filename[0];
    unsigned char cfg_power = 0;

    memset((void *)&filename, 0, sizeof(filename));
    memset((void *)&ffpah, 0, sizeof(ffpah));
    len = 0;

    trave_dir(BASE_DBC_PATH, 1);

    for (i = 0; i < len; i++)
    {
        memset((void *)&dbc_name, 0, sizeof(dbc_name));

        ptr2 = strchr(filename[i], '_');

        ptr1 = filename[i];

        if (ptr2 == NULL)
        {
            log_e(LOG_UDS, "dbc file name error!");
            continue;
        }
        else
        {
            memcpy((void *)&dbc_name.custom_name, ptr1, ptr2 - ptr1);
        }

        ptr1 = ptr2 + 1;
        ptr2 = strchr(ptr1, '_');

        if (ptr2 == NULL)
        {
            log_e(LOG_UDS, "dbc file name error!");
            continue;
        }
        else
        {
            memcpy((void *)&dbc_name.car_name, ptr1, ptr2 - ptr1);
        }

        ptr1 = strchr(ptr2 + 1, '[');
        ptr2 = strchr(ptr2 + 1, ']');

        if (ptr1 == NULL || ptr2 == NULL)
        {
            printf("canot find [ or ]\r\n");
            continue;
        }
        else
        {
            memset((void *)&temp, 0, sizeof(temp));
            memcpy(temp, ptr1 + 1, ptr2 - ptr1 - 1);
            temp[ptr2 - ptr1 - 1] = 0;
            dbc_name.type = atoi(temp);
        }


        ptr1 = ptr2 + 2;
        ptr2 = strchr(ptr1, '_');

        if (ptr1 == NULL || ptr2 == NULL)
        {
            log_e(LOG_UDS, "dbc file name error!");
            continue;
        }
        else
        {
            memset((void *)&temp, 0, sizeof(temp));
            memcpy(temp, ptr1, ptr2 - ptr1);
            temp[ptr2 - ptr1] = 0;
            dbc_name.batnum = atoi(temp);
        }

        ptr1 = ptr2 + 1;
        ptr2 = strchr(ptr1, '_');

        if (ptr1 == NULL || ptr2 == NULL)
        {
            log_e(LOG_UDS, "dbc file name error!");
            continue;
        }
        else
        {
            memset((void *)&temp, 0, sizeof(temp));
            memcpy(temp, ptr1, ptr2 - ptr1);
            temp[ptr2 - ptr1] = 0;
            dbc_name.tnum = atoi(temp);
        }

        ptr1 = ptr2 + 1;
        ptr2 = strchr(ptr1, '_');

        if (ptr1 == NULL || ptr2 == NULL)
        {
            log_e(LOG_UDS, "dbc file name error!");
            continue;
        }
        else
        {
            memset((void *)&temp, 0, sizeof(temp));
            memcpy(temp, ptr1, ptr2 - ptr1);
            temp[ptr2 - ptr1] = 0;
            dbc_name.power = atoi(temp);
        }

        ptr1 = ptr2 + 1;
        ptr2 = strchr(ptr1, '_');

        if (ptr1 == NULL || ptr2 == NULL)
        {
            log_e(LOG_UDS, "dbc file name error!");
            continue;
        }
        else
        {
            memset((void *)&temp, 0, sizeof(temp));
            memcpy(temp, ptr1, ptr2 - ptr1);
            temp[ptr2 - ptr1] = 0;
            dbc_name.baud = atoi(temp);
        }

        ptr1 = ptr2 + 1;
        ptr2 = strrchr(ptr1, '_');

        if (ptr1 == NULL || ptr2 == NULL)
        {
            log_e(LOG_UDS, "dbc file name error!");
            continue;
        }
        else
        {
            memcpy(dbc_name.ver, ptr1, ptr2 - ptr1);
        }

        if (dbc_name.type == type)
        {
            if (((mode == 1) && (can_get_auto_baud() == dbc_name.baud || dbc_name.baud  == get_can2_config_baud()))  || (mode == 0))
            {
                cfg_power = (unsigned char)(dbc_name.power); /*compare DBC can2 baud == auto_baud*/
                cfg_set_para(CFG_ITEM_FT_UDS_POWER, (void *)&cfg_power, 1);

                memcpy(ffpah, BASE_DBC_PATH, strlen(BASE_DBC_PATH));
                strcat(ffpah, "/");
                strcat(ffpah, filename[i]);
                can_set_dbc(ffpah);
                return 0;
            }                
            else
            {
                log_e(LOG_UDS, "DBC can2 baud != autobaud\r\n");
                return NRC_FailurePreventsExecutionOfRequestedAction;
            }

        }
    }

    return NRC_RequestOutOfRange;

}




