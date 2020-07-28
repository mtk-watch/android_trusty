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
#include <platform/gic.h>
#include <platform/teeargs.h>
#include <platform/uart.h>
#include <platform/gpt.h>
#include <string.h>
#include <reg.h>
#include <arch/arm.h>
#include <mt_rpmb.h>
#include <platform/reg_base.h>

#include "smc.h"

#define ARM_GENERIC_TIMER_INT_CNTV 27
#define ARM_GENERIC_TIMER_INT_CNTPS 29
#define ARM_GENERIC_TIMER_INT_CNTP 30

#define ARM_GENERIC_TIMER_INT_SELECTED(timer) ARM_GENERIC_TIMER_INT_ ## timer
#define XARM_GENERIC_TIMER_INT_SELECTED(timer) ARM_GENERIC_TIMER_INT_SELECTED(timer)
#define ARM_GENERIC_TIMER_INT XARM_GENERIC_TIMER_INT_SELECTED(TIMER_ARM_GENERIC_SELECTED)

#define MP0_AXI_CONFIG          (MCUSYS_CFG_BASE + 0x20)
#define ACINACTM                (1 << 4)
#define TIMER_FREQ 13000000

#define MAX_STR_SIZE 128

/* saved by the main entry point in lk */
extern ulong lk_boot_args[4];
/* boot argements from previous loader */
static tee_v7_arg_t *tee_args = 0;
static tee_v7_arg_t g_tee_args = {0};
unsigned int ns_entry = 0;
unsigned int ns_boot_args = 0;
unsigned int ns_boot_args_size = 0;

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
	  .virt = UART_LOG_REG_BASE,
	  .size = 0x1000,
	  .flags = MMU_INITIAL_MAPPING_FLAG_DEVICE,
	  .name = "uart" },
#endif
#ifdef AP_XGPT_BASE
	{ .phys = AP_XGPT_BASE,
	  .virt = AP_XGPT_BASE,
	  .size = 0x1000,
	  .flags = MMU_INITIAL_MAPPING_FLAG_DEVICE,
	  .name = "gpt" },
#endif
#ifdef INFRACFG_AO_BASE
	{ .phys = INFRACFG_AO_BASE,
	  .virt = INFRACFG_AO_BASE,
	  .size = 0x1000,
	  .flags = MMU_INITIAL_MAPPING_FLAG_DEVICE,
	  .name = "infracfg_ao" },
#endif
#ifdef GIC_BASE_VIRT
	{ .phys = GIC_BASE_VIRT,
	  .virt = GIC_BASE_VIRT,
	  .size = GICD_SIZE + GICC_SIZE,
	  .flags = MMU_INITIAL_MAPPING_FLAG_DEVICE,
	  .name = "gic" },
#endif
#ifdef MCUSYS_CFG_BASE
	{ .phys = MCUSYS_CFG_BASE,
	  .virt = MCUSYS_CFG_BASE,
	  .size = 0x1000,
	  .flags = MMU_INITIAL_MAPPING_FLAG_DEVICE,
	  .name = "mcusys_cfg" },
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

		if (strncmp(m->name, ram_arena.name, MAX_STR_SIZE) == 0) {
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

	ret = vmm_alloc_physical(vmm_get_kernel_aspace(), "gic",
				 GICC_SIZE, &vaddrp, 0, paddr,
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
#if 0
	paddr_t gicc = generic_arm64_get_reg_base(SMC_GET_GIC_BASE_GICC);
	paddr_t gicd = generic_arm64_get_reg_base(SMC_GET_GIC_BASE_GICD);
	
	generic_arm64_map_regs("gicc", GICC_BASE_VIRT, gicc, GICC_SIZE);
	generic_arm64_map_regs("gicd", GICD_BASE_VIRT, gicd, GICD_SIZE);
#else
	paddr_t gicc = g_tee_args.gicc_base;
	paddr_t gicd = g_tee_args.gicd_base;

	dprintf(INFO, "gicc 0x%lx, gicd 0x%lx\n", gicc, gicd);
#endif

	/* initialize the interrupt controller */
	arm_gic_init();

	/* initialize the timer block */
	dprintf(INFO, "timer interrupt id :%d\n", ARM_GENERIC_TIMER_INT);
	setup_syscnt();
	arm_generic_timer_init(ARM_GENERIC_TIMER_INT, TIMER_FREQ);
}

void platform_bootargs_init(uint level)
{	
	status_t err = 0;

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
			tee_args = (tee_v7_arg_t *)((uint8_t *)vptr + offset);
			memcpy(&g_tee_args, tee_args, lk_boot_args[2]);
			vmm_free_region(vmm_get_kernel_aspace(), (vaddr_t)vptr);
			tee_args = NULL;
		} else {
			tee_args = NULL;
			dprintf(SPEW, "Error mapping boot parameter block: %d\n", err);
		}
	}

	/* Parameter check */
	if (err || g_tee_args.magic != TEE_ARGUMENT_MAGIC) {
		panic("boot arguments is not correct!\n");
	}
	
