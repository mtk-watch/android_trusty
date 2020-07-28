/* 
* Copyright (C) 2015 MediaTek Inc. 
*
* Modification based on code covered by the below mentioned copyright
* and/or permission notice(S). 
*/

/*
 * Copyright (c) 2013-2015, Google, Inc. All rights reserved
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

#include <err.h>
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>
#include <trace.h>
#include <lib/sm.h>
#include <lib/sm/smcall.h>
#include <reg.h>
#include <lk/init.h>
#include <arch/arm.h>
#include <arch/defines.h>
#include <arch/ops.h>
#include <dev/interrupt/arm_gic.h>
#if WITH_TIMERS_MIGRATION
#include <kernel/timer.h>
#endif
#include <platform/reg_base.h>

#define	BOOTROM_BOOT_ADDR	(INFRACFG_AO_BASE + 0x800)
#define	BOOTROM_SEC_CTRL	(INFRACFG_AO_BASE + 0x804)
#define SW_ROM_PD		(1U << 31)

#if 0
asm __volatile__ ("b .");
#endif
#define LOCAL_TRACE 1
#define DISABLE_PRINT_FOR_DORMANT 1
#define ENABLE_ERRATA_802022 1

#if DISABLE_PRINT_FOR_DORMANT
#define DBG_MSG(level, ...) do {} while(0)
#else
#define DBG_MSG(level, ...) do {dprintf(level, ##__VA_ARGS__);} while(0)
#endif

unsigned int ns_reset_entry = 0;
unsigned int ns_resume_entry = 0;
static unsigned int cpu_is_initialized[SMP_MAX_CPUS] = {1, 0, 0, 0};
static unsigned int cpu_on_entry[SMP_MAX_CPUS] = {0, 0, 0, 0};
uint8_t resume_stack[ARCH_DEFAULT_STACK_SIZE * SMP_MAX_CPUS] __CPU_ALIGN;

enum cpu_power_on_type {
	CPU_ON_NORMAL = 0,
	CPU_ON_ERRATA_802022_FLOW = 1,
};
static unsigned int cpu_on_type[SMP_MAX_CPUS] = {CPU_ON_NORMAL};

void second_boot_reset(void);
void __inner_flush_dcache_all(void);

/* the sequence between reg_accessor_type and reg_accessors must be the same */
enum reg_accessor_type {
	PW_MGR_REG_dfsr = 0,
	PW_MGR_REG_ttbr0,
	PW_MGR_REG_ttbr1,
	PW_MGR_REG_ttbcr,
	PW_MGR_REG_tpidrprw,
	PW_MGR_REG_tpidrurw,
	PW_MGR_REG_tpidruro,
	PW_MGR_REG_dacr,
	PW_MGR_REG_ifsr,
	PW_MGR_REG_dfar,
	PW_MGR_REG_ifar,
	PW_MGR_REG_contextidr,
	PW_MGR_REG_vbar,
	PW_MGR_REG_scr,
	PW_MGR_REG_actlr,
	PW_MGR_REG_sctlr,
	PW_MGR_REG_nsacr,
	PW_MGR_REG_cpacr,
#if ARM_WITH_VFP	
	PW_MGR_REG_fpexc,
#endif	
};

struct reg {
	uint32_t val;
};

struct reg_accessor {
	uint32_t (*read)(void);
	void (*write)(uint32_t val);
	uint32_t idx;
};

#define CPU_CONTEXT_REG(name) \
	{ \
		.read = arm_read_##name, \
		.write = arm_write_##name, \
		.idx = PW_MGR_REG_##name, \
	}
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/* the sequence between reg_accessor_type and reg_accessors must be the same */
static struct reg_accessor reg_accessors[] = {
	CPU_CONTEXT_REG(dfsr),
	CPU_CONTEXT_REG(ttbr0),
	CPU_CONTEXT_REG(ttbr1),
	CPU_CONTEXT_REG(ttbcr),
	CPU_CONTEXT_REG(tpidrprw),
	CPU_CONTEXT_REG(tpidrurw),
	CPU_CONTEXT_REG(tpidruro),
	CPU_CONTEXT_REG(dacr),
	CPU_CONTEXT_REG(ifsr),
	CPU_CONTEXT_REG(dfar),
	CPU_CONTEXT_REG(ifar),
	CPU_CONTEXT_REG(contextidr),
	CPU_CONTEXT_REG(vbar),
	CPU_CONTEXT_REG(scr),
	CPU_CONTEXT_REG(actlr),//update for ACTLR:SMP[S/NS]
	CPU_CONTEXT_REG(sctlr),//update for SCTLR.C[S]=1
	CPU_CONTEXT_REG(nsacr),//update for NSACR:NS_SMP[S]
	CPU_CONTEXT_REG(cpacr),
#if ARM_WITH_VFP	
	CPU_CONTEXT_REG(fpexc),
#endif	
};

