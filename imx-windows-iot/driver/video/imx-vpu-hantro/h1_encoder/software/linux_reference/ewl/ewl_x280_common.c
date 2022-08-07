/*------------------------------------------------------------------------------
--       Copyright (c) 2015-2017, VeriSilicon Inc. All rights reserved        --
--         Copyright (c) 2011-2014, Google Inc. All rights reserved.          --
--         Copyright (c) 2007-2010, Hantro OY. All rights reserved.           --
--                                                                            --
-- This software is confidential and proprietary and may be used only as      --
--   expressly authorized by VeriSilicon in a written licensing agreement.    --
--                                                                            --
--         This entire notice must be reproduced on all copies                --
--                       and may not be removed.                              --
--                                                                            --
--------------------------------------------------------------------------------
-- Redistribution and use in source and binary forms, with or without         --
-- modification, are permitted provided that the following conditions are met:--
--   * Redistributions of source code must retain the above copyright notice, --
--       this list of conditions and the following disclaimer.                --
--   * Redistributions in binary form must reproduce the above copyright      --
--       notice, this list of conditions and the following disclaimer in the  --
--       documentation and/or other materials provided with the distribution. --
--   * Neither the names of Google nor the names of its contributors may be   --
--       used to endorse or promote products derived from this software       --
--       without specific prior written permission.                           --
--------------------------------------------------------------------------------
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"--
-- AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE  --
-- IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE --
-- ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE  --
-- LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR        --
-- CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF       --
-- SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS   --
-- INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN    --
-- CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)    --
-- ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE --
-- POSSIBILITY OF SUCH DAMAGE.                                                --
--------------------------------------------------------------------------------
--                                                                            --
--  Abstract : Encoder Wrapper Layer for 6280/7280/8270/8290/H1, common parts
--
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "ewl.h"
#include "ewl_linux_lock.h"
#include "ewl_x280_common.h"

#ifdef USE_ION
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0) || defined(ENABLE_DMABUF_HEAP)
#include <linux/dma-heap.h>
#else
#include <linux/ion.h>
#endif
#include <linux/dma-buf.h>
#include <linux/version.h>
#ifdef ANDROID
#if !defined(ENABLE_DMABUF_HEAP)
#include <linux/mxc_ion.h>
#include <ion_4.12.h>
#endif
#endif
#else
#include "memalloc.h"
#endif

#ifdef ENABLE_DMABUF_HEAP
#include <linux/dma-buf-imx.h>
#endif

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#ifdef USE_EFENCE
#include "efence.h"
#endif

#ifndef MEMALLOC_MODULE_PATH
#define MEMALLOC_MODULE_PATH        "/tmp/dev/memalloc"
#endif

#ifndef ENC_MODULE_PATH
#define ENC_MODULE_PATH             "/tmp/dev/hx280"
#endif

#ifndef SDRAM_LM_BASE
#define SDRAM_LM_BASE               0x00000000
#endif

/* macro to convert CPU bus address to ASIC bus address */
#if defined (PCIE_FPGA_VERIFICATION)
#define BUS_CPU_TO_ASIC(address)    ((address) - enc_ewl->linMemBase)
#elif defined (PC_PCI_FPGA_DEMO)
#define BUS_CPU_TO_ASIC(address)    (((address) & (~0xff000000)) | SDRAM_LM_BASE)
#else
#define BUS_CPU_TO_ASIC(address)    ((address) | SDRAM_LM_BASE)
#endif

#ifdef PCIE_FPGA_VERIFICATION
#define BUS_ASIC_TO_CPU(address) ((address) + enc_ewl->linMemBase)
#else
#define BUS_ASIC_TO_CPU(address) (address)
#endif

#ifdef TRACE_EWL
static const char *busTypeName[7] = { "UNKNOWN", "AHB", "OCP", "AXI", "PCI", "AXIAHB", "AXIAPB" };
static const char *synthLangName[3] = { "UNKNOWN", "VHDL", "VERILOG" };
#endif

volatile u32 asic_status = 0;

FILE *fEwl = NULL;

/* Function to test input line buffer with hardware handshake, should be set by app.
    only for internal test purpose
    expected to return written mb lines */
u32 (*pollInputLineBufTestFunc)(void) = NULL;

#ifdef PCIE_FPGA_VERIFICATION
/* Initialization for PCIE FPGA Verification */
static i32 EWLPcieFpgaVerificationInit(hx280ewl_t *enc)
{
    if (!enc)
        return EWL_ERROR;

    if(ioctl(enc->fd_memalloc, MEMALLOC_IOCGMEMBASE,  &enc->linMemBase) == -1)
    {
       	PTRACE("ioctl MEMALLOC_IOCGMEMBASE failed\n");
       	return EWL_ERROR;
  	}

#ifdef PCIE_FPGA_VERI_LINEBUF
    if(ioctl(enc->fd_enc, HX280ENC_IOCGSRAMOFFSET,  &enc->sram_base) == -1)
    {
        PTRACE("ioctl HX280ENC_IOCGSRAMOFFSET failed\n");
        return EWL_ERROR;
    }
    if(ioctl(enc->fd_enc, HX280ENC_IOCGSRAMEIOSIZE,  &enc->sram_size) == -1)
    {
        PTRACE("ioctl HX280ENC_IOCGSRAMEIOSIZE failed\n");
        return EWL_ERROR;
    }   

    /* map srame address to user space */
    enc->psrame = (u32 *) mmap(0, enc->sram_size, PROT_READ | PROT_WRITE, MAP_SHARED, enc->fd_mem, enc->sram_base);
    if(enc->psrame == MAP_FAILED)
    {
        PTRACE("EWLInit: Failed to mmap SRAM Address!\n");
        return EWL_ERROR;
    }
    enc->sram_base = 0x3FE00000;
#endif
    return EWL_OK;
}

