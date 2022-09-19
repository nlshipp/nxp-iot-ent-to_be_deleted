/* Copyright (c) Microsoft Corporation.
 * Copyright 2022 NXP
   Licensed under the MIT License. */

#include "precomp.h"
#include "GcKmdGlobal.h"
#include "GcKmdBaseDisplay.h"
#include "GcKmdImx8mqDisplay.h"
#include "GcKmdImx8mpDisplay.h"
#include "GcKmdImx8mnDisplay.h"
#include "GcKmdUtil.h"

#include "GcKmdLogging.h"
#include "dispctrl.tmh"

extern "C"
{

NTSTATUS DllInitialize(
    _In_ PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    return STATUS_SUCCESS;
}

NTSTATUS DllUnload(
    void)
{
    return STATUS_SUCCESS;
}


GcKmdDriverMode GcKmdGlobal::s_DriverMode = FullDriver;
BOOLEAN         GcKmdGlobal::s_bDodUseHwVSync = TRUE;


void
DisplayStopController(
    GcKmDisplay    *pGcDisplay)
{
    GcKmBaseDisplay    *pDisplay = (GcKmBaseDisplay*)pGcDisplay;

    WPP_CLEANUP(NULL);

    if (pDisplay)
    {
        pDisplay->Stop();
        delete pDisplay;
    }
}

//
// helpers to get display instance based on the platform
//
enum PlatformName {
    iMX8MM,
    iMX8MN,
    iMX8MQ,
    iMX8MP,
    iMX8QXP,
    PLAT_NAME_UNDEF,
};

struct Platform {
    enum PlatformName PlatformName;
    WCHAR* DeviceId;
};

static const struct Platform PlatformList[] = {
    { iMX8MQ,  L"ACPI\\VEN_VERI&DEV_7000" },
    { iMX8MP,  L"ACPI\\VEN_VERI&DEV_7001" },
    { iMX8MM,  L"ACPI\\VEN_VERI&DEV_7002" },
    { iMX8MN,  L"ACPI\\VEN_VERI&DEV_7003" },
    { iMX8QXP, L"ACPI\\VEN_VERI&DEV_7004" },
};


static NTSTATUS GetDeviceHID(
    DXGKRNL_INTERFACE* pDxgkInterface,
    ULONG              BufferLength,
    WCHAR*             DeviceId,
    ULONG*             ResultLen)
{
    DXGK_DEVICE_INFO DeviceInfo;

    NTSTATUS Status = pDxgkInterface->DxgkCbGetDeviceInformation(
        pDxgkInterface->DeviceHandle,
        &DeviceInfo);
    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    return IoGetDeviceProperty(
        DeviceInfo.PhysicalDeviceObject,
        DevicePropertyHardwareID, BufferLength,
        DeviceId, ResultLen);
}

static enum PlatformName GetPlatform(DXGKRNL_INTERFACE* pDxgkInterface)
{
    WCHAR DeviceId[512];
    ULONG ResultLen;

    NTSTATUS Status = GetDeviceHID(pDxgkInterface, sizeof(DeviceId), DeviceId, &ResultLen);
    if (!NT_SUCCESS(Status))
    {
        return PLAT_NAME_UNDEF;
    }

    for (int i = 0; i < ARRAYSIZE(PlatformList); i++)
    {
        if (!wcscmp(DeviceId, PlatformList[i].DeviceId))
        {
            return PlatformList[i].PlatformName;
        }
    }

    return PLAT_NAME_UNDEF;
}

static GcKmBaseDisplay* GetDisplay(
    enum PlatformName Name)
{
    switch (Name)
    {
    case iMX8MM:
        //return new (NonPagedPoolNx, 'PSID') GcKmImx8mmDisplay();
        return nullptr;
    case iMX8MN:
        return new (NonPagedPoolNx, 'PSID') GcKmImx8mnDisplay();
    case iMX8MQ:
        return new (NonPagedPoolNx, 'PSID') GcKmImx8mqDisplay();
    case iMX8MP:
        return new (NonPagedPoolNx, 'PSID') GcKmImx8mpDisplay();
    default:
        return nullptr;
    }
}

NTSTATUS
DisplayStartController(
    GcKmdDriverMode     DriverMode,
    DXGKRNL_INTERFACE  *pDxgkInterface,
    UINT                LocalVidMemPhysicalBase,
    GcKmDisplay       **ppDisplay,
    ULONG              *pNumberOfVideoPresentSources,
    ULONG              *pNumberOfChildren)
{
    WPP_INIT_TRACING(NULL, NULL);

    *ppDisplay = nullptr;
    *pNumberOfVideoPresentSources = 0;
    *pNumberOfChildren = 0;

    GcKmdGlobal::s_DriverMode = DriverMode;

    if ((FullDriver != DriverMode) &&
        (DisplayOnly != DriverMode))
    {
        return STATUS_SUCCESS;
    }

    enum PlatformName PlatName = GetPlatform(pDxgkInterface);
    if (PlatName == PLAT_NAME_UNDEF)
    {
        return STATUS_INVALID_PARAMETER;
    }

    GcKmBaseDisplay    *pDisplay = GetDisplay(PlatName);
    if (nullptr == pDisplay)
    {
        return STATUS_NO_MEMORY;
    }

    NTSTATUS    status;

    status = pDisplay->Start(
                        pDxgkInterface,
                        LocalVidMemPhysicalBase,
                        pNumberOfVideoPresentSources,
                        pNumberOfChildren);
    if (!NT_SUCCESS(status))
    {
        DisplayStopController(pDisplay);

        return status;
    }

    *ppDisplay = pDisplay;

    return STATUS_SUCCESS;
}

}
