/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "precomp.h"
#include "os/windows/src/i2c_comm.h"

extern "C"
{

#include "boot\dts\freescale\board.h"
#include "linux\completion.h"
#include "linux\delay.h"
#include "linux\dev_printk.h"
#include "linux\dma-mapping.h"
#include "linux\err.h"
#include "linux\interrupt.h"
#include "linux\gfp.h"
#include "linux\jiffies.h"
#include "linux\of.h"
#include "linux\platform_device.h"
#include "linux\slab.h"
#include "linux\spinlock.h"
#include "linux\types.h"
#include "linux\of_device.h"
#include "linux\atomic.h"
#include "linux\ktime.h"
#include "asm-generic\bug.h"
#include "linux\delay.h"
#include "drm\drm_fourcc.h"
#include "drm\drm_print.h"
#include "linux\iopoll.h"
#include "linux\clk-provider.h"
#include "linux\clk.h"
#include "linux\i2c.h"
#include "linux\regmap.h"

//
// memory allocation
//

void kfree(const void *ptr)
{
    ExFreePool((void*)ptr);
}

void *kmalloc(size_t size, gfp_t flags)
{
    void *ptr;

    ptr = ExAllocatePoolWithTag(NonPagedPoolNx, size, '8XMI');
    if (ptr && (flags & __GFP_ZERO))
    {
        memset(ptr, 0, size);
    }

    return ptr;
}

void *devm_kzalloc(struct device *dev, size_t size, gfp_t gfp)
{
    return kmalloc(size, gfp | __GFP_ZERO);
}

void devm_kfree(struct device *dev, const void *p)
{
    kfree(p);
}

//
// platform helpers
//

struct resource* platform_get_resource(struct platform_device* dev,
    unsigned int type, unsigned int num)
{
    if (type != IORESOURCE_MEM)
    {
        return nullptr;
    }

    if (num >= dev->num_resources)
    {
        return nullptr;
    }

    return &dev->resource[num];
}

void __iomem *devm_ioremap_resource(struct device *dev,
    const struct resource *res)
{
    if (!res)
    {
        return ERR_PTR(-EINVAL);
    }

    void* ptr = __ioremap(res->start, resource_size(res));
    if (!ptr)
    {
        return ERR_PTR(-ENOMEM);
    }

    return ptr;
}

void __iomem* devm_platform_ioremap_resource(struct platform_device *pdev,
    unsigned int index)
{
    resource* res = platform_get_resource(pdev, IORESOURCE_MEM, index);
    return devm_ioremap_resource(&pdev->dev, res);
}

void __iomem *devm_ioremap(struct device *dev, resource_size_t offset,
    resource_size_t size)
{
    void* ptr = __ioremap(offset, size);
    if (!ptr)
    {
        return ERR_PTR(-ENOMEM);
    }

    return ptr;
}

void devm_iounmap(struct device *dev, void __iomem *addr,
    resource_size_t size)
{
    iounmap(addr, size);
}

int platform_get_irq(struct platform_device *pdev, unsigned int num)
{
	return num;
}

int platform_get_irq_byname(struct platform_device* pdev, const char* name)
{
    for (int i = 0; i < NR_IRQS; i++)
    {
        if (!strcmp(irq_desc[i].name, name))
        {
            return i;
        }
    }

    return -1;
}

struct property* find_property_by_name(const struct device_node* np, const char *name)
{
    if (!np)
    {
        return nullptr;
    }

    property* p = np->properties;

    while (p && strlen(p->name))
    {
        if (!strcmp(p->name, name))
        {
            return p;
        }
        p++;
    }

    return nullptr;
}

const void *of_get_property(const struct device_node *np, const char *name,
    int *lenp)
{
    struct property *pp = find_property_by_name(np, name);

    if (pp) {
        if (lenp) {
            *lenp = pp->length;
        }
        return pp->value;
    }
    return nullptr;
}

int of_property_read_u32_array(const struct device_node *np,
    const char *propname,
    u32 *out_values, size_t sz)
{
    struct property* p = find_property_by_name(np, propname);
    if (!p)
    {
        return -EINVAL;
    }

    for (int i = 0; i < sz; i++)
    {
        out_values[i] = ((int*)p->value)[i];
    }

    return 0;
}

const struct of_device_id *of_match_device(const struct of_device_id *matches,
    const struct device *dev)
{
    const struct of_device_id *best_match = NULL;
    if ((!matches) || (!dev)) {
        return nullptr;
    }
    struct	property *properties = dev->of_node.properties;
    if (!properties) {
        return nullptr;
    }

    for (; matches->compatible[0]; matches++) {
        for (; properties->name[0]; properties++) {
            if (_stricmp(properties->name, "compatible") == 0) {
                if (_stricmp((char*)properties->value, matches->compatible) == 0) {
                    best_match = matches;
                    break;
                }
            }
        }
        properties = dev->of_node.properties;
    }

    return best_match;
}

int of_device_is_compatible(const struct device_node *device,
    const char *compat)
{
    if ((!device) || (!compat)) {
        return FALSE;
    }
    struct	property *properties = find_property_by_name(device, "compatible");
    if (properties == nullptr) {
        return FALSE;
    }
    if (_stricmp((char*)properties->value, compat) != 0) {
        return FALSE;
    }
    return TRUE;
}

struct clk *devm_clk_get(struct device *dev, const char *id)
{
    struct clk_init_data_desc *clk_desc;
    struct clk *clk = NULL;
    struct clk *clk_parent = NULL;
    if (!dev) {
        return NULL;
    }

    clk_desc = dev->of_clk;

    while (clk_desc && strlen(clk_desc->name)) {
        if (!strcmp(clk_desc->name, id)) {
            if (dev->get_clock_item) {
                clk = (dev->get_clock_item)(clk_desc->id);
            }
            if (!clk) {
                break;
            }
            if (clk_desc->parent_sel != 0) {
                clk_parent = (dev->get_clock_item)(clk_desc->parent_sel);
                if (clk_parent && clk->clk_set_parent) {
                    (void)(clk->clk_set_parent)(clk, clk_parent);
                }
            }
            if ((clk_desc->rate != 0) && clk->clk_set_rate) {
                (void)(clk->clk_set_rate)(clk, clk_desc->rate);
            }
            if ((clk_desc->parent_enable != 0) && clk->clk_enable) {
                /* Enable all parents recursively */
                (void)(clk->clk_enable)(clk->parent, true, true);
            }
            break;
        }
        clk_desc++;
    }
    return clk;
}

//
// interrupt handling
//

bool irq_handle(int irq)
{
    if (irq < 0 || irq > NR_IRQS)
    {
        DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL,
            "Invalid irq (%d)\n", irq);
        return false;
    }

    irq_data* data = &irq_desc[irq].irq_data;
    NT_ASSERT(data);
    if (data->chip.irq_check && data->chip.irq_check(data))
    {
        if (irq_desc[irq].handler)
        {
            irq_desc[irq].handler(data->irq, irq_desc[irq].dev_id);
            return true;
        }
    }

    return false;
}

