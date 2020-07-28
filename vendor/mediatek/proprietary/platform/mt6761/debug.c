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

#include <kernel/thread.h>
#include <platform/debug.h>
#include <platform/uart.h>

#include "smc.h"

static unsigned int uart_enabled = 1;

void platform_enable_uart()
{
	uart_enabled = 1;
}

void platform_disable_uart()
{
	uart_enabled = 0;
}

void platform_dputc(char c)
{
	if (uart_enabled)
		platform_put_uart_byte(c);
}

int platform_dgetc(char *c, bool wait)
{
	int ret = -1;

	while (wait)
		thread_sleep(100);

	return ret;
}
