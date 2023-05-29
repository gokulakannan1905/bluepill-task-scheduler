/*
 * main.h
 *
 *  Created on: May 29, 2023
 *      Author: gokulakannan
 */

#ifndef MAIN_H_
#define MAIN_H_


//some stack memory calculations
#define SRAM_END 				((0x20000000U) + (20 * 1024U))
// PSP values for different tasks
#define T1_STACK_START 			SRAM_END
#define T2_STACK_START 			((SRAM_END) - (1 * 1024U))
#define T3_STACK_START 			((SRAM_END) - (2 * 1024U))
#define T4_STACK_START 			((SRAM_END) - (3 * 1024U))
#define SCHED_STACK_START		((SRAM_END) - (4 * 1024U))

#define HSI_CLOCK 				8000000U
#define SYSTICK_TIM_CLK 		HSI_CLOCK

#define MAX_TASKS				4

#define PSR						0x01000000U


void task1(void);
void task2(void);
void task3(void);
void task4(void);
void init_systick_timer(uint32_t tick);
void init_scheduler_stack(uint32_t sp);
void init_tasks_stack(void);
void switch_to_psp(void);
void enable_processor_faults(void);
uint32_t get_psp_value(void);
void save_psp_value(uint32_t current_psp_value);


#endif /* MAIN_H_ */
