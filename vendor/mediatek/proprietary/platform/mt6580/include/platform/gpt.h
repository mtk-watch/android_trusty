/*
* Copyright (c) 2015 MediaTek Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _MT_GPT_H_
#define _MT_GPT_H_

#include <platform/reg_base.h>

#define GPT6            0x5
#define GPT_SYSCNT_ID       (GPT6)
#define GPT_CLK_SRC_SYS 0x0

#define GPT_IRQEN           (AP_XGPT_BASE + 0x0000)
#define GPT_IRQSTA          (AP_XGPT_BASE + 0x0004)
#define GPT_IRQACK          (AP_XGPT_BASE + 0x0008)
#define GPT1_BASE           (AP_XGPT_BASE + 0x0010)

#define GPT_CON             (0x00)
#define GPT_CLK             (0x04)
#define GPT_CNT             (0x08)
#define GPT_CMP             (0x0C)
#define GPT_CNTH            (0x18)
#define GPT_CMPH            (0x1C)


#define GPT_CON_ENABLE      (0x1 << 0)
#define GPT_CON_CLRCNT      (0x1 << 1)
#define GPT_CON_OPMODE      (0x3 << 4)

#define GPT_OPMODE_MASK     (0x3)
#define GPT_CLKDIV_MASK     (0xf)
#define GPT_CLKSRC_MASK     (0x1)

#define GPT_OPMODE_OFFSET   (4)
#define GPT_CLKSRC_OFFSET   (4)

#define GPT_ISR             (0x0010)
#define GPT_IN_USE          (0x0100)
#define GPT_FEAT_64_BIT     (0x0001)

#define GPT_FREE_RUN    0x3
#define GPT_NOAUTOEN    0x0001
#define GPT_NOIRQEN     0x0002

#define GPT_CLK_DIV_1   0x0000

void setup_syscnt(void);

#endif
