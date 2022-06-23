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
#if (DBG)
#define _DbgKdPrint KdPrint
// Uncomment _DbgFrameKdPrint to enable ISR debug messages.
#define _DbgFrameKdPrint(...) 
//KdPrint(__VA_ARGS__)
#else // !DBG
#define _DbgKdPrint(...)
#define _DbgFrameKdPrint(...) 
#endif // !DBG


/* Two frame buffer loaded to CSI register. Must be 3 or higher. */
#define CSI_COMMON_FRAME_NUM 3U

#if CSI_COMMON_FRAME_NUM < 3
#error CSI_COMMON_FRAME_NUM must be 3 or higher.
#endif

EXTERN_C_START

//
// WDFDRIVER Events
//

DRIVER_INITIALIZE DriverEntry;

EXTERN_C_END

struct WdfCsi_ctx;
typedef WdfCsi_ctx DEVICE_CONTEXT, *PDEVICE_CONTEXT;
struct WdfCsiDeviceChannel_ctx;
typedef WdfCsiDeviceChannel_ctx SUBDEVICE_QUEUE_CONTEXT, *PSUBDEVICE_QUEUE_CONTEXT;
struct WdfCsiRequest_ctx;
typedef WdfCsiRequest_ctx  REQUEST_CONTEXT, *PREQUEST_CONTEXT;
struct CsiIsrCtx_t;
typedef CsiIsrCtx_t DEVICE_INTERRUPT_CONTEXT, *PDEVICE_INTERRUPT_CONTEXT;
struct WdfCsiFile_ctx;
typedef WdfCsiFile_ctx DEVICE_FILE_CONTEXT, *PDEVICE_FILE_CONTEXT;


#define MAX_CHAN 2

#include "WdfIoTargets.h"

#include "CsiIomap.h"

struct WdfCsi_ctx: io::ctx_acpi_csr_stub
{
	enum : ULONG { IMX_CSI_POOL_TAG = 'ICXM' };
    struct DiscardBuffInfo_t {
		PHYSICAL_ADDRESS phys;
		PVOID virt;

		PMDL mdlPtr;
		enum {
			FREE,
			WORKING,
			DONE
		} state;
	};
    
    UINT32 m_RxLevel;

	const WDFDEVICE m_WdfDevice;
	WDFQUEUE m_Queue;

	WDFWAITLOCK m_CreateDevLock;
	bool m_availChan[MAX_CHAN];
	
	bool ChannelGetIfAvailable(ULONG n)
	{
		if ((n < MAX_CHAN) && (m_availChan[n])) {
			m_availChan[n] = false;
			return true;
		}
		else {
			return false;
		}
	}
	
    void ChannelFree(ULONG n)
	{
		if (n < MAX_CHAN) {
			m_availChan[n] = true;
		}
	}
	
	/* PNP static callbacks */
	static EVT_WDF_DEVICE_RELEASE_HARDWARE EvtReleaseHw; // Translates call to nonstatic member.
	static EVT_WDF_DEVICE_PREPARE_HARDWARE EvtPrepareHw;
	NTSTATUS PrepareHw(_In_ WDFCMRESLIST ResourcesRaw, _In_ WDFCMRESLIST ResourcesTranslated);
	NTSTATUS Get_DsdAcpiResources();
	AcpiDsdRes_t m_DsdRes;

	reg<CSI_REGS> m_Csi1Reg; // TODO consider moving to DSD or resource class
	UINT32 Csi1RegResId;
    volatile CSI_REGS *m_CsiRegistersPtr;
	Resources_t m_MipiCsiRes;


	CHAR DeviceEndpoint[256];
	UINT32 coreClockFrequencyHz;
	UINT32 phyClockFrequencyHz;
	UINT32 escClockFrequencyHz;


	static EVT_WDF_DEVICE_D0_ENTRY EvtD0Entry;
	static EVT_WDF_DEVICE_D0_EXIT EvtD0Exit;

	/* PNP worker methods */

	// interrupt servicing

	PDEVICE_INTERRUPT_CONTEXT m_IsrCtx;

	/* IRP */
	static EVT_WDF_DEVICE_FILE_CREATE EvtDeviceFileCreate;
	static EVT_WDF_FILE_CLOSE EvtDeviceFileClose;
	static EVT_WDF_FILE_CLEANUP EvtDeviceFileCleanup;

	static EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL EvtDeviceControl;
	static EVT_WDF_IO_QUEUE_IO_STOP EvtIoStop;
	static EVT_WDF_REQUEST_CANCEL EvtWdfRequestCancel;
	/* IRP MJ */
	NTSTATUS Close();
    void EvtFrameRequest(PREQUEST_CONTEXT RequestCtxPtr);
	void TerminateIo();
	void ReinitializeRequest(PREQUEST_CONTEXT RequestCtxPtr);

