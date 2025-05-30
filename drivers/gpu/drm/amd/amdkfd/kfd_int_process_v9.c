// SPDX-License-Identifier: GPL-2.0 OR MIT
/*
 * Copyright 2016-2022 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "kfd_priv.h"
#include "kfd_events.h"
#include "kfd_debug.h"
#include "soc15_int.h"
#include "kfd_device_queue_manager.h"
#include "kfd_smi_events.h"
#include "amdgpu_ras.h"

/*
 * GFX9 SQ Interrupts
 *
 * There are 3 encoding types of interrupts sourced from SQ sent as a 44-bit
 * packet to the Interrupt Handler:
 * Auto - Generated by the SQG (various cmd overflows, timestamps etc)
 * Wave - Generated by S_SENDMSG through a shader program
 * Error - HW generated errors (Illegal instructions, Memviols, EDC etc)
 *
 * The 44-bit packet is mapped as {context_id1[7:0],context_id0[31:0]} plus
 * 4-bits for VMID (SOC15_VMID_FROM_IH_ENTRY) as such:
 *
 * - context_id0[27:26]
 * Encoding type (0 = Auto, 1 = Wave, 2 = Error)
 *
 * - context_id0[13]
 * PRIV bit indicates that Wave S_SEND or error occurred within trap
 *
 * - {context_id1[7:0],context_id0[31:28],context_id0[11:0]}
 * 24-bit data with the following layout per encoding type:
 * Auto - only context_id0[8:0] is used, which reports various interrupts
 * generated by SQG.  The rest is 0.
 * Wave - user data sent from m0 via S_SENDMSG
 * Error - Error type (context_id1[7:4]), Error Details (rest of bits)
 *
 * The other context_id bits show coordinates (SE/SH/CU/SIMD/WAVE) for wave
 * S_SENDMSG and Errors.  These are 0 for Auto.
 */

enum SQ_INTERRUPT_WORD_ENCODING {
	SQ_INTERRUPT_WORD_ENCODING_AUTO = 0x0,
	SQ_INTERRUPT_WORD_ENCODING_INST,
	SQ_INTERRUPT_WORD_ENCODING_ERROR,
};

enum SQ_INTERRUPT_ERROR_TYPE {
	SQ_INTERRUPT_ERROR_TYPE_EDC_FUE = 0x0,
	SQ_INTERRUPT_ERROR_TYPE_ILLEGAL_INST,
	SQ_INTERRUPT_ERROR_TYPE_MEMVIOL,
	SQ_INTERRUPT_ERROR_TYPE_EDC_FED,
};

/* SQ_INTERRUPT_WORD_AUTO_CTXID */
#define SQ_INTERRUPT_WORD_AUTO_CTXID__THREAD_TRACE__SHIFT 0
#define SQ_INTERRUPT_WORD_AUTO_CTXID__WLT__SHIFT 1
#define SQ_INTERRUPT_WORD_AUTO_CTXID__THREAD_TRACE_BUF_FULL__SHIFT 2
#define SQ_INTERRUPT_WORD_AUTO_CTXID__REG_TIMESTAMP__SHIFT 3
#define SQ_INTERRUPT_WORD_AUTO_CTXID__CMD_TIMESTAMP__SHIFT 4
#define SQ_INTERRUPT_WORD_AUTO_CTXID__HOST_CMD_OVERFLOW__SHIFT 5
#define SQ_INTERRUPT_WORD_AUTO_CTXID__HOST_REG_OVERFLOW__SHIFT 6
#define SQ_INTERRUPT_WORD_AUTO_CTXID__IMMED_OVERFLOW__SHIFT 7
#define SQ_INTERRUPT_WORD_AUTO_CTXID__THREAD_TRACE_UTC_ERROR__SHIFT 8
#define SQ_INTERRUPT_WORD_AUTO_CTXID__SE_ID__SHIFT 24
#define SQ_INTERRUPT_WORD_AUTO_CTXID__ENCODING__SHIFT 26

