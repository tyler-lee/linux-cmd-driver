#ifndef _UAPI_ASM_X86_CMD_H
#define _UAPI_ASM_X86_CMD_H

#include <linux/types.h>
#include <linux/ioctl.h>

#define CMD_MAGIC 0xA5
//#define _IOWR(type,nr,size)	_IOC(_IOC_READ|_IOC_WRITE,(type),(nr),(_IOC_TYPECHECK(size)))
#define _CMD_IOCTL(type) _IOWR(CMD_MAGIC, type, struct cmd_params)
//#define _CMD_IOCTL(type) _IOW(CMD_MAGIC, type, struct cmd_params)

#define CMD_IOC_DISABLE_IRQ		_CMD_IOCTL(0x00)
#define CMD_IOC_ENABLE_IRQ		_CMD_IOCTL(0x01)

/* CMD leaf instruction return values */
#define CMD_SUCCESS			0
#define CMD_INVALID_ATTRIBUTE		2
#define CMD_MAC_COMPARE_FAIL		9

#pragma pack(push, 1)
struct cmd_params  {
	__u64	addr;
	//int para1;
	//char para2[12];
};
#pragma pack(pop)

#endif /* _UAPI_ASM_X86_CMD_H */