//
// I/O memory mapping functions.
//

void __iomem *__ioremap(phys_addr_t phys_addr, size_t size)
{
    PHYSICAL_ADDRESS PhysAddr;
    PhysAddr.QuadPart = phys_addr;

    return MmMapIoSpaceEx(
        PhysAddr,
        size,
        PAGE_READWRITE | PAGE_NOCACHE);
}

void iounmap(void __iomem *addr, size_t size)
{
    MmUnmapIoSpace(addr, size);
}

//
// debug functions
//

void _dev_err(const struct device *dev, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    vDbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, fmt, argp);
    va_end(argp);
}

void _dev_warn(const struct device *dev, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    vDbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_WARNING_LEVEL, fmt, argp);
    va_end(argp);
}

void _dev_notice(const struct device *dev, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    vDbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_INFO_LEVEL, fmt, argp);
    va_end(argp);
}

void _dev_info(const struct device *dev, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    vDbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_INFO_LEVEL, fmt, argp);
    va_end(argp);
}

void _dev_dbg(const struct device *dev, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    vDbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, fmt, argp);
    va_end(argp);
}

void __warn(const char* file, const char* func, int line)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, "ERROR assert condition not true. file: %s function: %s line: %d", file, func, line);
}

//
// spinlock
//

void _raw_spin_lock_init(raw_spinlock_t* lock)
{
    KeInitializeSpinLock(lock);
}