#define SQ_INTERRUPT_WORD_AUTO_CTXID__THREAD_TRACE_MASK 0x00000001
#define SQ_INTERRUPT_WORD_AUTO_CTXID__WLT_MASK 0x00000002
#define SQ_INTERRUPT_WORD_AUTO_CTXID__THREAD_TRACE_BUF_FULL_MASK 0x00000004
#define SQ_INTERRUPT_WORD_AUTO_CTXID__REG_TIMESTAMP_MASK 0x00000008
#define SQ_INTERRUPT_WORD_AUTO_CTXID__CMD_TIMESTAMP_MASK 0x00000010
#define SQ_INTERRUPT_WORD_AUTO_CTXID__HOST_CMD_OVERFLOW_MASK 0x00000020
#define SQ_INTERRUPT_WORD_AUTO_CTXID__HOST_REG_OVERFLOW_MASK 0x00000040
#define SQ_INTERRUPT_WORD_AUTO_CTXID__IMMED_OVERFLOW_MASK 0x00000080
#define SQ_INTERRUPT_WORD_AUTO_CTXID__THREAD_TRACE_UTC_ERROR_MASK 0x00000100
#define SQ_INTERRUPT_WORD_AUTO_CTXID__SE_ID_MASK 0x03000000
#define SQ_INTERRUPT_WORD_AUTO_CTXID__ENCODING_MASK 0x0c000000

/* SQ_INTERRUPT_WORD_WAVE_CTXID */
#define SQ_INTERRUPT_WORD_WAVE_CTXID__DATA__SHIFT 0
#define SQ_INTERRUPT_WORD_WAVE_CTXID__SH_ID__SHIFT 12
#define SQ_INTERRUPT_WORD_WAVE_CTXID__PRIV__SHIFT 13
#define SQ_INTERRUPT_WORD_WAVE_CTXID__WAVE_ID__SHIFT 14
#define SQ_INTERRUPT_WORD_WAVE_CTXID__SIMD_ID__SHIFT 18
#define SQ_INTERRUPT_WORD_WAVE_CTXID__CU_ID__SHIFT 20
#define SQ_INTERRUPT_WORD_WAVE_CTXID__SE_ID__SHIFT 24
#define SQ_INTERRUPT_WORD_WAVE_CTXID__ENCODING__SHIFT 26

#define SQ_INTERRUPT_WORD_WAVE_CTXID__DATA_MASK 0x00000fff
#define SQ_INTERRUPT_WORD_WAVE_CTXID__SH_ID_MASK 0x00001000
#define SQ_INTERRUPT_WORD_WAVE_CTXID__PRIV_MASK 0x00002000
#define SQ_INTERRUPT_WORD_WAVE_CTXID__WAVE_ID_MASK 0x0003c000
#define SQ_INTERRUPT_WORD_WAVE_CTXID__SIMD_ID_MASK 0x000c0000
#define SQ_INTERRUPT_WORD_WAVE_CTXID__CU_ID_MASK 0x00f00000
#define SQ_INTERRUPT_WORD_WAVE_CTXID__SE_ID_MASK 0x03000000
#define SQ_INTERRUPT_WORD_WAVE_CTXID__ENCODING_MASK 0x0c000000

/* GFX9 SQ interrupt 24-bit data from context_id<0,1> */
#define KFD_CONTEXT_ID_GET_SQ_INT_DATA(ctx0, ctx1)                             \
	((ctx0 & 0xfff) | ((ctx0 >> 16) & 0xf000) | ((ctx1 << 16) & 0xff0000))

#define KFD_SQ_INT_DATA__ERR_TYPE_MASK 0xF00000
#define KFD_SQ_INT_DATA__ERR_TYPE__SHIFT 20

/*
 * The debugger will send user data(m0) with PRIV=1 to indicate it requires
 * notification from the KFD with the following queue id (DOORBELL_ID) and
 * trap code (TRAP_CODE).
 */
#define KFD_INT_DATA_DEBUG_DOORBELL_MASK	0x0003ff
#define KFD_INT_DATA_DEBUG_TRAP_CODE_SHIFT	10
#define KFD_INT_DATA_DEBUG_TRAP_CODE_MASK	0x07fc00
#define KFD_DEBUG_DOORBELL_ID(sq_int_data)	((sq_int_data) &	\
				KFD_INT_DATA_DEBUG_DOORBELL_MASK)
