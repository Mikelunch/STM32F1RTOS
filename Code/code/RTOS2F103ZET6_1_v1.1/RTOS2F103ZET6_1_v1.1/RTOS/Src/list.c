#include "FreeRTOS.h"

void vListInitialiseItem( ListItem_t * const pxItem )  
{  
    // ȷ��������δ����¼��������
    pxItem->pxContainer = NULL;
}


void vListInitialise( List_t * const pxList )  
{  
    // ����ǰָ��ָ�� xListEnd
    pxList->pxIndex = ( ListItem_t * ) &( pxList->xListEnd );
    
    // ��������β����������ֵΪ��� ��֤ xListEnd �ᱻ���������β��
    pxList->xListEnd.xItemValue = portMAX_DELAY;
  
    // β������ xListEnd ��ǰ/��������ָ���ָ���Լ�
    pxList->xListEnd.pxNext = ( ListItem_t * ) &( pxList->xListEnd );
    pxList->xListEnd.pxPrevious = ( ListItem_t * ) &( pxList->xListEnd );
	// ��ʼ��ʱ�������� 0 ��������
    pxList->uxNumberOfItems = ( UBaseType_t ) 0U;
}

void vListInsertEnd( List_t * const pxList, ListItem_t * const pxNewListItem )  
{
	// ��ȡ�����е�ǰָ�� pxIndex λ��
	ListItem_t * const pxIndex = pxList->pxIndex;
	 
	// 1. �ı����� pxNext �� pxPrevious
    pxNewListItem->pxNext = pxIndex;
    pxNewListItem->pxPrevious = pxIndex->pxPrevious;
    
	// 2. �ı�ǰһ��������� pxNext
    pxIndex->pxPrevious->pxNext = pxNewListItem;
    // 3. �ı��һ��������� pxPrevious
    pxIndex->pxPrevious = pxNewListItem;
	  
    // ����²�������������ڵ����� 
    pxNewListItem->pxContainer = pxList;
	// ������������һ
    ( pxList->uxNumberOfItems )++;
}

void vListInsert( List_t * const pxList, ListItem_t * const pxNewListItem )  
{
	ListItem_t *pxIterator;
	// ��¼Ҫ����������������ֵ
	const TickType_t xValueOfInsertion = pxNewListItem->xItemValue;
	
	// ����²��������������ֵΪ���ֵ��ֱ�Ӳ嵽β�ڵ� xListEnd ��ǰ��
	if( xValueOfInsertion == portMAX_DELAY )  
	{  
	    pxIterator = pxList->xListEnd.pxPrevious;  
	}
	else  
	{
		/*
		1. ������������ǰ������ pxIterator ��Ҫ������µ������� pxNewListItem 
		�� xItemValue ֵ�Ƚϣ�ֱ�� pxIterator �� xItemValue ���� pxNewListItem 
		�� xItemValue ֵ����ʱ pxNewListItem Ӧ�ò��뵽 pxIterator �ĺ���
		*/
		for( pxIterator = ( ListItem_t * ) &( pxList->xListEnd ); 
			 pxIterator->pxNext->xItemValue <= xValueOfInsertion; 
			 pxIterator = pxIterator->pxNext ){}
	}
	// 2. �ı�Ҫ�������������� pxNext �� pxPrevious
	pxNewListItem->pxNext = pxIterator->pxNext;
	pxNewListItem->pxPrevious = pxIterator;
	// 3. �ı��һ��������� pxPrevious
	pxNewListItem->pxNext->pxPrevious = pxNewListItem;
	// 4. �ı�ǰһ��������� pxNext
	pxIterator->pxNext = pxNewListItem;
	
	// ����²�������������ڵ�����
	pxNewListItem->pxContainer = pxList;
	// ������������һ
	( pxList->uxNumberOfItems )++;
}

UBaseType_t uxListRemove( ListItem_t * const pxItemToRemove )  
{  
	List_t * const pxList = pxItemToRemove->pxContainer;  
	
	// 1. �ı��һ�������� pxPrevious
    pxItemToRemove->pxNext->pxPrevious = pxItemToRemove->pxPrevious;
    // 2. �ı�ǰһ�������� pxNext
    pxItemToRemove->pxPrevious->pxNext = pxItemToRemove->pxNext;
	  
    // ȷ������ָ����Ч����Ŀ
    if( pxList->pxIndex == pxItemToRemove )  
    {  
       pxList->pxIndex = pxItemToRemove->pxPrevious;  
    }
	
	// ���������Ƴ�������󣬸�����������κ�����
    pxItemToRemove->pxContainer = NULL;  
    // �������������������һ
    ( pxList->uxNumberOfItems )--;  
	
	// ���������������������
    return pxList->uxNumberOfItems;  
}


