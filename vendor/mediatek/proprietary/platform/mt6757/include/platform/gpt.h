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

#define INDEX_BASE  	(CPUXGPT_BASE+0x0674)
#define CTL_BASE    	(CPUXGPT_BASE+0x0670)

#define CLK_DIV1  (0x1 << 8)
#define CLK_DIV2  (0x2 << 8)
#define CLK_DIV4  (0x4 << 8)
#define CLK_DIV_MASK (~(0x7<<8))

#define INDEX_CTL_REG  0x000
#define EN_CPUXGPT 0x01

extern void setup_syscnt(void);
#endif