/* Release for PCIE FPGA Verification */
static void EWLPcieFpgaVerificationRelease(hx280ewl_t *enc)
{    
#ifdef PCIE_FPGA_VERI_LINEBUF
    /* Release the instance */
    if(enc->psrame != MAP_FAILED)
        munmap((void *) enc->psrame, enc->sram_size);
#endif
}
#endif

int MapAsicRegisters(hx280ewl_t * ewl)
{
    unsigned long base;
    unsigned int size;
    u32 *pRegs;

    ioctl(ewl->fd_enc, HX280ENC_IOCGHWOFFSET, &base);
    ioctl(ewl->fd_enc, HX280ENC_IOCGHWIOSIZE, &size);

#ifndef CFG_SECURE_IOCTRL_REGS
    /* map hw registers to user space */
    pRegs =
        (u32 *) mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED,
                     ewl->fd_enc, base);
    if(pRegs == MAP_FAILED)
    {
        PTRACE("EWLInit: Failed to mmap regs\n");
        return -1;
    }
#else
    pRegs = MAP_FAILED;
#endif

    ewl->regSize = size;
    ewl->regBase = base;
    ewl->pRegBase = pRegs;

    return 0;
}

/*******************************************************************************
 Function name   : EWLReadAsicID
 Description     : Read ASIC ID register, static implementation
 Return type     : u32 ID
 Argument        : void
*******************************************************************************/
u32 EWLReadAsicID()
{
    u32 id = ~0;
    //int fd_mem = -1;
    int fd_enc = -1;
    unsigned long base = ~0;
    unsigned int size;
    u32 *pRegs = MAP_FAILED;

    EWLHwConfig_t cfg_info;

    memset(&cfg_info, 0, sizeof(cfg_info));

#ifdef TRACE_EWL
    if(fEwl == NULL)
            fEwl = fopen("ewl.trc", "w");
#endif

    //fd_mem = open("/dev/mem", O_RDONLY);
    //if(fd_mem == -1)
    //{
    //    PTRACE("EWLReadAsicID: failed to open: %s\n", "/dev/mem");
    //    goto end;
    //}

    fd_enc = open(ENC_MODULE_PATH, O_RDONLY);
    if(fd_enc == -1)
    {
        PTRACE("EWLReadAsicID: failed to open: %s\n", ENC_MODULE_PATH);
        goto end;
    }

    ioctl(fd_enc, HX280ENC_IOCGHWOFFSET, &base);
    ioctl(fd_enc, HX280ENC_IOCGHWIOSIZE, &size);

    /* map hw registers to user space */
    pRegs = (u32 *) mmap(0, size, PROT_READ, MAP_SHARED, fd_enc, base);

    if(pRegs == MAP_FAILED)
    {
        PTRACE("EWLReadAsicID: Failed to mmap regs\n");
        goto end;
    }

    id = pRegs[0];

  end:
    if(pRegs != MAP_FAILED)
        munmap(pRegs, size);
    //if(fd_mem != -1)
    //    close(fd_mem);
    if(fd_enc != -1)
        close(fd_enc);

    PTRACE("EWLReadAsicID: 0x%08x at 0x%08lx\n", id, base);

    return id;
}

