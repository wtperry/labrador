/*
 *  Hardware Abstraction Layer
 */

#ifndef _KERNEL_HAL_H
#define _KERNEL_HAL_H
 
void init_hal(void);

void enable_interrupts(void);
void disable_interrupts(void);

#endif