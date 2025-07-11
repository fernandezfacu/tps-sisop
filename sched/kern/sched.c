#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

#define MAX_RUNS 24
#define MAX_ENV_RUNS 3

#define MAX_PRIO 0
#define MIN_PRIO 6

static int sched_calls = 0;
static int priority_boosts = 0;

void sched_halt(void);
void print_stats(void);

// Choose a user environment to run and run it.
void
sched_yield(void)
{
	sched_calls++;

	int start_index = 0;
	if (curenv)
		start_index = ENVX(curenv->env_id) + 1;

#ifdef SCHED_ROUND_ROBIN
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running. Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// Your code here - Round robin

	for (int i = 0; i < NENV; i++) {
		int idx = (start_index + i) % NENV;  // modulo for circular search
		struct Env *env = &envs[idx];
		if (env->env_status == ENV_RUNNABLE)
			env_run(env);
	}

	if (curenv && (curenv->env_status == ENV_RUNNING))
		env_run(curenv);

#endif

#ifdef SCHED_PRIORITIES
	// Implement simple priorities scheduling.
	//
	// Environments now have a "priority" so it must be consider
	// when the selection is performed.
	//
	// Be careful to not fall in "starvation" such that only one
	// environment is selected and run every time.

	// Your code here - Priorities
	if (sched_calls % MAX_RUNS == 0) {
		// boost priorities to avoid starvation
		for (int i = 0; i < NENV; i++) {
			struct Env *env = &envs[i];
			env->env_priority = MAX_PRIO;
			env->env_runs_since_prio_change = 0;
		}
		priority_boosts++;
	}

	struct Env *env_to_run = NULL;
	int best_priority = MIN_PRIO;
	for (int i = 0; i < NENV; i++) {
		int idx = (start_index + i) % NENV;  // modulo for circular search
		struct Env *env = &envs[idx];
		if ((env->env_status == ENV_RUNNABLE) &&
		    (env->env_priority < best_priority)) {
			env_to_run = env;
			best_priority = env_to_run->env_priority;
		}
	}

	if (!env_to_run && curenv && (curenv->env_status == ENV_RUNNING))
		env_to_run = curenv;

	if (env_to_run) {
		if (env_to_run->env_runs_since_prio_change >= MAX_ENV_RUNS) {
			env_to_run->env_runs_since_prio_change = 0;
			env_to_run->env_priority_losses++;
			if (env_to_run->env_priority != MIN_PRIO) {
				env_to_run->env_priority++;
				if (env_to_run->env_priority >
				    env_to_run->env_min_prio)
					env_to_run->env_min_prio =
					        env_to_run->env_priority;
			}

		} else {
			env_to_run->env_runs_since_prio_change++;
		}
		env_run(env_to_run);
	}

#endif

	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		print_stats();
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}

void
print_stats()
{
	cprintf("\nSCHEDULER STATS"
	        "\n");

	cprintf("times envs priorities were boosted: %d\n", priority_boosts);
	cprintf("times scheduler was called: %d\n", sched_calls);
	cprintf("\nENVS STATS\n");
	cprintf("env id \t | runs \t | priority losses | min prio"
	        "\n");
	cprintf("————————————————————————————————————————————————————————\n");
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_runs != 0) {
			cprintf("%d \t | %d \t | %d \t   | %d \n",
			        envs[i].env_id,
			        envs[i].env_runs,
			        envs[i].env_priority_losses,
			        envs[i].env_min_prio);
		}
	}
	cprintf("\n");
}