#define KFD_DEBUG_TRAP_CODE(sq_int_data)	(((sq_int_data) &	\
				KFD_INT_DATA_DEBUG_TRAP_CODE_MASK)	\
				>> KFD_INT_DATA_DEBUG_TRAP_CODE_SHIFT)
#define KFD_DEBUG_CP_BAD_OP_ECODE_MASK		0x3fffc00
#define KFD_DEBUG_CP_BAD_OP_ECODE_SHIFT		10
#define KFD_DEBUG_CP_BAD_OP_ECODE(ctxid0)	(((ctxid0) &		\
				KFD_DEBUG_CP_BAD_OP_ECODE_MASK)		\
				>> KFD_DEBUG_CP_BAD_OP_ECODE_SHIFT)

static void event_interrupt_poison_consumption_v9(struct kfd_node *dev,
				uint16_t pasid, uint16_t client_id)
{
	enum amdgpu_ras_block block = 0;
	uint32_t reset = 0;
	struct kfd_process *p = kfd_lookup_process_by_pasid(pasid, NULL);
	enum ras_event_type type = RAS_EVENT_TYPE_POISON_CONSUMPTION;
	u64 event_id;
	int old_poison, ret;

	if (!p)
		return;

	/* all queues of a process will be unmapped in one time */
	old_poison = atomic_cmpxchg(&p->poison, 0, 1);
	kfd_unref_process(p);
	if (old_poison)
		return;

	switch (client_id) {
	case SOC15_IH_CLIENTID_SE0SH:
	case SOC15_IH_CLIENTID_SE1SH:
	case SOC15_IH_CLIENTID_SE2SH:
	case SOC15_IH_CLIENTID_SE3SH:
	case SOC15_IH_CLIENTID_UTCL2:
		block = AMDGPU_RAS_BLOCK__GFX;
		if (amdgpu_ip_version(dev->adev, GC_HWIP, 0) == IP_VERSION(9, 4, 3)) {
			/* driver mode-2 for gfx poison is only supported by
			 * pmfw 0x00557300 and onwards */
			if (dev->adev->pm.fw_version < 0x00557300)
				reset = AMDGPU_RAS_GPU_RESET_MODE1_RESET;
			else
				reset = AMDGPU_RAS_GPU_RESET_MODE2_RESET;
		} else if (amdgpu_ip_version(dev->adev, GC_HWIP, 0) == IP_VERSION(9, 4, 4)) {
			/* driver mode-2 for gfx poison is only supported by
			 * pmfw 0x05550C00 and onwards */
			if (dev->adev->pm.fw_version < 0x05550C00)
				reset = AMDGPU_RAS_GPU_RESET_MODE1_RESET;
			else
				reset = AMDGPU_RAS_GPU_RESET_MODE2_RESET;
		} else {
			reset = AMDGPU_RAS_GPU_RESET_MODE2_RESET;
		}
		amdgpu_ras_set_err_poison(dev->adev, AMDGPU_RAS_BLOCK__GFX);
		break;
	case SOC15_IH_CLIENTID_VMC:
	case SOC15_IH_CLIENTID_VMC1:
		block = AMDGPU_RAS_BLOCK__MMHUB;
		reset = AMDGPU_RAS_GPU_RESET_MODE1_RESET;
		break;
	case SOC15_IH_CLIENTID_SDMA0:
	case SOC15_IH_CLIENTID_SDMA1:
	case SOC15_IH_CLIENTID_SDMA2:
	case SOC15_IH_CLIENTID_SDMA3:
	case SOC15_IH_CLIENTID_SDMA4:
		block = AMDGPU_RAS_BLOCK__SDMA;
		if (amdgpu_ip_version(dev->adev, SDMA0_HWIP, 0) == IP_VERSION(4, 4, 2)) {
			/* driver mode-2 for gfx poison is only supported by
			 * pmfw 0x00557300 and onwards */
			if (dev->adev->pm.fw_version < 0x00557300)
				reset = AMDGPU_RAS_GPU_RESET_MODE1_RESET;
			else
				reset = AMDGPU_RAS_GPU_RESET_MODE2_RESET;
		} else if (amdgpu_ip_version(dev->adev, SDMA0_HWIP, 0) == IP_VERSION(4, 4, 5)) {
			/* driver mode-2 for gfx poison is only supported by
			 * pmfw 0x05550C00 and onwards */
			if (dev->adev->pm.fw_version < 0x05550C00)
				reset = AMDGPU_RAS_GPU_RESET_MODE1_RESET;
			else
				reset = AMDGPU_RAS_GPU_RESET_MODE2_RESET;
		} else {
			reset = AMDGPU_RAS_GPU_RESET_MODE2_RESET;
		}
		amdgpu_ras_set_err_poison(dev->adev, AMDGPU_RAS_BLOCK__SDMA);
		break;
	default:
		dev_warn(dev->adev->dev,
			 "client %d does not support poison consumption\n", client_id);
		return;
	}

	ret = amdgpu_ras_mark_ras_event(dev->adev, type);
	if (ret)
		return;

	kfd_signal_poison_consumed_event(dev, pasid);

	event_id = amdgpu_ras_acquire_event_id(dev->adev, type);

	RAS_EVENT_LOG(dev->adev, event_id,
		      "poison is consumed by client %d, kick off gpu reset flow\n", client_id);

	amdgpu_amdkfd_ras_pasid_poison_consumption_handler(dev->adev,
		block, pasid, NULL, NULL, reset);
}

