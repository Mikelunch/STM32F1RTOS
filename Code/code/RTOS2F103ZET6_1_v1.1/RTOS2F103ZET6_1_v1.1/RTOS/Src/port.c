#include "FreeRTOS.h"

// 错误出口
static void prvTaskExitError(void)
{
    for(;;);
}
// 初始化栈内存
StackType_t* pxPortInitialiseStack(StackType_t* pxTopOfStack, 
								   TaskFunction_t pxCode, 
								   void* pvParameters)
{
    // 异常发生时，自动加载到CPU的内容
    pxTopOfStack --;
    *pxTopOfStack = portINITIAL_XPSR;
    pxTopOfStack --;
    *pxTopOfStack = ((StackType_t)pxCode) & portSTART_ADDRESS_MASK;
    pxTopOfStack --;
    *pxTopOfStack = (StackType_t)prvTaskExitError;
	
    // r12、r3、r2 和 r1 默认初始化为 0
    pxTopOfStack -= 5;
    *pxTopOfStack = (StackType_t)pvParameters;
	
    // 异常发生时，手动加载到 CPU 的内容
    pxTopOfStack -= 8;
	
    // 返回栈顶指针，此时 pxTopOfStack 指向空闲栈
    return pxTopOfStack;
}


// 启动调度器
BaseType_t xPortStartScheduler(void)
{
	// 设置 PendSV 和 SysTick 中断优先级为最低
	portNVIC_SHPR3_REG |= portNVIC_PENDSV_PRI;
	//portNVIC_SHPR3_REG |= portNVIC_SYSTICK_PRI;
	
	// 初始化滴答定时器
	if(SysTick_Config(SystemCoreClock / 1000)){
		while(1){}
	}
	
	// 启动第一个任务，不再返回
	prvStartFirstTask();
	
	// 正常不会运行到这里
	return 0;
}

// SVC 中断服务函数
__asm void vPortSVCHandler(void)
{
    extern pxCurrentTCB;
	// 8 字节对齐
    PRESERVE8
    // 将 pxCurrentTCB 指针的地址加载到寄存器 r3
    ldr r3,=pxCurrentTCB
    // 将 pxCurrentTCB 指针指向的地址（存储 TCB 的地址）加载到寄存器 r1
    ldr r1,[r3]
    // 将 pxCurrentTCB 指针指向的地址里的值（当前运行的 TCB 结构体）加载到寄存器 r0
    // 也即寄存器 r0 中存储了当前运行的任务的栈顶指针地址
    ldr r0,[r1]
    // 以寄存器 r0 为基地址，将任务栈中存储的值加载到寄存器 r4 ~ r11 中
    // 同时寄存器 r0 的值会自动更新
    ldmia r0!,{r4-r11}
    // 将更新后的寄存器 r0 的值加载给 psp
    msr psp,r0
    isb
    // 开中断
    mov r0,#0
    msr basepri,r0
    // 指示处理器在异常返回时使用 psp 作为堆栈指针
    orr r14,#0xd
	// 异常返回，自动加载剩下的 xPSR、PC、R14、R12 和 R3 ~ R0 寄存器
    bx r14
}

/* port.c */
__asm void prvStartFirstTask(void)
{
	// 8 字节对齐
    PRESERVE8
	
	// 在 cortex-M 中，0xE000ED08 是 SCB_VTOR 这个寄存器的地址，
	// 里面放的是向量表的起始地址 ，其第一项为 __initial_sp，也即 msp 的初始地址 
    ldr r0,=0xE000ED08
    ldr r0,[r0]
    ldr r0,[r0]
	// 设置主栈指针 msp 的值
    msr msp,r0
    // 启用全局中断
    cpsie i
    cpsie f
    dsb
    isb
    // 调用 SVC 启动第一个任务
    svc 0
    nop
    nop
}

// PendSV 中断服务函数，真正实现任务切换的函数
__asm void xPortPendSVHandler(void)
{
	// 只要触发 PendSV 中断，寄存器 xPSR、PC、R14、R12 和 R3 ~ R0，
	// 这 8 个寄存器的值会自动从 psp 指针指向的地址开始加载到任务栈中
	// 并且 psp 指针也会自动更新
    extern pxCurrentTCB;
    extern vTaskSwitchContext;
	
    PRESERVE8
	// 将 psp 的值存储到寄存器 r0 中 
    mrs r0,psp
    isb
	// 将 pxCurrentTCB 的地址存储到寄存器 r3 中
    ldr r3,=pxCurrentTCB
    // 将寄存器 r3 存储的地址里的内容存储到寄存器 r2 中
    // 此时寄存器 r2 中存储了当前执行任务的 TCB 地址
    ldr r2,[r3]
	// 以寄存器 r0 为基地址（r0 中现在存储了 psp 指针的值）
	// 将寄存器 r11 ~ r4 中存储的值存储到任务栈
	// 并且 r0 中存储的地址指针也会自动更新
    stmdb r0!,{r4-r11}
    // 将寄存器 r0 中的内容存储到上一个执行任务的 TCB 地址
    // 也即上一个任务的栈顶指针 pxTopOfStack 指向寄存器 r0 现在存储的地址
    str r0,[r2]
	// 上文保存完成
	// 开始加载下文
	// 将寄存器 r3 和 r14 压入栈
    stmdb sp!,{r3,r14}
    // 屏蔽 优先级值 高于或者等于 11 的中断
    mov r0,#configMAX_SYSCALL_INTERRUPT_PRIORITY
    msr basepri,r0
    dsb
    isb
    // 跳转到 vTaskSwitchContext 函数，得到下一个要执行任务的指针 pxCurrentTCB
    bl vTaskSwitchContext
    // 开中断
    mov r0,#0
    msr basepri,r0
    // 恢复寄存器 r3 和 r14
    ldmia sp!,{r3,r14}
	
	// 将 pxCurrentTCB 加载到寄存器 r1 
    ldr r1,[r3]
    // 将下一个要执行任务的栈顶指针加载到寄存器 r0 
    ldr r0,[r1]
    // 以寄存器 r0 为基地址（r0 中现在存储了任务的栈顶指针）
    // 将任务栈中存储的值加载到寄存器 r4 ~ r11 中
    // 同时 r0 中的值会自动更新
    ldmia r0!,{r4-r11}
    // 将更新后的 r0 的值加载到 psp
    msr psp,r0
    isb
    // 异常退出，自动加载剩下的 xPSR、PC、R14、R12 和 R3 ~ R0 寄存器
    bx r14
    nop
}


// 中断嵌套计数器
static UBaseType_t uxCriticalNesting = 0xAAAAAAAA;
// 进入临界区
void vPortEnterCritical(void)
{
	portDISABLE_INTERRUPTS();
	uxCriticalNesting++;
	if(uxCriticalNesting==1)
	{
		// configASSERT((portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK) == 0);
	}
}
// 退出临界区
void vPortExitCritical(void)
{
	// configASSERT(uxCriticalNesting);
	uxCriticalNesting--;
	if(uxCriticalNesting == 0)
	{
		portENABLE_INTERRUPTS();
	}
}

// SysTick 中断服务函数
void xPortSysTickHandler(void)
{	
    // 关中断
    vPortRaiseBASEPRI();
    // 更新任务延时参数
    if(xTaskIncrementTick()!= pdFALSE){
			portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
		}
    // 开中断
    vPortSetBASEPRI(0);
		
}
