/****************************************************************************
* Copyright (c) Microsoft Corporation.
*
*    Licensed under the MIT License.
*    Licensed under the GPL License.
*
*****************************************************************************
*
*    Note: This software is released under dual MIT and GPL licenses. A
*    recipient may use this file under the terms of either the MIT license or
*    GPL License. If you wish to use only one license not the other, you can
*    indicate your decision by deleting one of the above license notices in your
*    version of this file.
*
*****************************************************************************/

#pragma once

#include "Gc7LCmdBufPatch.h"

//
// GC_7L_CMD_COMPUTE_BITBLT_N : N is the ROP value
//

const unsigned int  GC_7L_CMD_COMPUTE_BITBLT_1[] =
{
    0x08010e00, 0x00000000,
    0x08010e80, 0x30000000,
    0x080101f6, 0x00000000,
    0x08010380, 0x00000001,
    0x0801028b, 0x34000001,
    0x08010221, 0x00000808,
    0x08015310, 0x00000001,
    0x08010e03, 0x00004c27,
    0x08010e03, 0x00000010,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701,
    0x080102a4, 0x00000000,
    0x08010420, 0x00000000,
    0x08010427, 0x00000007,
    0x08010416, 0x00000000,
    0x08010409, 0x00000100,
    0x0801021f, 0x00000000,
    0x08010424, 0x0000000f,
    0x0801040a, 0xfe2a1000,     // INST_GPUVA
    0x08015580, 0x00000002,
    0x0801021a, 0x00000001,
    0x08010425, 0x0000000e,
    0x08010403, 0x00000002,
    0x08010402, 0x00001f01,
    0x08010404, 0x00000000,
    0x08010228, 0x00000000,
    0x08010412, 0x00000000,
    0x0804dc00, 0x00100000,     // DST_STORAGE_IMAGE
    0x00000800, 0x02000200,
    0x321019d2, 0xdeadbeef,
    0x0804dc04, 0xfe200000,     // SRC_STORAGE_IMAGE
    0x00000400, 0x01000100,
    0x321019d2, 0xdeadbeef,
    0x0801dc08, 0x00014000,     // UNIFORM_GPUVA
    0x08010240, 0x03000003,
    0x08010249, 0x00010000,
    0x08010247, 0x00000010,
    0x08030253, 0x0000000f,
    0x0000000f, 0x00000000,
    0x08010e27, 0x0000001f,
    0x08010e03, 0x00000c20,
    0x08030250, 0x00000000,     // DISPATCH_WG
    0x00000000, 0x00000000,
    0x0801024b, 0x00000000,
    0x0801024d, 0x00000000,
    0x0801024f, 0x00000000,
    0x0803dc0c, 0x00000001,     // DISPATCH_WG2
    0x00000001, 0x00000001,
    0x08010248, 0xbadabeeb,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701,
    0x08010e03, 0x00000c67,
    0x08010e03, 0x00000010,
    0x0801022c, 0x0000001f,
    0x08015311, 0x00000000,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701
};