/*******************************************************************************
 Function name   : EWLReadAsicConfig
 Description     : Reads ASIC capability register, static implementation
 Return type     : EWLHwConfig_t 
 Argument        : void
*******************************************************************************/
EWLHwConfig_t EWLReadAsicConfig(void)
{
    //int fd_mem = -1;
    int fd_enc = -1;
    unsigned long base;
    unsigned int size;
    u32 *pRegs = MAP_FAILED, cfgval;

    EWLHwConfig_t cfg_info;

    memset(&cfg_info, 0, sizeof(cfg_info));

    //fd_mem = open("/dev/mem", O_RDONLY);
    //if(fd_mem == -1)
    //{
    //    PTRACE("EWLReadAsicConfig: failed to open: %s\n", "/dev/mem");
    //    goto end;
    //}

    fd_enc = open(ENC_MODULE_PATH, O_RDONLY);
    if(fd_enc == -1)
    {
        PTRACE("EWLReadAsicConfig: failed to open: %s\n", ENC_MODULE_PATH);
        goto end;
    }

    ioctl(fd_enc, HX280ENC_IOCGHWOFFSET, &base);
    ioctl(fd_enc, HX280ENC_IOCGHWIOSIZE, &size);

    /* map hw registers to user space */
    pRegs = (u32 *) mmap(0, size, PROT_READ, MAP_SHARED, fd_enc, base);

    if(pRegs == MAP_FAILED)
    {
        PTRACE("EWLReadAsicConfig: Failed to mmap regs\n");
        goto end;
    }

    cfgval = pRegs[63];

    cfg_info.maxEncodedWidth = cfgval & ((1 << 12) - 1);
    cfg_info.h264Enabled = (cfgval >> 27) & 1;
    cfg_info.vp8Enabled = (cfgval >> 26) & 1;
    cfg_info.jpegEnabled = (cfgval >> 25) & 1;
    cfg_info.vsEnabled = (cfgval >> 24) & 1;
    cfg_info.rgbEnabled = (cfgval >> 28) & 1;
    cfg_info.searchAreaSmall = (cfgval >> 29) & 1;
    cfg_info.scalingEnabled = (cfgval >> 30) & 1;

    cfg_info.busType = (cfgval >> 20) & 15;
    cfg_info.synthesisLanguage = (cfgval >> 16) & 15;
    cfg_info.busWidth = (cfgval >> 12) & 15;

    cfgval = pRegs[296];
    
    cfg_info.addr64Support = (cfgval >> 31) & 1;
    cfg_info.dnfSupport = (cfgval >> 30) & 1;
    cfg_info.rfcSupport = (cfgval >> 28) & 3;
    cfg_info.enhanceSupport = (cfgval >> 27) & 1;
    cfg_info.instantSupport = (cfgval >> 26) & 1;
    cfg_info.svctSupport = (cfgval >> 25) & 1;
    cfg_info.inAxiIdSupport = (cfgval >> 24) & 1;
    cfg_info.inLoopbackSupport = (cfgval >> 23) & 1;
    cfg_info.irqEnhanceSupport = (cfgval >> 22) & 1;

    PTRACE("EWLReadAsicConfig:\n"
           "    maxEncodedWidth   = %d\n"
           "    h264Enabled       = %s\n"
           "    jpegEnabled       = %s\n"
           "    vp8Enabled        = %s\n"
           "    vsEnabled         = %s\n"
           "    rgbEnabled        = %s\n"
           "    searchAreaSmall   = %s\n"
           "    scalingEnabled    = %s\n"
           "    address64bits     = %s\n"
           "    denoiseEnabled    = %s\n"
           "    rfcEnabled        = %s\n"
           "    instanctEnabled   = %s\n"
           "    busType           = %s\n"
           "    synthesisLanguage = %s\n"
           "    busWidth          = %d\n",
           cfg_info.maxEncodedWidth,
           cfg_info.h264Enabled == 1 ? "YES" : "NO",
           cfg_info.jpegEnabled == 1 ? "YES" : "NO",
           cfg_info.vp8Enabled == 1 ? "YES" : "NO",
           cfg_info.vsEnabled == 1 ? "YES" : "NO",
           cfg_info.rgbEnabled == 1 ? "YES" : "NO",
           cfg_info.searchAreaSmall == 1 ? "YES" : "NO",
           cfg_info.scalingEnabled == 1 ? "YES" : "NO",
           cfg_info.addr64Support == 1 ? "YES" : "NO",
           cfg_info.dnfSupport == 1 ? "YES" : "NO",
           cfg_info.rfcSupport == 0 ? "NO" : 
                  ( cfg_info.rfcSupport == 2 ? "LUMA"  :
                   (cfg_info.rfcSupport == 1 ? "CHROMA":"FULL")),
           cfg_info.instantSupport == 1 ? "YES" : "NO",
           cfg_info.busType < 7 ? busTypeName[cfg_info.busType] : "UNKNOWN",
           cfg_info.synthesisLanguage <
           3 ? synthLangName[cfg_info.synthesisLanguage] : "ERROR",
           cfg_info.busWidth * 32);

  end:
    if(pRegs != MAP_FAILED)
        munmap(pRegs, size);
    //if(fd_mem != -1)
    //    close(fd_mem);
    if(fd_enc != -1)
        close(fd_enc);
    return cfg_info;
}

/*******************************************************************************
 Function name   : EWLInit
 Description     : Allocate resources and setup the wrapper module
 Return type     : ewl_ret 
 Argument        : void
*******************************************************************************/
const void *EWLInit(EWLInitParam_t * param)
{
    hx280ewl_t *enc = NULL;


    PTRACE("EWLInit: Start\n");

    /* Check for NULL pointer */
    if(param == NULL || param->clientType > 4)
    {

        PTRACE(("EWLInit: Bad calling parameters!\n"));
        return NULL;
    }

    /* Allocate instance */
    if((enc = (hx280ewl_t *) EWLmalloc(sizeof(hx280ewl_t))) == NULL)
    {
        PTRACE("EWLInit: failed to alloc hx280ewl_t struct\n");
        return NULL;
    }
    memset(enc, 0, sizeof(hx280ewl_t));

    enc->clientType = param->clientType;
    enc->fd_mem = enc->fd_enc = enc->fd_memalloc = -1;

    /* New instance allocated */
    //enc->fd_mem = open("/dev/mem", O_RDWR | O_SYNC);
    //if(enc->fd_mem == -1)
    //{
    //    PTRACE("EWLInit: failed to open: %s\n", "/dev/mem");
    //    goto err;
    //}

    enc->fd_enc = open(ENC_MODULE_PATH, O_RDWR);
    if(enc->fd_enc == -1)
    {
        PTRACE("EWLInit: failed to open: %s\n", ENC_MODULE_PATH);
        goto err;
    }
 		
    enc->fd_memalloc = open(MEMALLOC_MODULE_PATH, O_RDWR);
    if(enc->fd_memalloc == -1)
    {
        PTRACE("EWLInit: failed to open: %s\n", MEMALLOC_MODULE_PATH);
        goto err;
    }

    /* map hw registers to user space */
    if(MapAsicRegisters(enc) != 0)
    {
        goto err;
    }

#ifdef PCIE_FPGA_VERIFICATION
    if (EWLPcieFpgaVerificationInit(enc) != EWL_OK)
    {
        PTRACE(("EWLInit: PCIE FPGA Verification Fail!\n"));
        goto err;
    }
#endif
  	
    PTRACE("EWLInit: mmap regs %d bytes --> %p\n", enc->regSize, enc->pRegBase);


    PTRACE("EWLInit: Return %0xd\n", (u32) enc);
    return enc;

  err:
    EWLRelease(enc);
    PTRACE("EWLInit: Return NULL\n");
    return NULL;
}