void _raw_spin_lock_irqsave(raw_spinlock_t *lock, unsigned char* flags)
{
    (void)flags;
}

void _raw_spin_unlock_irqrestore(raw_spinlock_t *lock, unsigned char flags)
{
    (void)flags;
}

//
// delay
//

void udelay(unsigned int usecs)
{
    KeStallExecutionProcessor(usecs);
}

void mdelay(unsigned int msecs)
{
    KeStallExecutionProcessor(msecs * 1000);
}

void msleep(unsigned int msecs)
{
    LARGE_INTEGER Interval;
    Interval.QuadPart = -10 * 1000 * msecs;  /* number of msecs in 100 ns units */
    KeDelayExecutionThread(KernelMode, FALSE, &Interval);
}

void usleep_range(unsigned long min, unsigned long max)
{
    LARGE_INTEGER native_delay;
    native_delay.QuadPart = -10;  /* 1 us relative time */
    native_delay.QuadPart *= min;
    KeDelayExecutionThread(KernelMode, FALSE, &native_delay);
}

//
// dma mapping
//

void *dma_alloc_coherent(struct device *dev, size_t size,
    dma_addr_t *dma_handle, gfp_t gfp)
{
    PHYSICAL_ADDRESS LowestAcceptableAddress, BoundaryAddressMultiple,
        HighestAcceptableAddress;
    LowestAcceptableAddress.QuadPart = 0;
    BoundaryAddressMultiple.QuadPart = 0;
    HighestAcceptableAddress.QuadPart = 3 * (1ULL << 30) - 1ULL;

    PVOID Ptr = MmAllocateContiguousMemorySpecifyCache(
        size,
        LowestAcceptableAddress,
        HighestAcceptableAddress,
        BoundaryAddressMultiple,
        MmWriteCombined);

    PHYSICAL_ADDRESS PhysAddr = MmGetPhysicalAddress(Ptr);

    *dma_handle = static_cast<UINT32>(PhysAddr.QuadPart);

    return Ptr;
}

void dma_free_coherent(struct device *dev, size_t size,
    void *cpu_addr, dma_addr_t dma_handle)
{
    MmFreeContiguousMemory(cpu_addr);
}

//
// time, jiffies
//

unsigned long long get_jiffies(void)
{
    LARGE_INTEGER Ticks;
    KeQueryTickCount(&Ticks);

    return Ticks.QuadPart;
}

unsigned long long _msecs_to_jiffies(const unsigned int m)
{
    ULONG Increment = KeQueryTimeIncrement();
    return (m * 10000L) / Increment;
}

ktime_t ktime_get(void)
{
    LARGE_INTEGER Time;
    KeQuerySystemTimePrecise(&Time);

    return Time.QuadPart / 100;
}

//
// DRM
//

static char printable_char(int c)
{
    return isprint(c) ? (char)c : '?';
}

/**
 * drm_get_format_name - fill a string with a drm fourcc format's name
 * @format: format to compute name of
 * @buf: caller-supplied buffer
 */
const char *drm_get_format_name(uint32_t format, struct drm_format_name_buf *buf)
{
    RtlStringCbPrintfA(buf->str, sizeof(buf->str),
        "%c%c%c%c %s-endian (0x%08x)",
        printable_char(format & 0xff),
        printable_char((format >> 8) & 0xff),
        printable_char((format >> 16) & 0xff),
        printable_char((format >> 24) & 0x7f),
        format & DRM_FORMAT_BIG_ENDIAN ? "big" : "little",
        format);

    return buf->str;
}

