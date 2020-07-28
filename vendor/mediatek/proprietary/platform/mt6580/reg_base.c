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

#include <platform/reg_base.h>
#include <sys/types.h>

paddr_t trng_base(void) { return TRNG_BASE; }
paddr_t trng_pdn_base(void) { return TRNG_PDN_BASE; }
uint32_t trng_pdn_offset(void) { return TRNG_PDN_OFFSET; }
uint32_t trng_pdn_set_offset(void) { return TRNG_PDN_SET_OFFSET; }
uint32_t trng_pdn_clr_offset(void) { return TRNG_PDN_CLEAR_OFFSET; }
uint32_t trng_pdn_sts_offset(void) { return TRNG_PDN_STATUS_OFFSET; }
paddr_t plat_hacc_phy_base(void) { return HACC_BASE; }

