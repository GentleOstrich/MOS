/*
 * operations on IDE disk.
 */

#include "serv.h"
#include <drivers/dev_disk.h>
#include <lib.h>
#include <mmu.h>
extern int map1[32] = {0};
extern int map2[32] = {0};
extern int map0[32] = {0};
// Overview:
//  read data from IDE disk. First issue a read request through
//  disk register and then copy data from disk buffer
//  (512 bytes, a sector) to destination array.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  dst: destination for data read from IDE disk.
//  nsecs: the number of sectors to read.
//
// Post-Condition:
//  Panic if any error occurs. (you may want to use 'panic_on')
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_OPERATION_READ',
//  'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS', 'DEV_DISK_BUFFER'
void ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;

	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		/* Exercise 5.3: Your code here. (1/2) */
		panic_on(syscall_write_dev(&temp, DEV_DISK_ID + DEV_DISK_ADDRESS, sizeof(uint32_t)));
		temp = begin + off;
		panic_on(syscall_write_dev(&temp, DEV_DISK_OFFSET + DEV_DISK_ADDRESS, sizeof(uint32_t)));
		temp = DEV_DISK_OPERATION_READ;
		panic_on(syscall_write_dev(&temp, DEV_DISK_START_OPERATION + DEV_DISK_ADDRESS, sizeof(uint32_t)));
		panic_on(syscall_read_dev(&temp, DEV_DISK_STATUS + DEV_DISK_ADDRESS, sizeof(uint32_t)));
		if (temp) {
			//debugf("0x%x + 0x%x\n", dst , off);
			panic_on(syscall_read_dev(dst + off, DEV_DISK_BUFFER + DEV_DISK_ADDRESS, DEV_DISK_BUFFER_LEN));
		} else {
			user_panic("ide read failed");
		}
	}
}

// Overview:
//  write data to IDE disk.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  src: the source data to write into IDE disk.
//  nsecs: the number of sectors to write.
//
// Post-Condition:
//  Panic if any error occurs.
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_BUFFER',
//  'DEV_DISK_OPERATION_WRITE', 'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS'
void ide_write(u_int diskno, u_int secno, void *src, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;

	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		/* Exercise 5.3: Your code here. (2/2) */
		panic_on(syscall_write_dev(&temp, DEV_DISK_ID + DEV_DISK_ADDRESS, sizeof(uint32_t)));
		temp = begin + off;
		panic_on(syscall_write_dev(&temp, DEV_DISK_OFFSET + DEV_DISK_ADDRESS, sizeof(uint32_t)));
		//先将数据放入缓存区
		panic_on(syscall_write_dev(src + off, DEV_DISK_BUFFER + DEV_DISK_ADDRESS, DEV_DISK_BUFFER_LEN));
		//然后再写入1启动
		temp = DEV_DISK_OPERATION_WRITE;
		panic_on(syscall_write_dev(&temp, DEV_DISK_START_OPERATION + DEV_DISK_ADDRESS, sizeof(uint32_t)));
		panic_on(syscall_read_dev(&temp, DEV_DISK_STATUS + DEV_DISK_ADDRESS, sizeof(uint32_t)));
		if (!temp) {
			user_panic("ide write failed");
		}
	}
}

void ssd_init() {
	for (int i = 0 ; i < 32; ++i) {
		map0[i] = 0xFFFFFFFF;
		map1[i] = 1;
		map2[i] = 0;
	}
}

int ssd_read(u_int logic_no, void *dst) {
	if (map0[logic_no] == 0xFFFFFFFF) {
		return -1;
	} else {
		ide_read(0, map0[logic_no], dst, 1);
		return 0;
	}
}

void ssd_write(u_int logic_no, void *src) {
	if (map0[logic_no] == 0xFFFFFFFF) {
		map0[logic_no] = ssd_alloc(logic_no);
         } else {
                ssd_erase(logic_no);
		map0[logic_no] = 0xFFFFFFFF;
		map0[logic_no] = ssd_alloc(logic_no);
	}
	//debugf("%d\n", map0[logic_no] );
	ide_write(0, map0[logic_no], src, 1 );
	map1[map0[logic_no]] = 0;
}


void ssd_erase(u_int logic_no) {
	if (map0[logic_no] == 0xFFFFFFFF) {
		return;
         } else {
		int temp = 0;
		ide_write(0, map0[logic_no], &temp , 1);
		map2[map0[logic_no]] ++;
		map1[map0[logic_no]] = 1;
		map0[logic_no] = 0xFFFFFFFF;
	}
}


int ssd_alloc(u_int logic_no ) {
	int aim;
	int min = 999999;
	for (int i = 0; i < 32; i++) {
		if (map2[i] < min && map1[i] == 1) {
			aim = i;
			min = map2[i];
		}
	}
	if (map2[aim] >= 5) {
		min = 999999;
		int aim2;
		for (int i = 0; i < 32; i++) {
			if (map1[i] == 0) {
				if (map2[i] < min) {
					aim2 = i;
					min = map2[i];
				}
			}
		}
		
		u_int data;
		ide_read(0, aim2, &data, 1);
		ide_write(0, aim, &data, 1);

		map1[aim] = 0;
		int n;
		for (int i = 0; i < 32; ++i) {
			if (map0[i] == aim2) {
				n = i;
				break;
			}
		}
		map0[n] = aim;
		int temp = 0;
		ide_write(0, aim2, &temp , 1);
		map2[aim2] ++;
		map1[aim2] = 1;
		return aim2;
	} else {
		return aim;
	}



}