	enum {
		S_FRAME_REQUESTED,
		S_STOP_REQUESTED,
		S_DISCARDING,
		S_STOPPED,
	} m_State;

	UINT32 m_CurrSrErrorMask;
	const UINT32 m_CurrSrFrameMask = (SR_DMA_FIELD1_DONE_BIT | SR_DMA_FIELD2_DONE_BIT); // FIXME rename
	void DisableAllInterruptsNofence();
	void EnableErrorInterruptsNofence(bool enable, bool clear = false);
	void EnableFrameInterruptsNofence(bool enable, bool clear = false);


	void CsiClearRxFifo();
	void CsiReflashFifoDma();
	void CsiEnableFifoDmaRequest(bool enable);
	void CsiStart(bool enable);
	void CsiResetAndStop();
	UINT8 CsiPopSrFrameId(volatile CSI_REGS *RegistersPtr, UINT32 Sr);
	NTSTATUS CsiInit(const camera_config_t &config);
	void FinishGetFrameRequest(PREQUEST_CONTEXT RequestCtxPtr);
    bool DpcCsiResetDmaIfError(UINT32 csiSr);

	PREQUEST_CONTEXT m_ActiveRequestCtxPtr;
	DiscardBuffInfo_t *m_FinishedBuffPtr;
	DiscardBuffInfo_t m_DiscardBuff[CSI_COMMON_FRAME_NUM];
	NTSTATUS AllocFb(DiscardBuffInfo_t &buffInfo);
	NTSTATUS MapFb(DiscardBuffInfo_t &buffInfo);
	void FreeFb(DiscardBuffInfo_t &buffInfo);
	NTSTATUS AcquireBuffers();
	static constexpr SIZE_T m_BufferRequiredSize = 1366 * 768 * 4;
	static constexpr SIZE_T m_FrameLenBytes = 1280 * 720 * 2;
	bool m_csi_two_8bit_sensor_mode; // Cfg - TODO: move to resource


	// Counters
	UINT32 m_ErrorDoubleFrameDoneCnt;
	UINT32 m_DiscardedFrameCnt;
	UINT32 m_ErrorHrespCnt;
	UINT32 m_ErrorAddrChangeCnt;
	UINT32 m_ErrorOverflowCnt;
	UINT32 m_CompleteFrameCnt;
public:
	WdfCsi_ctx(WDFDEVICE &device);
	NTSTATUS RegisterQueue();
	static EVT_WDF_OBJECT_CONTEXT_CLEANUP EvtDriverContextCleanup;
	static EVT_WDF_DEVICE_CONTEXT_CLEANUP EvtDeviceObjCtxCleanup;
	static EVT_WDF_DRIVER_DEVICE_ADD EvtDeviceAdd;
};
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext);

struct SubdevCtxPtrListItem : list_item_t
{
	WdfCsiDeviceChannel_ctx *ptr;

	SubdevCtxPtrListItem(WdfCsiDeviceChannel_ctx *Ptr) : ptr(Ptr) { ; }
};

struct CsiIsrCtx_t
{
	PDEVICE_CONTEXT m_CtxPtr;
	WDFINTERRUPT m_WdfInterrupt;
	WdfCsiRequest_ctx *m_ReqCtxPtr;

	WdfCsiDeviceChannel_ctx *m_SubdevQueueCtxPtr[MAX_CHAN];

	list_t<SubdevCtxPtrListItem> m_ActiveChannels;

	CsiIsrCtx_t(PDEVICE_CONTEXT ctxPtr, WDFINTERRUPT WdfInterrupt);
	static NTSTATUS registerInterruptHandler(PDEVICE_CONTEXT ctxPtr);
	static EVT_WDF_INTERRUPT_ENABLE EvtWdfInterruptEnable;
	static EVT_WDF_INTERRUPT_DISABLE EvtWdfInterruptDisable;
	static EVT_WDF_INTERRUPT_ISR EvtInterruptIsr;
	static EVT_WDF_INTERRUPT_DPC EvtInterruptDpc;

};
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_INTERRUPT_CONTEXT, DeviceGetInterruptContext);

struct WdfCsiRequest_ctx
{
	NTSTATUS m_status;
	ULONG m_CtlCode;
	WDFREQUEST m_WdfRequest;
	size_t m_InLen;
	FrameInfo_t m_FrameInfo;
};
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(REQUEST_CONTEXT, GetRequestContext);

struct WdfCsiFile_ctx
{
	WDFQUEUE m_WdfQueue;
	PDEVICE_CONTEXT	m_CtxPtr;
	PDEVICE_INTERRUPT_CONTEXT m_IsrCtxPtr;
	UCHAR m_ChanId;
};
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_FILE_CONTEXT, DeviceGetFileContext);