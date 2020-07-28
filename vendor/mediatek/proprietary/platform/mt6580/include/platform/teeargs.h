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

#ifndef __PLATFORM_TEEARGS_H
#define __PLATFORM_TEEARGS_H

/* TEE magic */
#define TEE_ARGUMENT_MAGIC  (0x4B54444DU)

typedef struct {
    unsigned int magic;           // Magic number
    unsigned int version;         // version
    unsigned int NWEntry;         // NW Entry point after t-base
    unsigned int NWBootArgs;      // NW boot args (propagated by t-base in r4 before jump)
    unsigned int NWBootArgsSize;  // NW boot args size (propagated by t-base in r5 before jump)
    unsigned int dRamBase;        // NonSecure DRAM start address
    unsigned int dRamSize;        // NonSecure DRAM size
    unsigned int secDRamBase;     // Secure DRAM start address
    unsigned int secDRamSize;     // Secure DRAM size
    unsigned int sRamBase;        // NonSecure Scratch RAM start address
    unsigned int sRamSize;        // NonSecure Scratch RAM size
    unsigned int secSRamBase;     // Secure Scratch RAM start address
    unsigned int secSRamSize;     // Secure Scratch RAM size
    unsigned int log_port;        // uart base address for logging
    unsigned int log_baudrate;    // uart baud rate
    unsigned int hwuid[4];        // HW Unique id for t-base used
    unsigned int gicd_base;       //GICD register address base
    unsigned int gicc_base;       //GICC register address base
} tee_v7_arg_t;

#endif
