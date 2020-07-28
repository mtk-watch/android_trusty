/* 
* Copyright (C) 2015 MediaTek Inc. 
*
* Modification based on code covered by the below mentioned copyright
* and/or permission notice(S). 
*/

/*
 * Copyright (c) 2013, Google Inc. All rights reserved.
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

#include <asm.h>
#include <platform/gic.h>
#if WITH_LIB_SM
#include <lib/sm/monitor.h>
#endif

#if 0
#define CONTEXT_SIZE_SCRATCH_GEN_REGS ((5 + 9) * 4)
#endif

.section ".text"

#if WITH_LIB_SM

FUNCTION(platform_mon_initial_ns_return)
	/* figure out our cpu number */
	mrc	p15, 0, ip, c0, c0, 5 /* read MPIDR */

	/* mask off the bottom 12 bits to test cluster number:cpu number */
	ubfx	ip, ip, #0, #12

	/* if we're not cpu 0:0, wait for ns reset addr */
	cmp	ip, #0
	bne	2f

	/* if we're cpu 0:0, set ns entry address, and boot parameter address */
	/* lr=ns boot addr, r4=boot argument addr, r5=argument size */
	ldr	r4, =ns_entry
	ldr	lr, [r4]
	ldr	r4, =ns_boot_args
	ldr	r4, [r4]
	str	r4, [sp]
	ldr	r4, =ns_boot_args_size
	ldr	r4, [r4]
	str	r4, [sp, #4]
	
	b	mon_initial_ns_return

2:
	ldr	r4, =ns_reset_entry
	ldr	lr, [r4]
#if 0
	mov	r4, #MODE_SVC
	orr	r4, r4, #MODE_SVC_IRQ_DISABLED
	str	r4, [sp, #(CONTEXT_SIZE_SCRATCH_GEN_REGS+4)]
#endif	
	dsb

	b	mon_initial_ns_return
#endif

.ltorg

