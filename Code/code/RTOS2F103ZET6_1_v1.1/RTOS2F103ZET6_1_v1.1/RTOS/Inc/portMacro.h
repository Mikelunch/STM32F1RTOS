#ifndef _PORTMARCO_H__
#define _PORTMARCO_H__

/* 用来统一 RTOS 中用到的类型和定义一些功能宏
2024/11/5
ConstantZ
*/

/*基本数据类型与RTOS宏定义
2024/11/8
Constant_z
*/
/* portMacro.h */
#include <stdint.h>
#include "FreeRTOS.h"

#define port_CHAR                   char
#define port_FLOAT                  float
#define port_DOUBLE                 double
#define port_LONG                   long
#define port_SHORT                  short
#define port_STACK_TYPE             unsigned int
#define port_BASE_TYPE              long

typedef port_STACK_TYPE             StackType_t;
typedef long                        BaseType_t;
typedef unsigned long               UBaseType_t;

typedef port_STACK_TYPE*            StackType_p;
typedef long*                       BaseType_p;
typedef unsigned long*              UBaseType_p;


#if(configUSE_16_BIT_TICKS == 1)
    typedef uint16_t                TickType_t;
    #define portMAX_DELAY           (TickType_t) 0xffff
#else
    typedef uint32_t                TickType_t;
    #define portMAX_DELAY           (TickType_t) 0xffffffffUL
#endif

#define pdFALSE                     ((BaseType_t) 0)
#define pdTRUE                      ((BaseType_t) 1)
#define pdPASS                      (pdTRUE)
#define pdFAIL                      (pdFALSE)


#define portINITIAL_XPSR            (0x01000000)
#define portSTART_ADDRESS_MASK      ((StackType_t) 0xFFFFFFFEUL)

#define portNVIC_SHPR3_REG                    ( *( ( volatile uint32_t * ) 0xe000ed20 ) )
#define portNVIC_PENDSV_PRI         (((uint32_t)configKERNEL_INTERRUPT_PRIORITY) << 16UL)
#define portNVIC_SYSTICK_PRI        (((uint32_t)configKERNEL_INTERRUPT_PRIORITY) << 24UL)


static void prvStartFirstTask(void);

#define vPortSVCHandler             SVC_Handler

#define xPortPendSVHandler          PendSV_Handler

#define portNVIC_INT_CTRL_REG       (*((volatile uint32_t*)0xE000ED04))
#define portNVIC_PENDSVSET_BIT      (1UL << 28UL)
#define portSY_FULL_READ_WRITE    ( 15 )
// 触发 PendSV，产生上下文切换
#define portYIELD() \
{ \
    portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT; \
    __dsb(portSY_FULL_READ_WRITE); \
    __isb(portSY_FULL_READ_WRITE); \
};

#define portSET_INTERRUPT_MASK_FROM_ISR()       ulPortRaiseBASEPRI()
// 带返回值关中断，将当前中断状态作为返回值返回
static __inline uint32_t ulPortRaiseBASEPRI(void)
{
	uint32_t ulReturn,ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;
	__asm 
	{
		mrs ulReturn,basepri        // 保存中断时 BASEPRI 寄存器的值
		msr basepri,ulNewBASEPRI    // 屏蔽 优先级值 大于等于 11 的中断
		dsb
		isb
	}
	return ulReturn;
}

#define portDISABLE_INTERRUPTS()                vPortRaiseBASEPRI()
// 不带返回值关中断
static __inline void vPortRaiseBASEPRI(void)
{
	uint32_t ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;
	__asm 
	{
		msr basepri,ulNewBASEPRI
		dsb
		isb
	}
}


// 设置 BASEPRI 为 0 开所有中断
#define portENABLE_INTERRUPTS()                 vPortSetBASEPRI(0)
// 设置 BASEPRI 为进入中断时的值则恢复原来的中断状态
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x)    vPortSetBASEPRI(x)
// 开中断
static __inline void vPortSetBASEPRI(uint32_t ulBASEPRI)
{
	__asm volatile
	{
		msr basepri,ulBASEPRI       
	}
}

//引用临界段
extern void vPortEnterCritical(void);
extern void vPortExitCritical(void);
#define portENTER_CRITICAL()         vPortEnterCritical()
#define portEXIT_CRITICAL()          vPortExitCritical()

//时基服务函数
#define xPortSysTickHandler         SysTick_Handler

//记录任务优先级
#define portRECORD_READY_PRIORITY(uxPriority, uxReadyPriorities) (uxReadyPriorities) |= (1UL << (uxPriority))

//寻找当前的最高优先级
#define portGET_HIGHEST_PRIORITY(uxTopPriority, uxReadyPriorities) uxTopPriority = (31UL - (uint32_t) __clz((uxReadyPriorities)))
//0000 0000 0000 0000 0000 0000 0000 1000 -> clz -> 28

//清除对应优先级在优先级位图上的信息
#define portRESET_READY_PRIORITY(uxPriority, uxReadyPriorities) (uxReadyPriorities) &= ~(1UL << (uxPriority))

#endif
