#include "FreeRTOS.h"

// �������
static void prvTaskExitError(void)
{
    for(;;);
}

static void prvStartFirstTask(void);
/*��ʼ��ջ�ڴ�
���������������Ҫ��Ϣд��ջ�У���ֹCPU�л������ʧ
2024/11/10
Constant_z
*/
StackType_t* pxPortInitialiseStack(StackType_t* pxTopOfStack, /*����ջ��ָ��*/
								   TaskFunction_t pxCode, /* ��������ַ*/
								   void* pvParameters) /*�������������*/
{
		/*����Cortex�ں� ������ͬ���ȡ����ͬ�Ĵ�����ֵ �ο�M3�����ֲ��ж��½�*/
    // �쳣����ʱ���Զ����ص�CPU������
    pxTopOfStack --;
		/* ��ջ��ѹ�룺xPSR �Ĵ�����ʼֵΪ 0x01000000 */
    *pxTopOfStack = portINITIAL_XPSR;
    pxTopOfStack --;
		/* ��ջ��ѹ��PC�Ĵ������������ĵ�ַ��PC �Ĵ����� ȷ�����һλΪ0 Ϊ���ڴ����*/
    *pxTopOfStack = ((StackType_t)pxCode) & portSTART_ADDRESS_MASK;
    pxTopOfStack --;
		/* ��ջ��ѹ�룺��������˳�������ַ��LR �Ĵ����� */
    *pxTopOfStack = (StackType_t)prvTaskExitError;
	
    // Ԥ�����Ĵ���r12��r3��r2 �� r1 Ĭ�ϳ�ʼ��Ϊ 0
    pxTopOfStack -= 5;
		// ��ջѹ�룺�������Ĵ��������R0 �Ĵ�����
    *pxTopOfStack = (StackType_t)pvParameters;
	
    /* Ϊ R11��R10��R9��R8��R7��R6��R5��R4 �Ĵ���Ԥ���ռ� */
    pxTopOfStack -= 8;
	
    // ����ջ��ָ�룬��ʱ pxTopOfStack ָ�����ջ
    return pxTopOfStack;
}

/*����������
����������PendSv��SysTick�ж����ȼ����
2024/11/12
Constant_z
*/
// ����������
BaseType_t xPortStartScheduler(void)
{
	// ���� PendSV �� SysTick �ж����ȼ�Ϊ���
	portNVIC_SHPR3_REG |= portNVIC_PENDSV_PRI;
	portNVIC_SHPR3_REG |= portNVIC_SYSTICK_PRI;
	
	// ��ʼ���δ�ʱ��
	
	// ������һ�����񣬲��ٷ���
	prvStartFirstTask();
	
	// �����������е�����
	return 0;
}

// ������һ������,ʵ�����Ǵ���SVC�ж�
//_asm��Ϊ�궨�壬__ASM����������Keil�������в���ARMָ����
__asm void prvStartFirstTask(void)
{	
	//��ϸ��ARMָ��ֱ�ӿ�ARM��ػ���鼮
	// 8�ֽڶ���
    PRESERVE8
	
    /* 
	�� cortex-M �У�0xE000ED08 �� SCB_VTOR ����Ĵ����ĵ�ַ��
	����ŵ�������������������ʼ��ַ���� msp �ĵ�ַ 
	*/
    ldr r0,=0xE000ED08
    ldr r0,[r0]
    ldr r0,[r0]
	
	// ������ջָ��msp��ֵ
    msr msp,r0
	
    // ����ȫ���ж�
    cpsie i //ʹ��IRQ�ж�
    cpsie f //ʹ�ܿ����ж� FIQ�ж�
    dsb   //����ͬ��ָ����ȷ��ִ�и�ָ��ǰ�������ݷ����Ѿ����
    isb   //ˢ��ָ����ˮ��
    // ����SVC������һ������
    svc 0
    nop
    nop
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
    // �����º�ļĴ��� r0 ��ֵ���ظ� psp(���⹦�ܼĴ������̶�ջָ��)
		//�ı䵱ǰ����Ķ�ջָ�룬�Ӷ��ı������ִ��������
		//PSP �� Cortex-M ϵ�д������������߳�ģʽ������Ĵ�����������ÿ������ӵ���Լ��Ķ�ջ��
		//������ջָ�루Main Stack Pointer��MSP�������
    msr psp,r0
    isb
    // ���ж�
		//Base Priority�Ĵ�������ȷ���жϵ����ȼ���������ж�ͬʱ����ʱ�������������Base Priority�Ĵ�����ֵ
		//�͸����жϵ����ȼ����������ȴ����ĸ��ж�
		//��Ȼ����ָ�����������ȼ�����0���жϣ������������ж� Ϊ����ִ�������������ʱ�򲻱������жϴ���
    mov r0,#0
    msr basepri,r0
    // ָʾ���������쳣����ʱʹ�� psp ��Ϊ��ջָ��
    orr r14,#0xd
	// �Զ�����ʣ�µ� xPSR��PC��R14��R12 �� R3 ~ R0 �Ĵ���
    bx r14
}

/* port.c */
// PendSV �жϷ�����������ʵ�������л��ĺ���
__asm void xPortPendSVHandler(void)
{
	// ֻҪ���� PendSV �жϣ��Ĵ��� xPSR��PC��R14��R12 �� R3 ~ R0��
	// �� 8 ���Ĵ�����ֵ���Զ��� psp ָ��ָ��ĵ�ַ��ʼ���ص�����ջ��
	// ���� psp ָ��Ҳ���Զ�����
	
		/*������Ҫ��ȫ�ֱ���*/
    extern pxCurrentTCB;
    extern vTaskSwitchContext; 
	
    PRESERVE8
		// �� psp ��ֵ�洢���Ĵ��� r0 �� ����ǰ�������������ջָ�� 
    mrs r0,psp
    isb
		// �� pxCurrentTCB �ĵ�ַ�洢���Ĵ��� r3 �� ��ָ��ǰ����������ƿ��ָ��
    ldr r3,=pxCurrentTCB
    // ���Ĵ��� r3 �洢�ĵ�ַ������ݴ洢���Ĵ��� r2 ��
    // ��ʱ�Ĵ��� r2 �д洢�˵�ǰִ������� TCB ��ַ ����ǰ����������ƿ���׵�ַ 
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
		// ���Ĵ��� r3 �� r14 ��ջ��MSPָ���ջ��
    stmdb sp!,{r3,r14}
    // ���� ���ȼ�ֵ ���ڻ��ߵ��� 11 ���ж� ��FreeRTOSConfig.h�ж��� ��������RTOS���Թ���Ͳ��ɹ�����ж�
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



