#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("i am environment %08x\n", thisenv->env_id);
	int r;
	if ((r = sys_change_prio(thisenv->env_id, 2)) == 0) {
		cprintf("i changed my prio to %d\n", thisenv->env_priority);
	} else {
		cprintf("%d\n", r);
		cprintf("i did not change my prio: %d\n", thisenv->env_priority);
	}
}