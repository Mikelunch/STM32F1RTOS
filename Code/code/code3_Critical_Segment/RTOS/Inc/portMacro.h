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

/*
与 port.h中 pxPortInitialiseStack初始化堆栈有关
*/
#define portINITIAL_XPSR            (0x01000000) //xPSR 寄存器初始值为 0x01000000
#define portSTART_ADDRESS_MASK      ((StackType_t) 0xFFFFFFFEUL) //32位寄存器地址掩码

/*
与PendSv和SysTick相关中断寄存器的定义
*/
#define portNVIC_SYSTICK_CTRL_REG             ( *( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG             ( *( ( volatile uint32_t * ) 0xe000e014 ) )
#define portNVIC_SYSTICK_CURRENT_VALUE_REG    ( *( ( volatile uint32_t * ) 0xe000e018 ) )
#define portNVIC_SHPR3_REG                    ( *( ( volatile uint32_t * ) 0xe000ed20 ) )
	
#define portNVIC_PENDSV_PRI                   ( ( ( uint32_t ) configKERNEL_INTERRUPT_PRIORITY ) << 16UL )
#define portNVIC_SYSTICK_PRI                  ( ( ( uint32_t ) configKERNEL_INTERRUPT_PRIORITY ) << 24UL )

#define vPortSVCHandler             SVC_Handler //给SVC固定的中断服务函数起个别名
#define xPortPendSVHandler          PendSV_Handler

// 触发 PendSV，产生上下文切换
#define portNVIC_INT_CTRL_REG       (*((volatile uint32_t*)0xE000ED04))
#define portNVIC_PENDSVSET_BIT      (1UL << 28UL)
#define portYIELD() \
{ \
    portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT; \
    __DSB(); \
    __ISB(); \
};

#endif
