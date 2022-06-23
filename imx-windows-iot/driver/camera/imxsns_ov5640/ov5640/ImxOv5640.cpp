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

#include <Ntddk.h>
#include "ImxOv5640.h"

#define ARRAY_SIZE(param) (sizeof(param)/ sizeof(param[0]))

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief Writes given data to OV5640 camera registers starting at specified address.
 *
 * @param RegAddr  Starting address from which to start loading data into camera.
 * @param ValArray Array of data to be loaded into camera.
 * @param Num Length of ValArray array.
 *
 * @return Returns @ref STATUS_SUCCESS if success, otherwise returns error code.
 */
NTSTATUS Ov5640_t::WriteMultiRegs(const UINT16 RegAddr, const UINT8 ValArray[], const UINT32 Num)
{
	NTSTATUS Status;
	PUINT8 p_Bytes = new (NonPagedPoolNx, 'miSH') UINT8[2 + Num];

	p_Bytes[0] = (UINT8)(RegAddr >> 8);
	p_Bytes[1] = (UINT8)(RegAddr & 0xFF);
	RtlCopyMemory(p_Bytes + 2, ValArray, Num);

	Status = m_Camera_res.m_I2c.WriteBytes(p_Bytes, sizeof(2 + Num));
	delete p_Bytes;
	return Status;
};

/**
 * @brief Writes given Address-data array to OV5640 camera registers.
 *
 * @param RegisterValuesArray Array of Address-data to be loaded into camera.
 * @param RegisterValuesArrayLength Length of RegisterValuesArray array.
 *
 * @return Returns @ref STATUS_SUCCESS if success, otherwise returns error code.
 */
NTSTATUS Ov5640_t::LoadRegVal(const reg_val_t RegisterValuesArray[], const UINT32 RegisterValuesArrayLength)
{
    UINT32 i;
    NTSTATUS Status = STATUS_SUCCESS;

	// TODO: optimize this to a single SPB request;
    for (i = 0; i < RegisterValuesArrayLength; i++) {
        Status = WriteReg(RegisterValuesArray[i].regAddr, RegisterValuesArray[i].regVal);

        if (!NT_SUCCESS(Status)) {
            break;
        }
    }
    
    return Status;
}

/**
 * @brief Restarts the 0V5640 camera.
 *
 * @return Returns @ref STATUS_SUCCESS if success, otherwise returns error code.
 */
NTSTATUS Ov5640_t::SoftwareReset()
{
    reg_val_t param[] = {{0x3103, 0x11}, {0x3008, 0x82}};

    return LoadRegVal(param, ARRAY_SIZE(param));
}

// Public interface -------------------------------------------------
/**
 * @brief Initialize the camera device at boot time to safe state.
 *
 * @return Returns @ref STATUS_SUCCESS if success, otherwise returns error code.
 */
NTSTATUS Ov5640_t::Init()
{
	NTSTATUS Status;
	LARGE_INTEGER delay{ 0u };

	m_Camera_res.m_PwnGpio.Set(0);
	if (m_Camera_res.m_HasRstGpio) {
		m_Camera_res.m_RstGpio.Set(1);
	}
	/* Delay 5ms. */
	delay.QuadPart = 5 * 10;
	Status = KeDelayExecutionThread(KernelMode, false, &delay);

	m_Camera_res.m_PwnGpio.Set(0);

	/* Delay 1ms. */
	delay.QuadPart = 1 * 10;
	Status = KeDelayExecutionThread(KernelMode, false, &delay);
	if (m_Camera_res.m_HasRstGpio) {
		m_Camera_res.m_RstGpio.Set(1);
	}
	/* Delay 20ms. */
	delay.QuadPart = 20 * 10;
	Status = KeDelayExecutionThread(KernelMode, false, &delay);
	m_Camera_res.m_PwnGpio.Set(0);
	Status = KeDelayExecutionThread(KernelMode, false, &delay);
	Status = SoftwareReset();

	/* Delay 5ms. */
	delay.QuadPart = 5 * 10;
	Status = KeDelayExecutionThread(KernelMode, false, &delay);
	/* Initialize. */
	if (NT_SUCCESS(Status)) {
		Status = Configure();
	}
	return Status;
}

