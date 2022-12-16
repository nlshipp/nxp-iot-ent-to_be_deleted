/******************************************************************************
 *
 * Copyright (C) 2016-2017 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 *
 * Copyright 2022 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. THE SOFTWARE IS PROVIDED "AS IS",
 * WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ******************************************************************************
 */

#pragma once

#include <ntddk.h>
#include "GcKmdBaseTransmitter.h"
#include "GcKmd7LIO.h"
#include "edidparser.h"

extern "C" {
#include "linux/platform_device.h"
#include "lvds/imx8mp-ldb.h"
#include "linux/i2c.h"
}

class MpLvdsTransmitter : public BaseTransmitter
{
public:

    NTSTATUS Start(DXGKRNL_INTERFACE* pDxgkInterface);

    NTSTATUS Stop();

    int GetCachedEdid(PVOID *pEdid);

    virtual NTSTATUS GetHotPlugDetectStatus(UINT8* status) override;

    virtual NTSTATUS GetEdid(PVOID Data, ULONG Length, UINT8 Block, UINT8 Segment) override;

    virtual void GetChildDescriptor(DXGK_CHILD_DESCRIPTOR* pDescriptor) override;

    struct platform_device lvds_pdev;
    struct i2c_client m_i2c_hdmi;
private:

    NTSTATUS GetI2CresourceNum(DXGKRNL_INTERFACE* pDxgkInterface, ULONG i2c_index, LARGE_INTEGER *i2c_connection_id);

    BOOLEAN InBounds(UINT8* MemoryAreaStart, int MemoryAreaSize, UINT8* RangeStart, int RangeSize)
    {
        return MemoryAreaStart <= RangeStart && RangeStart + RangeSize <= MemoryAreaStart + MemoryAreaSize;
    }

    void AdvancedPeripheralBusRead(UINT32 offset, UINT32* value)
    {
        *value = ReadDoubleWord(m_Base + offset);
    }


    void AdvancedPeripheralBusWrite(UINT32 offset, UINT32 value)
    {
        WriteDoubleWord(m_Base + offset, value);
    }

    NTSTATUS GetRegistryParams(DXGKRNL_INTERFACE* pDxgkInterface);
    CHAR* GetPrintableDispInterface();

    BYTE* m_Base{0};

    BYTE m_CachedEdid[EDID_SIZE];

    struct platform_device lvds_phy_pdev;

    imx_lvds_display_interface m_disp_interface;
    UINT32 m_bus_data_width;
    const char *m_bus_mapping;
    struct i2c_client m_i2c_lvds;

};
