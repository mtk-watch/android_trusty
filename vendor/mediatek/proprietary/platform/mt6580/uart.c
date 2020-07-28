/* 
* Copyright (C) 2015 MediaTek Inc. 
*
* Modification based on code covered by the below mentioned copyright
* and/or permission notice(S). 
*/

/*
 * Copyright (c) 2015 Google Inc. All rights reserved
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

#include <lk/init.h>
#include <reg.h>
#include <platform/reg_base.h>

#ifdef UART_LOG_REG_BASE
static unsigned int log_port = UART_LOG_REG_BASE;
static unsigned int log_baudrate = UART_LOG_BAUDRATE;
static unsigned int log_initialized = 1;
#else
static unsigned int log_port = 0;
static unsigned int log_baudrate = 921600;
static unsigned int log_initialized = 0;
#endif
static unsigned int disable_uart = 0;
#define UART_LSR_THRE		(1 << 5)
#define UART_BASE(uart)		(uart)
#define UART_THR(uart)		(UART_BASE(uart) + 0x0)       /* Write only */
#define UART_LSR(uart)		(UART_BASE(uart) + 0x14)

void platform_set_uart(unsigned int port, unsigned int rate)
{
	log_port = port;
	log_baudrate = rate;
	log_initialized = 1;
}

void platform_put_uart_byte (const char c)
{
	if(!log_initialized)
		return;

	if(disable_uart)
		return;

	while (!(readl(UART_LSR(log_port)) & UART_LSR_THRE))
	{
	}

	if (c == '\n')
		writel((unsigned int) '\r', UART_THR(log_port));

	writel((unsigned int) c, UART_THR(log_port));
}


static void platform_disable_uart(unsigned int level)
{
	disable_uart = 1;
}

LK_INIT_HOOK(platform_disable_uart, platform_disable_uart, LK_INIT_LEVEL_LAST);

