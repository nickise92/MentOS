/// @file ipc.c
/// @brief Inter-Process Communication (IPC) system call implementation.
/// @copyright (c) 2014-2023 This file is distributed under the MIT License.
/// See LICENSE.md for details.

#include "sys/unistd.h"
#include "system/syscall_types.h"
#include "sys/errno.h"
#include "stddef.h"
#include "string.h"
#include "io/debug.h"
#include "stdio.h"
#include "ipc/sem.h"
#include "ipc/shm.h"
#include "ipc/msg.h"
#include "stdlib.h"
#include "ipc/sem.h"
#include "ipc/shm.h"
#include "ipc/msg.h"
#include "sys/stat.h"
#include "io/debug.h"
#include "stdio.h"

_syscall3(void *, shmat, int, shmid, const void *, shmaddr, int, shmflg)

_syscall3(long, shmget, key_t, key, size_t, size, int, flag)

_syscall1(long, shmdt, const void *, shmaddr)

_syscall3(long, shmctl, int, shmid, int, cmd, struct shmid_ds *, buf)

_syscall3(long, semget, key_t, key, int, nsems, int, semflg)

_syscall4(long, semctl, int, semid, int, semnum, int, cmd, union semun *, arg)

_syscall2(long, msgget, key_t, key, int, msgflg)

_syscall4(long, msgsnd, int, msqid, struct msgbuf *, msgp, size_t, msgsz, int, msgflg)

_syscall5(long, msgrcv, int, msqid, struct msgbuf *, msgp, size_t, msgsz, long, msgtyp, int, msgflg)

_syscall3(long, msgctl, int, msqid, int, cmd, struct msqid_ds *, buf)

long semop(int semid, struct sembuf *sops, unsigned nsops)
{

    long __res;
    // Check the arguments.
    if ((nsops <= 0) || (sops == NULL)) {
        errno = EINVAL;
        return -1;
    }
    int flag_no_wait = 0;

    //this should be performed for each sops.
    for (size_t i = 0; i < nsops; i++){

        /*Checking for IPC_NOWAIT flag*/
        if (sops[i].sem_flg & IPC_NOWAIT){
            flag_no_wait = 1;
        }

       // The process continues to try to perform the operation until it completes
        // or receives an error.
        while (1) {
            // Calling the kernel-side function.
            __inline_syscall3(__res, semop, semid, &sops[i], 1);
            /*
            If we get an error, the operation has been taken care of we stop the loop.
            We also stop the loop if the operation is not allowed and the IPC_NOWAIT flag is 1
            */ 
            if (__res != OPERATION_NOT_ALLOWED || flag_no_wait)
                break;
        }

        /*If the operation couldn't be performed and we had the IPC_NOWAIT set to 1 then we ret*/
        if (flag_no_wait && __res == OPERATION_NOT_ALLOWED){
            errno = EAGAIN;
            return -1;
        }
        //printf("op eseguita: %d\n", __res);
    }
    
    // Now, we can return the value.
    __syscall_return(long, __res);
}

key_t ftok(char *path, int id)
{
    // Create a struct containing the serial number and the device number of the
    // file we use to generate the key.
    struct stat_t st;
    if (stat(path, &st) < 0) {
        errno = ENOENT;
        pr_debug("Error finding the serial number, check Errno...\n");
        return -1;
    }
    // Taking the upper 8 bits from the lower 8 bits of id, the second upper 8
    // bits from the lower 8 bits of the device number of the provided pathname,
    // and the lower 16 bits from the lower 16 bits of the inode number of the
    // provided pathname.
    return ((st.st_ino & 0xffff) | ((st.st_dev & 0xff) << 16) | ((id & 0xffu) << 24));
}
