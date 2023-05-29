/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Gokulakannan
 * @brief          : Main program body
 ******************************************************************************
 *
 * Simple round robin task scheduler for STM32F103C8T6 bluepill board
 ******************************************************************************
 */

#include <stdint.h>
#include <stdio.h>
#include "main.h"
#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

uint32_t psp_of_tasks[MAX_TASKS] = {T1_STACK_START,T2_STACK_START,T3_STACK_START,T4_STACK_START};
uint32_t task_handlers[MAX_TASKS];
uint8_t current_task = 0; //task1 is running


int main(void)
{
	//enable processor faults
	enable_processor_faults();

	//First step is to init MSP
	init_scheduler_stack(SCHED_STACK_START);

	task_handlers[0] = (uint32_t)task1;
	task_handlers[1] = (uint32_t)task2;
	task_handlers[2] = (uint32_t)task3;
	task_handlers[3] = (uint32_t)task4;

	init_tasks_stack();							//init dummy stack frame for each tasks

	init_systick_timer(1000);					//scheduler

	//switch the stack pointer before calling task
	switch_to_psp();

	//call to tasks
	task1();
	for(;;);
}
void task1(void){
	while(1){
		printf("task1\n");
	}
}
void task2(void){
	while(1){
		printf("task2\n");
	}
}
void task3(void){
	while(1){
		printf("task3\n");
	}
}
void task4(void){
	while(1){
		printf("task4\n");
	}
}
void save_psp_value(uint32_t current_psp_value){
	psp_of_tasks[current_task] = current_psp_value;
}
void update_next_task(void){
	//This is round robin method (algorithm)
	current_task++;
	current_task %= MAX_TASKS;
}
__attribute__((naked)) void SysTick_Handler(void){ //Handler mode
//	scheduler of our program

//	save the context of the current task
	//1. Get current running task's psp value
	__asm volatile("MRS R0,PSP");
	//2. Using that PSP value store SF2 (R4-R11)
	__asm volatile("STMDB R0!,{R4-R11}");
	//3. Save the current value of PSP
	__asm volatile("PUSH {LR}");
	__asm volatile("BL save_psp_value");

//  Retrieve the context of next task
	//1. Decide next task to run
	__asm volatile("BL update_next_task");
	//2. get its past PSP value
	__asm volatile("BL get_psp_value");
	//3. Using that PSP value retrieve SF2 (R4-R11)
	__asm volatile("LDMIA R0!,{R4-R11}");
	//4. Update PSP and exit
	__asm volatile("MSR PSP,R0");
	__asm volatile("POP {LR}");
	__asm volatile("BX LR"); //manual return since using naked function
}
void init_systick_timer(uint32_t tick_hz){
	uint32_t* pSRVR = (uint32_t*)0xE000E014;
	uint32_t count_value = (SYSTICK_TIM_CLK/tick_hz)-1;

	//clear the value of RVR
	*pSRVR &= ~(0x00FFFFFF);

	//load the value into RVR
	*pSRVR |= count_value;

	//do some settings in systick control and status register
	uint32_t* pSCSR = (uint32_t*)0xE000E010;

	*pSCSR |= (1 << 1); //enables SysTick exception request
	*pSCSR |= (1 << 2); //indicates the clock source, processor clock source
	*pSCSR |= (1 << 0); //enables the counter
}
__attribute__((naked)) void init_scheduler_stack(uint32_t sp){
	__asm volatile("MSR MSP,%0"::"r"(sp));
	__asm volatile("BX LR");
}
void init_tasks_stack(void){
	uint32_t* psp;
	for(int i=0;i<MAX_TASKS;i++){
		psp = (uint32_t*)psp_of_tasks[i];
		//push --> decrement first and then push the value
		psp--;
		*psp = PSR;
		psp--;
		*psp = task_handlers[i];
		psp--;
		*psp = 0xFFFFFFFD; //return to thread mode using PSP stack pointer
		for(int j=0;j<13;j++){
			psp--;
			*psp = 0;
		}
		psp_of_tasks[i] = (uint32_t)psp;
	}
}
uint32_t get_psp_value(void){
	return psp_of_tasks[current_task];
}
__attribute__((naked)) void switch_to_psp(){
	//1.initialize the psp with TASK1 stack start address

	//get the value of psp of current task
	__asm volatile("PUSH {LR}"); //LR has return address to main, we need that because calling get_psp_value changes LR
	__asm volatile("BL get_psp_value");
	__asm volatile("MSR PSP,R0");
	__asm volatile("POP {LR}");

	//2.change SP to PSP using CONTROL reg
	__asm volatile("MOV R0,#0x02");
	__asm volatile("MSR CONTROL,R0");
	__asm volatile("BX LR");
}
void enable_processor_faults(void){
	uint32_t * pSHCSR = (uint32_t*)0xE000ED24;
	*pSHCSR |= (1 << 16); //mem manage
	*pSHCSR |= (1 << 17); //bus fault
	*pSHCSR |= (1 << 18); //usage fault
}
void HardFault_Handler(){
	printf("Hard fault exception\n");
	while(1);
}
void MemManage_Handler(){
	printf("MemManage exception\n");
	while(1);
}
void BusFault_Handler(){
	printf("Bus fault exception\n");
	while(1);
}
void UsageFault_Handler(){
	uint32_t* pUFSR = (uint32_t*)0xE000ED2A;
	printf("Usage fault exception\n");
	printf("UFSR : %lX\n",(*pUFSR) & 0xffff);
	while(1);
}
