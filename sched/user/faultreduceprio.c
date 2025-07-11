#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("i am environment %08x\n", thisenv->env_id);
	sys_change_prio(thisenv->env_id, 3);
	cprintf("my prio is %d\n", thisenv->env_priority);
	if (sys_change_prio(thisenv->env_id, 2) == -3) {  // -E_INVAL = -3
		cprintf("couldn´t change my prio to 2\n");
		cprintf("my prio is %d\n", thisenv->env_priority);
	}
	if (sys_change_prio(thisenv->env_id, -3) == -3) {
		cprintf("couldn´t change my prio to -3\n");
		cprintf("my prio is %d\n", thisenv->env_priority);
	}
}