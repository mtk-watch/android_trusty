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

#include <stdio.h>
#include <reg.h>
#include <platform/gpt.h>

typedef unsigned int    UINT32;

#define DRV_Reg32(addr)             readl(addr)
#define DRV_WriteReg32(addr, data)  writel((data), (addr));
#define DRV_SetReg32(addr, data)    writel(DRV_Reg32(addr) | (data), (addr));

struct gpt_device {
	unsigned int id; 
	unsigned int mode;
	unsigned int clksrc;
	unsigned int clkdiv;
	unsigned int cmp[2];
	void (*func)(unsigned long);
	int flags;
	int features;
	unsigned long base_addr;
};

struct gpt_device gpt_devs;

static void __gpt_start(struct gpt_device *dev)
{
	DRV_SetReg32(dev->base_addr + GPT_CON, GPT_CON_ENABLE);
}


static void __gpt_enable_irq(struct gpt_device *dev)
{
	DRV_SetReg32(GPT_IRQEN, 0x1 << (dev->id));
}

static void __gpt_set_cmp(struct gpt_device *dev, unsigned int cmpl, 
		unsigned int cmph)
{
	DRV_WriteReg32(dev->base_addr + GPT_CMP, cmpl);
	dev->cmp[0] = cmpl;

	if (dev->features & GPT_FEAT_64_BIT) {
		DRV_WriteReg32(dev->base_addr + GPT_CMPH, cmph);
		dev->cmp[1] = cmpl;
	} 
}

static void __gpt_set_handler(struct gpt_device *dev, void (*func)(unsigned long))
{
	if (func) {
/*
		if (dev->flags & GPT_ISR)
			handlers[dev->id] = func;
		else {
			tasklet_init(&task[dev->id], func, 0);
			handlers[dev->id] = task_sched;
		}
*/
	}
	dev->func = func;
}


static void __gpt_set_clk(struct gpt_device *dev, unsigned int clksrc, unsigned int clkdiv)
{
	unsigned int clk = (clksrc << GPT_CLKSRC_OFFSET) | clkdiv;
	DRV_WriteReg32(dev->base_addr + GPT_CLK, clk);

	dev->clksrc = clksrc;
	dev->clkdiv = clkdiv;
}

static void __gpt_set_mode(struct gpt_device *dev, unsigned int mode)
{
	unsigned int ctl = DRV_Reg32(dev->base_addr + GPT_CON);
	mode <<= GPT_OPMODE_OFFSET;

	ctl &= ~GPT_CON_OPMODE;
	ctl |= mode;

	DRV_WriteReg32(dev->base_addr + GPT_CON, ctl);

	dev->mode = mode;
}

static void __gpt_set_flags(struct gpt_device *dev, unsigned int flags)
{
	dev->flags |= flags; 
}


static void setup_gpt_dev_locked(struct gpt_device *dev, unsigned int mode, 
		unsigned int clksrc, unsigned int clkdiv, unsigned int cmp, 
		void (*func)(unsigned long), unsigned int flags)
{
	__gpt_set_flags(dev, flags | GPT_IN_USE);

	__gpt_set_mode(dev, mode & GPT_OPMODE_MASK);
	__gpt_set_clk(dev, clksrc & GPT_CLKSRC_MASK, clkdiv & GPT_CLKDIV_MASK);

	if (func)
		__gpt_set_handler(dev, func);

	if (dev->mode != GPT_FREE_RUN) {
		__gpt_set_cmp(dev, cmp, 0);
		if (!(dev->flags & GPT_NOIRQEN)) {
			__gpt_enable_irq(dev);
		}
	}

	if (!(dev->flags & GPT_NOAUTOEN))
		__gpt_start(dev);
}

void setup_syscnt(void)
{
	struct gpt_device *dev = &gpt_devs;

	dev->id = GPT_SYSCNT_ID;
	dev->base_addr = (unsigned long)GPT1_BASE + 0x10 * GPT_SYSCNT_ID;
	dev->features |= GPT_FEAT_64_BIT;

	setup_gpt_dev_locked(dev, GPT_FREE_RUN, GPT_CLK_SRC_SYS, GPT_CLK_DIV_1,
			0, NULL, 0);   

	//    printk("fwq sysc count \n");
}
