#ifndef TF_SEM_OP_UTILS_H
#define TF_SEM_OP_UTILS_H

#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>

inline bool incrementSemSet(int semId, int semNumber) {
    sembuf semIncOp;
    semIncOp.sem_op = 1;
    semIncOp.sem_flg = 0;
    semIncOp.sem_num = semNumber;
    if (semop(semId, &semIncOp, 1) == -1) {
        perror("failed when incrementing sem set");
        return false;
    }
    return true;
}

inline bool incrementAllInSemSet(int semId, int numSems) {
    sembuf* semIncOpBuff = new sembuf[numSems];
    for (int i = 0; i < numSems; ++i) {
        semIncOpBuff[i].sem_op = 1;
        semIncOpBuff[i].sem_flg = 0;
        semIncOpBuff[i].sem_num = i;
    }
    
    if (semop(semId, semIncOpBuff, numSems) == -1) {
        delete[] semIncOpBuff;
        perror("failed when incrementing all sems in set");
        return false;
    }
    delete[] semIncOpBuff;
    return true;
}

inline bool decrementSemSet(int semId, int semNumber) {
    sembuf semDecrOp;
    semDecrOp.sem_op = -1;
    semDecrOp.sem_flg = 0;
    semDecrOp.sem_num = semNumber;
    if (semop(semId, &semDecrOp, 1) == -1) {
        perror("failed when decrementing sem set");
        return false;
    }
    return true;
}


#endif //TF_SEM_OP_UTILS_H
