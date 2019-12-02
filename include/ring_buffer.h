/****************************************************************
file:         ring_buffer.h
description:  the header file of ring_buffer definition
date:         2016/9/22
author        yuzhimin
****************************************************************/

#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

//�ж�x�Ƿ���2�Ĵη�
#define is_power_of_2(x)((x) != 0 &&(((x) &((x) - 1)) == 0))
//ȡa��b����Сֵ
#define min(a, b)(((a) <(b)) ?(a) :(b))

struct ring_buffer
{
    unsigned char  *buffer;        //������
    unsigned int    size;           //��������С
    unsigned int    in;             //���λ��
    unsigned int    out;            //����λ��
};

int rb_init(struct ring_buffer *ring_buf, unsigned char *buffer, unsigned int size);

unsigned int rb_used_len(const struct ring_buffer *ring_buf);

unsigned int rb_unused_len(const struct ring_buffer *ring_buf);

int rb_out(struct ring_buffer *ring_buf, unsigned char *buffer, unsigned int size);

int rb_in(struct ring_buffer *ring_buf, const unsigned char *buffer, unsigned int size);

int rb_get(struct ring_buffer *ring_buf, unsigned char *buffer, unsigned int offset,
           unsigned int size);

int rb_empty(const struct ring_buffer *ring_buf);

void rb_clean(struct ring_buffer *ring_buf);

#endif /* !__RING_BUFFER_H__ */

