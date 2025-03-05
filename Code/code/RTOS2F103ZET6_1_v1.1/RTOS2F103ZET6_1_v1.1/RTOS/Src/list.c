#include "FreeRTOS.h"

void vListInitialiseItem( ListItem_t * const pxItem )  
{  
    // 确保链表项未被记录在链表中
    pxItem->pxContainer = NULL;
}


void vListInitialise( List_t * const pxList )  
{  
    // 链表当前指针指向 xListEnd
    pxList->pxIndex = ( ListItem_t * ) &( pxList->xListEnd );
    
    // 设置链表尾链表项排序值为最大， 保证 xListEnd 会被放在链表的尾部
    pxList->xListEnd.xItemValue = portMAX_DELAY;
  
    // 尾链表项 xListEnd 的前/后链表项指针均指向自己
    pxList->xListEnd.pxNext = ( ListItem_t * ) &( pxList->xListEnd );
    pxList->xListEnd.pxPrevious = ( ListItem_t * ) &( pxList->xListEnd );
	// 初始化时链表中有 0 个链表项
    pxList->uxNumberOfItems = ( UBaseType_t ) 0U;
}

void vListInsertEnd( List_t * const pxList, ListItem_t * const pxNewListItem )  
{
	// 获取链表中当前指针 pxIndex 位置
	ListItem_t * const pxIndex = pxList->pxIndex;
	 
	// 1. 改变自身 pxNext 和 pxPrevious
    pxNewListItem->pxNext = pxIndex;
    pxNewListItem->pxPrevious = pxIndex->pxPrevious;
    
	// 2. 改变前一个链表项的 pxNext
    pxIndex->pxPrevious->pxNext = pxNewListItem;
    // 3. 改变后一个链表项的 pxPrevious
    pxIndex->pxPrevious = pxNewListItem;
	  
    // 标记新插入的链表项所在的链表 
    pxNewListItem->pxContainer = pxList;
	// 链表数量增加一
    ( pxList->uxNumberOfItems )++;
}

void vListInsert( List_t * const pxList, ListItem_t * const pxNewListItem )  
{
	ListItem_t *pxIterator;
	// 记录要插入的链表项的排序值
	const TickType_t xValueOfInsertion = pxNewListItem->xItemValue;
	
	// 如果新插入的链表项排序值为最大值，直接插到尾节点 xListEnd 的前面
	if( xValueOfInsertion == portMAX_DELAY )  
	{  
	    pxIterator = pxList->xListEnd.pxPrevious;  
	}
	else  
	{
		/*
		1. 遍历链表，将当前链表项 pxIterator 与要插入的新的链表项 pxNewListItem 
		的 xItemValue 值比较，直到 pxIterator 的 xItemValue 大于 pxNewListItem 
		的 xItemValue 值，此时 pxNewListItem 应该插入到 pxIterator 的后面
		*/
		for( pxIterator = ( ListItem_t * ) &( pxList->xListEnd ); 
			 pxIterator->pxNext->xItemValue <= xValueOfInsertion; 
			 pxIterator = pxIterator->pxNext ){}
	}
	// 2. 改变要插入链表项自身 pxNext 和 pxPrevious
	pxNewListItem->pxNext = pxIterator->pxNext;
	pxNewListItem->pxPrevious = pxIterator;
	// 3. 改变后一个链表项的 pxPrevious
	pxNewListItem->pxNext->pxPrevious = pxNewListItem;
	// 4. 改变前一个链表项的 pxNext
	pxIterator->pxNext = pxNewListItem;
	
	// 标记新插入的链表项所在的链表
	pxNewListItem->pxContainer = pxList;
	// 链表数量增加一
	( pxList->uxNumberOfItems )++;
}

UBaseType_t uxListRemove( ListItem_t * const pxItemToRemove )  
{  
	List_t * const pxList = pxItemToRemove->pxContainer;  
	
	// 1. 改变后一个链表项 pxPrevious
    pxItemToRemove->pxNext->pxPrevious = pxItemToRemove->pxPrevious;
    // 2. 改变前一个链表项 pxNext
    pxItemToRemove->pxPrevious->pxNext = pxItemToRemove->pxNext;
	  
    // 确保索引指向有效的项目
    if( pxList->pxIndex == pxItemToRemove )  
    {  
       pxList->pxIndex = pxItemToRemove->pxPrevious;  
    }
	
	// 从链表中移除链表项后，该链表项不属于任何链表
    pxItemToRemove->pxContainer = NULL;  
    // 链表中链表项的数量减一
    ( pxList->uxNumberOfItems )--;  
	
	// 返回链表中链表项的数量
    return pxList->uxNumberOfItems;  
}


