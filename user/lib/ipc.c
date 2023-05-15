// User-level IPC library routines

#include <env.h>
#include <lib.h>
#include <mmu.h>

// Send val to whom.  This function keeps trying until
// it succeeds.  It should panic() on any error other than
// -E_IPC_NOT_RECV.
//
// Hint: use syscall_yield() to be CPU-friendly.


u_int get_time(u_int *us) {
	u_int rtc = 0x15000000;
	
	u_int update = rtc + 0x0000;
	u_int rzm = rtc + 0x0010;
	u_int rwm = rtc + 0x0020;

	u_int zm;
	u_int wm;
	
	u_int temp = 0;
//	syscall_write_dev(&temp, DEV_DISK_START_OPERATION + DEV_DISK_ADDRESS, sizeof(uint32_t));
//	syscall_read_dev(&temp, DEV_DISK_STATUS + DEV_DISK_ADDRESS, sizeof(uint32_t));

	syscall_read_dev(&temp, update, 4);
	syscall_read_dev(&zm, rzm, 4);
	syscall_read_dev(&wm, rwm, 4);
	*us = wm;
	return zm;

}


void usleep(u_int us) {
	u_int bwm;
	u_int bzm = get_time(&bwm); 
	u_int bt = bzm * 1000000 + bwm;
	while (1) {
		u_int nwm;
		u_int nzm = get_time(&nwm);
		int t1 = (int) (nzm - bzm);
		int t2 = (int) (nwm - bwm);

		int t = t1 * 1000000 + t2;

		if ( t >= 0 ) {
			return ;
		} else {
			syscall_yield();
		}


	}


}





void ipc_send(u_int whom, u_int val, const void *srcva, u_int perm) {
	int r;
	while ((r = syscall_ipc_try_send(whom, val, srcva, perm)) == -E_IPC_NOT_RECV) {
		syscall_yield();
	}
	user_assert(r == 0);
}

// Receive a value.  Return the value and store the caller's envid
// in *whom.
//
// Hint: use env to discover the value and who sent it.
u_int ipc_recv(u_int *whom, void *dstva, u_int *perm) {
	int r = syscall_ipc_recv(dstva);
	if (r != 0) {
		user_panic("syscall_ipc_recv err: %d", r);
	}

	if (whom) {
		*whom = env->env_ipc_from;
	}

	if (perm) {
		*perm = env->env_ipc_perm;
	}

	return env->env_ipc_value;
}
