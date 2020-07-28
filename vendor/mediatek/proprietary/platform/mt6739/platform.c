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

#include <debug.h>
#include <dev/interrupt/arm_gic.h>
#include <dev/timer/arm_generic.h>
#include <kernel/vm.h>
#include <lk/init.h>
#include <mt_rpmb.h>
#include <platform/gic.h>
#include <platform/teeargs.h>
#include <platform/uart.h>
#include <platform/gpt.h>
#include <platform/reg_base.h>
#include <string.h>

#include "smc.h"

#define ARM_GENERIC_TIMER_INT_CNTV 27
#define ARM_GENERIC_TIMER_INT_CNTPS 29
#define ARM_GENERIC_TIMER_INT_CNTP 30

#define ARM_GENERIC_TIMER_INT_SELECTED(timer) ARM_GENERIC_TIMER_INT_ ## timer
#define XARM_GENERIC_TIMER_INT_SELECTED(timer) ARM_GENERIC_TIMER_INT_SELECTED(timer)
#define ARM_GENERIC_TIMER_INT XARM_GENERIC_TIMER_INT_SELECTED(TIMER_ARM_GENERIC_SELECTED)

#define TIMER_FREQ 13000000

/* saved by the main entry point in lk */
extern ulong lk_boot_args[4];

/* rpmb_key generated in preloader */
uint8_t tee_rpmb_key[RPMB_KEY_SIZE] = {0};

/* boot argements from previous loader */
static tee_v8_arg_t *tee_args = 0;

/* initial memory mappings. parsed by start.S */
struct mmu_initial_mapping mmu_initial_mappings[] = {
	/* Mark next entry as dynamic as it might be updated
	   by platform_reset code to specify actual size and
	   location of RAM to use */
	{ .phys = MEMBASE + KERNEL_LOAD_OFFSET,
	  .virt = KERNEL_BASE + KERNEL_LOAD_OFFSET,
	  .size = MEMSIZE,
	  .flags = MMU_INITIAL_MAPPING_FLAG_DYNAMIC,
	  .name = "ram" },
#ifdef UART_LOG_REG_BASE
	{ .phys = UART_LOG_REG_BASE,
	  .virt = KERNEL_BASE + UART_LOG_REG_BASE,
	  .size = 0x1000,
	  .flags = MMU_INITIAL_MAPPING_FLAG_DEVICE,
	  .name = "uart" },
#endif
#ifdef CPUXGPT_BASE
	{ .phys = CPUXGPT_BASE,
	  .virt = KERNEL_BASE + CPUXGPT_BASE,
	  .size = 0x1000,
	  .flags = MMU_INITIAL_MAPPING_FLAG_DEVICE,
	  .name = "cpuxgpt" },
#endif

	/* null entry to terminate the list */
	{ 0 }
};

static pmm_arena_t ram_arena = {
    .name  = "ram",
    .base  =  MEMBASE + KERNEL_LOAD_OFFSET,
    .size  =  MEMSIZE,
    .flags =  PMM_ARENA_FLAG_KMAP
};

void platform_init_mmu_mappings(void)
{
	/* go through mmu_initial_mapping to find dynamic entry
	 * matching ram_arena (by name) and adjust it.
	 */
	struct mmu_initial_mapping *m = mmu_initial_mappings;
	for (uint i = 0; i < countof(mmu_initial_mappings); i++, m++) {
		if (!(m->flags & MMU_INITIAL_MAPPING_FLAG_DYNAMIC))
			continue;

		if (strcmp(m->name, ram_arena.name) == 0) {
			/* update ram_arena */
			ram_arena.base = m->phys;
			ram_arena.size = m->size;
			ram_arena.flags = PMM_ARENA_FLAG_KMAP;

			break;
		}
	}
	pmm_add_arena(&ram_arena);
}