#ifndef UART_LOG_REG_BASE
	/* Map UART port if supplied by the bootloader */
	if (g_tee_args.log_port) {
		ulong offset = g_tee_args.log_port & (PAGE_SIZE - 1);
		paddr_t paddr = ROUNDDOWN(tee_args.log_port, PAGE_SIZE);
		size_t size   = PAGE_SIZE;
		void  *vptr;

		err = vmm_alloc_physical(vmm_get_kernel_aspace(), "uart",
				 size, &vptr, PAGE_SIZE_SHIFT, paddr,
				 0, ARCH_MMU_FLAG_UNCACHED_DEVICE);
		if (err) {
			dprintf(CRITICAL, "%s: failed %d\n", __func__, err);
		}
		platform_set_uart((uint8_t *)vptr + offset, g_tee_args.log_baudrate);
	}
#endif
	
	/* Dump all boot arguments */
	ns_entry = g_tee_args.NWEntry;
	ns_boot_args = g_tee_args.NWBootArgs;
	ns_boot_args_size = g_tee_args.NWBootArgsSize;
	dprintf(INFO, "tee_args.magic: 0x%x\n", g_tee_args.magic);
	dprintf(INFO, "tee_args.version: 0x%x\n", g_tee_args.version);
	dprintf(INFO, "tee_args.NWEntry: 0x%x\n", g_tee_args.NWEntry);
	dprintf(INFO, "tee_args.NWBootArgs: 0x%x\n", g_tee_args.NWBootArgs);
	dprintf(INFO, "tee_args.NWBootArgsSize: 0x%x\n", g_tee_args.NWBootArgsSize);
	dprintf(INFO, "tee_args.dRamBase: 0x%x\n", g_tee_args.dRamBase);
	dprintf(INFO, "tee_args.dRamSize: 0x%x\n", g_tee_args.dRamSize);
	dprintf(INFO, "tee_args.secDRamBase: 0x%x\n", g_tee_args.secDRamBase);
	dprintf(INFO, "tee_args.secDRamSize: 0x%x\n", g_tee_args.secDRamSize);
	dprintf(INFO, "tee_args.sRamBase: 0x%x\n", g_tee_args.sRamBase);
	dprintf(INFO, "tee_args.sRamSize: 0x%x\n", g_tee_args.sRamSize);
	dprintf(INFO, "tee_args.secSRamBase: 0x%x\n", g_tee_args.secSRamBase);
	dprintf(INFO, "tee_args.secSRamSize: 0x%x\n", g_tee_args.secSRamSize);
	dprintf(INFO, "tee_args.log_port: 0x%x\n", g_tee_args.log_port);
	dprintf(INFO, "tee_args.log_baudrate: 0x%x\n", g_tee_args.log_baudrate);
	dprintf(INFO, "tee_args.gicd_base: 0x%x\n", g_tee_args.gicd_base);
	dprintf(INFO, "tee_args.gicc_base: 0x%x\n", g_tee_args.gicc_base);

	/* Config HWUID for RPMB key generate from secure ISRAM */
	plat_set_rpmb_ss_auth_key((uint8_t *)g_tee_args.hwuid, 16);
}

LK_INIT_HOOK(platform_after_vm, platform_after_vm_init, LK_INIT_LEVEL_VM + 2);
LK_INIT_HOOK(bootargs_init, platform_bootargs_init, LK_INIT_LEVEL_VM + 1);

void platform_enable_snoop(void)
{
	volatile unsigned int axi_cfg = 0x0;

	axi_cfg = readl(MP0_AXI_CONFIG);
	axi_cfg &= ~ACINACTM;

	writel(axi_cfg, MP0_AXI_CONFIG);
	DSB;
}

