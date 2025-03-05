#ifndef __FREERTOSCONFIG_H__
#define __FREERTOSCONFIG_H__


// 设置 TickType_t 类型位 16 位 
#define configUSE_16_BIT_TICKS                  0

// 任务名称字符串长度
#define configMAX_TASK_NAME_LEN                 24

// 是否支持静态方式创建任务
#define configSUPPORT_STATIC_ALLOCATION         1

// 设置 RTOS 可管理的最大中断优先级 11级以下
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    11 << 4

// 设置 RTOS 支持的最大优先级
#define configMAX_PRIORITIES                    5

#endif

