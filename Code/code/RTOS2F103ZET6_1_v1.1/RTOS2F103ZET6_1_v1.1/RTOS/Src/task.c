#include "FreeRTOS.h"

// ������������
List_t pxReadyTasksLists[configMAX_PRIORITIES];

// ��ǰ TCB ָ��
TCB_t volatile *pxCurrentTCB = NULL;
// ʹ�õ��ⲿ��������
extern BaseType_t xPortStartScheduler(void);

// �� main.c �ж����������������
extern TCB_t Task1TCB;
extern TCB_t Task2TCB;

/* task.c */
// 32λ�����ȼ�λͼ��Ĭ��ȫ 0 ����¼�����д��ڵ����ȼ�
static volatile UBaseType_t uxTopReadyPriority = 0;

// �����������ȼ���λ���ȼ�λͼ
#define taskRECORD_READY_PRIORITY(uxPriority)	portRECORD_READY_PRIORITY(uxPriority, uxTopReadyPriority)

// �����������ȼ�������񵽶�Ӧ�ľ�������
#define prvAddTaskToReadyList(pxTCB) \
	taskRECORD_READY_PRIORITY((pxTCB)->uxPriority); \
	vListInsertEnd(&(pxReadyTasksLists[(pxTCB)->uxPriority]), \
	               &((pxTCB)->xStateListItem)); \

// �ҵ������б�������ȼ������񲢸��µ� pxCurrentTCB
#define taskSELECT_HIGHEST_PRIORITY_TASK() \
{ \
	UBaseType_t uxTopPriority; \
	/* Ѱ��������ȼ� */ \
	portGET_HIGHEST_PRIORITY(uxTopPriority, uxTopReadyPriority); \
	/* ��ȡ���ȼ���ߵľ�������� TCB��Ȼ����µ� pxCurrentTCB */ \
	listGET_OWNER_OF_NEXT_ENTRY(pxCurrentTCB, \
	                            &(pxReadyTasksLists[uxTopPriority])); \
}

// �����������ȼ�������ȼ�λͼ
#define taskRESET_READY_PRIORITY(uxPriority) \
{ \
	portRESET_READY_PRIORITY((uxPriority), (uxTopReadyPriority)); \
}


// �����������
TCB_t IdleTaskTCB;
#define confgiMINIMAL_STACK_SIZE 128
StackType_t	IdleTasKStack[confgiMINIMAL_STACK_SIZE];

// ������������
void prvIdleTask(void *p_arg)
{
    for(;;){
			
		}
}



// �������������
void vTaskStartScheduler(void)
{
	// ������������
	TaskHandle_t xIdleTaskHandle = xTaskCreateStatic((TaskFunction_t)prvIdleTask,
										(char *)"IDLE",
										(uint32_t)confgiMINIMAL_STACK_SIZE,
										(void *)NULL,
										 (UBaseType_t)taskIDLE_PRIORITY,
										(StackType_t *)IdleTasKStack,
										(TCB_t *)&IdleTaskTCB);
	
	if(xPortStartScheduler() != pdFALSE){}
}

// ������ʱ����
void vTaskDelay(const TickType_t xTicksToDelay)
{
    TCB_t *pxTCB = NULL;

    // ��ȡ��ǰҪ��ʱ������ TCB
    pxTCB = (TCB_t *)pxCurrentTCB;
    // ��¼��ʱʱ��
    pxTCB->xTicksToDelay = xTicksToDelay;
		// ����������ȼ�λͼ���������������ʱ�Ͳ����ҵ�������
    taskRESET_READY_PRIORITY(pxTCB->uxPriority);
    // ��������������ȣ��ó� MCU 
    taskYIELD();
}


// ��������ʼ������
void prvInitialiseTaskLists(void)
{
	UBaseType_t uxPriority;
	// ��ʼ��������������
	for(uxPriority = (UBaseType_t)0U;uxPriority < (UBaseType_t)configMAX_PRIORITIES; uxPriority++)
	{
		vListInitialise(&(pxReadyTasksLists[uxPriority]));
	}
}


// ȫ�����������
static volatile UBaseType_t uxCurrentNumberOfTasks = (UBaseType_t)0U;

// ������񵽾���������
static void prvAddNewTaskToReadyList(TCB_t* pxNewTCB)
{
	// �����ٽ��
	taskENTER_CRITICAL();
	{
		// ȫ�������������һ����
		uxCurrentNumberOfTasks++;
			
		// ��� pxCurrentTCB Ϊ�գ��� pxCurrentTCB ָ���´���������
		if(pxCurrentTCB == NULL)
		{
			pxCurrentTCB = pxNewTCB;
			// ����ǵ�һ�δ�����������Ҫ��ʼ��������ص��б�
			if(uxCurrentNumberOfTasks == (UBaseType_t)1)
			{
				// ��ʼ��������ص��б�
				prvInitialiseTaskLists();
			}
		}
		else 
		// ���pxCurrentTCB��Ϊ��
		// �������������ȼ��� pxCurrentTCB ָ��������ȼ������ TCB 
		{
			if(pxCurrentTCB->uxPriority <= pxNewTCB->uxPriority)
			{
				pxCurrentTCB = pxNewTCB;
			}
		}
		
		// ��������ӵ������б�
		prvAddTaskToReadyList(pxNewTCB);
	}
	// �˳��ٽ��
	taskEXIT_CRITICAL();
}


