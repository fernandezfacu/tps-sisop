// idle loop - this one ends

#include <inc/x86.h>
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	binaryname = "idle";

	int i = 0;
	while (i < 100000) {
		sys_yield();
		i++;
	}
}