#ifndef __FREERTOS_H__
#define __FREERTOS_H__

/* 该文件包含所有RTOS的头文件
2024/11/5
Constant_z
*/

#include "FreeRTOSConfig.h"
#include "list.h"
#include "portMacro.h"
#include "task.h"
#include "stm32f1xx_hal.h"

#define NULL 0

#define LED0_GPIO_PORT 			GPIOB 
#define LED1_GPIO_PORT 			GPIOE
#define LED0_PIN 						GPIO_PIN_5
#define LED1_PIN 						GPIO_PIN_5
#define SYSTICK_GPIO_PORT 	GPIOA
#define TIM3_GPIO_PORT 			GPIOA
#define SYSTICK_PIN 				GPIO_PIN_4
#define TIM3_PIN_PORT 			GPIO_PIN_5

// 设置内核中断优先级（最低优先级）
#define configKERNEL_INTERRUPT_PRIORITY         15

#endif
