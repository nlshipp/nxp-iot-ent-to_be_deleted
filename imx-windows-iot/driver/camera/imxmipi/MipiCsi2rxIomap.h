/*
 * Copyright 2017, 2019-2020, 2022 NXP
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

#pragma once

#include <ntddk.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

struct Resources_t;

typedef NTSTATUS ACPI_READ_UINT32(UINT32 &Val);
typedef NTSTATUS ACPI_WRITE_UINT32(UINT32 Val);

class MipiCsi2_t {
public: // Type definitions

#pragma pack( push )
#pragma pack( 4 )
    struct MIPI_CSI2RX_REGS
        /** MIPI_CSI2RX - Register Layout Typedef */
    {
        /* ____ */ UINT8 RESERVED_0[256]; // 100h
        /* __IO */ UINT32 NUM_LANES;                     /**< Lane Configuration Register, offset: 0x100 */
        /* __IO */ UINT32 DISABLE_DATA_LANES;            /**< Disable Data Lane Register, offset: 0x104 */
        /* __I  */ UINT32 BIT_ERR;                           /**< ECC and CRC Error Status Register, offset: 0x108 */
        /* __I  */ UINT32 IRQ_STATUS;                        /**< IRQ Status Register, offset: 0x10C */
        /* __IO */ UINT32 IRQ_MASK;                          /**< IRQ Mask Setting Register, offset: 0x110 */
        /* __I  */ UINT32 ULPS_STATUS;                       /**< Ultra Low Power State (ULPS) Status Register, offset: 0x114 */
        /* __I  */ UINT32 PPI_ERRSOT_HS;                     /**< ERRSot HS Status Register, offset: 0x118 */
        /* __I  */ UINT32 PPI_ERRSOTSYNC_HS;                 /**< ErrSotSync HS Status Register, offset: 0x11C */
        /* __I  */ UINT32 PPI_ERRESC;                        /**< ErrEsc Status Register, offset: 0x120 */
        /* __I  */ UINT32 PPI_ERRSYNCESC;                    /**< ErrSyncEsc Status Register, offset: 0x124 */
        /* __I  */ UINT32 PPI_ERRCONTROL;                    /**< ErrControl Status Register, offset: 0x128 */
        /* __IO */ UINT32 DISABLE_PAYLOAD_0;             /**< Disable Payload 0 Register, offset: 0x12C */
        /* __IO */ UINT32 DISABLE_PAYLOAD_1;             /**< Disable Payload 1 Register, offset: 0x130 */
        /* ____ */ UINT8 RESERVED_1[76];
        /* __IO */ UINT32 IGNORE_VC;                     /**< Ignore Virtual Channel Register, offset: 0x180 */
        /* __IO */ UINT32 VID_VC;                        /**< Virtual Channel value Register, offset: 0x184 */
        /* __IO */ UINT32 VID_P_FIFO_SEND_LEVEL;         /**< FIFO Send Level Configuration Register, offset: 0x188 */
        /* __IO */ UINT32 VID_VSYNC;                     /**< VSYNC Configuration Register, offset: 0x18C */
        /* __IO */ UINT32 VID_HSYNC_FP;                  /**< Start of HSYNC Delay control Register, offset: 0x190 */
        /* __IO */ UINT32 VID_HSYNC;                     /**< HSYNC Configuration Register, offset: 0x194 */
        /* __IO */ UINT32 VID_HSYNC_BP;                 /**< End of HSYNC Delay Control Register, offset: 0x198 */

        static constexpr UINT32 DISABLE_PAYLOAD_0_RGB888 = (1u << 20);
        static constexpr UINT32 DISABLE_PAYLOAD_0_RGB565 = (1u << 18);
        static constexpr UINT32 DISABLE_PAYLOAD_0_YUV422_10B = (1u << 15);
        static constexpr UINT32 DISABLE_PAYLOAD_0_YUV422_8B = (1u << 14);
        static constexpr UINT32 DISABLE_PAYLOAD_0_YUV420_8B = (1u << 10);
    };

    struct MIPI_GPR_REGS // Address: 3034_0000h base + 88h offset = 3034_0088h
    {
        UINT32 GPR[48];
    };

    struct MIPI_SCR_REGS // Address: 3034_0000h base + 88h offset = 3034_0088h
    {
        UINT32 _dummy[19];
        UINT32 MIPIPHY1_RCR;
        UINT32 MIPIPHY2_RCR;

        static constexpr UINT32 MIPI_RCR_DOM_EN_BIT = (1u << 31);
        static constexpr UINT32 MIPI_RCR_LOCK_BIT = (1u << 30);
        static constexpr UINT32 MIPI_RCR_DOMAIN3_BIT = (1u << 27);
        static constexpr UINT32 MIPI_RCR_DOMAIN2_BIT = (1u << 26);
        static constexpr UINT32 MIPI_RCR_DOMAIN1_BIT = (1u << 25);
        static constexpr UINT32 MIPI_RCR_DOMAIN0_BIT = (1u << 24);
        static constexpr UINT32 MIPI_RCR_ESC_RESET_BIT = (1u << 2);
        static constexpr UINT32 MIPI_RCR_PHY_REF_RESET_BIT = (1u << 1);
        static constexpr UINT32 MIPI_RCR_CORE_RESET_BIT = (1u << 0);
    };