// ʹ�õ��ⲿ��������
extern StackType_t* pxPortInitialiseStack(StackType_t* pxTopOfStack, 
                                          TaskFunction_t pxCode, 
                                          void* pvParameters);
// �����Ĵ���������																 
static void prvInitialiseNewTask(TaskFunction_t pxTaskCode,    // ������
                            const char* const pcName,          // ��������
                            const uint32_t ulStackDepth,       // ����ջ���
                            void* const pvParameters,          // �������
														UBaseType_t uxPriority,           // ���ȼ�
                            TaskHandle_t* const pxCreatedTask, // ������
                            TCB_t* pxNewTCB)                   // ����ջ����ָ��
{
	StackType_t *pxTopOfStack;
	UBaseType_t x;
	
	// ջ��ָ�룬����ָ����������ջ�ռ����ߵ�ַ
	pxTopOfStack = pxNewTCB->pxStack + (ulStackDepth - (uint32_t)1);
	// 8 �ֽڶ���
	pxTopOfStack = (StackType_t*)(((uint32_t)pxTopOfStack) 
	                             & (~((uint32_t)0x0007)));
	// �����������Ƶ�TCB��
	for(x = (UBaseType_t)0;x < (UBaseType_t)configMAX_TASK_NAME_LEN;x++)
	{
		pxNewTCB->pcTaskName[x] = pcName[x];
		if(pcName[x] == 0x00)
			break;
	}
	pxNewTCB->pcTaskName[configMAX_TASK_NAME_LEN-1] = '\0';
	
	// ��ʼ��������
	vListInitialiseItem(&(pxNewTCB->xStateListItem));
	
	// ���ø��������ӵ����Ϊ pxNewTCB
	listSET_LIST_ITEM_OWNER(&(pxNewTCB->xStateListItem), pxNewTCB);
	
	// ��ʼ�����ȼ�
	if(uxPriority >= (UBaseType_t)configMAX_PRIORITIES)
	{
		uxPriority = (UBaseType_t)configMAX_PRIORITIES - (UBaseType_t)1U;
	}
	pxNewTCB->uxPriority = uxPriority;

		
	// ��ʼ������ջ
	pxNewTCB->pxTopOfStack = 
	          pxPortInitialiseStack(pxTopOfStack, pxTaskCode, pvParameters);
	
	if((void*)pxCreatedTask != NULL)
	{
	    *pxCreatedTask = (TaskHandle_t)pxNewTCB;
	}
}

// ��̬����������
#if (configSUPPORT_STATIC_ALLOCATION == 1)
TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,     // ������
                            const char* const pcName,         // ��������
                            const uint32_t ulStackDepth,      // ����ջ���
                            void* const pvParameters,         // �������
														UBaseType_t uxPriority,           // ���ȼ�
                            StackType_t* const puxTaskBuffer, // ����ջ��ʼָ��
                            TCB_t* const pxTaskBuffer)        // ����ջ����ָ��
{
	TCB_t* pxNewTCB;
	TaskHandle_t xReturn;
	// ����ջ����ָ�������ջ��ʼָ�벻Ϊ��
	if((pxTaskBuffer != NULL) && (puxTaskBuffer != NULL))
	{
		pxNewTCB = (TCB_t*)pxTaskBuffer;
		// ��������ƿ�� pxStack ָ��ָ������ջ��ʼ��ַ
		pxNewTCB->pxStack = (StackType_t*)puxTaskBuffer;
		
		// �����Ĵ���������
		prvInitialiseNewTask(pxTaskCode,
							 pcName,
							 ulStackDepth,
							 pvParameters,
							 uxPriority,
							 &xReturn,
							 pxNewTCB);
		// �����������Զ���������ӵ���������
		prvAddNewTaskToReadyList(pxNewTCB);
	}
	else
	{
		xReturn = NULL;
	}
	// ���񴴽��ɹ���Ӧ�÷��������������򷵻� NULL
	return xReturn;
}
#endif


// ������Ⱥ���
void vTaskSwitchContext(void)
{
    taskSELECT_HIGHEST_PRIORITY_TASK();
}

// �δ�ʱ������ֵ
static volatile TickType_t xTickCount = (TickType_t)0U;
// ����������ʱ����
BaseType_t xTaskIncrementTick(void)
{
	TCB_t *pxTCB = NULL;
	uint8_t i =0;
	BaseType_t xSwitchRequired = pdFALSE;
	
	// ���� xTickCount ϵͳʱ��������
	const TickType_t xConstTickCount = xTickCount + 1;
	xTickCount = xConstTickCount;
	
	// ɨ������б�����������,�����ʱʱ�䲻Ϊ 0 ��� 1 
	
	for(i=0; i<configMAX_PRIORITIES; i++)
	{
		//������Ϊ�յ�����·���
		if( listLIST_IS_EMPTY( &pxReadyTasksLists[i]) == pdFALSE ){
			pxTCB = (TCB_t *)listGET_OWNER_OF_HEAD_ENTRY((&pxReadyTasksLists[i]));
			if(pxTCB->xTicksToDelay > 0)
			{
				pxTCB->xTicksToDelay--;
			}
			
			// ��ʱʱ�䵽�����������
			if( pxTCB->xTicksToDelay == 0)
			{ 
				taskRECORD_READY_PRIORITY(pxTCB->uxPriority);
				xSwitchRequired = pdTRUE;
			}
		}	
	}
	return xSwitchRequired;
}


