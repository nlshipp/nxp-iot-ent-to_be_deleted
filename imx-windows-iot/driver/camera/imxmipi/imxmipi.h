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

extern "C" {
#include <ntddk.h>
#include <wdf.h>
#include <initguid.h>
}
#include "Trace.h"
#include "Public.h"

#define DISPATCH_CODE() PAGED_ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

/* Two frame buffer loaded to CSI register. Must be 3 or higher. */
#define CSI_COMMON_FRAME_NUM 3U

#if CSI_COMMON_FRAME_NUM < 3
#error CSI_COMMON_FRAME_NUM must be 3 or higher.
#endif

extern "C" {
    DRIVER_INITIALIZE DriverEntry;
}

struct WdfMipi_ctx;
typedef WdfMipi_ctx DEVICE_CONTEXT, *PDEVICE_CONTEXT;
struct WdfMipiRequest_ctx;
typedef WdfMipiRequest_ctx  REQUEST_CONTEXT, *PREQUEST_CONTEXT;
struct WdfMipiFile_ctx;
typedef WdfMipiFile_ctx DEVICE_FILE_CONTEXT, *PDEVICE_FILE_CONTEXT;

#include "WdfIoTargets.hpp"
#include "MipiCsi2rxIomap.h"

struct WdfMipi_ctx
{
    const WDFDEVICE m_WdfDevice;
    WDFQUEUE m_Queue;
    bool m_IsOpen;

    /* PNP static callbacks */
    static EVT_WDF_DEVICE_RELEASE_HARDWARE EvtReleaseHw; // Translates call to non-static member.
    static EVT_WDF_DEVICE_PREPARE_HARDWARE EvtPrepareHw;
    NTSTATUS PrepareHw(_In_ WDFCMRESLIST ResourcesRaw, _In_ WDFCMRESLIST ResourcesTranslated);
    NTSTATUS CreateInterface();
    Resources_t m_Resources;
    MipiCsi2_t m_Mipi;

    static EVT_WDF_DEVICE_D0_ENTRY EvtD0Entry;
    static EVT_WDF_DEVICE_D0_EXIT EvtD0Exit;

    /* IRP */
    static EVT_WDF_DEVICE_FILE_CREATE EvtDeviceFileCreate;
    static EVT_WDF_FILE_CLOSE EvtDeviceFileClose;
    static EVT_WDF_FILE_CLEANUP EvtDeviceFileCleanup;

    static EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL EvtDeviceControl;
    static EVT_WDF_IO_QUEUE_IO_STOP EvtIoStop;
    static EVT_WDF_REQUEST_CANCEL EvtWdfRequestCancel;

    /* IRP MJ */
    NTSTATUS Close();
    void ConfigureRequest(PREQUEST_CONTEXT RequestCtxPtr);
    PREQUEST_CONTEXT m_ActiveRequestCtxPtr;

public:
    WdfMipi_ctx(WDFDEVICE &device);
    NTSTATUS RegisterQueue();
    static EVT_WDF_OBJECT_CONTEXT_CLEANUP EvtDriverContextCleanup;
    static EVT_WDF_DEVICE_CONTEXT_CLEANUP EvtDeviceObjCtxCleanup;
    static EVT_WDF_DRIVER_DEVICE_ADD EvtDeviceAdd;
};
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext);

struct WdfMipiRequest_ctx
{
    NTSTATUS m_Status;
    ULONG m_CtlCode;
    WDFREQUEST m_WdfRequest;
    size_t m_InLen;
};
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(REQUEST_CONTEXT, GetRequestContext);

struct WdfMipiFile_ctx
{
    WDFQUEUE m_WdfQueue;
    PDEVICE_CONTEXT m_DevCtxPtr;
    UCHAR m_ChanId;
};
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_FILE_CONTEXT, DeviceGetFileContext);
