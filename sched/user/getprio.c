#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("i am environment %08x\n", thisenv->env_id);
	int prio;
	if (sys_get_env_prio() == 0)
		cprintf("my priority is %d\n", thisenv->env_priority);
}