static bool context_id_expected(struct kfd_dev *dev)
{
	switch (KFD_GC_VERSION(dev)) {
	case IP_VERSION(9, 0, 1):
		return dev->mec_fw_version >= 0x817a;
	case IP_VERSION(9, 1, 0):
	case IP_VERSION(9, 2, 1):
	case IP_VERSION(9, 2, 2):
	case IP_VERSION(9, 3, 0):
	case IP_VERSION(9, 4, 0):
		return dev->mec_fw_version >= 0x17a;
	default:
		/* Other GFXv9 and later GPUs always sent valid context IDs
		 * on legitimate events
		 */
		return KFD_GC_VERSION(dev) >= IP_VERSION(9, 4, 1);
	}
}

static bool event_interrupt_isr_v9(struct kfd_node *dev,
					const uint32_t *ih_ring_entry,
					uint32_t *patched_ihre,
					bool *patched_flag)
{
	uint16_t source_id, client_id, pasid, vmid;
	const uint32_t *data = ih_ring_entry;

	source_id = SOC15_SOURCE_ID_FROM_IH_ENTRY(ih_ring_entry);
	client_id = SOC15_CLIENT_ID_FROM_IH_ENTRY(ih_ring_entry);

	/* Only handle interrupts from KFD VMIDs */
	vmid = SOC15_VMID_FROM_IH_ENTRY(ih_ring_entry);
	if (!KFD_IRQ_IS_FENCE(client_id, source_id) &&
	   (vmid < dev->vm_info.first_vmid_kfd ||
	    vmid > dev->vm_info.last_vmid_kfd))
		return false;

	pasid = SOC15_PASID_FROM_IH_ENTRY(ih_ring_entry);

	/* Only handle clients we care about */
	if (client_id != SOC15_IH_CLIENTID_GRBM_CP &&
	    client_id != SOC15_IH_CLIENTID_SDMA0 &&
	    client_id != SOC15_IH_CLIENTID_SDMA1 &&
	    client_id != SOC15_IH_CLIENTID_SDMA2 &&
	    client_id != SOC15_IH_CLIENTID_SDMA3 &&
	    client_id != SOC15_IH_CLIENTID_SDMA4 &&
	    client_id != SOC15_IH_CLIENTID_SDMA5 &&
	    client_id != SOC15_IH_CLIENTID_SDMA6 &&
	    client_id != SOC15_IH_CLIENTID_SDMA7 &&
	    client_id != SOC15_IH_CLIENTID_VMC &&
	    client_id != SOC15_IH_CLIENTID_VMC1 &&
	    client_id != SOC15_IH_CLIENTID_UTCL2 &&
	    client_id != SOC15_IH_CLIENTID_SE0SH &&
	    client_id != SOC15_IH_CLIENTID_SE1SH &&
	    client_id != SOC15_IH_CLIENTID_SE2SH &&
	    client_id != SOC15_IH_CLIENTID_SE3SH &&
	    !KFD_IRQ_IS_FENCE(client_id, source_id))
		return false;

	/* This is a known issue for gfx9. Under non HWS, pasid is not set
	 * in the interrupt payload, so we need to find out the pasid on our
	 * own.
	 */
	if (!pasid && dev->dqm->sched_policy == KFD_SCHED_POLICY_NO_HWS) {
		const uint32_t pasid_mask = 0xffff;

		*patched_flag = true;
		memcpy(patched_ihre, ih_ring_entry,
				dev->kfd->device_info.ih_ring_entry_size);

		pasid = dev->dqm->vmid_pasid[vmid];

		/* Patch the pasid field */
		patched_ihre[3] = cpu_to_le32((le32_to_cpu(patched_ihre[3])
					& ~pasid_mask) | pasid);
	}

	dev_dbg(dev->adev->dev,
		"client id 0x%x, source id %d, vmid %d, pasid 0x%x. raw data:\n",
		client_id, source_id, vmid, pasid);
	dev_dbg(dev->adev->dev, "%8X, %8X, %8X, %8X, %8X, %8X, %8X, %8X.\n",
		data[0], data[1], data[2], data[3], data[4], data[5], data[6],
		data[7]);

	/* If there is no valid PASID, it's likely a bug */
	if (WARN_ONCE(pasid == 0, "Bug: No PASID in KFD interrupt"))
		return false;

	/* Workaround CP firmware sending bogus signals with 0 context_id.
	 * Those can be safely ignored on hardware and firmware versions that
	 * include a valid context_id on legitimate signals. This avoids the
	 * slow path in kfd_signal_event_interrupt that scans all event slots
	 * for signaled events.
	 */
	if (source_id == SOC15_INTSRC_CP_END_OF_PIPE) {
		uint32_t context_id =
			SOC15_CONTEXT_ID0_FROM_IH_ENTRY(ih_ring_entry);

		if (context_id == 0 && context_id_expected(dev->kfd))
			return false;
	}

	/* Interrupt types we care about: various signals and faults.
	 * They will be forwarded to a work queue (see below).
	 */
	return source_id == SOC15_INTSRC_CP_END_OF_PIPE ||
		source_id == SOC15_INTSRC_SDMA_TRAP ||
		source_id == SOC15_INTSRC_SDMA_ECC ||
		source_id == SOC15_INTSRC_SQ_INTERRUPT_MSG ||
		source_id == SOC15_INTSRC_CP_BAD_OPCODE ||
		KFD_IRQ_IS_FENCE(client_id, source_id) ||
		((client_id == SOC15_IH_CLIENTID_VMC ||
		client_id == SOC15_IH_CLIENTID_VMC1 ||
		client_id == SOC15_IH_CLIENTID_UTCL2) &&
		!amdgpu_no_queue_eviction_on_vm_fault);
}