void __drm_info(const char *format, ...)
{
    va_list argp;
    va_start(argp, format);

    vDbgPrintExWithPrefix("[drm] ", DPFLTR_IHVVIDEO_ID,
        DPFLTR_INFO_LEVEL, format, argp);

    va_end(argp);
}

void __drm_dbg(enum drm_debug_category category, const char *format, ...)
{
    va_list argp;
    va_start(argp, format);

    vDbgPrintExWithPrefix("[drm] ", DPFLTR_IHVVIDEO_ID,
        DPFLTR_TRACE_LEVEL, format, argp);

    va_end(argp);
}

void __drm_err(const char *format, ...)
{
    va_list argp;
    va_start(argp, format);

    vDbgPrintExWithPrefix("[drm] ", DPFLTR_IHVVIDEO_ID,
        DPFLTR_ERROR_LEVEL, format, argp);

    va_end(argp);
}

void __drm_warn(const char *format, ...)
{
    va_list argp;
    va_start(argp, format);

    vDbgPrintExWithPrefix("[drm] ", DPFLTR_IHVVIDEO_ID,
        DPFLTR_WARNING_LEVEL, format, argp);

    va_end(argp);
}

//
// atomic functions
//
int atomic_read(atomic_t *v)
{
    return InterlockedOr(v, 0);
}

void atomic_set(atomic_t *v, int i)
{
    (void)InterlockedExchange(v, i);
}

int atomic_dec_return(atomic_t *v)
{
    return (int)InterlockedDecrement(v);
}

int atomic_inc_return(atomic_t *v)
{
    return (int)InterlockedIncrement(v);
}

//
// completion
//

static void CompletionDpcRoutine(KDPC *pDPC, PVOID DeferredContext,
    PVOID SystemArgument1, PVOID SystemArgument2)
{
    completion *pDisableCompletion = (completion*)DeferredContext;

    UNREFERENCED_PARAMETER(pDPC);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    KeSetEvent(&pDisableCompletion->event, 0, FALSE);
}

void init_completion(completion *x)
{
    KeInitializeEvent(&x->event, SynchronizationEvent, FALSE);
    KeInitializeDpc(&x->dpc, CompletionDpcRoutine, x);
}

void reinit_completion(completion *x)
{
    KeResetEvent(&x->event);
}

void wait_for_completion(completion *x)
{
    KeWaitForSingleObject(&x->event, Executive, KernelMode, FALSE, NULL);
}

unsigned long wait_for_completion_timeout(completion *x,
    unsigned long long timeout)
{
    LARGE_INTEGER   Timeout;
    LONG Inc = KeQueryTimeIncrement();

    // convert timeout in jiffies to 100 ns units
    Timeout.QuadPart = -1 * timeout * Inc;

    NTSTATUS Status = KeWaitForSingleObject(&x->event, Executive,
        KernelMode, TRUE, &Timeout);

    return (Status == STATUS_SUCCESS) ? 1 : 0;
}

void complete(completion *x)
{
    KeInsertQueueDpc(&x->dpc, nullptr, nullptr);
}

//
// REGMAP-I2C
//

struct regmap *devm_regmap_init_i2c(struct i2c_client *i2c)
{
    struct regmap *map;
    map = (struct regmap *)kmalloc(sizeof(*map), GFP_KERNEL | __GFP_ZERO);
    if (map == NULL) {
        return (struct regmap *)ERR_PTR(-ENOMEM);
    }
    map->regmap_read = &i2c_read;
    map->regmap_write = &i2c_write;
    i2c_clear_handle(&map->io_target);
    i2c_initialize(i2c->connection_id, &map->io_target);
    return map;
}

void regmap_release_i2c(struct regmap *map)
{
    if (map) {
        i2c_close(&map->io_target);
        kfree(map);
    }
}

int regmap_write(struct regmap *map, unsigned int reg, unsigned int val)
{
    NTSTATUS status;

    if (!map || !map->regmap_write) {
        return -EINVAL;
    }
    status = (map->regmap_write)(&map->io_target, (UINT8)reg, &val, 1);
    return (NT_SUCCESS(status) ? 0 : -EIO);
}

