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
#include "getresrc.h"

NTSTATUS GetReslist(DXGKRNL_INTERFACE* pDxgkInterface, PWCHAR pReslist,
    ULONG ResListLen)
{
    NTSTATUS Status;
    DXGK_DEVICE_INFO DeviceInfo;
    ULONG Len;

    if (!pReslist || !pDxgkInterface)
    {
        return STATUS_INVALID_PARAMETER;
    }

    Status = pDxgkInterface->DxgkCbGetDeviceInformation(
        pDxgkInterface->DeviceHandle, &DeviceInfo);
    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    Status = IoGetDeviceProperty(
        DeviceInfo.PhysicalDeviceObject,
        DevicePropertyBootConfigurationTranslated,
        ResListLen, pReslist, &Len);
    if (!NT_SUCCESS(Status))
    {
        return Status;
    }
    if (Len >= ResListLen)
    {
        // Resource list was probably truncated due to insufficient buffer
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

//
// Parse resource list - currently used for I2C (Connection) and Memory resources
//
// Param ResList  resource list to be parsed
// Param ResType  resource type we are looking for. Supported CmResourceTypeConnection for I2C and CmResourceTypeMemory
// Param pConnMem For CmResourceTypeConnection (I2C) returns the connection ID
//                For CmResourceTypeMemory returns the Address (Memory.Start)
// Param pSize  For CmResourceTypeConnection (I2C) not used - can be NULL
//             For CmResourceTypeMemory returns the Size (Memory.Length)
// Param Index index (order) of the resource in the ACPI
NTSTATUS ParseReslist(PCM_RESOURCE_LIST Reslist, UCHAR ResType,
    LARGE_INTEGER *pConnMem, ULONG *pSize, ULONG Index)
{
    PCM_FULL_RESOURCE_DESCRIPTOR List;
    ULONG Count = 0;
    ULONG Found = 0;

    if (!Reslist || !pConnMem)
    {
        return STATUS_INVALID_PARAMETER;
    }
    List = Reslist->List;
    for (ULONG i = 0; i < Reslist->Count; ++i)
    {
        // Process resources in CM_FULL_RESOURCE_DESCRIPTOR block number i.

        for (ULONG j = 0; j < List->PartialResourceList.Count; ++j)
        {
            PCM_PARTIAL_RESOURCE_DESCRIPTOR Desc;
            Desc = List->PartialResourceList.PartialDescriptors + j;
            switch (Desc->Type)
            {
            case CmResourceTypeConnection:  // Check for I2C resource
                if (Desc->u.Connection.Class == CM_RESOURCE_CONNECTION_CLASS_SERIAL
                    && Desc->u.Connection.Type == CM_RESOURCE_CONNECTION_TYPE_SERIAL_I2C)
                {
                    if (Desc->Type == ResType)
                    {
                        if (Count == Index)
                        {
                            pConnMem->LowPart = Desc->u.Connection.IdLowPart;
                            pConnMem->HighPart = Desc->u.Connection.IdHighPart;
                            Found++;
                        }
                        Count++;
                    }
                }
                break;
            case CmResourceTypeMemory: // Check for Memory resource
                if (Desc->Type == ResType)
                {
                    if (Count == Index)
                    {
                        pConnMem->QuadPart = Desc->u.Memory.Start.QuadPart;
                        if (pSize)
                        {
                            *pSize = Desc->u.Memory.Length;
                        }
                        Found++;
                    }
                    Count++;
                }
            default:
                break; // We don't care about other descriptors.
            }
        }
        // Advance to next CM_FULL_RESOURCE_DESCRIPTOR block in memory.
        List = (PCM_FULL_RESOURCE_DESCRIPTOR)(List->PartialResourceList.PartialDescriptors +
            List->PartialResourceList.Count);
    }
    if (!Found)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}
