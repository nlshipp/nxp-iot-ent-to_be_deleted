/**************************************************************************

    AVStream Simulated Hardware Sample

    Copyright (c) 2001, Microsoft Corporation.
	Copyright 2022 NXP

    File:

        device.h

    Abstract:

        The header for the device level of the simulated hardware.  This is
        not actually the hardware simulation itself.  The hardware simulation
        is contained in hwsim.*, image.*.
        
    History:

        created 3/9/2001

**************************************************************************/
#define RESHUB_USE_HELPER_ROUTINES
#include <reshub.h>
extern "C" {
    #include <acpiioct.h>
    #include <initguid.h>
} // extern "C"
#include "acpiutil.hpp"
#include "ImxVideoCommon.h"

BOOLEAN GetAssignedResources(PCM_RESOURCE_LIST reslist);
//
// The device context performs the same job as
// a WDM device extension in the driver frameworks
//
typedef struct _DEVICE_CONTEXT
{   
    
} DEVICE_CONTEXT, *PDEVICE_CONTEXT;


struct iotarget_t
{
	HANDLE m_TargetHandle;
	OBJECT_ATTRIBUTES m_FileAttributes;
	IO_STATUS_BLOCK ioStatus;
	PZZWSTR m_FilePath;
	PFILE_OBJECT m_TargetFileObjectPtr;
	PDEVICE_OBJECT  m_TargetDevicePtr;
	ACCESS_MASK DesiredAccess = GENERIC_WRITE;
	// Have a default event and stuff
	KEVENT m_Event;
	IO_STATUS_BLOCK m_IoStatus;

	PIRP m_OngiongIrpPtr;

	iotarget_t() : m_FilePath(NULL), m_OngiongIrpPtr(NULL)
	{
		KeInitializeEvent(&m_Event, NotificationEvent, FALSE);
	}

	~iotarget_t()
	{
		Close();
	}

	iotarget_t *getTarget()
	{
		if (m_TargetHandle != NULL) {
			return this;
		}
		else {
			return NULL;
		}
	}

	NTSTATUS SetFile(PZZWSTR filePath)
	{
		NTSTATUS Status = STATUS_SUCCESS;

		if (m_FilePath != NULL) {
			ExFreePool(m_FilePath);
		}
		SIZE_T lenBytes = wcslen(filePath) * 2;
		m_FilePath = (PZZWSTR)ExAllocatePool(PagedPool, lenBytes + 2);
		if (m_FilePath == NULL) {
			Status = STATUS_INSUFFICIENT_RESOURCES;
		}
		else {
			RtlCopyMemory(m_FilePath, filePath, lenBytes);
			m_FilePath[lenBytes/2] = 0x0;
		}
		return Status;
	}

	NTSTATUS Open(PZZWSTR filePath)
	{
		NTSTATUS Status = STATUS_SUCCESS;

		Status = SetFile(filePath);
		if (NT_SUCCESS(Status)) {
			Status = Open();
		}
		return Status;
	}

	NTSTATUS Open()
	{
		NTSTATUS Status = STATUS_SUCCESS;
		UNICODE_STRING filename;

		if (m_FilePath == NULL) {
			Status = STATUS_INVALID_DEVICE_STATE;
		}
		if (NT_SUCCESS(Status)) {
			RtlInitUnicodeString(&filename, m_FilePath);
			InitializeObjectAttributes(&m_FileAttributes, &filename, OBJ_KERNEL_HANDLE, NULL, NULL);
			Status = ZwOpenFile(&m_TargetHandle, DesiredAccess, &m_FileAttributes, &ioStatus, 0, 0);
			if (NT_SUCCESS(Status)) {
				Status = ObReferenceObjectByHandle(m_TargetHandle, DesiredAccess, *IoFileObjectType, KernelMode, (PVOID*)&m_TargetFileObjectPtr, NULL);
				if (NT_SUCCESS(Status)) {
					m_TargetDevicePtr = IoGetRelatedDeviceObject(m_TargetFileObjectPtr);
					if (m_TargetDevicePtr == NULL) {
						Status = STATUS_NO_SUCH_DEVICE;
					}
				}
			}
			if (!NT_SUCCESS(Status)) {
				Close();
			}
		}
		return Status;
	}

