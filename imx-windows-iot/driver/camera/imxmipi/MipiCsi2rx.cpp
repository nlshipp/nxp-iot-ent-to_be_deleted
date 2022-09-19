/*
 * Copyright 2022 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the name of the copyright holder nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "imxmipi.h"
#include "MipiCsi2rx.tmh"

NTSTATUS MipiCsi2_t::PhyAssertReset()
/*!
 * Assert reset of MIPI D-Phy.
 *
 * @returns STATUS_SUCCESS or error code.
 */
{
    NTSTATUS status;
    volatile UINT16 d = 1;

    status = m_ResourcePtr->AcpiSetMipiRcr(1);
    while (d > 0) {
        ++d;
    }
    return status;
}

NTSTATUS MipiCsi2_t::PhyDeassertReset()
/*!
 * De-assert reset of MIPI D-Phy.
 *
 * @returns STATUS_SUCCESS or error code.
 */
{
    NTSTATUS status;
    volatile UINT16 d = 1;

    status = m_ResourcePtr->AcpiSetMipiRcr(0);
    while (d > 0) {
        ++d;
    }
    return status;
}

NTSTATUS MipiCsi2_t::EnablePhy(UINT8 tHsSettle_EscClk)
/*!
 * Configures Hs Settle delay and enables Mipi D-Phy.
 *
 * @param tHsSettle_EscClk Hs Settle delay as number of Esc clock periods.
 *
 * @returns STATUS_SUCCESS or error code.
 */
{
    NTSTATUS status;
    UINT32 GPR34;

    status = m_ResourcePtr->AcpiRgpr(GPR34);
    if (NT_SUCCESS(status)) {
        GPR34 = (GPR34 & ~0x3FFF)
            | GPR34_RX_ENABLE
            | GPR34_VID_INTFC_ENB
            | GPR34_S_PRG_RXHS_SETTLE(tHsSettle_EscClk - 1UL)
            | GPR34_RX_RCAL_HIGHER;

        UNREFERENCED_PARAMETER(tHsSettle_EscClk);
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "MipiCsi2_t::EnablePhy (settle = 0x%x)", (unsigned)tHsSettle_EscClk);
        status = m_ResourcePtr->AcpiWgpr(GPR34);
    }
    return status;
}

NTSTATUS MipiCsi2_t::DisablePhy()
/*!
 * Disables MIPI D-Phy.
 */
{
    NTSTATUS status;
    UINT32 GPR34;

    status = m_ResourcePtr->AcpiRgpr(GPR34); 
    if (NT_SUCCESS(status)) {
        GPR34 = (GPR34 & ~0x3FFF)
                | GPR34_PD_RX; // Power down D-PHY

        status = m_ResourcePtr->AcpiWgpr(GPR34);
        if (NT_SUCCESS(status)) {
            status = PhyDeassertReset();
        }
    }
    return status;
}

void MipiCsi2_t::DisableInterrupts(UINT32 mask)
/*!
 * Disable the MIPI interrupts.
 *
 * @param mask OR'ed interrupt mask.
 */
{
    m_RegistersPtr->IRQ_MASK |= mask;
}

NTSTATUS MipiCsi2_t::Init(const camera_config_t &Config)
/*!
 * Configures the CSI2RX peripheral module.
 *
 * @param Config information about video stream being processed (resolution, frame rate, color format ..).
 *
 * @returns STATUS_SUCCESS or error code.
 */
{
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(Config);
    // 1. Reset DPHY
    // 2. Init MIPI
    // 3. Enable DPHY
    DisablePhy();
    PhyAssertReset();

    m_RegistersPtr->DISABLE_PAYLOAD_0 &=
        ~(MIPI_CSI2RX_REGS::DISABLE_PAYLOAD_0_RGB888
            | MIPI_CSI2RX_REGS::DISABLE_PAYLOAD_0_RGB565
            | MIPI_CSI2RX_REGS::DISABLE_PAYLOAD_0_YUV422_10B
            | MIPI_CSI2RX_REGS::DISABLE_PAYLOAD_0_YUV422_8B
            | MIPI_CSI2RX_REGS::DISABLE_PAYLOAD_0_YUV420_8B
        );

    if (NT_SUCCESS(status)) {
        m_RegistersPtr->IGNORE_VC = 1;
        m_RegistersPtr->VID_VC = 1;
        m_RegistersPtr->VID_P_FIFO_SEND_LEVEL = 64;
    }
    DisableInterrupts(0x1FF);

    return status;
}

NTSTATUS MipiCsi2_t::Start(const camera_config_t &Config)
/*!
 * Enables configured number data lanes and configures Hs Settle delay.
 *
 * @param Config information about video stream being processed (resolution, frame rate, color format ..).
 *
 * @returns STATUS_SUCCESS or error code.
 */
{
    UINT32 hs_settle, hs_prg_settle;

    /* Hs-settle timeout = 115 + 8 * data_rate[Gbps] */
    hs_settle = 115 + ((8 * 1000) / (Config.mipiLaneClk / 1000000));
    hs_prg_settle = ((hs_settle * (m_ResourcePtr->escClockFrequencyHz / 1000000)) / 1000) - 1;

    m_RegistersPtr->NUM_LANES = Config.csiLanes - 1UL;
    m_RegistersPtr->DISABLE_DATA_LANES = 0xF & ~((1UL << (UINT32)Config.csiLanes) - 1UL);

    EnablePhy((UINT8)hs_prg_settle);

    return STATUS_SUCCESS;
}

NTSTATUS MipiCsi2_t::Stop()
/*!
 * Disables all CSI2RX data lanes. Keeps CSI fifo state.
 *
 * @returns STATUS_SUCCESS or error code.
 */
{
    m_RegistersPtr->DISABLE_DATA_LANES |= 0xF;

    return STATUS_SUCCESS;
}

NTSTATUS MipiCsi2_t::Deinit()
/*!
 * Disables the CSI2RX peripheral module.
 *
 * @returns STATUS_SUCCESS or error code.
 */
{
    Stop();
    DisablePhy();

    return STATUS_SUCCESS;
}

NTSTATUS MipiCsi2_t::PrepareHw(Resources_t &MipiRes)
/*!
 * Passes Initializes CSI2RX peripheral module defaults.
 *
 * @param MipiRes containing resources and information from ACPI.
 *
 * @returns STATUS_SUCCESS or error code.
 */
{
    NTSTATUS status = STATUS_SUCCESS;

    ASSERT(this != NULL);
    if (this == NULL) {
        status = STATUS_EXPIRED_HANDLE;
    }
    m_ResourcePtr = &MipiRes;
    m_RegistersPtr = (MIPI_CSI2RX_REGS*)MipiRes.m_MipiRegistersPtr; // Shouldn't probably cast here
    if (m_RegistersPtr == NULL) {
        status = STATUS_INVALID_PARAMETER_1;
    }

    /* Don't disable data types. */
    m_RegistersPtr->DISABLE_PAYLOAD_0 = 0xFFFFFFFF;
    m_RegistersPtr->DISABLE_PAYLOAD_1 = 0xFFFFFFFF;
    DisableInterrupts(0x1FF);

    /* Disable all interrupts. */
    m_RegistersPtr->IRQ_MASK = MIPI_CSI2RX_IRQ_MASK_IRQ_MASK_MASK;

    return status;
}