static void event_interrupt_wq_v9(struct kfd_node *dev,
					const uint32_t *ih_ring_entry)
{
	uint16_t source_id, client_id, pasid, vmid;
	uint32_t context_id0, context_id1;
	uint32_t sq_intr_err, sq_int_data, encoding;

	source_id = SOC15_SOURCE_ID_FROM_IH_ENTRY(ih_ring_entry);
	client_id = SOC15_CLIENT_ID_FROM_IH_ENTRY(ih_ring_entry);
	pasid = SOC15_PASID_FROM_IH_ENTRY(ih_ring_entry);
	vmid = SOC15_VMID_FROM_IH_ENTRY(ih_ring_entry);
	context_id0 = SOC15_CONTEXT_ID0_FROM_IH_ENTRY(ih_ring_entry);
	context_id1 = SOC15_CONTEXT_ID1_FROM_IH_ENTRY(ih_ring_entry);

	if (client_id == SOC15_IH_CLIENTID_GRBM_CP ||
	    client_id == SOC15_IH_CLIENTID_SE0SH ||
	    client_id == SOC15_IH_CLIENTID_SE1SH ||
	    client_id == SOC15_IH_CLIENTID_SE2SH ||
	    client_id == SOC15_IH_CLIENTID_SE3SH) {
		if (source_id == SOC15_INTSRC_CP_END_OF_PIPE)
			kfd_signal_event_interrupt(pasid, context_id0, 32);
		else if (source_id == SOC15_INTSRC_SQ_INTERRUPT_MSG) {
			sq_int_data = KFD_CONTEXT_ID_GET_SQ_INT_DATA(context_id0, context_id1);
			encoding = REG_GET_FIELD(context_id0, SQ_INTERRUPT_WORD_WAVE_CTXID, ENCODING);
			switch (encoding) {
			case SQ_INTERRUPT_WORD_ENCODING_AUTO:
				dev_dbg_ratelimited(
					dev->adev->dev,
					"sq_intr: auto, se %d, ttrace %d, wlt %d, ttrac_buf_full %d, reg_tms %d, cmd_tms %d, host_cmd_ovf %d, host_reg_ovf %d, immed_ovf %d, ttrace_utc_err %d\n",
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_AUTO_CTXID,
						SE_ID),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_AUTO_CTXID,
						THREAD_TRACE),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_AUTO_CTXID,
						WLT),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_AUTO_CTXID,
						THREAD_TRACE_BUF_FULL),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_AUTO_CTXID,
						REG_TIMESTAMP),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_AUTO_CTXID,
						CMD_TIMESTAMP),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_AUTO_CTXID,
						HOST_CMD_OVERFLOW),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_AUTO_CTXID,
						HOST_REG_OVERFLOW),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_AUTO_CTXID,
						IMMED_OVERFLOW),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_AUTO_CTXID,
						THREAD_TRACE_UTC_ERROR));
				break;
			case SQ_INTERRUPT_WORD_ENCODING_INST:
				dev_dbg_ratelimited(
					dev->adev->dev,
					"sq_intr: inst, se %d, data 0x%x, sh %d, priv %d, wave_id %d, simd_id %d, cu_id %d, intr_data 0x%x\n",
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_WAVE_CTXID,
						SE_ID),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_WAVE_CTXID,
						DATA),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_WAVE_CTXID,
						SH_ID),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_WAVE_CTXID,
						PRIV),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_WAVE_CTXID,
						WAVE_ID),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_WAVE_CTXID,
						SIMD_ID),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_WAVE_CTXID,
						CU_ID),
					sq_int_data);
				if (context_id0 & SQ_INTERRUPT_WORD_WAVE_CTXID__PRIV_MASK) {
					if (kfd_set_dbg_ev_from_interrupt(dev, pasid,
							KFD_DEBUG_DOORBELL_ID(sq_int_data),
							KFD_DEBUG_TRAP_CODE(sq_int_data),
							NULL, 0))
						return;
				}
				break;
			case SQ_INTERRUPT_WORD_ENCODING_ERROR:
				sq_intr_err = REG_GET_FIELD(sq_int_data, KFD_SQ_INT_DATA, ERR_TYPE);
				dev_warn_ratelimited(
					dev->adev->dev,
					"sq_intr: error, se %d, data 0x%x, sh %d, priv %d, wave_id %d, simd_id %d, cu_id %d, err_type %d\n",
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_WAVE_CTXID,
						SE_ID),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_WAVE_CTXID,
						DATA),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_WAVE_CTXID,
						SH_ID),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_WAVE_CTXID,
						PRIV),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_WAVE_CTXID,
						WAVE_ID),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_WAVE_CTXID,
						SIMD_ID),
					REG_GET_FIELD(
						context_id0,
						SQ_INTERRUPT_WORD_WAVE_CTXID,
						CU_ID),
					sq_intr_err);
				if (sq_intr_err != SQ_INTERRUPT_ERROR_TYPE_ILLEGAL_INST &&
					sq_intr_err != SQ_INTERRUPT_ERROR_TYPE_MEMVIOL) {
					event_interrupt_poison_consumption_v9(dev, pasid, client_id);
					return;
				}
				break;
			default:
				break;
			}
			kfd_signal_event_interrupt(pasid, sq_int_data, 24);
		} else if (source_id == SOC15_INTSRC_CP_BAD_OPCODE &&
			   KFD_DBG_EC_TYPE_IS_PACKET(KFD_DEBUG_CP_BAD_OP_ECODE(context_id0))) {
			kfd_set_dbg_ev_from_interrupt(dev, pasid,
				KFD_DEBUG_DOORBELL_ID(context_id0),
				KFD_EC_MASK(KFD_DEBUG_CP_BAD_OP_ECODE(context_id0)),
				NULL, 0);
		}
	} else if (client_id == SOC15_IH_CLIENTID_SDMA0 ||
		   client_id == SOC15_IH_CLIENTID_SDMA1 ||
		   client_id == SOC15_IH_CLIENTID_SDMA2 ||
		   client_id == SOC15_IH_CLIENTID_SDMA3 ||
		   client_id == SOC15_IH_CLIENTID_SDMA4 ||
		   client_id == SOC15_IH_CLIENTID_SDMA5 ||
		   client_id == SOC15_IH_CLIENTID_SDMA6 ||
		   client_id == SOC15_IH_CLIENTID_SDMA7) {
		if (source_id == SOC15_INTSRC_SDMA_TRAP) {
			kfd_signal_event_interrupt(pasid, context_id0 & 0xfffffff, 28);
		} else if (source_id == SOC15_INTSRC_SDMA_ECC) {
			event_interrupt_poison_consumption_v9(dev, pasid, client_id);
			return;
		}
	} else if (client_id == SOC15_IH_CLIENTID_VMC ||
		   client_id == SOC15_IH_CLIENTID_VMC1 ||
		   client_id == SOC15_IH_CLIENTID_UTCL2) {
		struct kfd_vm_fault_info info = {0};
		uint16_t ring_id = SOC15_RING_ID_FROM_IH_ENTRY(ih_ring_entry);
		struct kfd_hsa_memory_exception_data exception_data;

		if (source_id == SOC15_INTSRC_VMC_UTCL2_POISON) {
			event_interrupt_poison_consumption_v9(dev, pasid, client_id);
			return;
		}

		info.vmid = vmid;
		info.mc_id = client_id;
		info.page_addr = ih_ring_entry[4] |
			(uint64_t)(ih_ring_entry[5] & 0xf) << 32;
		info.prot_valid = ring_id & 0x08;
		info.prot_read  = ring_id & 0x10;
		info.prot_write = ring_id & 0x20;

		memset(&exception_data, 0, sizeof(exception_data));
		exception_data.gpu_id = dev->id;
		exception_data.va = (info.page_addr) << PAGE_SHIFT;
		exception_data.failure.NotPresent = info.prot_valid ? 1 : 0;
		exception_data.failure.NoExecute = info.prot_exec ? 1 : 0;
		exception_data.failure.ReadOnly = info.prot_write ? 1 : 0;
		exception_data.failure.imprecise = 0;

		kfd_set_dbg_ev_from_interrupt(dev,
						pasid,
						-1,
						KFD_EC_MASK(EC_DEVICE_MEMORY_VIOLATION),
						&exception_data,
						sizeof(exception_data));
		kfd_smi_event_update_vmfault(dev, pasid);
	} else if (KFD_IRQ_IS_FENCE(client_id, source_id)) {
		kfd_process_close_interrupt_drain(pasid);
	}
}