const unsigned int  GC_7L_CMD_COMPUTE_BITBLT_2_3_4[] =
{
    0x08010e00, 0x00000000,
    0x08010e80, 0x30000000,
    0x080101f6, 0x00000000,
    0x08010380, 0x00000001,
    0x0801028b, 0x34000001,
    0x08010221, 0x00000808,
    0x08015310, 0x00000001,
    0x08010e03, 0x00004c27,
    0x08010e03, 0x00000010,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701,
    0x080102a4, 0x00000000,
    0x08010420, 0x00000000,
    0x08010427, 0x00000007,
    0x08010416, 0x00000000,
    0x08010409, 0x00000100,
    0x0801021f, 0x00000000,
    0x08010424, 0x00000012,
    0x0801040a, 0xfe2a1000,     // INST_GPUVA
    0x08015580, 0x00000002,
    0x0801021a, 0x00000001,
    0x08010425, 0x00000011,
    0x08010403, 0x00000003,
    0x08010402, 0x00001f01,
    0x08010404, 0x00000000,
    0x08010228, 0x00000000,
    0x08010412, 0x00000000,
    0x0804dc00, 0x00100000,     // DST_STORAGE_IMAGE
    0x00000800, 0x02000200,
    0x321019d2, 0xdeadbeef,
    0x0804dc04, 0xfe200000,     // SRC_STORAGE_IMAGE
    0x00000400, 0x01000100,
    0x321019d2, 0xdeadbeef,
    0x0801dc08, 0x00014000,     // UNIFORM_GPUVA
    0x08010240, 0x03000003,
    0x08010249, 0x00010000,
    0x08010247, 0x00000010,
    0x08030253, 0x0000000f,
    0x0000000f, 0x00000000,
    0x08010e27, 0x0000001f,
    0x08010e03, 0x00000c20,
    0x08030250, 0x00000000,     // DISPATCH_WG
    0x00000000, 0x00000000,
    0x0801024b, 0x00000000,
    0x0801024d, 0x00000000,
    0x0801024f, 0x00000000,
    0x0803dc0c, 0x00000001,     // DISPATCH_WG2
    0x00000001, 0x00000001,
    0x08010248, 0xbadabeeb,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701,
    0x08010e03, 0x00000c67,
    0x08010e03, 0x00000010,
    0x0801022c, 0x0000001f,
    0x08015311, 0x00000000,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701
};

const unsigned int  GC_7L_CMD_COMPUTE_BITBLT_1_A8[] =
{
    0x08010e00, 0x00000000,
    0x08010e80, 0x30000000,
    0x080101f6, 0x00000000,
    0x08010380, 0x00000001,
    0x0801028b, 0x34000001,
    0x08010221, 0x00000808,
    0x08015310, 0x00000001,
    0x08010e03, 0x00004c27,
    0x08010e03, 0x00000010,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701,
    0x080102a4, 0x00000000,
    0x08010420, 0x00000000,
    0x08010416, 0x00000000,
    0x08010409, 0x00000100,
    0x0801021f, 0x00000000,
    0x08010424, 0x0000000f,
    0x0801040a, 0xfc0f5000,     // INST_GPUVA
    0x08015580, 0x00000002,
    0x0801021a, 0x00000001,
    0x08010425, 0x0000000e,
    0x08010403, 0x00000002,
    0x08010402, 0x00001f01,
    0x08010404, 0x00000000,
    0x08010228, 0x00000000,
    0x08010412, 0x00000000,
    0x0804dc00, 0x00020000,     // DST_STORAGE_IMAGE
    0x00000200, 0x02000200,
    0x544059d0, 0xdeadbeef,
    0x0804dc04, 0xfc0d0000,     // SRC_STORAGE_IMAGE
    0x00000100, 0x01000100,
    0x544059d0, 0xdeadbeef,
    0x0801dc08, 0x00013000,     // UNIFORM_GPUVA
    0x08010240, 0x03000003,
    0x08010249, 0x00000000,
    0x08010247, 0x00000020,
    0x08030253, 0x0000000f,
    0x0000000f, 0x00000000,
    0x08010e27, 0x00000026,
    0x08010e03, 0x00000c20,
    0x08030250, 0x00000000,     // DISPATCH_WG
    0x00000000, 0x00000000,
    0x0801024b, 0x00000000,
    0x0801024d, 0x00000000,
    0x0801024f, 0x00000000,
    0x0803dc0c, 0x00000001,     // DISPATCH_WG2
    0x00000001, 0x00000001,
    0x08010248, 0xbadabeeb,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701,
    0x08010e03, 0x00000c67,
    0x08010e03, 0x00000010,
    0x0801022c, 0x0000001f,
    0x08015311, 0x00000000,
    0x08010e02, 0x00000701,
    0x48000000, 0x00000701
};


GC_7L_PATCH_ENTRY  GC_7L_CMD_BITBLT_PATCH_TABLE[]
{
    {  37, GC7L_PATCH_INST_GPUVA        },
    {  55, GC7L_PATCH_DST_STORAGE_IMAGE },
    {  61, GC7L_PATCH_SRC_STORAGE_IMAGE },
    {  67, GC7L_PATCH_UNIFORM_GPUVA     },
    {  83, GC7L_PATCH_DISPATCH_WG       },
    {  93, GC7L_PATCH_DISPATCH_WG2      }
};

