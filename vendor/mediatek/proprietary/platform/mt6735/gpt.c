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

#include <platform/gpt.h>

#define READ_REGISTER_UINT32(reg) \
    (*(volatile unsigned int * const)(reg))

#define WRITE_REGISTER_UINT32(reg, val) \
    (*(volatile unsigned int * const)(reg)) = (val)

#define INREG32(x)          READ_REGISTER_UINT32((unsigned int *)(x))
#define OUTREG32(x, y)      WRITE_REGISTER_UINT32((unsigned int *)(x), (unsigned int )(y))
#define DRV_Reg32(addr)             INREG32(addr)
#define DRV_WriteReg32(addr, data)  OUTREG32(addr, data)

#define mcusys_smc_write_phy(addr, data)	DRV_WriteReg32(addr, data)
#define mcusys_smc_write(addr, data)		DRV_WriteReg32(addr, data)

static unsigned int __read_cpuxgpt(unsigned int reg_index )
{
  	unsigned int value = 0;
	mcusys_smc_write(INDEX_BASE,reg_index);
	value = DRV_Reg32(CTL_BASE);
  	return value;
}

static void __write_cpuxgpt(unsigned int reg_index,unsigned int value )
{
	mcusys_smc_write(INDEX_BASE,reg_index);
	mcusys_smc_write(CTL_BASE,value);
}

static void __cpuxgpt_set_clk(unsigned int div)
{
	unsigned int tmp = 0;
   	
	//printk("%s fwq  div is  0x%x \n",__func__, div);
	if( div!=CLK_DIV1 &&  div!=CLK_DIV2 && div!=CLK_DIV4)
	{
//	     printk("%s error: div is not right \n",__func__);
	}
//	spin_lock(&cpuxgpt_reg_lock);
   	tmp = __read_cpuxgpt(INDEX_CTL_REG);
  	tmp &= CLK_DIV_MASK;
  	tmp |= div;
  	__write_cpuxgpt(INDEX_CTL_REG,tmp);
//	spin_unlock(&cpuxgpt_reg_lock);
}

static void set_cpuxgpt_clk(unsigned int div)
{
   __cpuxgpt_set_clk(div);
//   printk("%s: reg(%x) \n",__func__,__read_cpuxgpt(INDEX_CTL_REG)); 
}

static void  __cpuxgpt_enable(void)
{
   	unsigned int tmp = 0;
//	spin_lock(&cpuxgpt_reg_lock);
   	tmp = __read_cpuxgpt(INDEX_CTL_REG);
   	tmp  |= EN_CPUXGPT;
   	__write_cpuxgpt(INDEX_CTL_REG,tmp);
//	spin_unlock(&cpuxgpt_reg_lock);
}

static void enable_cpuxgpt(void)
{
   __cpuxgpt_enable();
//   printk("%s: reg(%x) \n",__func__,__read_cpuxgpt(INDEX_CTL_REG));
}

void setup_syscnt(void) 
{
   //set cpuxgpt free run,cpuxgpt always free run & oneshot no need to set
   //set cpuxgpt 13Mhz clock
   set_cpuxgpt_clk(CLK_DIV2);
   // enable cpuxgpt
   enable_cpuxgpt();
}
