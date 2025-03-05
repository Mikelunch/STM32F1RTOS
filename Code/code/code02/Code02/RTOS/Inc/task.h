#ifndef __TASK_H__
#define __TASK_H__

/* 任务管理头文件
2024/11/5
Constant_z
*/
#include "FreeRTOS.h"

// 任务句柄指针 用于表示任务句柄
typedef void* TaskHandle_t;
// 任务函数指针 用于表示任务函数s
typedef void (*TaskFunction_t)(void *);


/*任务控制模块TCB
2024/11/10
Constant_z
*/
typedef struct tskTaskControlBlock
{
    volatile StackType_t  *pxTopOfStack;                        // 栈顶
    ListItem_t            xStateListItem;                       // 任务节点->链表项
    StackType_t           *pxStack;                             // 任务栈起始地址
    char                  pcTaskName[configMAX_TASK_NAME_LEN];  // 任务名称 最多24个字符
}tskTCB;
typedef tskTCB TCB_t;

// 函数声明
TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,
                            const char* const pcName,
                            const uint32_t ulStackDepth,
                            void* const pvParameters,
                            StackType_t* const puxTaskBuffer,
                            TCB_t* const pxTaskBuffer);		
														
//初始化就绪链表
void prvInitialiseTaskLists(void);
//启动任务调度器
void vTaskStartScheduler(void);
// 任务切换
void vTaskSwitchContext(void);
// 主动产生任务调度
#define taskYIELD() portYIELD()

														
#endif