GC_7L_PATCH_ENTRY  GC_7L_CMD_BITBLT_PATCH_TABLE_A8[]
{
    {  35, GC7L_PATCH_INST_GPUVA        },
    {  53, GC7L_PATCH_DST_STORAGE_IMAGE },
    {  59, GC7L_PATCH_SRC_STORAGE_IMAGE },
    {  65, GC7L_PATCH_UNIFORM_GPUVA     },
    {  81, GC7L_PATCH_DISPATCH_WG       },
    {  91, GC7L_PATCH_DISPATCH_WG2      }
};


const unsigned int GC_7L_BITBLT_SHADER_SRCCOPY[] =
{
    0x02001032, 0x00202c04, 0xa0000450, 0x00000007,
    0x020010b1, 0x00200c04, 0x81540040, 0x76ffdff8,
    0x0200158f, 0x2aa00c04, 0xa00000c0, 0x7800000f,
    0x04001009, 0x00000004, 0x40000000, 0x002a8008,
    0x00000156, 0x2a800804, 0x50000040, 0x00000407,
    0x04001032, 0x00202c04, 0xa0000650, 0x00000007,
    0x040010b1, 0x15600c04, 0x81fe0040, 0x76ffdff8,
    0x0400158f, 0x3fe00c04, 0xa00000c0, 0x7800000f,
    0x00000156, 0x3fc00804, 0x50000040, 0x00000787,
    0x06001032, 0x00002c04, 0x20000c50, 0x00000007,
    0x06011032, 0x00002c04, 0x60000050, 0x00000007,
    0x0181102e, 0x3f800804, 0x40000000, 0x00000000,
    0x07801001, 0x11000804, 0x40000000, 0x00390018,
    0x07811039, 0x39201c00, 0x80a90050, 0x00000000,
    0x0780083a, 0x39200c00, 0x81fd0050, 0x00390018,
    0x00000000, 0x00000000, 0x00000000, 0x00000000
};

const unsigned int GC_7L_BITBLT_SHADER_SRCINVERT[] =
{
    0x02001032, 0x00202c04, 0xa0000450, 0x00000007,
    0x020010b1, 0x00200c04, 0x81540040, 0x76ffdff8,
    0x0200158f, 0x2aa00c04, 0xa00000c0, 0x7800000f,
    0x04001009, 0x00000004, 0x40000000, 0x002a8008,
    0x00000156, 0x2a800804, 0x50000040, 0x00000407,
    0x04001032, 0x00202c04, 0xa0000650, 0x00000007,
    0x040010b1, 0x15600c04, 0x81fe0040, 0x76ffdff8,
    0x0400158f, 0x3fe00c04, 0xa00000c0, 0x7800000f,
    0x00000156, 0x3fc00804, 0x50000040, 0x00000907,
    0x06001032, 0x00002c04, 0x20000c50, 0x00000007,
    0x01811032, 0x00002c04, 0x60000050, 0x00000007,
    0x0600102e, 0x3a800804, 0x40000000, 0x00000000,
    0x06001001, 0x10000804, 0x40000000, 0x003a8008,
    0x01801001, 0x15000804, 0x40000000, 0x00150018,
    0x07811039, 0x39200c00, 0x80a90050, 0x00000000,
    0x07821039, 0x39201c00, 0x81fd0050, 0x00000000,
    0x0781101e, 0x39202804, 0x80010000, 0x00390018,
    0x0780083a, 0x39200c00, 0x80a90050, 0x00390018,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000
};

