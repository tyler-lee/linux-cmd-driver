#ifndef _UAPI_ASM_X86_CMD_H
#define _UAPI_ASM_X86_CMD_H

#include <linux/types.h>
#include <linux/ioctl.h>

#define CMD_MAGIC 0xA4

#define CMD_IOC_ENCLAVE_CREATE \
	_IOW(CMD_MAGIC, 0x00, struct cmd_enclave_create)
#define CMD_IOC_ENCLAVE_ADD_PAGE \
	_IOW(CMD_MAGIC, 0x01, struct cmd_enclave_add_page)
#define CMD_IOC_ENCLAVE_INIT \
	_IOW(CMD_MAGIC, 0x02, struct cmd_enclave_init)

/* CMD leaf instruction return values */
#define CMD_SUCCESS			0
#define CMD_INVALID_SIG_STRUCT		1
#define CMD_INVALID_ATTRIBUTE		2
#define CMD_BLKSTATE			3
#define CMD_INVALID_MEASUREMENT		4
#define CMD_NOTBLOCKABLE		5
#define CMD_PG_INVLD			6
#define CMD_LOCKFAIL			7
#define CMD_INVALID_SIGNATURE		8
#define CMD_MAC_COMPARE_FAIL		9
#define CMD_PAGE_NOT_BLOCKED		10
#define CMD_NOT_TRACKED			11
#define CMD_VA_SLOT_OCCUPIED		12
#define CMD_CHILD_PRESENT		13
#define CMD_ENCLAVE_ACT			14
#define CMD_ENTRYEPOCH_LOCKED		15
#define CMD_INVALID_LICENSE		16
#define CMD_PREV_TRK_INCMPL		17
#define CMD_PG_IS_SECS			18
#define CMD_INVALID_CPUSVN		32
#define CMD_INVALID_ISVSVN		64
#define CMD_UNMASKED_EVENT		128
#define CMD_INVALID_KEYNAME		256

/* IOCTL return values */
#define CMD_POWER_LOST_ENCLAVE		0x40000000
#define CMD_LE_ROLLBACK			0x40000001

/**
 * struct cmd_enclave_create - parameter structure for the
 *                             %CMD_IOC_ENCLAVE_CREATE ioctl
 * @src:	address for the SECS page data
 */
struct cmd_enclave_create  {
	__u64	src;
} __packed;

/**
 * struct cmd_enclave_add_page - parameter structure for the
 *                               %CMD_IOC_ENCLAVE_ADD_PAGE ioctl
 * @addr:	address in the ELRANGE
 * @src:	address for the page data
 * @secinfo:	address for the SECINFO data
 * @mrmask:	bitmask for the 256 byte chunks that are to be measured
 */
struct cmd_enclave_add_page {
	__u64	addr;
	__u64	src;
	__u64	secinfo;
	__u16	mrmask;
} __packed;

/**
 * struct cmd_enclave_init - parameter structure for the
 *                           %CMD_IOC_ENCLAVE_INIT ioctl
 * @addr:	address in the ELRANGE
 * @sigstruct:	address for the page data
 * @einittoken:	address for the SECINFO data
 */
struct cmd_enclave_init {
	__u64	addr;
	__u64	sigstruct;
	__u64	einittoken;
} __packed;

struct cmd_enclave_destroy {
	__u64	addr;
} __packed;

#endif /* _UAPI_ASM_X86_CMD_H */
