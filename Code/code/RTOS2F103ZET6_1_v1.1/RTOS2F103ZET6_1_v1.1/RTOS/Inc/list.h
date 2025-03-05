#ifndef _LIST_H__
#define _LIST_H__

/*用于在RTOS中使用 volatile关键字，如果需要使用，则在FreeRTOSConfig.h定义
"#define configLIST_VOLATILE volatile"
*/
#ifndef configLIST_VOLATILE
	#define configLIST_VOLATILE
#endif /* configSUPPORT_CROSS_MODULE_OPTIMISATION */

#include "FreeRTOS.h" 
//头文件不支持嵌套复制 即FreeRTOS.h里面只有 #include "portMacro.h" 这句话，但不支持
//复制portMacro.h的内容
#include "portMacro.h"

#define listFIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE

#define listSECOND_LIST_ITEM_INTEGRITY_CHECK_VALUE

#define listFIRST_LIST_INTEGRITY_CHECK_VALUE

#define listSECOND_LIST_INTEGRITY_CHECK_VALUE

#define listSET_LIST_ITEM_OWNER( pxListItem, pxOwner )    ( ( pxListItem )->pvOwner = ( void * ) ( pxOwner ) )

#define listGET_LIST_ITEM_OWNER( pxListItem )             ( ( pxListItem )->pvOwner )

#define listSET_LIST_ITEM_VALUE( pxListItem, xValue )     ( ( pxListItem )->xItemValue = ( xValue ) )


#define listGET_LIST_ITEM_VALUE( pxListItem )             ( ( pxListItem )->xItemValue )


#define listGET_ITEM_VALUE_OF_HEAD_ENTRY( pxList )        ( ( ( pxList )->xListEnd ).pxNext->xItemValue )

#define listGET_HEAD_ENTRY( pxList )                      ( ( ( pxList )->xListEnd ).pxNext )


#define listGET_NEXT( pxListItem )                        ( ( pxListItem )->pxNext )


#define listGET_END_MARKER( pxList )                      ( ( ListItem_t const * ) ( &( ( pxList )->xListEnd ) ) )


#define listLIST_IS_EMPTY( pxList )                       ( ( ( pxList )->uxNumberOfItems == ( UBaseType_t ) 0 ) ? pdTRUE : pdFALSE )

#define listGET_OWNER_OF_NEXT_ENTRY( pxTCB, pxList )										\
{																							\
List_t * const pxConstList = ( pxList );													\
	/* Increment the index to the next item and return the item, ensuring */				\
	/* we don't return the marker used at the end of the list.  */							\
	( pxConstList )->pxIndex = ( pxConstList )->pxIndex->pxNext;							\
	if( ( void * ) ( pxConstList )->pxIndex == ( void * ) &( ( pxConstList )->xListEnd ) )	\
	{																						\
		( pxConstList )->pxIndex = ( pxConstList )->pxIndex->pxNext;						\
	}																						\
	( pxTCB ) = ( pxConstList )->pxIndex->pvOwner;											\
}

#define listGET_OWNER_OF_HEAD_ENTRY( pxList )            ( ( &( ( pxList )->xListEnd ) )->pxNext->pvOwner )

struct xLIST_ITEM
{
	// 检验一个 链表项 数据是否完整
    listFIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE
    // 排序值
    configLIST_VOLATILE TickType_t xItemValue;
    // 下一个 链表项
    struct xLIST_ITEM * configLIST_VOLATILE pxNext;
    // 前一个 链表项
    struct xLIST_ITEM * configLIST_VOLATILE pxPrevious;
    // 记录此 链表项 归谁拥有，通常是 TCB (任务控制块)
    void * pvOwner;
    // 拥有该 链表项 的 链表 
    struct xLIST * configLIST_VOLATILE pxContainer;
    // 检验一个 链表项 数据是否完整
    listSECOND_LIST_ITEM_INTEGRITY_CHECK_VALUE
};

struct xMINI_LIST_ITEM
{
	// 检验一个 MINI链表项 数据是否完整
    listFIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE
    // 排序值
    configLIST_VOLATILE TickType_t xItemValue;
    // 下一个 链表项
    struct xLIST_ITEM * configLIST_VOLATILE pxNext;
    // 前一个 链表项
    struct xLIST_ITEM * configLIST_VOLATILE pxPrevious;
};
typedef struct xMINI_LIST_ITEM MiniListItem_t;

typedef struct xLIST_ITEM ListItem_t;

typedef struct xLIST
{
	// 检验一个 链表 数据是否完整
    listFIRST_LIST_INTEGRITY_CHECK_VALUE 
    // 记录 链表 中 链表项 数目
    volatile UBaseType_t uxNumberOfItems;
    // 遍历 链表 的指针
    ListItem_t * configLIST_VOLATILE pxIndex;
    // 使用 MINI链表项 表示 链表尾部
    MiniListItem_t xListEnd;
    // 检验一个 链表 数据是否完整
    listSECOND_LIST_INTEGRITY_CHECK_VALUE
} List_t;

void vListInitialiseItem( ListItem_t * const pxItem );
void vListInitialise( List_t * const pxList );
void vListInsertEnd( List_t * const pxList, ListItem_t * const pxNewListItem );
void vListInsert( List_t * const pxList, ListItem_t * const pxNewListItem );


#endif