const unsigned int GC_7L_BITBLT_SHADER_SRCAND[] =
{
    0x02001032, 0x00202c04, 0xa0000450, 0x00000007,
    0x020010b1, 0x00200c04, 0x81540040, 0x76ffdff8,
    0x0200158f, 0x2aa00c04, 0xa00000c0, 0x7800000f,
    0x04001009, 0x00000004, 0x40000000, 0x002a8008,
    0x00000156, 0x2a800804, 0x50000040, 0x00000407,
    0x04001032, 0x00202c04, 0xa0000650, 0x00000007,
    0x040010b1, 0x15600c04, 0x81fe0040, 0x76ffdff8,
    0x0400158f, 0x3fe00c04, 0xa00000c0, 0x7800000f,
    0x00000156, 0x3fc00804, 0x50000040, 0x00000907,
    0x06001032, 0x00002c04, 0x20000c50, 0x00000007,
    0x01811032, 0x00002c04, 0x60000050, 0x00000007,
    0x0600102e, 0x3a800804, 0x40000000, 0x00000000,
    0x06001001, 0x10000804, 0x40000000, 0x003a8008,
    0x01801001, 0x15000804, 0x40000000, 0x00150018,
    0x07811039, 0x39200c00, 0x80a90050, 0x00000000,
    0x07821039, 0x39201c00, 0x81fd0050, 0x00000000,
    0x0781101d, 0x39202804, 0x80010000, 0x00390018,
    0x0780083a, 0x39200c00, 0x80a90050, 0x00390018,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000
};

const unsigned int GC_7L_BITBLT_SHADER_SRCOR[] =
{
    0x02001032, 0x00202c04, 0xa0000450, 0x00000007,
    0x020010b1, 0x00200c04, 0x81540040, 0x76ffdff8,
    0x0200158f, 0x2aa00c04, 0xa00000c0, 0x7800000f,
    0x04001009, 0x00000004, 0x40000000, 0x002a8008,
    0x00000156, 0x2a800804, 0x50000040, 0x00000407,
    0x04001032, 0x00202c04, 0xa0000650, 0x00000007,
    0x040010b1, 0x15600c04, 0x81fe0040, 0x76ffdff8,
    0x0400158f, 0x3fe00c04, 0xa00000c0, 0x7800000f,
    0x00000156, 0x3fc00804, 0x50000040, 0x00000907,
    0x06001032, 0x00002c04, 0x20000c50, 0x00000007,
    0x01811032, 0x00002c04, 0x60000050, 0x00000007,
    0x0600102e, 0x3a800804, 0x40000000, 0x00000000,
    0x06001001, 0x10000804, 0x40000000, 0x003a8008,
    0x01801001, 0x15000804, 0x40000000, 0x00150018,
    0x07811039, 0x39200c00, 0x80a90050, 0x00000000,
    0x07821039, 0x39201c00, 0x81fd0050, 0x00000000,
    0x0781101c, 0x39202804, 0x80010000, 0x00390018,
    0x0780083a, 0x39200c00, 0x80a90050, 0x00390018,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000
};

const unsigned int GC_7L_BITBLT_SHADER_SRCCOPY_A8[] =
{
    0x02001032, 0x00202c04, 0xa0000450, 0x00000007,
    0x020010b1, 0x00200c04, 0x81540040, 0x76ffdff8,
    0x0200158f, 0x2aa00c04, 0xa00000c0, 0x7800000f,
    0x04001009, 0x00000004, 0x40000000, 0x002a8008,
    0x00000156, 0x2a800804, 0x50000040, 0x00000407,
    0x04001032, 0x00202c04, 0xa0000650, 0x00000007,
    0x040010b1, 0x15600c04, 0x81fe0040, 0x76ffdff8,
    0x0400158f, 0x3fe00c04, 0xa00000c0, 0x7800000f,
    0x00000156, 0x3fc00804, 0x50000040, 0x00000787,
    0x06001032, 0x00002c04, 0x20000c50, 0x00000007,
    0x06011032, 0x00002c04, 0x60000050, 0x00000007,
    0x0181102e, 0x3f800804, 0x40000000, 0x00000000,
    0x07801001, 0x11000804, 0x40000000, 0x00390018,
    0x07811039, 0x39201c00, 0x80a90050, 0x00000000,
    0x0780083a, 0x39200c00, 0x81fd0050, 0x00390018,
    0x00000000, 0x00000000, 0x00000000, 0x00000000
};

