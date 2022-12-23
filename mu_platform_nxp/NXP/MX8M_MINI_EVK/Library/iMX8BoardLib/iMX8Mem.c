/** @file
*
*  Copyright (c) 2018, Microsoft Corporation. All rights reserved.
*  Copyright 2020, 2022 NXP
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>

#include "iMX8.h"

#define MEMORY_ATTRIBUTES_PCDCACHEENABLE    -1

#define ARM_PERIPHERALS_REGISTERS_PHYSICAL  0x31000000
#define ARM_PERIPHERALS_REGISTERS_LENGTH    0x00400000

/* AIPS1 - AIPS3 */
#define ARM_IP_BUS_REGISTERS_PHYSICAL       0x30000000
#define ARM_IP_BUS_REGISTERS_LENGTH         0x00C00000

/* PCIe registers */
#define PCIE_REG_REGISTER_PHYSICAL          0x33800000
#define PCIE_REG_REGISTER_LENGTH            0x00400000

/* PCIe mapped area */
#define PCIE_MAPPED_AREA_PHYSICAL           0x18000000
#define PCIE_MAPPED_AREA_LENGTH             0x08000000

/* USB1,2 and PCIE_PHY (part of AIPS4) */
#define ARM_USB_REG_REGISTER_PHYSICAL       0x32E40000
#define ARM_USB_REG_REGISTER_LENGTH         0x000D0000

#define ARM_GIC_REG_REGISTER_PHYSICAL       0x38800000
#define ARM_GIC_REG_REGISTER_LENGTH         0x00100000

#define ARM_LCDIF_REG_REGISTER_PHYSICAL     0x32E00000
#define ARM_LCDIF_REG_REGISTER_LENGTH       0x00100000

#define ARM_MIPI_DSI_REG_REGISTER_PHYSICAL  0x32E10000
#define ARM_MIPI_DSI_REG_REGISTER_LENGTH    0x00100000

ARM_MEMORY_REGION_DESCRIPTOR iMX8MemoryDescriptor[] =
{
#ifndef CONFIG_HEADLESS
  // Main memory
  {
    FixedPcdGet64 (PcdSystemMemoryBase) + FixedPcdGet64 (PcdArmLcdDdrFrameBufferSize),
    FixedPcdGet64 (PcdSystemMemoryBase) + FixedPcdGet64 (PcdArmLcdDdrFrameBufferSize),
    FixedPcdGet64 (PcdSystemMemorySize) - FixedPcdGet64 (PcdArmLcdDdrFrameBufferSize),
    MEMORY_ATTRIBUTES_PCDCACHEENABLE,
  },
  // Frame buffer
  {
    FixedPcdGet64 (PcdArmLcdDdrFrameBufferBase),
    FixedPcdGet64 (PcdArmLcdDdrFrameBufferBase),
    FixedPcdGet64 (PcdArmLcdDdrFrameBufferSize),
    ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED,
  },
#else
  // Main memory
  {
    FixedPcdGet64 (PcdSystemMemoryBase),
    FixedPcdGet64 (PcdSystemMemoryBase),
    FixedPcdGet64 (PcdSystemMemorySize),
    MEMORY_ATTRIBUTES_PCDCACHEENABLE,
  },
#endif
#if FixedPcdGet32(PcdBank1MemorySize) > 0
  // Main memory above 32 bit address space
  {
    FixedPcdGet64 (PcdBank1MemoryBase),
    FixedPcdGet64 (PcdBank1MemoryBase),
    FixedPcdGet64 (PcdBank1MemorySize),
    MEMORY_ATTRIBUTES_PCDCACHEENABLE,
  },
#endif
#ifdef CONFIG_OPTEE
  {
    FixedPcdGet64 (PcdTrustZoneSharedMemoryBase),
    FixedPcdGet64 (PcdTrustZoneSharedMemoryBase),
    FixedPcdGet64 (PcdTrustZoneSharedMemorySize),
    MEMORY_ATTRIBUTES_PCDCACHEENABLE,
  },
#endif
  {
    ARM_PERIPHERALS_REGISTERS_PHYSICAL,
    ARM_PERIPHERALS_REGISTERS_PHYSICAL,
    ARM_PERIPHERALS_REGISTERS_LENGTH,
    ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
  },
  {
    ARM_IP_BUS_REGISTERS_PHYSICAL,
    ARM_IP_BUS_REGISTERS_PHYSICAL,
    ARM_IP_BUS_REGISTERS_LENGTH,
    ARM_MEMORY_REGION_ATTRIBUTE_DEVICE
  },
  {
    PCIE_REG_REGISTER_PHYSICAL,
    PCIE_REG_REGISTER_PHYSICAL,
    PCIE_REG_REGISTER_LENGTH,
    ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
  },
  {
    PCIE_MAPPED_AREA_PHYSICAL,
    PCIE_MAPPED_AREA_PHYSICAL,
    PCIE_MAPPED_AREA_LENGTH,
    ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
  },
  {
    ARM_GIC_REG_REGISTER_PHYSICAL,
    ARM_GIC_REG_REGISTER_PHYSICAL,
    ARM_GIC_REG_REGISTER_LENGTH,
    ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
  },
  {
    ARM_USB_REG_REGISTER_PHYSICAL,
    ARM_USB_REG_REGISTER_PHYSICAL,
    ARM_USB_REG_REGISTER_LENGTH,
    ARM_MEMORY_REGION_ATTRIBUTE_DEVICE
  },
  {
    ARM_LCDIF_REG_REGISTER_PHYSICAL,
    ARM_LCDIF_REG_REGISTER_PHYSICAL,
    ARM_LCDIF_REG_REGISTER_LENGTH,
    ARM_MEMORY_REGION_ATTRIBUTE_DEVICE
  },
  {
    ARM_MIPI_DSI_REG_REGISTER_PHYSICAL,
    ARM_MIPI_DSI_REG_REGISTER_PHYSICAL,
    ARM_MIPI_DSI_REG_REGISTER_LENGTH,
    ARM_MEMORY_REGION_ATTRIBUTE_DEVICE
  },
  {
    //
    // End of table
    //
    0,
    0,
    0,
    0,
  },
};