/**
 * @brief Initialize the camera device to 720p 30fps UYUV 2-lane and start streaming.
 *
 * @return Returns @ref STATUS_SUCCESS if success, otherwise returns error code.
 */
NTSTATUS Ov5640_t::Configure(camera_config_t *aConfigPtr)
{
	UINT8 pixFmt = 0;
	UINT8 rgbYuvMux = 0; // 0 yuv, 1 rgb
#define MUX_YUV 0
#define MUX_RGB 1
	NTSTATUS status = STATUS_SUCCESS;

	if (aConfigPtr == nullptr) {
		aConfigPtr = &m_Defaults;
	}

	switch (aConfigPtr->cameraPixelFormat)
	{
	case kVIDEO_PixelFormatRGB888:
		_DbgKdPrint(("kVIDEO_PixelFormatRGB888\r\n"));
		rgbYuvMux = MUX_RGB;
		pixFmt = 0x20;
		break;
	case kVIDEO_PixelFormatRGB565:
		_DbgKdPrint(("kVIDEO_PixelFormatRGB565\r\n"));
		rgbYuvMux = MUX_RGB;
		pixFmt = 0x61;
		break;
	case kVIDEO_PixelFormatYUYV:
		_DbgKdPrint(("kVIDEO_PixelFormatYUYV\r\n"));
		rgbYuvMux = MUX_YUV;
		pixFmt = 0x30;
		break;
	case kVIDEO_PixelFormatUYVY:
		_DbgKdPrint(("kVIDEO_PixelFormatUYVY\r\n"));
		rgbYuvMux = MUX_YUV;
		pixFmt = 0x32;
		break;
	default:
		_DbgKdPrint(("kVIDEO_Invalid: 0x%x\r\n", aConfigPtr->cameraPixelFormat));
		status = STATUS_INVALID_PARAMETER;
	}

    Ov5640_t::reg_val_t InitParam[232] = {
        {0x3008, 0x42, 0, 0}, /* Start writing to registers */
        /* System setting 0 */
        {0x3103, 0x03, 0, 0}, /* Clock from PLL */
        {0x3004, 0xFF, 0, 0}, /* Enable all clocks */
        {0x3006, 0xC3, 0, 0}, /* Enable all clocks (JPEG 2x clock disabled) */
        {0x3000, 0x00, 0, 0}, /* Reset arry ctrl */
        {0x3002, 0x1C, 0, 0}, /* Reset individual blocks */
        {0x3003, 0x01, 0, 0}, /* reset MIPI */
        {0x302E, 0x08, 0, 0}, /* unknown */
        {0x3034, 0x18, 0, 0}, /* MIPI, 8-bit mode */
        {0x3037, 0x13, 0, 0}, /* PLL div 8 */
        {0x3108, 0x01, 0, 0}, /* clock divider, SCLK = PLL/2 */
        {0x3618, 0x00, 0, 0},
        {0x3612, 0x29, 0, 0},
        {0x3708, 0x64, 0, 0},
        {0x3709, 0x52, 0, 0},
        {0x370C, 0x03, 0, 0},
        {0x3820, 0x41, 0, 0},
        {0x3821, 0x07, 0, 0}, /* timing */
        {0x3630, 0x36, 0, 0},
        {0x3631, 0x0E, 0, 0},
        {0x3632, 0xE2, 0, 0},
        {0x3633, 0x12, 0, 0},
        {0x3621, 0xE0, 0, 0},
        {0x3704, 0xA0, 0, 0},
        {0x3703, 0x5A, 0, 0},
        {0x3715, 0x78, 0, 0},
        {0x3717, 0x01, 0, 0},
        {0x370B, 0x60, 0, 0},
        {0x3705, 0x1A, 0, 0},
        {0x3905, 0x02, 0, 0},
        {0x3906, 0x10, 0, 0},
        {0x3901, 0x0A, 0, 0},
        {0x3731, 0x12, 0, 0},
        {0x3600, 0x08, 0, 0},
        {0x3601, 0x33, 0, 0}, /* debug */
        {0x302D, 0x60, 0, 0},
        /* system ctrl */
        {0x3620, 0x52, 0, 0},
        {0x371B, 0x20, 0, 0},
        {0x471C, 0x50, 0, 0},
        {0x3A13, 0x43, 0, 0},

        {0x3A18, 0x00, 0, 0},
        {0x3A19, 0x7C, 0, 0},
        {0x3635, 0x13, 0, 0},
        {0x3636, 0x03, 0, 0},
        {0x3634, 0x40, 0, 0},
        {0x3622, 0x01, 0, 0},
        {0x3A00, 0x58, 0, 0},
        /* AEC ctrl */
        {0x4001, 0x02, 0, 0},
        {0x4004, 0x02, 0, 0},
        {0x4005, 0x1A, 0, 0}, /* BLC ctrl */
        {0x3A0F, 0x30, 0, 0},
        {0x3A10, 0x28, 0, 0},
        {0x3A1B, 0x30, 0, 0}, /* AEC ctrl */
        {0x3A1E, 0x26, 0, 0},
        {0x3A11, 0x60, 0, 0},
        {0x3A1F, 0x14, 0, 0}, /* AEC ctrl */

        /* AWB */
        {0x5180, 0xFF, 0, 0},
        {0x5181, 0xF2, 0, 0},
        {0x5182, 0x00, 0, 0},
        {0x5183, 0x14, 0, 0},
        {0x5184, 0x25, 0, 0},
        {0x5185, 0x24, 0, 0},
        {0x5186, 0x09, 0, 0},
        {0x5187, 0x09, 0, 0},
        {0x5188, 0x09, 0, 0},
        {0x5189, 0x88, 0, 0},
        {0x518A, 0x54, 0, 0},
        {0x518B, 0xEE, 0, 0},
        {0x518C, 0xB2, 0, 0},
        {0x518D, 0x50, 0, 0},
        {0x518E, 0x34, 0, 0},
        {0x518F, 0x6B, 0, 0},
        {0x5190, 0x46, 0, 0},
        {0x5191, 0xF8, 0, 0},
        {0x5192, 0x04, 0, 0},
        {0x5193, 0x70, 0, 0},
        {0x5194, 0xF0, 0, 0},
        {0x5195, 0xF0, 0, 0},
        {0x5196, 0x03, 0, 0},
        {0x5197, 0x01, 0, 0},
        {0x5198, 0x04, 0, 0},
        {0x5199, 0x6C, 0, 0},
        {0x519A, 0x04, 0, 0},
        {0x519B, 0x00, 0, 0},
        {0x519C, 0x09, 0, 0},
        {0x519D, 0x2B, 0, 0},
        {0x519E, 0x38, 0, 0},

        /* Color Matrix */
        {0x5381, 0x1E, 0, 0},
        {0x5382, 0x5B, 0, 0},
        {0x5383, 0x08, 0, 0},

        {0x5384, 0x0A, 0, 0},
        {0x5385, 0x7E, 0, 0},
        {0x5386, 0x88, 0, 0},

        {0x5387, 0x7C, 0, 0},
        {0x5388, 0x6C, 0, 0},
        {0x5389, 0x10, 0, 0},

        {0x538A, 0x01, 0, 0},
        {0x538B, 0x98, 0, 0},

        /* Sharp */
        {0x5300, 0x08, 0, 0},
        {0x5301, 0x30, 0, 0},
        {0x5302, 0x10, 0, 0},
        {0x5303, 0x00, 0, 0},
        {0x5304, 0x08, 0, 0},

        {0x5305, 0x30, 0, 0},
        {0x5306, 0x08, 0, 0},
        {0x5307, 0x16, 0, 0},

        {0x5309, 0x08, 0, 0},
        {0x530A, 0x30, 0, 0},
        {0x530B, 0x04, 0, 0},
        {0x530C, 0x06, 0, 0},

        /* Gamma */
        {0x5480, 0x01, 0, 0},
        {0x5481, 0x08, 0, 0},
        {0x5482, 0x14, 0, 0},
        {0x5483, 0x28, 0, 0},
        {0x5484, 0x51, 0, 0},
        {0x5485, 0x65, 0, 0},
        {0x5486, 0x71, 0, 0},
        {0x5487, 0x7D, 0, 0},
        {0x5488, 0x87, 0, 0},
        {0x5489, 0x91, 0, 0},
        {0x548A, 0x9A, 0, 0},
        {0x548B, 0xAA, 0, 0},
        {0x548C, 0xB8, 0, 0},
        {0x548D, 0xCD, 0, 0},
        {0x548E, 0xDD, 0, 0},
        {0x548F, 0xEA, 0, 0},
        {0x5490, 0x1D, 0, 0},
        {0x5580, 0x02, 0, 0},
        {0x5583, 0x40, 0, 0},
        {0x5584, 0x10, 0, 0},
        {0x5589, 0x10, 0, 0},
        {0x558A, 0x00, 0, 0},

        /* Lens correction */
        {0x558B, 0xF8, 0, 0},
        {0x5800, 0x23, 0, 0},
        {0x5801, 0x14, 0, 0},

        {0x5802, 0x0F, 0, 0},
        {0x5803, 0x0F, 0, 0},
        {0x5804, 0x12, 0, 0},

        {0x5805, 0x26, 0, 0},
        {0x5806, 0x0C, 0, 0},
        {0x5807, 0x08, 0, 0},

        {0x5808, 0x05, 0, 0},
        {0x5809, 0x05, 0, 0},
        {0x580A, 0x08, 0, 0},

        {0x580B, 0x0D, 0, 0},
        {0x580C, 0x08, 0, 0},
        {0x580D, 0x03, 0, 0},

        {0x580E, 0x00, 0, 0},
        {0x580F, 0x00, 0, 0},
        {0x5810, 0x03, 0, 0},

        {0x5811, 0x09, 0, 0},
        {0x5812, 0x07, 0, 0},
        {0x5813, 0x03, 0, 0},

        {0x5814, 0x00, 0, 0},
        {0x5815, 0x01, 0, 0},
        {0x5816, 0x03, 0, 0},

        {0x5817, 0x08, 0, 0},
        {0x5818, 0x0D, 0, 0},
        {0x5819, 0x08, 0, 0},

        {0x581A, 0x05, 0, 0},
        {0x581B, 0x06, 0, 0},
        {0x581C, 0x08, 0, 0},

        {0x581D, 0x0E, 0, 0},
        {0x581E, 0x29, 0, 0},
        {0x581F, 0x17, 0, 0},

        {0x5820, 0x11, 0, 0},
        {0x5821, 0x11, 0, 0},
        {0x5822, 0x15, 0, 0},

        {0x5823, 0x28, 0, 0},
        {0x5824, 0x46, 0, 0},
        {0x5825, 0x26, 0, 0},

        {0x5826, 0x08, 0, 0},
        {0x5827, 0x26, 0, 0},
        {0x5828, 0x64, 0, 0},

        {0x5829, 0x26, 0, 0},
        {0x582A, 0x24, 0, 0},
        {0x582B, 0x22, 0, 0},

        {0x582C, 0x24, 0, 0},
        {0x582D, 0x24, 0, 0},
        {0x582E, 0x06, 0, 0},

        {0x582F, 0x22, 0, 0},
        {0x5830, 0x40, 0, 0},
        {0x5831, 0x42, 0, 0},

        {0x5832, 0x24, 0, 0},
        {0x5833, 0x26, 0, 0},
        {0x5834, 0x24, 0, 0},

        {0x5835, 0x22, 0, 0},
        {0x5836, 0x22, 0, 0},
        {0x5837, 0x26, 0, 0},

        {0x5838, 0x44, 0, 0},
        {0x5839, 0x24, 0, 0},
        {0x583A, 0x26, 0, 0},

        {0x583B, 0x28, 0, 0},
        {0x583C, 0x42, 0, 0},
        {0x583D, 0xCE, 0, 0},

        { 0x4805, 0x0, 0, 0},

        /* ov5640_setting_720p_1280_720 */
        { 0x3820, 0x41, 0, 0 }, { 0x3821, 0x07, 0, 0 },
        { 0x3800, 0x00, 0, 0 }, { 0x3801, 0x00, 0, 0 }, /* X address start */
        { 0x3802, 0x00, 0, 0 }, { 0x3803, 0xFA, 0, 0 }, /* Y address start */
        { 0x3804, 0x0A, 0, 0 }, { 0x3805, 0x3F, 0, 0 }, /* X address end */
        { 0x3806, 0x06, 0, 0 }, { 0x3807, 0xA9, 0, 0 }, /* Y address end */
        { 0x3808, 0x05, 0, 0 }, { 0x3809, 0x00, 0, 0 }, /* DVP width */
        { 0x380A, 0x02, 0, 0 }, { 0x380B, 0xD0, 0, 0 }, /* DVP height */

		{ 0x380C, 0x07, 0, 0 }, { 0x380D, 0x64, 0, 0 }, /* Total horizontal size */
		{ 0x380E, 0x02, 0, 0 }, { 0x380F, 0xE4, 0, 0 }, /* Total vertical size */
        { 0x3810, 0x00, 0, 0 }, { 0x3811, 0x10, 0, 0 }, /* Vertical offset */
        { 0x3812, 0x00, 0, 0 }, { 0x3813, 0x04, 0, 0 }, /* Horizontal offset */
        { 0x3814, 0x31, 0, 0 }, { 0x3815, 0x31, 0, 0 }, /* Horizontal/vertical subsample increment */

        /* ov5640_mipi_pll_720P_30fps_1280_720 */
		{ 0x3035, 0x21, 0, 0 }, { 0x3036, 0x54, 0, 0 }, { 0x460C, 0x20, 0, 0 },
		{ 0x3824, 0x04, 0, 0 }, { 0x4837, 0x16, 0, 0 },

        /* imx_ov5640_mipi_cfg */
        { 0x4300, pixFmt, 0, 0 }, /* 0x30 YUV422 UYVY */
		/* 0x30 YUV422 UYVY */
        /* 0x31 YUV422 VYUY */
        /* 0x32 YUV422 YUYV */
        /* 0x33 YUV422 VYUY */
        /* 0x3F YUV422 YUYV */
        { 0x501F, rgbYuvMux, 0, 0 }, /* ISP YUV422 */
        { 0x3034, 0x18, 0, 0 }, /* Interface 8-bits mode */
        { 0x300E, 0x45, 0, 0 }, /* MIPI mode, 2-lane mode */
        { 0x5000, 0xA7, 0, 0 }, /* Corrections enabled */
        { 0x5001, 0xA3, 0, 0 }, /* enable all features */
        { 0x3008, 0x02, 0, 0 },
    };
    
	return LoadRegVal(InitParam, ARRAY_SIZE(InitParam));
}

/**
 * @brief De-initialize the camera device.
 *
 * @return Returns @ref STATUS_SUCCESS if success, otherwise returns error code.
 */
NTSTATUS Ov5640_t::Deinit()
{
    return STATUS_NOT_IMPLEMENTED;
}

/**
 * @brief Stop the camera device outputting data.
 *
 * @return Returns @ref STATUS_SUCCESS if success, otherwise returns error code.
 */
NTSTATUS Ov5640_t::Stop()
{
    return WriteReg(0x3008, 0x42);
}
