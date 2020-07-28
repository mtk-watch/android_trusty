#ifndef __KMGETBOOT_H__
#define __KMGETBOOT_H__

#include <sys/types.h>

#define KMGETBOOT_PORT		"com.mediatek.kmgetboot"
#define MAX_MSG_SIZE		4096
#define IPC_MSG_SIZE		16
#define RESP_FLAG		0x80000000
#define DONE_FLAG		0x40000000
#define SHA256_LENGTH		32

enum kmgetboot_cmd {
	GET_VERIFIED_BOOT = 0x0,
#if 0
	SET_KEY_LEN = 0x10,
	SEND_KEY_BUF = 0x11,
	DEC_KEY_BUF = 0x12,
	READ_KEY_BUF = 0x13,
#endif
};

struct boot_param {
	uint32_t os_version;
	uint32_t os_patchlevel;
	uint32_t device_locked;
	uint32_t verified_boot_state;
	uint8_t verified_boot_key[SHA256_LENGTH];
};

struct kmgetboot_msg {
	uint32_t cmd;
	uint8_t payload[0];
};

#endif