/*******************************************************************************
 Function name   : EWLRelease
 Description     : Release the wrapper module by freeing all the resources
 Return type     : ewl_ret 
 Argument        : void
*******************************************************************************/
i32 EWLRelease(const void *inst)
{
    hx280ewl_t *enc = (hx280ewl_t *) inst;

    assert(enc != NULL);

    if(enc == NULL)
        return EWL_OK;

    /* Release the instance */
    if(enc->pRegBase != MAP_FAILED)
        munmap((void *) enc->pRegBase, enc->regSize);

#ifdef PCIE_FPGA_VERIFICATION
	EWLPcieFpgaVerificationRelease(enc);
#endif

    if(enc->fd_mem != -1)
        close(enc->fd_mem);
    if(enc->fd_enc != -1)
        close(enc->fd_enc);
    if(enc->fd_memalloc != -1)
        close(enc->fd_memalloc);

    EWLfree(enc);

    PTRACE("EWLRelease: instance freed\n");
   
    if(fEwl != NULL)
        fclose(fEwl);

    return EWL_OK;
}

/*------------------------------------------------------------------------------
    Function name   : EWLIoctlWriteRegs
    Description     : Write values to HW register through ioctl.
    Return type     : int - return 0 on success, or retrun -1.
    Argument        : int fd - encoder device fd.
    Argument        : u32 offset - start write offset of HW register.
    Argument        : u32 size - write size.
    Argument        : u32 *val - write data pointer.
------------------------------------------------------------------------------*/
int EWLIoctlWriteRegs(int fd, u32 offset, u32 size, u32 *val)
{
    struct enc_regs_buffer regs;

    memset(&regs, 0, sizeof(regs));
    regs.offset = offset;
    regs.regs = val;
    regs.size = size;
    if (ioctl(fd, HX280ENC_IOC_WRITE_REGS, &regs) != 0)
    {
        PTRACE("ERROR write enc reg fail, offset:%08x\n", offset);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 Function name   : EWLWriteReg
 Description     : Set the content of a hadware register
 Return type     : void 
 Argument        : u32 offset
 Argument        : u32 val
*******************************************************************************/
void EWLWriteReg(const void *inst, u32 offset, u32 val)
{
    hx280ewl_t *enc = (hx280ewl_t *) inst;

    assert(enc != NULL && offset < enc->regSize);
#ifndef CFG_SECURE_IOCTRL_REGS
    if(offset == 0x04)
    {
        asic_status = val;
    }

    *(enc->pRegBase + (offset / 4)) = val;
#else
    EWLIoctlWriteRegs(enc->fd_enc, offset, 4, &val);
#endif

    PTRACE("EWLWriteReg 0x%02x with value %08x\n", offset, val);
}

/*------------------------------------------------------------------------------
    Function name   : EWLEnableHW
    Description     : 
    Return type     : i32
    Argument        : const void *inst
------------------------------------------------------------------------------*/
i32 EWLEnableHW(const void *inst)
{
    hx280ewl_t *ewl = (hx280ewl_t *) inst;

    return ioctl(ewl->fd_enc, HX280ENC_IOCG_EN_CORE);
}

/*------------------------------------------------------------------------------
    Function name   : EWLDisableHW
    Description     : 
    Return type     : void 
    Argument        : const void *inst
    Argument        : u32 offset
    Argument        : u32 val
------------------------------------------------------------------------------*/
void EWLDisableHW(const void *inst, u32 offset, u32 val)
{
    hx280ewl_t *enc = (hx280ewl_t *) inst;
    u32 stats;

    assert(enc != NULL && offset < enc->regSize);

    if(offset == 0x10)
    {
        EWLWriteReg(inst, offset, val);
        stats = val;
        asic_status = val;
    }
    else
    {
        stats = EWLReadReg(inst, offset);
        stats = stats & (~1);
        EWLWriteReg(inst, offset, stats);
    }

    PTRACE("EWLDisableHW 0x%02x with value %08x\n", offset, stats);
}

/*------------------------------------------------------------------------------
    Function name   : EWLIoctlReadRegs
    Description     : Read values to HW register through ioctl.
    Return type     : int - return 0 on success, or retrun -1.
    Argument        : int fd - encoder device fd.
    Argument        : u32 offset - start read offset of HW register.
    Argument        : u32 size - read size.
    Argument        : u32 *val - read data pointer.
------------------------------------------------------------------------------*/
int EWLIoctlReadRegs(int fd, u32 offset, u32 size, u32 *val)
{
    struct enc_regs_buffer regs;

    memset(&regs, 0, sizeof(regs));
    regs.offset = offset;
    regs.regs = val;
    regs.size = size;
    if (ioctl(fd, HX280ENC_IOC_READ_REGS, &regs) != 0)
    {
        PTRACE("ERROR read enc reg fail, offset:%08x\n", offset);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 Function name   : EWLReadReg
 Description     : Retrive the content of a hadware register
                    Note: The status register will be read after every MB
                    so it may be needed to buffer it's content if reading
                    the HW register is slow.
 Return type     : u32 
 Argument        : u32 offset
*******************************************************************************/
u32 EWLReadReg(const void *inst, u32 offset)
{
    u32 val;
    hx280ewl_t *enc = (hx280ewl_t *) inst;

    assert(offset < enc->regSize);
#ifndef CFG_SECURE_IOCTRL_REGS
    if(offset == 0x04)
    {
        return asic_status;
    }

    val = *(enc->pRegBase + (offset / 4));
#else
    EWLIoctlReadRegs(enc->fd_enc, offset, 4, &val);
#endif

    PTRACE("EWLReadReg 0x%02x --> %08x\n", offset, val);
    return val;
}

/*******************************************************************************
 Function name   : EWLGetShadowReg
 Description     : Retrive the content of a mirror registers. used when IRQ is 
                   receivedor Status change in polling mode.
 Return type     : u32 
 Argument        : u32 offset
*******************************************************************************/
u32 EWLGetShadowReg(const void *inst, u32 offset)
{
    u32 val;
    hx280ewl_t *enc = (hx280ewl_t *) inst;

    assert(offset < enc->regSize);

    if(offset == 0x04)
    {
        return asic_status;
    }

    offset = offset / 4;
    val = enc->regMirror[offset];

    PTRACE("EWLReadReg 0x%02x --> %08x\n", offset * 4, val);

    return val;
}

/*******************************************************************************
 Function name   : EWLSetShadowReg
 Description     : Set the content of a mirror register
 Return type     : u32 
 Argument        : u32 offset
*******************************************************************************/
void EWLSetShadowReg(const void *inst, u32 offset, u32 val)
{
    hx280ewl_t *enc = (hx280ewl_t *) inst;

    assert(enc != NULL && offset < enc->regSize);

    if(offset == 0x04)
    {
        asic_status = val;
    }

    offset = offset / 4;
    enc->regMirror[offset] = val;

    PTRACE("EWLWriteReg 0x%02x with value %08x\n", offset * 4, val);
}

/*------------------------------------------------------------------------------
    Function name   : EWLMallocRefFrm
    Description     : Allocate a frame buffer (contiguous linear RAM memory)
    
    Return type     : i32 - 0 for success or a negative error code 
    
    Argument        : const void * instance - EWL instance
    Argument        : u32 size - size in bytes of the requested memory
    Argument        : EWLLinearMem_t *info - place where the allocated memory
                        buffer parameters are returned
------------------------------------------------------------------------------*/
i32 EWLMallocRefFrm(const void *instance, u32 size, EWLLinearMem_t * info)
{
    hx280ewl_t *enc_ewl = (hx280ewl_t *) instance;
    EWLLinearMem_t *buff = (EWLLinearMem_t *) info;
    i32 ret;

    assert(enc_ewl != NULL);
    assert(buff != NULL);

    PTRACE("EWLMallocRefFrm\t%8d bytes\n", size);

    ret = EWLMallocLinear(enc_ewl, size, buff);

    PTRACE("EWLMallocRefFrm %08x --> %p\n", buff->busAddress,
           buff->virtualAddress);

    return ret;
}

/*------------------------------------------------------------------------------
    Function name   : EWLFreeRefFrm
    Description     : Release a frame buffer previously allocated with 
                        EWLMallocRefFrm.
    
    Return type     : void 
    
    Argument        : const void * instance - EWL instance
    Argument        : EWLLinearMem_t *info - frame buffer memory information
------------------------------------------------------------------------------*/
void EWLFreeRefFrm(const void *instance, EWLLinearMem_t * info)
{
    hx280ewl_t *enc_ewl = (hx280ewl_t *) instance;
    EWLLinearMem_t *buff = (EWLLinearMem_t *) info;

    assert(enc_ewl != NULL);
    assert(buff != NULL);

    EWLFreeLinear(enc_ewl, buff);

    PTRACE("EWLFreeRefFrm\t%p\n", buff->virtualAddress);
}

/*------------------------------------------------------------------------------
    Function name   : EWLMallocLinear
    Description     : Allocate a contiguous, linear RAM  memory buffer
    
    Return type     : i32 - 0 for success or a negative error code  
    
    Argument        : const void * instance - EWL instance
    Argument        : u32 size - size in bytes of the requested memory
    Argument        : EWLLinearMem_t *info - place where the allocated memory
                        buffer parameters are returned
------------------------------------------------------------------------------*/
#ifdef USE_ION
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)

i32 EWLMallocLinear(const void *instance, u32 size, EWLLinearMem_t * info)
{
    hx280ewl_t *enc_ewl = (hx280ewl_t *) instance;
    EWLLinearMem_t *buff = (EWLLinearMem_t *) info;
#ifdef USE_ION
    struct ion_allocation_data allocation_data = { 0 };
    struct ion_custom_data custom_data = { 0 };
    struct ion_fd_data fd_data = { 0 };
    struct ion_handle_data handle_data = { 0 };
    struct ion_phys_dma_data dma_data = { 0 };
    int ret;
#else
    MemallocParams params;
#endif
    u32 pgsize = getpagesize();

    assert(enc_ewl != NULL);
    assert(buff != NULL);

    PTRACE("EWLMallocLinear\t%8d bytes\n", size);

    buff->size = (size + (pgsize - 1)) & (~(pgsize - 1));

#ifdef USE_ION
      buff->ion_fd = -1;
      allocation_data.len = buff->size;
      allocation_data.heap_id_mask = 1;
      if (ioctl (enc_ewl->fd_memalloc, ION_IOC_ALLOC, &allocation_data) < 0) {
        PTRACE("ERROR! No linear buffer available\n");
        return EWL_ERROR;
      }
      handle_data.handle = fd_data.handle = allocation_data.handle;
      ret = ioctl (enc_ewl->fd_memalloc, ION_IOC_MAP, &fd_data);
      if (ret < 0 || fd_data.fd < 0) {
        PTRACE("ERROR! map ioctl failed or returned negative fd\n");
        goto bail;
      }
      buff->ion_fd = dma_data.dmafd = fd_data.fd;
      custom_data.cmd = ION_IOC_PHYS_DMA;
      custom_data.arg = (unsigned long)&dma_data;
    
      ret = ioctl (enc_ewl->fd_memalloc, ION_IOC_CUSTOM, &custom_data);
      if (ret < 0 || dma_data.phys == NULL) {
        PTRACE("ERROR! gst_ion_phys_dma failed\n");
        goto bail;
      }
      buff->busAddress = dma_data.phys;
      PTRACE("physical address: %p\n", buff->busAddress);
    
      buff->virtualAddress = (u32 *)mmap(0, buff->size, PROT_READ | PROT_WRITE, MAP_SHARED,
                    buff->ion_fd, 0);
      if (buff->virtualAddress == MAP_FAILED) {
        PTRACE("ERROR! mmap failed\n");
        goto bail;
      }
    
      ioctl(enc_ewl->fd_memalloc, ION_IOC_FREE, &handle_data);
#else
    params.size = size;

    buff->virtualAddress = 0;
    /* get memory linear memory buffers */
    ioctl(enc_ewl->fd_memalloc, MEMALLOC_IOCXGETBUFFER, &params);
    if(params.busAddress == 0)
    {
        PTRACE("EWLMallocLinear: Linear buffer not allocated\n");
        return EWL_ERROR;
    }

    /* Map the bus address to virtual address */
    buff->virtualAddress = MAP_FAILED;
    buff->virtualAddress = (u32 *) mmap(0, buff->size, PROT_READ | PROT_WRITE,
                                        MAP_SHARED, enc_ewl->fd_enc,
                                        params.busAddress);

    if(buff->virtualAddress == MAP_FAILED)
    {
        PTRACE("EWLInit: Failed to mmap busAddress: 0x%08x\n",
               params.busAddress);
        return EWL_ERROR;
    }
    /* ASIC might be in different address space */
    buff->busAddress = BUS_CPU_TO_ASIC(params.busAddress);
    PTRACE("EWLMallocLinear 0x%08x (CPU) 0x%08x (ASIC) --> %p\n",
           params.busAddress, buff->busAddress, buff->virtualAddress);

#endif //USE_ION

    return EWL_OK;

#ifdef USE_ION
    bail:
      if (buff->ion_fd >= 0)
        close(buff->ion_fd);
      ioctl(enc_ewl->fd_memalloc, ION_IOC_FREE, &handle_data);
    
      return EWL_ERROR;
#endif
}

#elif (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0)) && !defined(ENABLE_DMABUF_HEAP) //LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
i32 EWLMallocLinear(const void *instance, u32 size, EWLLinearMem_t * info)
{
    hx280ewl_t *enc_ewl = (hx280ewl_t *) instance;
    EWLLinearMem_t *buff = (EWLLinearMem_t *) info;

    int i;
    int heap_cnt;
    int heap_mask = 0;
    struct ion_heap_query query;
#ifdef ANDROID
    /* new struct of 4.14 ion.h defined in libion */
    struct ion_new_allocation_data allocation_data = {0};
#else
    struct ion_allocation_data allocation_data = {0};
#endif
    struct dma_buf_phys dma_phys;
    int ret;

    u32 pgsize = getpagesize();

    assert(enc_ewl != NULL);
    assert(buff != NULL);

    PTRACE("EWLMallocLinear\t%8d bytes\n", size);

    buff->size = (size + (pgsize - 1)) & (~(pgsize - 1));

    buff->ion_fd = -1;
    memset(&query, 0, sizeof(query));
    ret = ioctl (enc_ewl->fd_memalloc, ION_IOC_HEAP_QUERY, &query);
    if (ret != 0 || query.cnt == 0) {
      PTRACE("can't query heap count \n");
      return EWL_ERROR;
    }
    heap_cnt = query.cnt;

    struct ion_heap_data ihd[heap_cnt];
    memset(&ihd, 0, sizeof(ihd));
    query.cnt = heap_cnt;
    query.heaps = (unsigned long)&ihd;
    ret = ioctl (enc_ewl->fd_memalloc, ION_IOC_HEAP_QUERY, &query);
    if (ret != 0) {
      PTRACE("can't get ion heaps \n");
      return EWL_ERROR;
    }

    for (i=0; i<heap_cnt; i++) {
      if (ihd[i].type == ION_HEAP_TYPE_DMA) {
        heap_mask |=  1 << ihd[i].heap_id;
      }
    }

    allocation_data.heap_id_mask = heap_mask;
    allocation_data.len = buff->size;
#ifdef ANDROID
    ret = ioctl (enc_ewl->fd_memalloc, ION_IOC_NEW_ALLOC, &allocation_data);
#else
    ret = ioctl (enc_ewl->fd_memalloc, ION_IOC_ALLOC, &allocation_data);
#endif
    if (ret < 0) {
      PTRACE("ion allocate failed. \n");
      return EWL_ERROR;
    }
    info->ion_fd = allocation_data.fd;

    ret = ioctl(info->ion_fd, DMA_BUF_IOCTL_PHYS, &dma_phys);
    if (ret < 0) {
      PTRACE("ion get phys failed. \n");
      goto bail;
    }

    buff->busAddress = dma_phys.phys;
    PTRACE("physical address: %p\n", buff->busAddress);
    
    buff->virtualAddress = (u32 *)mmap(0, buff->size, PROT_READ | PROT_WRITE, MAP_SHARED,
                  buff->ion_fd, 0);
    if (buff->virtualAddress == MAP_FAILED) {
      PTRACE("ERROR! mmap failed\n");
      goto bail;
    }

    return EWL_OK;

    bail:
      if (buff->ion_fd >= 0)
        close(buff->ion_fd);
    
      return EWL_ERROR;
}

#else (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)) || defined(ENABLE_DMABUF_HEAP)  // (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
i32 EWLMallocLinear(const void *instance, u32 size, EWLLinearMem_t * info)
{
    hx280ewl_t *enc_ewl = (hx280ewl_t *) instance;
    EWLLinearMem_t *buff = (EWLLinearMem_t *) info;

    struct dma_heap_allocation_data data = { 0 };
    struct dma_buf_phys dma_phys;
    int ret;

    u32 pgsize = getpagesize();

    assert(enc_ewl != NULL);
    assert(buff != NULL);

    PTRACE("EWLMallocLinear\t%8d bytes\n", size);

    buff->size = (size + (pgsize - 1)) & (~(pgsize - 1));

    buff->ion_fd = -1;
    data.len = buff->size;
    data.fd_flags = O_RDWR | O_CLOEXEC;
    data.heap_flags = 0;
    ret = ioctl (enc_ewl->fd_memalloc, DMA_HEAP_IOCTL_ALLOC, &data);
    if (ret < 0) {
      PTRACE("ion allocate failed. \n");
      goto bail;
    }
    info->ion_fd = data.fd;

    ret = ioctl(info->ion_fd, DMA_BUF_IOCTL_PHYS, &dma_phys);
    if (ret < 0) {
#ifdef ENABLE_DMABUF_HEAP
        PTRACE("ion DMA_BUF_IOCTL_PHYS get phys failed. \n");
        struct dmabuf_imx_phys_data data;
        int fd_;
        fd_ = open("/dev/dmabuf_imx", O_RDONLY | O_CLOEXEC);
        if (fd_ < 0) {
            PTRACE("open /dev/dambuf_imx failed: %s", strerror(errno));
            goto bail;
        }
        data.dmafd = info->ion_fd;
        if (ioctl(fd_, DMABUF_GET_PHYS, &data) < 0) {
            PTRACE("%s ION_GET_PHYS  failed",__func__);
            close(fd_);
            goto bail;
        } else {
            buff->busAddress = data.phys;
        }
        close(fd_);
    } else {
        buff->busAddress = dma_phys.phys;
    }
#else
        PTRACE("ion get phys failed. \n");
        goto bail;
    }

    buff->busAddress = dma_phys.phys;
#endif

    PTRACE("physical address: %p\n", buff->busAddress);

    buff->virtualAddress = (u32 *)mmap(0, buff->size, PROT_READ | PROT_WRITE, MAP_SHARED,
                  buff->ion_fd, 0);
    if (buff->virtualAddress == MAP_FAILED) {
      PTRACE("ERROR! mmap failed\n");
      goto bail;
    }

    return EWL_OK;

bail:
    if (buff->ion_fd >= 0)
      close(buff->ion_fd);

    return EWL_ERROR;
}
#endif // LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)

#else //USE_ION

//non-ION not supported

#endif  //USE_ION

/*------------------------------------------------------------------------------
    Function name   : EWLFreeLinear
    Description     : Release a linera memory buffer, previously allocated with 
                        EWLMallocLinear.
    
    Return type     : void 
    
    Argument        : const void * instance - EWL instance
    Argument        : EWLLinearMem_t *info - linear buffer memory information
------------------------------------------------------------------------------*/
void EWLFreeLinear(const void *instance, EWLLinearMem_t * info)
{
    hx280ewl_t *enc_ewl = (hx280ewl_t *) instance;
    EWLLinearMem_t *buff = (EWLLinearMem_t *) info;

    assert(enc_ewl != NULL);
    assert(buff != NULL);

#ifdef USE_ION
    if (buff->ion_fd >= 0)
        close(buff->ion_fd);
#else
    if(buff->busAddress != 0)
    {
        buff->busAddress = BUS_ASIC_TO_CPU(buff->busAddress);
        ioctl(enc_ewl->fd_memalloc, MEMALLOC_IOCSFREEBUFFER, &buff->busAddress);
	}
#endif

    if(buff->virtualAddress != MAP_FAILED)
        munmap(buff->virtualAddress, buff->size);

    PTRACE("EWLFreeLinear\t%p\n", buff->virtualAddress);
}

/*******************************************************************************
 Function name   : EWLReserveHw
 Description     : Reserve HW resource for currently running codec
*******************************************************************************/
i32 EWLReserveHw(const void *inst)
{
    hx280ewl_t *ewl = (hx280ewl_t *) inst;
    i32 ret;
    u32 temp = 0;
    
    /* Check invalid parameters */
    if(ewl == NULL)
      return EWL_ERROR;
    
    PTRACE("EWLReserveHw: PID %d trying to reserve ...\n", getpid());
    
    ret = ioctl(ewl->fd_enc, HX280ENC_IOCH_ENC_RESERVE, &temp);
    
    if (ret < 0)
    {
     PTRACE("EWLReserveHw failed\n");
     return EWL_ERROR;
    }
    else
    {
     PTRACE("EWLReserveHw successed\n");
    }
    
    EWLWriteReg(ewl, 0x38, 0);//disable encoder
    
    PTRACE("EWLReserveHw: ENC HW locked by PID %d\n", getpid());

    return EWL_OK;
}

/*******************************************************************************
 Function name   : EWLReleaseHw
 Description     : Release HW resource when frame is ready
*******************************************************************************/
void EWLReleaseHw(const void *inst)
{
    hx280ewl_t *enc = (hx280ewl_t *) inst;
    u32 val, temp;

    assert(enc != NULL);

    val = EWLReadReg(inst, 0x38);
    EWLWriteReg(inst, 0x38, val & (~0x01)); /* reset ASIC */

    PTRACE("EWLReleaseHw: PID %d trying to release ...\n", getpid());

    ioctl(enc->fd_enc, HX280ENC_IOCH_ENC_RELEASE, &temp);

    PTRACE("EWLReleaseHw: HW released by PID %d\n", getpid());
    return ;
}

/* SW/SW shared memory */
/*------------------------------------------------------------------------------
    Function name   : EWLmalloc
    Description     : Allocate a memory block. Same functionality as
                      the ANSI C malloc()
    
    Return type     : void pointer to the allocated space, or NULL if there
                      is insufficient memory available
    
    Argument        : u32 n - Bytes to allocate
------------------------------------------------------------------------------*/
void *EWLmalloc(u32 n)
{

    void *p = malloc((size_t) n);

    PTRACE("EWLmalloc\t%8d bytes --> %p\n", n, p);

    return p;
}

/*------------------------------------------------------------------------------
    Function name   : EWLfree
    Description     : Deallocates or frees a memory block. Same functionality as
                      the ANSI C free()
    
    Return type     : void 
    
    Argument        : void *p - Previously allocated memory block to be freed
------------------------------------------------------------------------------*/
void EWLfree(void *p)
{
    PTRACE("EWLfree\t%p\n", p);
    if(p != NULL)
        free(p);
}

/*------------------------------------------------------------------------------
    Function name   : EWLcalloc
    Description     : Allocates an array in memory with elements initialized
                      to 0. Same functionality as the ANSI C calloc()
    
    Return type     : void pointer to the allocated space, or NULL if there
                      is insufficient memory available
    
    Argument        : u32 n - Number of elements
    Argument        : u32 s - Length in bytes of each element.
------------------------------------------------------------------------------*/
void *EWLcalloc(u32 n, u32 s)
{
    void *p = calloc((size_t) n, (size_t) s);

    PTRACE("EWLcalloc\t%8d bytes --> %p\n", n * s, p);

    return p;
}

/*------------------------------------------------------------------------------
    Function name   : EWLmemcpy
    Description     : Copies characters between buffers. Same functionality as
                      the ANSI C memcpy()
    
    Return type     : The value of destination d
    
    Argument        : void *d - Destination buffer
    Argument        : const void *s - Buffer to copy from
    Argument        : u32 n - Number of bytes to copy
------------------------------------------------------------------------------*/
void *EWLmemcpy(void *d, const void *s, u32 n)
{
    return memcpy(d, s, (size_t) n);
}

/*------------------------------------------------------------------------------
    Function name   : EWLmemset
    Description     : Sets buffers to a specified character. Same functionality
                      as the ANSI C memset()
    
    Return type     : The value of destination d
    
    Argument        : void *d - Pointer to destination
    Argument        : i32 c - Character to set
    Argument        : u32 n - Number of characters
------------------------------------------------------------------------------*/
void *EWLmemset(void *d, i32 c, u32 n)
{
    return memset(d, (int) c, (size_t) n);
}

/*------------------------------------------------------------------------------
    Function name   : EWLmemcmp
    Description     : Compares two buffers. Same functionality
                      as the ANSI C memcmp()

    Return type     : Zero if the first n bytes of s1 match s2

    Argument        : const void *s1 - Buffer to compare
    Argument        : const void *s2 - Buffer to compare
    Argument        : u32 n - Number of characters
------------------------------------------------------------------------------*/
int EWLmemcmp(const void *s1, const void *s2, u32 n)
{
    return memcmp(s1, s2, (size_t) n);
}


/*------------------------------------------------------------------------------
    Function name   : EWLGetInputLineBufferBase
    Description        : Get the base address of on-chip sram used for input MB line buffer.
    
    Return type     : i32 - 0 for success or a negative error code  
    
    Argument        : const void * instance - EWL instance
    Argument        : EWLLinearMem_t *info - place where the sram parameters are returned
------------------------------------------------------------------------------*/
i32 EWLGetInputLineBufferBase(const void *instance, EWLLinearMem_t * info)
{
    hx280ewl_t *enc_ewl = (hx280ewl_t *) instance;

    assert(enc_ewl != NULL);
    assert(info != NULL);

    info->virtualAddress = NULL;
    info->busAddress = 0;
    info->size = 0;

#ifdef PCIE_FPGA_VERI_LINEBUF
    /* sram params have been got in EWLInit */
    info->virtualAddress = (u32 *)enc_ewl->psrame;
    info->busAddress = enc_ewl->sram_base;
    info->size = enc_ewl->sram_size;

    if(info->virtualAddress == MAP_FAILED)
    {
        info->virtualAddress = NULL;
        PTRACE("EWLInit: Failed to get sram virtual address: 0x%08x\n", info->virtualAddress);
        return EWL_ERROR;
    }
#endif
    PTRACE("EWLMallocLinear 0x%08x (ASIC) --> %p\n", info->busAddress, info->virtualAddress);

    return EWL_OK;
}

