#include "FreeRTOS.h"

// �������
static void prvTaskExitError(void)
{
    for(;;);
}
// ��ʼ��ջ�ڴ�
StackType_t* pxPortInitialiseStack(StackType_t* pxTopOfStack, 
								   TaskFunction_t pxCode, 
								   void* pvParameters)
{
    // �쳣����ʱ���Զ����ص�CPU������
    pxTopOfStack --;
    *pxTopOfStack = portINITIAL_XPSR;
    pxTopOfStack --;
    *pxTopOfStack = ((StackType_t)pxCode) & portSTART_ADDRESS_MASK;
    pxTopOfStack --;
    *pxTopOfStack = (StackType_t)prvTaskExitError;
	
    // r12��r3��r2 �� r1 Ĭ�ϳ�ʼ��Ϊ 0
    pxTopOfStack -= 5;
    *pxTopOfStack = (StackType_t)pvParameters;
	
    // �쳣����ʱ���ֶ����ص� CPU ������
    pxTopOfStack -= 8;
	
    // ����ջ��ָ�룬��ʱ pxTopOfStack ָ�����ջ
    return pxTopOfStack;
}


// ����������
BaseType_t xPortStartScheduler(void)
{
	// ���� PendSV �� SysTick �ж����ȼ�Ϊ���
	portNVIC_SHPR3_REG |= portNVIC_PENDSV_PRI;
	//portNVIC_SHPR3_REG |= portNVIC_SYSTICK_PRI;
	
	// ��ʼ���δ�ʱ��
	if(SysTick_Config(SystemCoreClock / 1000)){
		while(1){}
	}
	
	// ������һ�����񣬲��ٷ���
	prvStartFirstTask();
	
	// �����������е�����
	return 0;
}

// SVC �жϷ�����
__asm void vPortSVCHandler(void)
{
    extern pxCurrentTCB;
	// 8 �ֽڶ���
    PRESERVE8
    // �� pxCurrentTCB ָ��ĵ�ַ���ص��Ĵ��� r3
    ldr r3,=pxCurrentTCB
    // �� pxCurrentTCB ָ��ָ��ĵ�ַ���洢 TCB �ĵ�ַ�����ص��Ĵ��� r1
    ldr r1,[r3]
    // �� pxCurrentTCB ָ��ָ��ĵ�ַ���ֵ����ǰ���е� TCB �ṹ�壩���ص��Ĵ��� r0
    // Ҳ���Ĵ��� r0 �д洢�˵�ǰ���е������ջ��ָ���ַ
    ldr r0,[r1]
    // �ԼĴ��� r0 Ϊ����ַ��������ջ�д洢��ֵ���ص��Ĵ��� r4 ~ r11 ��
    // ͬʱ�Ĵ��� r0 ��ֵ���Զ�����
    ldmia r0!,{r4-r11}
    // �����º�ļĴ��� r0 ��ֵ���ظ� psp
    msr psp,r0
    isb
    // ���ж�
    mov r0,#0
    msr basepri,r0
    // ָʾ���������쳣����ʱʹ�� psp ��Ϊ��ջָ��
    orr r14,#0xd
	// �쳣���أ��Զ�����ʣ�µ� xPSR��PC��R14��R12 �� R3 ~ R0 �Ĵ���
    bx r14
}

/* port.c */
__asm void prvStartFirstTask(void)
{
	// 8 �ֽڶ���
    PRESERVE8
	
	// �� cortex-M �У�0xE000ED08 �� SCB_VTOR ����Ĵ����ĵ�ַ��
	// ����ŵ������������ʼ��ַ �����һ��Ϊ __initial_sp��Ҳ�� msp �ĳ�ʼ��ַ 
    ldr r0,=0xE000ED08
    ldr r0,[r0]
    ldr r0,[r0]
	// ������ջָ�� msp ��ֵ
    msr msp,r0
    // ����ȫ���ж�
    cpsie i
    cpsie f
    dsb
    isb
    // ���� SVC ������һ������
    svc 0
    nop
    nop
}