static void generic_arm64_map_regs(const char *name, vaddr_t vaddr,
				   paddr_t paddr, size_t size)
{
	status_t ret;
	void *vaddrp = (void *)vaddr;

	ret = vmm_alloc_physical(vmm_get_kernel_aspace(), name,
				 size, &vaddrp, 0, paddr,
				 VMM_FLAG_VALLOC_SPECIFIC,
				 ARCH_MMU_FLAG_UNCACHED_DEVICE);
	if (ret) {
		dprintf(CRITICAL, "%s: failed %d\n", __func__, ret);
	}
}

static paddr_t generic_arm64_get_reg_base(int reg)
{
#if ARCH_ARM64
	return generic_arm64_smc(SMC_FC64_GET_REG_BASE, reg, 0, 0);
#else
	return generic_arm64_smc(SMC_FC_GET_REG_BASE, reg, 0, 0);
#endif
}

static void platform_after_vm_init(uint level)
{
	/* TODO: Implement SMC in ATF for Trusty to get GICC/GICD reg_base */
	// generic_arm64_get_reg_base(SMC_GET_GIC_BASE_GICC);
	// generic_arm64_get_reg_base(SMC_GET_GIC_BASE_GICD);
	paddr_t gicd = tee_args->gicd_base;
	paddr_t gicr = tee_args->gicr_base;
	size_t gicd_size = GICD_SIZE;
	size_t gicr_size = GICR_SIZE;
	vaddr_t gicdv = GICD_BASE_VIRT;
	vaddr_t gicrv = GICR_BASE_VIRT;

	dprintf(INFO, "gicd 0x%lx, gicr 0x%lx\n", gicd, gicr);
	dprintf(INFO, "gicdv 0x%lx, gicrv 0x%lx\n", gicdv, gicrv);

	generic_arm64_map_regs("gicd", gicdv, gicd, gicd_size);
	generic_arm64_map_regs("gicr", gicrv, gicr, gicr_size);

	/* initialize the interrupt controller */
#if 0   /* ATF already initialize GIC */
	arm_gic_init();
#endif

	/* initialize the timer block */
#if 0
	/* ATF already initialize CPUXGPT */
	setup_syscnt();
#endif
	arm_generic_timer_init(ARM_GENERIC_TIMER_INT, TIMER_FREQ);
}