	void Close()
	{
		if (m_TargetFileObjectPtr != NULL) {
			ObDereferenceObject(m_TargetFileObjectPtr);
			m_TargetFileObjectPtr = NULL;
		}
		if (m_TargetHandle != NULL) {
			ZwClose(m_TargetHandle);
			m_TargetHandle = NULL;
		}
	}

	void PnpStop()
	{
		Close();
		if (m_FilePath != NULL) {
			ExFreePool(m_FilePath);
			m_FilePath = NULL;
		}
	}

	NTSTATUS SendIrp(PIRP irpPtr, KEVENT *keEventPtr)
	{
		NTSTATUS Status = STATUS_SUCCESS;
		ASSERT(irpPtr != NULL);
		PIO_STACK_LOCATION irpSp = IoGetNextIrpStackLocation(irpPtr);
		if (irpSp == NULL) {
			Status = STATUS_INVALID_DEVICE_REQUEST;
		}
		irpSp->FileObject = m_TargetFileObjectPtr;
		if (keEventPtr != NULL) {
			KeClearEvent(keEventPtr);
		}
		// lock
		ASSERT(m_OngiongIrpPtr == NULL);
		m_OngiongIrpPtr = irpPtr;
		// unlock
		Status = IoCallDriver(m_TargetDevicePtr, irpPtr);
		if (Status == STATUS_PENDING){
			if (keEventPtr != NULL) {
				Status = KeWaitForSingleObject(keEventPtr, Executive, KernelMode, FALSE, NULL);
				// lock
				ASSERT(m_OngiongIrpPtr == irpPtr);
				m_OngiongIrpPtr = NULL;
				// unlock
			}
		}
		return Status;
	}

	BOOLEAN Cancel()
	{
		BOOLEAN notPending = true;
		if (m_OngiongIrpPtr != NULL) {
			notPending = IoCancelIrp(m_OngiongIrpPtr);
		}
		return notPending;
	}
};

class CCaptureDevice //: public IHardwareSink {
{
private:
    //
    // The AVStream device we're associated with.
    //
    PKSDEVICE m_Device;

    //
    // Number of pins with resources acquired.  This is used as a locking
    // mechanism for resource acquisition on the device.
    //
    LONG m_PinsWithResources;

    //
    // Since we don't have physical hardware, this provides the hardware
    // simulation.  m_HardwareSimulation provides the fake ISR, fake DPC,
    // etc...  m_ImageSynth provides RGB24 and UYVY image synthesis and
    // overlay in software.
    //
    // CHardwareSimulation *m_HardwareSimulation;
    // CImageSynthesizer *m_ImageSynth;

    //
    // The number of ISR's that have occurred since capture started.
    //
    ULONG m_InterruptTime;

    //
    // The last reading of mappings completed.
    //
    ULONG m_LastMappingsCompleted;

    //
    // The Dma adapter object we acquired through IoGetDmaAdapter() during
    // Pnp start.  This must be initialized with AVStream in order to perform
    // Dma directly into the capture buffers.
    //
    PADAPTER_OBJECT m_DmaAdapterObject;

    //
    // The number of map registers returned from IoGetDmaAdapter().
    //
    ULONG m_NumberOfMapRegisters;

    //
    // The video info header we're basing hardware settings on. The pin
    // provides this to us when acquiring resources and must guarantee its
    // stability until resources are released.
    //
    PKS_VIDEOINFOHEADER m_VideoInfoHeader;

    //
    // The video pixel format we're basing hardware settings on. Same way as for m_VideoInfoHeader the pin
    // provides this to us when acquiring resources and must guarantee its
    // stability until resources are released.
    //
    video_pixel_format_t m_PixelFmt;

    //
    // Cleanup():
    //
    // This is the free callback for the bagged capture device.  Not providing
    // one will call ExFreePool, which is not what we want for a constructed
    // C++ object.  This simply deletes the capture device.
    //
    static
    void
    Cleanup (
        IN CCaptureDevice *CapDevice
        )
    {
        delete CapDevice;
    }

    //
    // PnpStart():
    //
    // This is the Pnp start routine for our simulated hardware.  Note that
    // DispatchStart bridges to here in the context of the CCaptureDevice.
    //
    NTSTATUS
    PnpStart (
		// IN PIRP Irp,
        IN PCM_RESOURCE_LIST TranslatedResourceList,
        IN PCM_RESOURCE_LIST UntranslatedResourceList
        );

