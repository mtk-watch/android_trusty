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

#ifndef __PLATFORM_REG_BASE_H
#define __PLATFORM_REG_BASE_H

#define AP_XGPT_BASE		0x10008000

#define MCUSYS_CFG_BASE		0x10200000

#define INFRACFG_AO_BASE	0x10001000

#define UART0_BASE		0x11005000
#define UART1_BASE		0x11006000
#define UART2_BASE		0x11007000

#define GIC_BASE_PHY_ADDR	0x10211000

#define TRNG_BASE		0x1020C000
#define TRNG_PDN_BASE		0x10000000
#define TRNG_PDN_OFFSET		(5)
#define TRNG_PDN_SET_OFFSET	(0x6C)
#define TRNG_PDN_CLEAR_OFFSET	(0x9C)
#define TRNG_PDN_STATUS_OFFSET	(0x3C)

#define HACC_BASE		0x1000A000

#endif

