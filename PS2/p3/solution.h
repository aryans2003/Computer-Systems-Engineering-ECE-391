#ifndef SOLUTION_H
#define SOLUTION_H
#include "spinlock_ece391.h"

// You may add up to 5 elements to this struct.
// The type of synchronization primitive you may use is SpinLock.
typedef struct zs_enter_exit_lock{

}zs_lock;


int zombie_enter(zs_lock* zs);

int zombie_exit(zs_lock* zs);

int scientist_enter(zs_lock* zs);

int scientist_exit(zs_lock* zs);



#endif /* SOLUTION_H */