#pragma pack( pop )

    // D-Phy termination: 0b +25% (highest), 1b +15%, 10b mid, 11b -25% (lowest)
    static constexpr UINT32 GPR34_RX_RCAL_HIGHEST = (0u << 0);
    static constexpr UINT32 GPR34_RX_RCAL_HIGHER = (1u << 0);
    static constexpr UINT32 GPR34_RX_RCAL_LOWEST = (11u << 0);
    static constexpr UINT32 GPR34_RX_RCAL_MEDIUM = (10u << 0);

    static constexpr UINT32 GPR34_S_PRG_RXHS_SETTLE(UINT32 x) { // T_HS_SETTLE transition wait
        return ((x) & 0x3F) << 2;
    }
    static constexpr UINT32 GPR34_CONT_CLK_MODE = (1u << 8); // Slave clock lane maintain HS reception in contignous mode
    static constexpr UINT32 GPR34_AUTO_PD_EN = (1u << 9); // 0 - inactive lanes in LP11, 1 - only NUM_LANES other inactive
    static constexpr UINT32 GPR34_HSEL = (1u << 10); // 0 - 1 Gbps, 1 - 1.2 Gbps
    static constexpr UINT32 GPR34_PD_RX = (1u << 11); // Power down D-PHY
    static constexpr UINT32 GPR34_VID_INTFC_ENB = (1u << 12);
    static constexpr UINT32 GPR34_RX_ENABLE = (1u << 13); // Discard received packets (since packet boundary)
    static constexpr UINT32 GPR35_RX_DPHY_RDY = (1u << 16); // Discard received packets (since packet boundary)

#define MIPI_CSI2RX_IRQ_MASK_IRQ_MASK_MASK       (0x1FFU)

private:
    volatile MIPI_CSI2RX_REGS *m_RegistersPtr;
    Resources_t *m_ResourcePtr;
    NTSTATUS PhyAssertReset();
    NTSTATUS PhyDeassertReset();
    NTSTATUS EnablePhy(UINT8 tHsSettle_EscClk);
    NTSTATUS DisablePhy();
    void DisableInterrupts(UINT32 mask);

public:
    MipiCsi2_t() :m_ResourcePtr(NULL) {};

    // Public interface -------------------------------------------------
    NTSTATUS Check() {return STATUS_SUCCESS; };

    NTSTATUS Init(const camera_config_t &config);

    NTSTATUS Deinit();

    NTSTATUS MipiCsi2_t::Start(const camera_config_t &Config);
    NTSTATUS Stop();
    NTSTATUS PrepareHw(Resources_t &MipiRes);
};


struct Resources_t {
    DEVICE_CONTEXT *m_DeviceCtxPtr;
    MipiCsi2_t::MIPI_CSI2RX_REGS *m_MipiRegistersPtr;

    ACPI_READ_UINT32 AcpiRgpr;
    ACPI_WRITE_UINT32 AcpiWgpr;
    ACPI_WRITE_UINT32 AcpiSetMipiRcr;

    UINT32 m_CpuId;

    UINT32 coreClockFrequencyHz;
    UINT32 phyClockFrequencyHz;
    UINT32 escClockFrequencyHz;
    PCHAR  csiDevNameA;

    Resources_t(DEVICE_CONTEXT *deviceCtxPtr) : m_DeviceCtxPtr(deviceCtxPtr) {};
};