static void platform_bootargs_init(uint level)
{
	status_t err = -1;

	/* Map the boot arguments if supplied by the bootloader */
	if (lk_boot_args[1] && lk_boot_args[2]) {
		ulong offset = lk_boot_args[1] & (PAGE_SIZE - 1);
		paddr_t paddr = ROUNDDOWN(lk_boot_args[1], PAGE_SIZE);
		size_t size   = ROUNDUP(lk_boot_args[2] + offset, PAGE_SIZE);
		void  *vptr;

		err = vmm_alloc_physical(vmm_get_kernel_aspace(), "bootargs",
				 size, &vptr, PAGE_SIZE_SHIFT, paddr,
				 0, ARCH_MMU_FLAG_UNCACHED_DEVICE);
		if (!err) {
			tee_args = (tee_v8_arg_t *)((uint8_t *)vptr + offset);
		} else {
			tee_args = NULL;
			dprintf(SPEW, "Error mapping boot parameter block: %d\n", err);
		}
	}

	/* Parameter check */
	if (err || tee_args->magic != TEE_ARGUMENT_MAGIC) {
		panic("boot arguments is not correct!\n");
	}

#ifndef UART_LOG_REG_BASE
		/* Map UART port if supplied by the bootloader */
	if (tee_args->log_port) {
		ulong offset = tee_args->log_port & (PAGE_SIZE - 1);
		paddr_t paddr = ROUNDDOWN(tee_args->log_port, PAGE_SIZE);
		size_t size   = PAGE_SIZE;
		void  *vptr;

		err = vmm_alloc_physical(vmm_get_kernel_aspace(), "uart",
				 size, &vptr, PAGE_SIZE_SHIFT, paddr,
				 0, ARCH_MMU_FLAG_UNCACHED_DEVICE);
		if (err) {
			dprintf(CRITICAL, "%s: failed %d\n", __func__, err);
		}
		platform_set_uart((uint32_t)((uint8_t *)vptr + offset), tee_args->log_baudrate);
	}
#endif

	/* Dump all boot arguments */
	dprintf(INFO, "tee_args->magic: 0x%x\n", tee_args->magic);
	dprintf(INFO, "tee_args->version: 0x%x\n", tee_args->version);
	dprintf(INFO, "tee_args->NWEntry: 0x%x\n", tee_args->NWEntry);
	dprintf(INFO, "tee_args->NWBootArgs: 0x%x\n", tee_args->NWBootArgs);
	dprintf(INFO, "tee_args->NWBootArgsSize: 0x%x\n", tee_args->NWBootArgsSize);
	dprintf(INFO, "tee_args->dRamBase: 0x%x\n", tee_args->dRamBase);
	dprintf(INFO, "tee_args->dRamSize: 0x%x\n", tee_args->dRamSize);
	dprintf(INFO, "tee_args->secDRamBase: 0x%x\n", tee_args->secDRamBase);
	dprintf(INFO, "tee_args->secDRamSize: 0x%x\n", tee_args->secDRamSize);
	dprintf(INFO, "tee_args->sRamBase: 0x%x\n", tee_args->sRamBase);
	dprintf(INFO, "tee_args->sRamSize: 0x%x\n", tee_args->sRamSize);
	dprintf(INFO, "tee_args->secSRamBase: 0x%x\n", tee_args->secSRamBase);
	dprintf(INFO, "tee_args->secSRamSize: 0x%x\n", tee_args->secSRamSize);
	dprintf(INFO, "tee_args->gicd_base: 0x%x\n", tee_args->gicd_base);
	dprintf(INFO, "tee_args->gicr_base: 0x%x\n", tee_args->gicr_base);
	dprintf(INFO, "tee_args->gic_ver: 0x%x\n", tee_args->gic_ver);
	dprintf(INFO, "tee_args->log_port: 0x%x\n", tee_args->log_port);
	dprintf(INFO, "tee_args->log_baudrate: 0x%x\n", tee_args->log_baudrate);
	memcpy(tee_rpmb_key, tee_args->rpmb_key, RPMB_KEY_SIZE);

#if 0
	dprintf(INFO, "tee_args->hwuid[0-3] : 0x%x 0x%x 0x%x 0x%x\n",
			tee_args->hwuid[0], tee_args->hwuid[1],
			tee_args->hwuid[2], tee_args->hwuid[3]);
	{
		int i;
		for (i = 0; i < RPMB_KEY_SIZE; i += 8)
		dprintf(INFO, "tee_rpmb_key[%d-%d] : 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
			i, i+7,
			tee_rpmb_key[i], tee_rpmb_key[i+1],
			tee_rpmb_key[i+2], tee_rpmb_key[i+3],
			tee_rpmb_key[i+4], tee_rpmb_key[i+5],
			tee_rpmb_key[i+6], tee_rpmb_key[i+7]);
	}
#endif

	plat_set_rpmb_ss_auth_key((uint8_t*)tee_args->hwuid, 16);
}

static void platform_disable_uart_after_bootup(uint level)
{
	dprintf(INFO, "Disable UART after Trusty bootup!\n");
	platform_disable_uart();
}

LK_INIT_HOOK(bootargs_init, platform_bootargs_init, LK_INIT_LEVEL_VM + 1);
LK_INIT_HOOK(platform_after_vm, platform_after_vm_init, LK_INIT_LEVEL_VM + 2);
LK_INIT_HOOK(disable_uart_after_bootup, platform_disable_uart_after_bootup, LK_INIT_LEVEL_LAST);