static bool event_interrupt_isr_v9_4_3(struct kfd_node *node,
				const uint32_t *ih_ring_entry,
				uint32_t *patched_ihre,
				bool *patched_flag)
{
	uint16_t node_id, vmid;

	/*
	 * For GFX 9.4.3, process the interrupt if:
	 * - NodeID field in IH entry matches the corresponding bit
	 *   set in interrupt_bitmap Bits 0-15.
	 *   OR
	 * - If partition mode is CPX and interrupt came from
	 *   Node_id 0,4,8,12, then check if the Bit (16 + client id)
	 *   is set in interrupt bitmap Bits 16-31.
	 */
	node_id = SOC15_NODEID_FROM_IH_ENTRY(ih_ring_entry);
	vmid = SOC15_VMID_FROM_IH_ENTRY(ih_ring_entry);
	if (kfd_irq_is_from_node(node, node_id, vmid))
		return event_interrupt_isr_v9(node, ih_ring_entry,
					patched_ihre, patched_flag);
	return false;
}

const struct kfd_event_interrupt_class event_interrupt_class_v9 = {
	.interrupt_isr = event_interrupt_isr_v9,
	.interrupt_wq = event_interrupt_wq_v9,
};

const struct kfd_event_interrupt_class event_interrupt_class_v9_4_3 = {
	.interrupt_isr = event_interrupt_isr_v9_4_3,
	.interrupt_wq = event_interrupt_wq_v9,
};
