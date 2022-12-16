/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2017-2020 NXP
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 * Header file for the RPC implementation.
 */

#ifndef SC_RPC_H
#define SC_RPC_H

/* Includes */

#include <svc/types.h>
#include <svc/ipc.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* Defines */

/*!
 * @name SCFW API version
 */
/** @{ */
#define SCFW_API_VERSION_MAJOR  1U
#define SCFW_API_VERSION_MINOR  30U
/** @} */

/*! RPC version */
#define SC_RPC_VERSION          1U

/*!
 * @name Defines for service types
 */
/** @{ */
#define SC_RPC_SVC_UNKNOWN      0U
#define SC_RPC_SVC_RETURN       1U
#define SC_RPC_SVC_PM           2U
#define SC_RPC_SVC_RM           3U
#define SC_RPC_SVC_TIMER        5U
#define SC_RPC_SVC_PAD          6U
#define SC_RPC_SVC_MISC         7U
#define SC_RPC_SVC_IRQ          8U
#define SC_RPC_SVC_SECO         9U
#define SC_RPC_SVC_ABORT        10U
/** @} */

/* Internal Defines */

#define SC_RPC_MAX_MSG          8U

#define RPC_VER(MESG)           ((MESG)->version)
#define RPC_SIZE(MESG)          ((MESG)->size)
#define RPC_SVC(MESG)           ((MESG)->svc)
#define RPC_FUNC(MESG)          ((MESG)->func)
#define RPC_R8(MESG)            ((MESG)->func)
#define RPC_I64(MESG, IDX)      ((I64(RPC_U32((MESG), (IDX))) << 32ULL) \
                                    | I64(RPC_U32((MESG), (IDX) + 4U)))
#define RPC_I32(MESG, IDX)      ((MESG)->DATA.i32[(IDX) / 4U])
#define RPC_I16(MESG, IDX)      ((MESG)->DATA.i16[(IDX) / 2U])
#define RPC_I8(MESG, IDX)       ((MESG)->DATA.i8[(IDX)])
#define RPC_U64(MESG, IDX)      ((U64(RPC_U32((MESG), (IDX))) << 32ULL) \
                                    | U64(RPC_U32((MESG), (IDX) + 4U)))
#define RPC_U32(MESG, IDX)      ((MESG)->DATA.u32[(IDX) / 4U])
#define RPC_U16(MESG, IDX)      ((MESG)->DATA.u16[(IDX) / 2U])
#define RPC_U8(MESG, IDX)       ((MESG)->DATA.u8[(IDX)])

#define SC_RPC_ASYNC_STATE_RD_START      0U
#define SC_RPC_ASYNC_STATE_RD_ACTIVE     1U
#define SC_RPC_ASYNC_STATE_RD_DONE       2U
#define SC_RPC_ASYNC_STATE_WR_START      3U
#define SC_RPC_ASYNC_STATE_WR_ACTIVE     4U
#define SC_RPC_ASYNC_STATE_WR_DONE       5U

/* SC -> Client general-purpose MU IRQs */
#define SC_RPC_MU_GIR_SVC       0x1U
#define SC_RPC_MU_GIR_WAKE      0x2U
#define SC_RPC_MU_GIR_BOOT      0x4U
#define SC_RPC_MU_GIR_DBG       0x8U

/* Client -> SC general-purpose MU IRQs */
#define SC_RPC_MU_GIR_RST       0x1U

#define I8(X)       ((INT8) (X))
#define I16(X)      ((INT16) (X))
#define I32(X)      ((INT32) (X))
#define I64(X)      ((INT64) (X))
#define U8(X)       ((UINT8) (X))
#define U16(X)      ((UINT16) (X))
#define U32(X)      ((UINT32) (X))
#define U64(X)      ((UINT64) (X))
#define F32(X)      ((float) (X))

#define PTR_I8(X)   ((INT8 *) (X))
#define PTR_I16(X)  ((INT16 *) (X))
#define PTR_I32(X)  ((INT32 *) (X))
#define PTR_I64(X)  ((INT64 *) (X))
#define PTR_U8(X)   ((UINT8 *) (X))
#define PTR_U16(X)  ((UINT16 *) (X))
#define PTR_U32(X)  ((UINT32 *) (X))
#define PTR_U64(X)  ((UINT64 *) (X))

#define U2B(X)      (((X) != 0U) ? SC_TRUE : SC_FALSE)
#define U2B32(X)    (((X) != 0UL) ? SC_TRUE : SC_FALSE)
#define B2U8(X)     (((X) != SC_FALSE) ? U8(0x01U) : U8(0x00U))
#define B2U16(X)    (((X) != SC_FALSE) ? U16(0x01U) : U16(0x00U))
#define B2U32(X)    (((X) != SC_FALSE) ? U32(0x01U) : U32(0x00U))

/* Types */

typedef struct
{
    UINT8 version;
    UINT8 size;
    UINT8 svc;
    UINT8 func;
    union
    {
        INT32 i32[(SC_RPC_MAX_MSG - 1U)];
        INT16 i16[(SC_RPC_MAX_MSG - 1U) * 2U];
        INT8 i8[(SC_RPC_MAX_MSG - 1U) * 4U];
        UINT32 u32[(SC_RPC_MAX_MSG - 1U)];
        UINT16 u16[(SC_RPC_MAX_MSG - 1U) * 2U];
        UINT8 u8[(SC_RPC_MAX_MSG - 1U) * 4U];
    } DATA;
} sc_rpc_msg_t;

typedef UINT8 sc_rpc_async_state_t;

typedef struct
{
    sc_rpc_async_state_t state;
    UINT8 wordIdx;
    sc_rpc_msg_t msg;
    UINT32 timeStamp;
} sc_rpc_async_msg_t;

/* Functions */

/*!
 * This is an internal function to send an RPC message over an IPC
 * channel. It is called by client-side SCFW API function shims.
 *
 * @param[in]     ipc         IPC handle
 * @param[in,out] msg         handle to a message
 * @param[in]     no_resp     response flag
 *
 * If \a no_resp is SC_FALSE then this function waits for a response
 * and returns the result in \a msg.
 */
void sc_call_rpc(sc_ipc_t ipc, sc_rpc_msg_t *msg, sc_bool_t no_resp);

/*!
 * This is an internal function to dispatch an RPC call that has
 * arrived via IPC over an MU. It is called by server-side SCFW.
 *
 * @param[in]     mu          MU message arrived on
 * @param[in,out] msg         handle to a message
 *
 * The function result is returned in \a msg.
 */
void sc_rpc_dispatch(sc_rsrc_t mu, sc_rpc_msg_t *msg);

/*!
 * This function translates an RPC message and forwards on to the
 * normal RPC API.  It is used only by hypervisors.
 *
 * @param[in]     ipc         IPC handle
 * @param[in,out] msg         handle to a message
 *
 * This function decodes a message, calls macros to translate the
 * resources, pads, addresses, partitions, memory regions, etc. and
 * then forwards on to the hypervisors SCFW API.Return results are
 * translated back abd placed back into the message to be returned
 * to the original API.
 */
void sc_rpc_xlate(sc_ipc_t ipc, sc_rpc_msg_t *msg);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif /* SC_RPC_H */

