#include "FreeRTOS.h"

// 错误出口
static void prvTaskExitError(void)
{
    for(;;);
}

static void prvStartFirstTask(void);
/*初始化栈内存
操作：将任务的重要信息写入栈中，防止CPU切换任务后丢失
2024/11/10
Constant_z
*/
StackType_t* pxPortInitialiseStack(StackType_t* pxTopOfStack, /*任务栈顶指针*/
								   TaskFunction_t pxCode, /* 任务函数地址*/
								   void* pvParameters) /*任务函数传入参数*/
{
		/*根据Cortex内核 发生不同情况取出不同寄存器的值 参考M3技术手册中断章节*/
    // 异常发生时，自动加载到CPU的内容
    pxTopOfStack --;
		/* 堆栈中压入：xPSR 寄存器初始值为 0x01000000 */
    *pxTopOfStack = portINITIAL_XPSR;
    pxTopOfStack --;
		/* 堆栈中压入PC寄存器：任务函数的地址（PC 寄存器） 确保最后一位为0 为了内存对齐*/
    *pxTopOfStack = ((StackType_t)pxCode) & portSTART_ADDRESS_MASK;
    pxTopOfStack --;
		/* 堆栈中压入：任务错误退出函数地址（LR 寄存器） */
    *pxTopOfStack = (StackType_t)prvTaskExitError;
	
    // 预留：寄存器r12、r3、r2 和 r1 默认初始化为 0
    pxTopOfStack -= 5;
		// 堆栈压入：任务函数的传入参数（R0 寄存器）
    *pxTopOfStack = (StackType_t)pvParameters;
	
    /* 为 R11、R10、R9、R8、R7、R6、R5、R4 寄存器预留空间 */
    pxTopOfStack -= 8;
	
    // 返回栈顶指针，此时 pxTopOfStack 指向空闲栈
    return pxTopOfStack;
}

/*启动调度器
操作：设置PendSv和SysTick中断优先级最低
2024/11/12
Constant_z
*/
// 启动调度器
BaseType_t xPortStartScheduler(void)
{
	// 设置 PendSV 和 SysTick 中断优先级为最低
	portNVIC_SHPR3_REG |= portNVIC_PENDSV_PRI;
	portNVIC_SHPR3_REG |= portNVIC_SYSTICK_PRI;
	
	// 初始化滴答定时器
	
	// 启动第一个任务，不再返回
	prvStartFirstTask();
	
	// 正常不会运行到这里
	return 0;
}

// 启动第一个任务,实际上是触发SVC中断
//_asm即为宏定义，__ASM，其允许在Keil编译器中插入ARM指令用
__asm void prvStartFirstTask(void)
{	
	//详细的ARM指令直接看ARM相关汇编书籍
	// 8字节对齐
    PRESERVE8
	
    /* 
	在 cortex-M 中，0xE000ED08 是 SCB_VTOR 这个寄存器的地址，
	里面放的是向量表的向量表的起始地址，即 msp 的地址 
	*/
    ldr r0,=0xE000ED08
    ldr r0,[r0]
    ldr r0,[r0]
	
	// 设置主栈指针msp的值
    msr msp,r0
	
    // 启用全局中断
    cpsie i //使能IRQ中断
    cpsie f //使能快速中断 FIQ中断
    dsb   //数据同步指定，确保执行该指令前所有数据访问已经完成
    isb   //刷新指定流水线
    // 调用SVC启动第一个任务
    svc 0
    nop
    nop
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
    // 将更新后的寄存器 r0 的值加载给 psp(特殊功能寄存器进程堆栈指针)
		//改变当前任务的堆栈指针，从而改变任务的执行上下文
		//PSP 是 Cortex-M 系列处理器中用于线程模式的特殊寄存器，它允许每个任务拥有自己的堆栈，
		//与主堆栈指针（Main Stack Pointer，MSP）相独立
    msr psp,r0
    isb
    // 开中断
		//Base Priority寄存器用于确定中断的优先级。当多个中断同时发生时，处理器会根据Base Priority寄存器的值
		//和各个中断的优先级来决定首先处理哪个中断
		//显然这条指定是屏蔽优先级低于0的中断，即屏蔽所有中断 为的是执行这个服务函数的时候不被其他中断打扰
    mov r0,#0
    msr basepri,r0
    // 指示处理器在异常返回时使用 psp 作为堆栈指针
    orr r14,#0xd
	// 自动加载剩下的 xPSR、PC、R14、R12 和 R3 ~ R0 寄存器
    bx r14
}

/* port.c */
// PendSV 中断服务函数，真正实现任务切换的函数
__asm void xPortPendSVHandler(void)
{
	// 只要触发 PendSV 中断，寄存器 xPSR、PC、R14、R12 和 R3 ~ R0，
	// 这 8 个寄存器的值会自动从 psp 指针指向的地址开始加载到任务栈中
	// 并且 psp 指针也会自动更新
	
		/*导入需要的全局变量*/
    extern pxCurrentTCB;
    extern vTaskSwitchContext; 
	
    PRESERVE8
		// 将 psp 的值存储到寄存器 r0 中 即当前运行任务的任务栈指针 
    mrs r0,psp
    isb
		// 将 pxCurrentTCB 的地址存储到寄存器 r3 中 即指向当前运行任务控制块的指针
    ldr r3,=pxCurrentTCB
    // 将寄存器 r3 存储的地址里的内容存储到寄存器 r2 中
    // 此时寄存器 r2 中存储了当前执行任务的 TCB 地址 即当前运行任务控制块的首地址 
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
		// 将寄存器 r3 和 r14 入栈到MSP指向的栈中
    stmdb sp!,{r3,r14}
    // 屏蔽 优先级值 高于或者等于 11 的中断 在FreeRTOSConfig.h中定义 用于区分RTOS可以管理和不可管理的中断
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



