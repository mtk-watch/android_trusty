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

#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <dev/uart.h>
#include <arch/arm.h>
#include <arch/arm/mmu.h>
#include <arch/ops.h>
#include <reg.h>
#include <platform/reg_base.h>

#define DRV_WriteReg32(reg, value) writel(value, reg)
#define DRV_Reg32(reg) readl(reg)

#if ENABLE_L2_SHARING

#define ADDR_CA7L_CACHE_CONFIG_MP(x) (MCUSYS_CFG_BASE)
#define L2C_SIZE_CFG_OFFSET  5
/* 4'b1111: 2048KB(not support)
 * 4'b0111: 1024KB(not support)
 * 4'b0011: 512KB
 * 4'b0001: 256KB
 * 4'b0000: 128KB (not support)
 */

int is_l2_need_config(void)
{
    volatile unsigned int cache_cfg, addr;

    addr = ADDR_CA7L_CACHE_CONFIG_MP(0);
    cache_cfg = DRV_Reg32(addr);
    cache_cfg = cache_cfg >> L2C_SIZE_CFG_OFFSET;

    /* only read 256KB need to be config.*/
    if((cache_cfg &(0x7)) == 0x1)
    {
        return 1;
    }
    return 0;
}

void cluster_l2_share_enable(int cluster)
{
    volatile unsigned int cache_cfg, addr;

    addr = ADDR_CA7L_CACHE_CONFIG_MP(cluster);
    /* set L2C size to 256KB */
    cache_cfg = DRV_Reg32(addr);
    cache_cfg &= ~(0x7 << L2C_SIZE_CFG_OFFSET);
    cache_cfg |= 0x1 << L2C_SIZE_CFG_OFFSET;
    DRV_WriteReg32(addr, cache_cfg);
}

void cluster_l2_share_disable(int cluster)
{
    volatile unsigned int cache_cfg, addr;

    addr = ADDR_CA7L_CACHE_CONFIG_MP(cluster);
    /* set L2C size to 512KB */
    cache_cfg = DRV_Reg32(addr);
    cache_cfg &= ~(0x7 << L2C_SIZE_CFG_OFFSET);
    cache_cfg |= 0x3 << L2C_SIZE_CFG_OFFSET;
    DRV_WriteReg32(addr, cache_cfg);
}

/* config L2 cache and sram to its size */
void config_L2_size(void)
{
    cluster_l2_share_disable(0);
}

/* config SRAM back from L2 cache for DA relocation */
void config_shared_SRAM_size(void)
{
    cluster_l2_share_enable(0);
}

#endif