#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS (sizeof(iMX8MemoryDescriptor) / sizeof(iMX8MemoryDescriptor[0]))

// DDR attributes
#define DDR_ATTRIBUTES_CACHED           ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK
#define DDR_ATTRIBUTES_UNCACHED         ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED

/**
  Return the Virtual Memory Map of your platform

  This Virtual Memory Map is used by MemoryInitPei Module to initialize the MMU on your platform.

  @param[out]   VirtualMemoryMap    Array of ARM_MEMORY_REGION_DESCRIPTOR describing a Physical-to-
                                    Virtual Memory mapping. This array must be ended by a zero-filled
                                    entry

**/
VOID
ArmPlatformGetVirtualMemoryMap (
  IN ARM_MEMORY_REGION_DESCRIPTOR **VirtualMemoryMap
  )
{
  ARM_MEMORY_REGION_ATTRIBUTES cacheAttributes;
  UINTN index;
  ARM_MEMORY_REGION_DESCRIPTOR *virtualMemoryTable;
  EFI_RESOURCE_ATTRIBUTE_TYPE  ResourceAttributes;

  ASSERT (VirtualMemoryMap != NULL);

  DEBUG ((EFI_D_VERBOSE, "Enter: ArmPlatformGetVirtualMemoryMap\n"));

  ResourceAttributes = (
                       EFI_RESOURCE_ATTRIBUTE_PRESENT |
                       EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
                       EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
                       EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
                       EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |
                       EFI_RESOURCE_ATTRIBUTE_TESTED
                       );

  virtualMemoryTable = (ARM_MEMORY_REGION_DESCRIPTOR *)AllocatePages (EFI_SIZE_TO_PAGES (sizeof(ARM_MEMORY_REGION_DESCRIPTOR) * MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS));
  if (virtualMemoryTable == NULL) {
    return;
  }

  cacheAttributes = ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK;

  DEBUG ((EFI_D_VERBOSE, "cacheAttributes=0x%d\n", cacheAttributes));

  for (index = 0; index < MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS; index++) {

    virtualMemoryTable[index].PhysicalBase = iMX8MemoryDescriptor[index].PhysicalBase;
    virtualMemoryTable[index].VirtualBase = iMX8MemoryDescriptor[index].VirtualBase;
    virtualMemoryTable[index].Length = iMX8MemoryDescriptor[index].Length;

    if (iMX8MemoryDescriptor[index].Attributes == MEMORY_ATTRIBUTES_PCDCACHEENABLE) {
      virtualMemoryTable[index].Attributes = cacheAttributes;
    } else {
      virtualMemoryTable[index].Attributes = iMX8MemoryDescriptor[index].Attributes;
    }
  }

  ASSERT ((index) <= MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS);

#ifndef CONFIG_HEADLESS
  // Reserve frame buffer
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_RESERVED,
    EFI_RESOURCE_ATTRIBUTE_PRESENT |
      EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
      EFI_RESOURCE_ATTRIBUTE_TESTED,
    FixedPcdGet64 (PcdArmLcdDdrFrameBufferBase),
    FixedPcdGet64 (PcdArmLcdDdrFrameBufferSize));
#endif

#ifdef CONFIG_OPTEE
  // Reserve OP-TEE private memory
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_RESERVED,
    EFI_RESOURCE_ATTRIBUTE_PRESENT |
    EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
    EFI_RESOURCE_ATTRIBUTE_TESTED,
    FixedPcdGet32 (PcdTrustZonePrivateMemoryBase),
    FixedPcdGet32 (PcdTrustZonePrivateMemorySize)
    );
#endif

  // Reserve TPM2 Control Area
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_RESERVED,
    EFI_RESOURCE_ATTRIBUTE_PRESENT |
    EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
    EFI_RESOURCE_ATTRIBUTE_TESTED,
    FixedPcdGet32 (PcdTpm2AcpiBufferBase),
    FixedPcdGet32 (PcdTpm2AcpiBufferSize)
    );

  // Reserve Global Data area
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_RESERVED,
    EFI_RESOURCE_ATTRIBUTE_PRESENT |
      EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
      EFI_RESOURCE_ATTRIBUTE_TESTED,
    FixedPcdGet32 (PcdGlobalDataBaseAddress),
    FixedPcdGet32 (PcdGlobalDataSize)
    );

  // Declare main system memory
  BuildResourceDescriptorHob (
    EFI_RESOURCE_SYSTEM_MEMORY,
    ResourceAttributes,
    FixedPcdGet64 (PcdSystemMemoryBase),
    FixedPcdGet64 (PcdSystemMemorySize)
    );

#if FixedPcdGet64 (PcdBank1MemorySize) > 0
  // Declare system memory outside 32bit address space
  BuildResourceDescriptorHob (
    EFI_RESOURCE_SYSTEM_MEMORY,
    ResourceAttributes,
    FixedPcdGet64 (PcdBank1MemoryBase),
    FixedPcdGet64 (PcdBank1MemorySize)
    );
#endif

  *VirtualMemoryMap = virtualMemoryTable;
}
