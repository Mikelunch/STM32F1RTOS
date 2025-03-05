#include "FreeRTOS.h"
#include "task.h"


/*��������������������
2024/11/11
Constant_Z
*/
// ��������
List_t pxReadyTasksLists;

/*���õ�ǰ����ָ����������
2024/11/12
Constant_Z
*/
extern TCB_t Task1TCB; 
extern TCB_t Task2TCB; //main.c����
extern BaseType_t xPortStartScheduler(void); //port.c����

//��ǰ����ָ��
TCB_t volatile *pxCurrentTCB = NULL;

// ��ʼ����������ʹ�����ʹ��
void prvInitialiseTaskLists(void)
{
	vListInitialise(&pxReadyTasksLists);
}

/* ����������
��������ʼ��һ�����񣬲�������ջ�ڴ�ȳ�Ա�������г�ʼ��s
2024/11/10
Constant_Z
*/
// ʹ�õ��ⲿ��������
extern StackType_t* pxPortInitialiseStack(StackType_t* pxTopOfStack, 
                                          TaskFunction_t pxCode, 
                                          void* pvParameters);
// �����Ĵ���������																 
static void prvInitialiseNewTask(TaskFunction_t pxTaskCode,    // ������
                            const char* const pcName,          // ��������
                            const uint32_t ulStackDepth,       // ����ջ���
                            void* const pvParameters,          // �������
                            TaskHandle_t* const pxCreatedTask, // ������
                            TCB_t* pxNewTCB)                   // ����ջ����ָ��
{
	StackType_t *pxTopOfStack;//
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
	pxNewTCB->pcTaskName[configMAX_TASK_NAME_LEN-1] = '\0';//������ֹ��
	
	// ��ʼ��������
	vListInitialiseItem(&(pxNewTCB->xStateListItem));
	
	// ���ø��������ӵ����Ϊ pxNewTCB
	listSET_LIST_ITEM_OWNER(&(pxNewTCB->xStateListItem), pxNewTCB);
	
	// ��ʼ������ջ
	pxNewTCB->pxTopOfStack = 
	          pxPortInitialiseStack(pxTopOfStack, pxTaskCode, pvParameters);
	
	if((void*)pxCreatedTask != NULL)
	{
	    *pxCreatedTask = (TaskHandle_t)pxNewTCB;
	}
}

/* ��̬����������
���������о�̬��ʼ��һ������
2024/11/10
Constant_Z
*/
#if (configSUPPORT_STATIC_ALLOCATION == 1)
TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,     // ������
                            const char* const pcName,         // ��������
                            const uint32_t ulStackDepth,      // ����ջ���
                            void* const pvParameters,         // �������
                            StackType_t* const puxTaskBuffer, // ����ջ��ʼָ��
                            TCB_t* const pxTaskBuffer)        // ����ջ����ָ��
{
	TCB_t* pxNewTCB;
	TaskHandle_t xReturn;
	// ����ջ����ָ�������ջ��ʼָ�벻Ϊ�� ��ʼ��������
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
							 &xReturn, 
							 pxNewTCB);
	}
	else //����ʧ��
	{
		xReturn = NULL;
	}
	// ���񴴽��ɹ���Ӧ�÷��������������򷵻� NULL
	return xReturn;
}
#endif

/*�����������غ���
2024/11/12
Constant_z
*/
// ��������������
void vTaskStartScheduler(void)
{
	// Ŀǰ��֧�����ȼ����������ֶ�ָ����һ�����е�����
	pxCurrentTCB = &Task1TCB;
	// ����������
	if(xPortStartScheduler() != pdFALSE)//����������ʧ��
	{
		
	}
}

/* task.c */
// ������Ⱥ���
void vTaskSwitchContext(void)
{
	// �������������л�
	if(pxCurrentTCB == &Task1TCB)
	{
		pxCurrentTCB = &Task2TCB;
	}
	else
	{
		pxCurrentTCB = &Task1TCB;
	}
}



