/****************************************************************
file:         dev_rw.c
description:  the source file of device read and write implementation
date:         2016/9/28
author        liuzhongwen
****************************************************************/
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>
#include <memory.h>
#include "dev_rw.h"
#include "log.h"

/*********************************************
function:     dev_read
description:  read specified bytes data from file descriptor
input:        int fd,  file descriptor;
              unsigned int len, the expected read data len
output:       unsigned char *buf, read data into the buf
return:       0 indicates success;
              others indicates failed
*********************************************/
int dev_read(int fd, unsigned char *buf, unsigned int len)
{
    int readcnt = 0;
    int ret;

    if (fd < 0)
    {
        log_e(LOG_MID, "dev_read invalid fd");
        return DEV_RW_INVALID_PARAMETER;
    }

    while (readcnt < len)
    {
        ret = read(fd, buf + readcnt, len - readcnt);

        if (ret > 0)
        {
            readcnt +=  ret;
        }
        else if (0 == ret)   /* timeout */
        {
            continue;        /* continue to monitor the incomging data */
        }
        else
        {
            if (EINTR == errno)  /* interrupted by signal */
            {
                continue;
            }

            log_e(LOG_MID, "read ret:%s", strerror(errno));
            return DEV_RW_READ_FAILED;
        }
    }

    return 0;
}

/*********************************************
function:     dev_write
description:  write specified bytes data to file descriptor
input:        int fd,  file descriptor;
              unsigned int len, the expected write data len
output:       unsigned char *buf, write data from the buf
return:       0 indicates success;
              others indicates failed
*********************************************/
int dev_write(int fd, unsigned char *buf, unsigned int len)
{
    int writecnt = 0;
    int ret;

    if (fd < 0)
    {
        log_e(LOG_MID, "dev_write invalid fd");
        return DEV_RW_INVALID_PARAMETER;
    }

    while (writecnt < len)
    {
        ret = write(fd, buf + writecnt, len - writecnt);

        if (ret > 0)
        {
            writecnt +=  ret;
        }
        else if (0 == ret)   /* timeout */
        {
            continue;        /* continue to monitor the incomging data */
        }
        else
        {
            if (EINTR == errno)  /* interrupted by signal */
            {
                continue;
            }

            log_e(LOG_MID, "write ret:%s", strerror(errno));
            return DEV_RW_READ_FAILED;
        }
    }

    return 0;
}