struct cpu_context {
	struct reg cpu_regs[ARRAY_SIZE(reg_accessors)];
};
static struct cpu_context cpu_contexts[SMP_MAX_CPUS] __CPU_ALIGN;

static void save_cpu_context(struct cpu_context *ctx) {
	uint i, num = ARRAY_SIZE(reg_accessors);

	for (i = 0; i < num; i++) {
		ctx->cpu_regs[i].val = reg_accessors[i].read();
	}

#if 0
	DBG_MSG(INFO, "save_cpu_context : reg dfsr = 0x%x, 0x%x \n", arm_read_dfsr(), ctx->cpu_regs[PW_MGR_REG_dfsr].val);
	DBG_MSG(INFO, "save_cpu_context : reg ttbr0 = 0x%x, 0x%x \n", arm_read_ttbr0(), ctx->cpu_regs[PW_MGR_REG_ttbr0].val);
	DBG_MSG(INFO, "save_cpu_context : reg ttbr1 = 0x%x, 0x%x \n", arm_read_ttbr1(), ctx->cpu_regs[PW_MGR_REG_ttbr1].val);
	DBG_MSG(INFO, "save_cpu_context : reg ttbcr = 0x%x, 0x%x \n", arm_read_ttbcr(), ctx->cpu_regs[PW_MGR_REG_ttbcr].val);
	DBG_MSG(INFO, "save_cpu_context : reg tpidrprw = 0x%x, 0x%x \n", arm_read_tpidrprw(), ctx->cpu_regs[PW_MGR_REG_tpidrprw].val);
	DBG_MSG(INFO, "save_cpu_context : reg tpidrurw = 0x%x, 0x%x \n", arm_read_tpidrurw(), ctx->cpu_regs[PW_MGR_REG_tpidrurw].val);
	DBG_MSG(INFO, "save_cpu_context : reg tpidruro = 0x%x, 0x%x \n", arm_read_tpidruro(), ctx->cpu_regs[PW_MGR_REG_tpidruro].val);
	DBG_MSG(INFO, "save_cpu_context : reg dacr = 0x%x, 0x%x \n", arm_read_dacr(), ctx->cpu_regs[PW_MGR_REG_dacr].val);
	DBG_MSG(INFO, "save_cpu_context : reg ifsr = 0x%x, 0x%x \n", arm_read_ifsr(), ctx->cpu_regs[PW_MGR_REG_ifsr].val);
	DBG_MSG(INFO, "save_cpu_context : reg dfar = 0x%x, 0x%x \n", arm_read_dfar(), ctx->cpu_regs[PW_MGR_REG_dfar].val);
	DBG_MSG(INFO, "save_cpu_context : reg ifar = 0x%x, 0x%x \n", arm_read_ifar(), ctx->cpu_regs[PW_MGR_REG_ifar].val);
	DBG_MSG(INFO, "save_cpu_context : reg contextidr = 0x%x, 0x%x \n", arm_read_contextidr(), ctx->cpu_regs[PW_MGR_REG_contextidr].val);
	DBG_MSG(INFO, "save_cpu_context : reg vbar = 0x%x, 0x%x \n", arm_read_vbar(), ctx->cpu_regs[PW_MGR_REG_vbar].val);
	DBG_MSG(INFO, "save_cpu_context : reg scr = 0x%x, 0x%x \n", arm_read_scr(), ctx->cpu_regs[PW_MGR_REG_scr].val);
	DBG_MSG(INFO, "save_cpu_context : reg actlr = 0x%x, 0x%x \n", arm_read_actlr(), ctx->cpu_regs[PW_MGR_REG_actlr].val);
	DBG_MSG(INFO, "save_cpu_context : reg sctlr = 0x%x, 0x%x \n", arm_read_sctlr(), ctx->cpu_regs[PW_MGR_REG_sctlr].val);
	DBG_MSG(INFO, "save_cpu_context : reg nsacr = 0x%x, 0x%x \n", arm_read_nsacr(), ctx->cpu_regs[PW_MGR_REG_nsacr].val);
	DBG_MSG(INFO, "save_cpu_context : reg cpacr = 0x%x, 0x%x \n", arm_read_cpacr(), ctx->cpu_regs[PW_MGR_REG_cpacr].val);
#if ARM_WITH_VFP
	DBG_MSG(INFO, "save_cpu_context : reg fpexc = 0x%x, 0x%x \n", arm_read_fpexc(), ctx->cpu_regs[PW_MGR_REG_fpexc].val);
#endif
#endif
}

