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

#pragma once

#include <Ntddk.h> /* UINT types */
#include "WdfIoTargets.h"
#include "ImxVideoCommon.h"

class CamWdf_Res {
public:
	UINT8 m_test_var;

	// DSD config
	CHAR DeviceEndpoint0[256];
	UINT32 cameraClockFrequencyHz;
	UINT32 GpioPwnResId;
	UINT32 GpioRstResId;
	UINT32 I2cResId;

	i2c_bus  m_I2c;
	gpio_t   m_RstGpio;
	gpio_t   m_PwnGpio;
	bool     m_HasRstGpio;

	CamWdf_Res(const WDFDEVICE &WdfDevice) : m_I2c(WdfDevice), m_RstGpio(WdfDevice), m_PwnGpio(WdfDevice), m_HasRstGpio(false) {};
};

struct CameraOperations_t {
	NTSTATUS init(const camera_config_t *config, const CamWdf_Res &CameraWdfResource);
	NTSTATUS deinit();
	NTSTATUS start();
	NTSTATUS stop();
    // FIXME
};