int regmap_bulk_write(struct regmap *map, unsigned int reg, const void *val, size_t val_count)
{
    NTSTATUS status;

    if (!map || !map->regmap_write) {
        return -EINVAL;
    }
    status = (map->regmap_write)(&map->io_target, (UINT8)reg, (PVOID)val, (ULONG)val_count);
    return (NT_SUCCESS(status) ? 0 : -EIO);
}

int regmap_read(struct regmap *map, unsigned int reg, unsigned int *val)
{
    NTSTATUS status;

    if (!map || !map->regmap_read) {
        return -EINVAL;
    }

    status = (map->regmap_read)(&map->io_target, (UINT8)reg, (PVOID)val, 1);
    return (NT_SUCCESS(status) ? 0 : -EIO);
}

int regmap_bulk_read(struct regmap *map, unsigned int reg, void *val, size_t val_count)
{
    NTSTATUS status;

    if (!map || !map->regmap_read) {
        return -EINVAL;
    }

    status = (map->regmap_read)(&map->io_target, (UINT8)reg, (PVOID)val, (ULONG)val_count);
    return (NT_SUCCESS(status) ? 0 : -EIO);
}

int regmap_update_bits(struct regmap *map, unsigned int reg, unsigned int mask, unsigned int val)
{
    NTSTATUS status;
    unsigned int tmp, orig;

    if (!map || !map->regmap_read || !map->regmap_write) {
        return -EINVAL;
    }
    status = (map->regmap_read)(&map->io_target, (UINT8)reg, &orig, 1);
    if (!NT_SUCCESS(status)) {
        return -EIO;
    }
    tmp = orig & ~mask;
    tmp |= val & mask;
    status = (map->regmap_write)(&map->io_target, (UINT8)reg, &tmp, 1);

    return (NT_SUCCESS(status) ? 0 : -EIO);
}

int regmap_register_patch(struct regmap *map, const struct reg_sequence *regs, int num_regs)
{
    NTSTATUS status;
    int i;
    int ret = 0;

    if (!map || !map->regmap_write) {
        return -EINVAL;
    }
    for (i = 0; i < num_regs; i++) {
        status = (map->regmap_write)(&map->io_target, (UINT8)regs[i].reg, (PVOID)&regs[i].def, 1);
        ret |= (NT_SUCCESS(status) ? 0 : -EIO);
        if (regs[i].delay_us) {
            udelay(regs[i].delay_us);
        }
    }
    return ret;
}

/*
* Polls specific address until condition is met or timeout occurs
* param addr - address to poll (32bit value will be read into _read_val)
* param mask - mask to apply on the value read from address
* param cmp_val - value compared against masked read from address: (_read_val & mask) == cmp_val
* param non_equal - compare condition changed to (_read_val & mask) != cmp_val
* param delay_us - delay in us to wait after each poll
* param timeout_us - timeout in us after which the polling is terminated
* param sleep_before_read - delay (delay_us) applied also before first poll
*/
int _readl_poll_timeout(u32 *addr, u32 mask, u32 cmp_val,
    bool non_equal, unsigned long delay_us, u64 timeout_us, bool sleep_before_read)
{
    u32 _read_val;
    ktime_t __timeout = ktime_add_us(ktime_get(), timeout_us);
    if (sleep_before_read && delay_us) {
        usleep_range(delay_us, delay_us);
    }
    for (;;) {
        _read_val = readl(addr);
        if (non_equal) {
            if ((_read_val & mask) != cmp_val) {
                break;
            }
        }
        else
        {
            if ((_read_val & mask) == cmp_val) {
                break;
            }
        }
        if (timeout_us && ktime_compare(ktime_get(), __timeout) > 0) {
            _read_val = readl(addr);
            break;
        }
        if (delay_us) {
            usleep_range(delay_us, delay_us);
        }
    }
    if (non_equal) {
        if ((_read_val & mask) != cmp_val) {
            return 0;
        }
    }
    else
    {
        if ((_read_val & mask) == cmp_val) {
            return 0;
        }
    }
    return -ETIMEDOUT;
}

} // extern "C"