static void restore_cpu_context(struct cpu_context *ctx) {
	uint i, num = ARRAY_SIZE(reg_accessors);

	for (i = 0; i < num; i++) {
		reg_accessors[i].write(ctx->cpu_regs[i].val);
	}
}

static void mt_platform_save_context(uint32_t cpu_id)
{
	uint32_t sctlr_val = 0;

	/* Save driver related context if have */
	lk_init_level_all(LK_INIT_FLAG_CPU_SUSPEND);

	/* Save platform related context */
	save_cpu_context(&cpu_contexts[cpu_id]);
	arch_clean_invalidate_cache_range((addr_t)&cpu_contexts[cpu_id], sizeof(struct cpu_context)*SMP_MAX_CPUS);

	/* Disable dcache & flush L1/L2 */
	sctlr_val = arm_read_sctlr();
	sctlr_val &= ~(1<<2);
	arm_write_sctlr(sctlr_val);

	if (cpu_id == 0) {
		__asm __volatile__ ("blx __inner_flush_dcache_all");
	} else {
		__asm __volatile__ ("blx __inner_flush_dcache_all");
	}
}

static void mt_platform_restore_context(uint32_t cpu_id)
{
	struct cpu_context *ctx = &cpu_contexts[cpu_id];
	uint32_t tmp_val = 0;

	/* update for ACTLR:SMP[S/NS]=1 */
	tmp_val = ctx->cpu_regs[PW_MGR_REG_actlr].val;
	tmp_val |= (1<<6);
	ctx->cpu_regs[PW_MGR_REG_actlr].val = tmp_val;

#if ENABLE_ERRATA_802022
	if (cpu_id == 0 || CPU_ON_ERRATA_802022_FLOW == cpu_on_type[cpu_id]) {
		/* For dormant, cache should be disabled here and later enabled in fastcall SMC_FC_CPU_ERRATA_802022 */
		tmp_val = ctx->cpu_regs[PW_MGR_REG_sctlr].val;
		tmp_val &= ~(1<<2);
		ctx->cpu_regs[PW_MGR_REG_sctlr].val = tmp_val;
	} else {
		/* For hot-plug, update for SCTLR.C[S]=1 to enable cache */
		tmp_val = ctx->cpu_regs[PW_MGR_REG_sctlr].val;
		tmp_val |= (1<<2);
		ctx->cpu_regs[PW_MGR_REG_sctlr].val = tmp_val;
	}
#else
	/* update for SCTLR.C[S]=1 */
	tmp_val = ctx->cpu_regs[PW_MGR_REG_sctlr].val;
	tmp_val |= (1<<2);
	ctx->cpu_regs[PW_MGR_REG_sctlr].val = tmp_val;
#endif

	/* update for NSACR:NS_SMP[S]=1 */
	tmp_val = ctx->cpu_regs[PW_MGR_REG_nsacr].val;
	tmp_val |= (1<<18);
	ctx->cpu_regs[PW_MGR_REG_nsacr].val = tmp_val;

	/* Restore platform related context */
	restore_cpu_context(ctx);

	/* For the secondary CPUs which are powered on for errata_802022, no need to restore driver context
	    Because it will be powered down soon later by core 0
	*/
	if(CPU_ON_ERRATA_802022_FLOW == cpu_on_type[cpu_id])
		return;

	/* Restore driver related context if have */
	lk_init_level_all(LK_INIT_FLAG_CPU_RESUME);
}

void arm_cpu_resume(uint32_t cpu_id) {

	mt_platform_restore_context(cpu_id);
}

