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

#include <err.h>
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>
#include <trace.h>
#include <lk/init.h>
#include <arch/mmu.h>
#include <lib/sm.h>
#include <lib/sm/smcall.h>
#include <platform/power_mgr.h>

#define LOCAL_TRACE 1

/*
 *  Handle SIP Service calls function
 */
static long trusty_sip_fastcall(smc32_args_t *args)
{
	long res;

/*
	LTRACEF("Trusty SIP call service func %u args 0x%x 0x%x 0x%x\n",
		SMC_FUNCTION(args->smc_nr),
		args->params[0],
		args->params[1],
		args->params[2]);
*/

	switch (args->smc_nr) {

	case SMC_FC_CPU_ON:
		res = mt_cpu_on(args->params[0], args->params[1], args->params[2]);
		break;

	case SMC_FC_CPU_DORMANT:
		res = mt_cpu_dormant(args->params[0], args->params[1], args->params[2]);
		break;

	case SMC_FC_CPU_DORMANT_CANCEL:
		res = mt_cpu_dormant_cancel(args->params[0], args->params[1], args->params[2]);
		break;
	
	case SMC_FC_CPU_OFF:
		res = mt_cpu_off(args->params[0], args->params[1], args->params[2]);
		break;
		
	case SMC_FC_CPU_ERRATA_802022:
		res = mt_cpu_errata_802022(args->params[0], args->params[1], args->params[2]);
		break;

	default:
		LTRACEF("unknown func 0x%x\n", SMC_FUNCTION(args->smc_nr));
		res = ERR_NOT_SUPPORTED;
		break;
	}

	return res;
}

static smc32_entity_t trusty_sipcall_entity = {
	.fastcall_handler = trusty_sip_fastcall,
};

static void trusty_sipcall_init(uint level)
{
	int err;

	dprintf(INFO, "Initializing SIP SMC handler\n");

	err = sm_register_entity(SMC_ENTITY_SIP, &trusty_sipcall_entity);
	if (err) {
		TRACEF("WARNING: Cannot register SIP SMC entity! (%d)\n", err);
	}
}
LK_INIT_HOOK(trusty_sipcall, trusty_sipcall_init, LK_INIT_LEVEL_APPS);

