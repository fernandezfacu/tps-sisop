/*
 * Restore the register values in the Trapframe with the 'iret' instruction.
 * This exits the kernel and starts executing some environment's code.
 *
 * This function does not return.
 */

.globl context_switch;
context_switch:

	/* 
	 * Recupera el puntero a Trapframe 
	 * los primeros 4 bytes son puntero a return adress,
	 * se encuentra después de eso 
	 */
	movl 4(%esp), %esp	 

	/* 
	 * Guardar registros en stack desde Trapframe
	 * 
	 * De propósito general, es y ds, en ese orden.
	 */
	popal 
	pop %es 
	pop	%ds	

	/*
     * Para hacer iret, se debe apuntar a tf_eip
	 * Se aumenta en 8 el stack pointer para esto
	 * (trapno y err son de 4 bytes cada uno y están entre ds y eip)
	 */
	add	$8,%esp	 
	iret
spin:
	jmp spin
