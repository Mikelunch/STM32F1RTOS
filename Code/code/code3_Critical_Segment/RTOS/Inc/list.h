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

/*
 * Access macro to set the owner of a list item.  The owner of a list item
 * is the object (usually a TCB) that contains the list item.
 *
 * \page listSET_LIST_ITEM_OWNER listSET_LIST_ITEM_OWNER
 * \ingroup LinkedList
 * 快速设置 pxListItem的pxOwner属于哪个链表
 */
#define listSET_LIST_ITEM_OWNER( pxListItem, pxOwner )		( ( pxListItem )->pvOwner = ( void * ) ( pxOwner ) )

// 设置 pxListItem 的 xValue 成员值
#define listSET_LIST_ITEM_VALUE( pxListItem, xValue )

/*获取相关的宏定义*/
// 获取 pxListItem 的 pxOwner 成员
#define listGET_LIST_ITEM_OWNER( pxListItem )

// 获取 pxListItem 的 xValue 成员值
#define listGET_LIST_ITEM_VALUE( pxListItem )

// 获取链表中头链表项的 xItemValue 成员值
#define listGET_ITEM_VALUE_OF_HEAD_ENTRY( pxList )

// 获取链表中头链表项地址
#define listGET_HEAD_ENTRY( pxList )

// 获取某个链表项的下一个链表项地址
#define listGET_NEXT( pxListItem )

// 获取链表中 xListEnd 的地址
#define listGET_END_MARKER( pxList )

// 获取链表当前长度
#define listCURRENT_LIST_LENGTH( pxList )

// 将链表中 pxIndex 指向下一个链表项，用于获取下一个链表项（任务）
#define listGET_OWNER_OF_NEXT_ENTRY( pxTCB, pxList )

// 获取链表中头链表项的 pvOwner 成员
#define listGET_OWNER_OF_HEAD_ENTRY( pxList )

// 获取链表项的 pxContainer 成员
#define listLIST_ITEM_CONTAINER( pxListItem )

/*判断相关的宏定义*/

// 判断链表是否为空
#define listLIST_IS_EMPTY( pxList )

// 判断链表项是否和链表匹配（链表项是否在该链表中）
#define listIS_CONTAINED_WITHIN( pxList, pxListItem )

// 判断链表是否被初始化
#define listLIST_IS_INITIALISED( pxList )


/* 双向链表头文件
2024/11/5
ConstantZ
*/

/*链表项数据结构体
2024/11/6
ConstantZ
*/
struct xLIST_ITEM
{
	// 检验一个 链表项 数据是否完整
    //listFIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE
    // 排序值 用于按照升序对链表中的链表项进行排序
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
    //listSECOND_LIST_ITEM_INTEGRITY_CHECK_VALUE
};
typedef struct xLIST_ITEM ListItem_t;

/*MINI链表项
2024/11/6
ConstantZ
*/
struct xMINI_LIST_ITEM //双向链表
{
	// 检验一个 MINI链表项 数据是否完整
    //listFIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE
    // 排序值
    configLIST_VOLATILE TickType_t xItemValue;
    // 下一个 链表项
    struct xLIST_ITEM * configLIST_VOLATILE pxNext;
    // 前一个 链表项
    struct xLIST_ITEM * configLIST_VOLATILE pxPrevious;
};
typedef struct xMINI_LIST_ITEM MiniListItem_t;


/*xList 任务链表
2024/11/6
ConstantZ
*/
typedef struct xLIST
{
	// 检验一个 链表 数据是否完整
    //listFIRST_LIST_INTEGRITY_CHECK_VALUE
    // 记录 链表 中 链表项 数目
    volatile UBaseType_t uxNumberOfItems;
    // 遍历 链表 的指针
    ListItem_t * configLIST_VOLATILE pxIndex;
    // 使用 MINI链表项 表示 链表尾部
    MiniListItem_t xListEnd;
    // 检验一个 链表 数据是否完整
    //listSECOND_LIST_INTEGRITY_CHECK_VALUE
} List_t;

//函数
void vListInitialiseItem( ListItem_t * const pxItem );
//初始化链表
void vListInitialise( List_t * const pxList );
//插入链表项到链表末尾
void vListInsertEnd( List_t * const pxList, ListItem_t * const pxNewListItem );
#endif
