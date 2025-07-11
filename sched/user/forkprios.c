#include <inc/lib.h>
void
umain(int argc, char **argv)
{
	envid_t who;
	sys_change_prio(thisenv->env_id, 2);

	if ((who = fork()) == 0) {
		if (sys_get_env_prio() == 2)
			cprintf("El proceso hijo de un proceso de prioridad 2 "
			        "tiene también prioridad 2\n");
	} else {
		if (sys_get_env_prio() == 2)
			cprintf("El proceso padre mantiene una prioridad de 2 "
			        "luego del fork\n");
	}
}