    //
    // PnpStop():
    //
    // This is the Pnp stop routine for our simulated hardware.  Note that
    // DispatchStop bridges to here in the context of the CCaptureDevice.
    //
    void
    PnpStop (
        );

	NTSTATUS InitResources(IN PCM_RESOURCE_LIST TranslatedResourceList);
	NTSTATUS GetAssigned_DsdAcpiResources();
	NTSTATUS GetAssigned_CrsAcpiResources(PCM_RESOURCE_LIST reslist);

public:

	iotarget_t m_Sensor;
	iotarget_t m_Csi;
	iotarget_t m_Mipi;

    //
    // CCaptureDevice():
    //
    // The capture device class constructor.  Since everything should have
    // been zero'ed by the new operator, don't bother setting anything to
    // zero or NULL.  Only initialize non-NULL, non-0 fields.
    //
    CCaptureDevice (
        IN PKSDEVICE Device
        ) :
        m_Device (Device)
    {}

    //
    // ~CCaptureDevice():
    //
    // The capture device destructor.
    //
    ~CCaptureDevice (
        )
    {}

    //
    // DispatchCreate():
    //
    // This is the Add Device dispatch for the capture device.  It creates
    // the CCaptureDevice and associates it with the device via the bag.
    //
    static
    NTSTATUS
    DispatchCreate (
        IN PKSDEVICE Device
        );
    //
    // DispatchPnpStart():
    //
    // This is the Pnp Start dispatch for the capture device.  It simply
    // bridges to PnpStart() in the context of the CCaptureDevice.
    //
    static
    NTSTATUS
    DispatchPnpStart (
        IN PKSDEVICE Device,
        IN PIRP Irp,
        IN PCM_RESOURCE_LIST TranslatedResourceList,
        IN PCM_RESOURCE_LIST UntranslatedResourceList
        )
    {
        return 
            (reinterpret_cast <CCaptureDevice *> (Device -> Context)) ->
            PnpStart (
                TranslatedResourceList,
                UntranslatedResourceList
                );
    }

    //
    // DispatchPnpStop():
    //
    // This is the Pnp stop dispatch for the capture device.  It simply
    // bridges to PnpStop() in the context of the CCaptureDevice.
    //
    static
    void
    DispatchPnpStop (
        IN PKSDEVICE Device,
        IN PIRP Irp
        )
    {
        return
            (reinterpret_cast <CCaptureDevice *> (Device -> Context)) ->
            PnpStop (
                );
    }

    //
    // AcquireHardwareResources():
    //
    // Called to acquire hardware resources for the device based on a given
    // video info header.  This will fail if another object has already
    // acquired hardware resources since we emulate a single capture
    // device.
    //
    NTSTATUS
    AcquireHardwareResources (
        IN PKS_VIDEOINFOHEADER VideoInfoHeader,
        IN video_pixel_format_t PixelFmt
        );

    //
    // ReleaseHardwareResources():
    //
    // Called to release hardware resources for the device.
    //
    void
    ReleaseHardwareResources (
        );

    //
    // Start():
    //
    // Called to start the hardware simulation.  This causes us to simulate
    // interrupts, simulate filling buffers with synthesized data, etc...
    //
    NTSTATUS
    Start (
        );

    //
    // Pause():
    //
    // Called to pause or unpause the hardware simulation.  This will be
    // indentical to a start or stop but it will not reset formats and 
    // counters.
    //
    NTSTATUS
    Pause (
        IN BOOLEAN Pausing
        );

    //
    // Stop():
    //
    // Called to stop the hardware simulation.  This causes interrupts to
    // stop issuing.  When this call returns, the "fake" hardware has
    // stopped accessing all s/g buffers, etc...
    //
    NTSTATUS
    Stop (
        );

    //
    // ProgramScatterGatherMappings():
    //
    // Called to program the hardware simulation's scatter / gather table.
    // This synchronizes with the "fake" ISR and hardware simulation via
    // a spinlock.
    //
    // ULONG
    // ProgramScatterGatherMappings (
    //     IN PKSSTREAM_POINTER Clone,
    //     IN PUCHAR *Buffer,
    //     IN PKSMAPPING Mappings,
    //     IN ULONG MappingsCount
    //     );

    //
    // QueryInterruptTime():
    //
    // Determine the frame number that this frame corresponds to.  
    //
    ULONG
    QueryInterruptTime (
        );
};