long mt_cpu_on(uint32_t nwd_reset_addr, uint32_t cpu_id, uint32_t cpu_pwd_on_type)
{
	long res = NO_ERROR;

	DBG_MSG(INFO, "cpu %d on function : nwd reset addr 0x%x\n", cpu_id, nwd_reset_addr);
	DBG_MSG(INFO, "cpu %d on function : cpu_pwd_on_type 0x%x\n", cpu_id, cpu_pwd_on_type);
	DBG_MSG(INFO, "cpu %d on function : power down ctrl 0x%x\n", cpu_id, readl(BOOTROM_SEC_CTRL));
	DBG_MSG(INFO, "cpu %d on function : power down addr 0x%x\n", cpu_id, readl(BOOTROM_BOOT_ADDR));

	ns_reset_entry = nwd_reset_addr;

	/* enable BROM power down mode */
	writel(readl(BOOTROM_SEC_CTRL) | SW_ROM_PD, BOOTROM_SEC_CTRL);

	/* set BROM power down addr */
	if (!cpu_is_initialized[cpu_id]) {
		cpu_is_initialized[cpu_id] = 1;
		cpu_on_entry[cpu_id] = (MEMBASE + KERNEL_LOAD_OFFSET);
	} else {
		//Change to another reset address for CPU restore flow
		cpu_on_entry[cpu_id] = (uint32_t)second_boot_reset;
	}

	writel(cpu_on_entry[cpu_id], BOOTROM_BOOT_ADDR);

	/* save & flush cpu power on type for later used by secondary core */
	cpu_on_type[cpu_id] = cpu_pwd_on_type;
	arch_clean_invalidate_cache_range((addr_t)cpu_on_type, sizeof(unsigned int)*SMP_MAX_CPUS);

	DBG_MSG(INFO, "cpu %d on function : power down ctrl 0x%x\n", cpu_id, readl(BOOTROM_SEC_CTRL));
	DBG_MSG(INFO, "cpu %d on function : power down addr 0x%x\n", cpu_id, readl(BOOTROM_BOOT_ADDR));

	return res;
}

long mt_cpu_dormant(uint32_t nwd_resume_addr, uint32_t cpu_id, uint32_t reserved)
{
	long res = NO_ERROR;

	/* Set REE resume entry and flush cache */
	ns_resume_entry = nwd_resume_addr;
	arch_clean_invalidate_cache_range((addr_t)&ns_resume_entry, sizeof(unsigned int));

	/* enable BROM power down mode */
	writel(readl(BOOTROM_SEC_CTRL) | SW_ROM_PD, BOOTROM_SEC_CTRL);

	/* set BROM power down addr */
	writel(cpu_on_entry[cpu_id], BOOTROM_BOOT_ADDR);

	/* do backup for CPU core 0 */
	mt_platform_save_context(cpu_id);

	return res;
}

long mt_cpu_dormant_cancel(uint32_t reserved1, uint32_t reserved2, uint32_t reserved3)
{
	long res = NO_ERROR;
	uint32_t tmp_val = 0;

	/* actually system register settings are still remained if CPU is not powered down */

	/* update for SCTLR.C[S]=1 */
	tmp_val = arm_read_sctlr();
	tmp_val |= (1<<2);
	arm_write_sctlr(tmp_val);
	
	/* update for NSACR:NS_SMP[S]=1 */
	tmp_val = arm_read_nsacr();
	tmp_val |= (1<<18);
	arm_write_nsacr(tmp_val);

	tm_resume_cpu(0);

	return res;
}

long mt_cpu_off(uint32_t reserved1, uint32_t cpu_id, uint32_t reserved2)
{
	long res = NO_ERROR;

	DBG_MSG(INFO, "cpu %d off : reserved param1 0x%x\n", cpu_id, reserved1);
	DBG_MSG(INFO, "cpu %d off : reserved param2 0x%x\n", cpu_id, reserved2);

	/* do backup for CPU core 1/2/3 */
	mt_platform_save_context(cpu_id);

	return res;
}

long mt_cpu_errata_802022(uint32_t reserved1, uint32_t reserved2, uint32_t reserved3)
{
	long res = NO_ERROR;

#if ENABLE_ERRATA_802022
	uint32_t tmp_val = 0;

	/* update for SCTLR.C[S]=1 to enable dcache */
	tmp_val = arm_read_sctlr();
	tmp_val |= (1<<2);
	arm_write_sctlr(tmp_val);
#endif

	return res;
}

static void platform_power_mgr_init(uint level)
{
	//Change to another reset address for CPU restore flow for core 0
	cpu_on_entry[0] = (uint32_t)second_boot_reset;
	
	cpu_on_entry[1] = (MEMBASE + KERNEL_LOAD_OFFSET);
	cpu_on_entry[2] = (MEMBASE + KERNEL_LOAD_OFFSET);
	cpu_on_entry[3] = (MEMBASE + KERNEL_LOAD_OFFSET);

#if 0	
	DBG_MSG(INFO, "cpu_on_entry[0]: 0x%x\n", cpu_on_entry[0]);
	DBG_MSG(INFO, "cpu_on_entry[1]: 0x%x\n", cpu_on_entry[1]);
	DBG_MSG(INFO, "cpu_on_entry[2]: 0x%x\n", cpu_on_entry[2]);
	DBG_MSG(INFO, "cpu_on_entry[3]: 0x%x\n", cpu_on_entry[3]);
#endif
}

LK_INIT_HOOK(platform_init_pm, platform_power_mgr_init, LK_INIT_LEVEL_VM + 3);

