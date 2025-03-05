#ifndef __FREERTOSCONFIG_H__
#define __FREERTOSCONFIG_H__

/* 用来配置裁剪 RTOS 的功能
2024/11/5
Constant_z
*/
// 设置 TickType_t 类型位 32 位 
#define configUSE_16_BIT_TICKS                  0
// 任务名称字符串长度
#define configMAX_TASK_NAME_LEN                 24
// 是否支持静态方式创建任务
#define configSUPPORT_STATIC_ALLOCATION         1

/*中断嵌套行为配置*/
#define configKERNEL_INTERRUPT_PRIORITY       15 //直接写死最低中断优先级为15   

// 设置 RTOS 可管理的最大中断优先级
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    11<<4

// 设置 RTOS 可管理的最大中断优先级
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    11<<4


#endif