// PendSV �жϷ�����������ʵ�������л��ĺ���
__asm void xPortPendSVHandler(void)
{
	// ֻҪ���� PendSV �жϣ��Ĵ��� xPSR��PC��R14��R12 �� R3 ~ R0��
	// �� 8 ���Ĵ�����ֵ���Զ��� psp ָ��ָ��ĵ�ַ��ʼ���ص�����ջ��
	// ���� psp ָ��Ҳ���Զ�����
    extern pxCurrentTCB;
    extern vTaskSwitchContext;
	
    PRESERVE8
	// �� psp ��ֵ�洢���Ĵ��� r0 �� 
    mrs r0,psp
    isb
	// �� pxCurrentTCB �ĵ�ַ�洢���Ĵ��� r3 ��
    ldr r3,=pxCurrentTCB
    // ���Ĵ��� r3 �洢�ĵ�ַ������ݴ洢���Ĵ��� r2 ��
    // ��ʱ�Ĵ��� r2 �д洢�˵�ǰִ������� TCB ��ַ
    ldr r2,[r3]
	// �ԼĴ��� r0 Ϊ����ַ��r0 �����ڴ洢�� psp ָ���ֵ��
	// ���Ĵ��� r11 ~ r4 �д洢��ֵ�洢������ջ
	// ���� r0 �д洢�ĵ�ַָ��Ҳ���Զ�����
    stmdb r0!,{r4-r11}
    // ���Ĵ��� r0 �е����ݴ洢����һ��ִ������� TCB ��ַ
    // Ҳ����һ�������ջ��ָ�� pxTopOfStack ָ��Ĵ��� r0 ���ڴ洢�ĵ�ַ
    str r0,[r2]
	// ���ı������
	// ��ʼ��������
	// ���Ĵ��� r3 �� r14 ѹ��ջ
    stmdb sp!,{r3,r14}
    // ���� ���ȼ�ֵ ���ڻ��ߵ��� 11 ���ж�
    mov r0,#configMAX_SYSCALL_INTERRUPT_PRIORITY
    msr basepri,r0
    dsb
    isb
    // ��ת�� vTaskSwitchContext �������õ���һ��Ҫִ�������ָ�� pxCurrentTCB
    bl vTaskSwitchContext
    // ���ж�
    mov r0,#0
    msr basepri,r0
    // �ָ��Ĵ��� r3 �� r14
    ldmia sp!,{r3,r14}
	
	// �� pxCurrentTCB ���ص��Ĵ��� r1 
    ldr r1,[r3]
    // ����һ��Ҫִ�������ջ��ָ����ص��Ĵ��� r0 
    ldr r0,[r1]
    // �ԼĴ��� r0 Ϊ����ַ��r0 �����ڴ洢�������ջ��ָ�룩
    // ������ջ�д洢��ֵ���ص��Ĵ��� r4 ~ r11 ��
    // ͬʱ r0 �е�ֵ���Զ�����
    ldmia r0!,{r4-r11}
    // �����º�� r0 ��ֵ���ص� psp
    msr psp,r0
    isb
    // �쳣�˳����Զ�����ʣ�µ� xPSR��PC��R14��R12 �� R3 ~ R0 �Ĵ���
    bx r14
    nop
}


// �ж�Ƕ�׼�����
static UBaseType_t uxCriticalNesting = 0xAAAAAAAA;
// �����ٽ���
void vPortEnterCritical(void)
{
	portDISABLE_INTERRUPTS();
	uxCriticalNesting++;
	if(uxCriticalNesting==1)
	{
		// configASSERT((portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK) == 0);
	}
}
// �˳��ٽ���
void vPortExitCritical(void)
{
	// configASSERT(uxCriticalNesting);
	uxCriticalNesting--;
	if(uxCriticalNesting == 0)
	{
		portENABLE_INTERRUPTS();
	}
}

// SysTick �жϷ�����
void xPortSysTickHandler(void)
{	
    // ���ж�
    vPortRaiseBASEPRI();
    // ����������ʱ����
    if(xTaskIncrementTick()!= pdFALSE){
			portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
		}
    // ���ж�
    vPortSetBASEPRI(0);
		
}
