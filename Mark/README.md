# 基于STM32的RTOS内核简单实现
* 在同学的推荐下，看到一篇手写实现基于STM32F4的RTOS帖子，链接为[FreeRTOS 简单内核实现](https://blog.csdn.net/lc_guo/article/details/139676590)。
为了熟悉RTOS内核为Linux打下基础，也为了锻炼自己的动手能力，我决定使用手上一块**正点原子STM32F1精英开发板**复现这个帖子的RTOS代码。

* 这个.md作为记录文件，看看自己能走多远

## 2024/11/3
准备工作：\
&emsp;(1) 一块正点原子的STM32精英开发板，主控是STM32F103ZET6。具体参数见图:![参数图](.\pic\1.png).\
开发板的实际图如下：\
![实物图](.\pic\2.jpg)

* 在进行正式的RTOS书写之前，我需要验证这个开发板是否能够正常下载和运行程序。为此，我需要首先就是下载这个开发板的部分资料，网址如下\
[正点原子精英版资料](http://www.openedv.com/docs/boards/stm32/zdyz_stm32f103_jingyingV2.html);给的百度网盘，这点绝对差评！

### 启动第一个程序
为了验证开发板的基础工能是否正常，我决定第一个程序是点亮开发板上的一个LED来验证.为了方便配置和快速生成代码，我决定学习STM32cube的相关内容，这可以帮助我快速上手，只需要修改相应的管脚信息我就能点亮LED.STM32cube的安装教程附上[安装教程](https://blog.csdn.net/Brendon_Tan/article/details/107685563)。\
接下来开始正式配指STM32CUBE来启动开发板。参考视频教程[正点原子CUBE](https://www.bilibili.com/video/BV1Wp42127Cx?vd_source=9cfe8c96e4fed1941ab35fc34dd2dde3&p=3&spm_id_from=333.788.videopod.episodes)    参考的文字教程是正点原子的STM32精英版资料和[SRM32Cube工程](https://blog.csdn.net/lc_guo/article/details/135071885?spm=1001.2014.3001.5502). \
我的具体操作如下：\
* 安装STM32CUBE 需要在官网注册账号并下载 其次 需要下载JAVA便于顺利安装STM32CUBE
* 接下来，打开STM32CUB。界面如图所示：
![界面](.\pic\3.png)
* 进入下面的界面
![界面](.\pic\6.png) \
下一个界面如图所示 \
![界面](.\pic\7.png) \
* 我们需要依次配置上方的`pinout` `Clock` `Project`三个主菜单和它们的子项目 \

* 首先来配指管脚的相关的工能，因为我们需要是LED点亮，因此，我们需要查看硬件原理图，查阅到我这块开发板是LED0是连接的
`PB5`管脚，且是需要下来输出才行。因此按照配指如图所示 \
![界面](.\pic\8.png) \

* 接下来使能RCC始终了时钟来启动GPIO，配置如下
![界面](.\pic\9.png) 
这里面值得注意的是时钟源的选择，一般会HSE会外挂一个8Mhz的晶振作为时钟源，LSE会外挂一个32Khz
的晶振做时钟源。因此，按图中配指即可，没有时钟源择选`disable` 

* 再设置`sys`项目，该项用于配指DEBUG模式
![界面](.\pic\10.png) 
我们日后若用到再来配指，这里选择图中的选项，则只会用到最少的管脚资源

* 接下来配指STM32的时钟树，如图所示
![界面](.\pic\11.png) 

* 接下里，配指具体的STM32工程的生成文件
![界面](.\pic\12.png)

* 对上一个点的(4)做一个说明，最后下载STM32F1的依赖包，我们可以回到主页面(一开始打开那个界面)，点击如图所示的选项
![界面](.\pic\13.png)
进入之后，选择对应的最新依赖包即可
![界面](.\pic\14.png)

* 最后便可以在指定文件生成代码
![界面](.\pic\15.png)

* 最后下载需要参考正点原子的官方教程，我们使用串口下载，标准的下载步骤是：\
1， 把 B0 接 V3.3 \
2， 保持 B1 接 GND \
3， 按一下复位按键 \
4， 使用上位机软件下载代码 \
5，配指下载工具中的下载时序
![界面](.\pic\16.png)
* 下载验证，完美！
![界面](.\pic\17.jpg)
## 2024/11/4
* 完善昨天的内容编写，昨天只是程序搞定了，文档部分得今天来弄，哈哈。

* 在github上建仓，以后学习学习git，方便快速上传和版本管理

## 2024/11/5
今天继续STM32 FreeRTOS内核实现。我们的任务是利用STM32CubeMX快速准备一个STM32空项目为RTOS做准备。\
项目中提到需要将NVIC选择4为抢占优先级，并设置SysTick 和 PendSV 中断设置为最低优先级，所以如下图所示
![界面](.\pic\17.png)
其余在设置以下外部时钟晶振即可和配置系统时钟即可

创建好内容后，我们需要做的就是先搭好框架。在工程的根目录下，我们创建一个RTOS内核文件夹，里面子文件加为Inc Src 分别为头文件和源文件的存放文件夹 \
RTOS的目录如下\
1 Inc\
&emsp;1.1FreeRTOS.h，用来包含 RTOS 所有的头文件\
&emsp;1.2FreeRTOSConfig.h，用来配置裁剪 RTOS 的功能\
&emsp;1.3list.h，双向链表数据结构头文件\
&emsp;1.4portMacro.h，用来统一 RTOS 中用到的类型和定义一些功能宏\
&emsp;1.5task.h，任务管理头文件\
2 Src\
&emsp;2.1list.c，双向链表数据结构源文件\
&emsp;2.2prot.c，用来定义与底层芯片架构有关的函数和中断服务函数\
&emsp;2.3task.c，任务管理源文件\

相应的，MDK中需要添加进去\
![界面](.\pic\18.png)

MDK添加新头文件的基础操作，不在赘述\
![界面](.\pic\19.png)

## 2024/11/6 双向链表
### 数据结构
首先来看看双向链表结构体长什么样
```
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
typedef struct xLIST_ITEM ListItem_t;
```
它的组织形式如下图所示
![界面](.\pic\20.png)

除此之外，还有一个MINI链表,但迷你列表项仅用于标记列表的末尾和挂载其他插入列表中的列表项，用户是用不到迷你列表项的，
通常在在 list.h 文件中。\
可以理解成专用于表示链表的尾节点的数据结构
```
/*MINI链表项
2024/11/6
ConstantZ
*/
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
```
它的组织形式如下图所示

![界面](.\pic\21.png)

接下里介绍的数据结构就是串联上述两个数据结构的东西，比较的重要，即**xLIST**,它的结构如下
```
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
```
**xLIST**由多个链表项构成的链表，常用于区分不同任务状态或任务优先级，比如就绪状态的任务放在就绪链表中，阻塞状态的任务放在阻塞链表中，方便任务管理.它的结构框图如下:

![界面](.\pic\22.png)

### 函数
需要实现的基础函数是一下几个:


* **void vListInitialiseItem( ListItem_t * const pxItem )**\
初始化的时候该链表项未被任何链表包含，即初始化表示任何`xLIST`结构体变量都不拥有它(`xMINI_LIST_ITEM`)
```
void vListInitialiseItem( ListItem_t * const pxItem )  
{  
    // 确保链表项未被记录在链表中
    pxItem->pxContainer = NULL; 
}
```
## 2024/11/7
* **void vListInitialise( List_t * const pxList )**\
初始化`xList`结构体，即初始化链表，需要做四件事情\
&emsp;(1) 当前链表指针`pxIndex`指向末尾`xListEnd`\
&emsp;(2) 确保尾链表项 `xListEnd` 被排在链表最尾部 由代码可见是升序排序这些链表项的值的\
&emsp;(3) 将尾链表项 `xListEnd` 的前/后链表项指针均指向自己，因为初始化链表时只有尾链表项\
&emsp;(4) 初始化链表有0个链表项
```
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
```
执行完找个函数后，`xList`结构体内部的值如下\
![界面](.\pic\24.png)

* **void vListInsertEnd( List_t * const pxList, ListItem_t * const pxNewListItem )**
链表的**插入**操作，将一个新的链表项目`pxNewListItem`插入到当前的链表`pxList`的后面，并实现环形连接\
为了方便，使用如下的图来解释代码的运作方式。首先，初始化`pxList`后应该这样的\
![界面](.\pic\25.png)\
而后，运行这个函数后是,连接是这样的
![界面](.\pic\26.png)\
我们从全部的`nxet`出发，就会发现，当前的`Index`结构体中的`next`指向新的`pxNewListItem`，新的`pxNewListItem`的`next`又会指向`ListEnd`\
从全部的`pre`出发，`pxNewListItem`的`pre`永远指向最新插入的`pxNewListItem`以实现环形，而新的`pxNewListItem`的`pre`会指向上一个`pxNewListItem`。
所以插入两个是如下：
![界面](.\pic\27.png)\
于是，我们简其他不必要的数据结构，可以得到如下的简化图\
![界面](.\pic\28.jpg)\
代码如下
```
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
```

2024/11/8

* **void vListInsert( List_t * const pxNewListItem, ListItem_t * const pxNewListItem )**
仍然是链表的**插入操作**,但支持将`pxNewListItem`链表项按升序的模式插入到`pxNewListItem`中，具体操作和普通链表的插入操作很像\
操作如下图所示\
![界面](.\pic\28.png)\
代码实现如下：
```
/*支持链表的升序插入操作
操作：按照链表项中的xItemValue值升序插入到链表中
2024/11/8
Constant_Z
*/
void vListInsert( List_t * const pxList, ListItem_t * const pxNewListItem )  
{
	ListItem_t *pxIterator; //迭代器指针 指向pxList各个链表项
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
```

说完了插入操作，再实现另外一个重要工作：**删除** \
* **UBaseType_t uxListRemove( ListItem_t * const pxItemToRemove )**\
删除操作就十分简单了，同普通链表的删除操作一样，让前面指向当前的后面，让当前的后面指向前面即可。再删除该链表的地址，并确保当前链表的指针不丢失
如图所示\
![界面](.\pic\29.png)\
代码实现如下
```
UBaseType_t uxListRemove( ListItem_t * const pxItemToRemove )  
{  
	List_t * const pxList = pxItemToRemove->pxContainer;  //获取链表项的拥有链表
	
	// 1. 改变后一个链表项 pxPrevious
    pxItemToRemove->pxNext->pxPrevious = pxItemToRemove->pxPrevious;
    // 2. 改变前一个链表项 pxNext
    pxItemToRemove->pxPrevious->pxNext = pxItemToRemove->pxNext;
	  
    // 如果当前链表的指针指向的是要删除的链表项，则确保索引指向有效的项目
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
```
## 2024/11/8 
补充注释即一些宏定义，使得链表部分的代码能正常编译\
我们前面的链表已经实现了最基础的两种操作，即**增、删**，接下来我们完善我们的一些宏定义，使得代码能正常编译通过\
* 首先，我们需要移植基本的数据结构定义给`portMarco.h`文件，它定义了在RTOS中用的基础变量类型的宏定义，内容如下
```
/* portMacro.h */
#include <stdint.h>

#define port_CHAR                   char
#define port_FLOAT                  float
#define port_DOUBLE                 double
#define port_LONG                   long
#define port_SHORT                  short
#define port_STACK_TYPE             unsigned int
#define port_BASE_TYPE              long

typedef port_STACK_TYPE             StackType_t;
typedef long                        BaseType_t;
typedef unsigned long               UBaseType_t;

typedef port_STACK_TYPE*            StackType_p;
typedef long*                       BaseType_p;
typedef unsigned long*              UBaseType_p;


#if(configUSE_16_BIT_TICKS == 1)
    typedef uint16_t                TickType_t;
    #define portMAX_DELAY           (TickType_t) 0xffff
#else
    typedef uint32_t                TickType_t;
    #define portMAX_DELAY           (TickType_t) 0xffffffffUL
#endif

#define pdFALSE                     ((BaseType_t) 0)
#define pdTRUE                      ((BaseType_t) 1)
#define pdPASS                      (pdTRUE)
#define pdFAIL                      (pdFALSE)
```

* 其次，我们需要移植一部分命令宏在`list.h`中，这样以后，在操作链表的时候，我们使用这些宏更方便。
```
*
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
```

如此，我们`list`部分的内容大致结束了，链表是RTOS中重要的基础数据结构，链表与任务息息相关，所以保证编译成功后进入下一节的学习

## 创建TASK任务
RTOS中，任务统一由结构体`TCB_t`来管理，它包含了任务句柄、任务控制堆栈指针、任务堆栈指针等重要数据结构，如下代码所示
```
typedef struct tskTaskControlBlock
{
    volatile StackType_t  *pxTopOfStack;                        // 栈顶
    ListItem_t            xStateListItem;                       // 任务节点
    StackType_t           *pxStack;                             // 任务栈起始地址
    char                  pcTaskName[configMAX_TASK_NAME_LEN];  // 任务名称
}tskTCB;
typedef tskTCB TCB_t;
```
其中，需要注意的是栈顶指针`pxTopOfStack`地址也是`TCB_t`的地址，之后会用这个指针来推理栈内存放的数据及其存放顺序\
还有就是`configMAX_TASK_NAME_LEN`定义在`2FreeRTOSConfig.h`中，一般是24个字节长度\

有了`TCB`结构体，自然要初始化它，这就是接下来的任务创建函数要做的事。\

### xTaskCreateStatic
FreeRTOS允许了静态创建任务(即程序设定好)和动态创建任务两种方式，为简单起见，说明静态创建任务方式，也是最常用的方式。静态创建任务的函数为`xTaskCreateStatic( )`
，具体实现如下.
```
#if (configSUPPORT_STATIC_ALLOCATION == 1)
TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,     // 任务函数
                            const char* const pcName,         // 任务名称
                            const uint32_t ulStackDepth,      // 任务栈深度
                            void* const pvParameters,         // 任务参数
                            StackType_t* const puxTaskBuffer, // 任务栈起始指针
                            TCB_t* const pxTaskBuffer)        // 任务栈控制指针
{
	TCB_t* pxNewTCB;
	TaskHandle_t xReturn;
	// 任务栈控制指针和任务栈起始指针不为空 则开始创建任务
	if((pxTaskBuffer != NULL) && (puxTaskBuffer != NULL))
	{
		pxNewTCB = (TCB_t*)pxTaskBuffer;
		// 将任务控制块的 pxStack 指针指向任务栈起始地址
		pxNewTCB->pxStack = (StackType_t*)puxTaskBuffer;
		
		// 真正的创建任务函数
		prvInitialiseNewTask(pxTaskCode,
							 pcName,
							 ulStackDepth,
							 pvParameters,
							 &xReturn,
							 pxNewTCB);
	}
	else //创建失败
	{
		xReturn = NULL;
	}
	// 任务创建成功后应该返回任务句柄，否则返回 NULL
	return xReturn;
}
#endif
```

* `configSUPPORT_STATIC_ALLOCATION`定义在`FreeRTOSconfig.h`中，表示允许静态创建任务\
仔细观察代码，我们会发现，真正创建任务的函数是`prvInitialiseNewTask()`这个函数。详细见它的介绍\
该函数的核心就在于调用了实际创建任务的函数`prvInitialiseNewTask`

### prvInitialiseNewTask()
初始化任务其实只在做一件事，就是初始化各个结构体中的成员变量，并根据内核，将寄存器的信息写入到栈中保护起来。实现如下
```
/* task.c */
// 使用的外部函数声明
extern StackType_t* pxPortInitialiseStack(StackType_t* pxTopOfStack, 
                                          TaskFunction_t pxCode, 
                                          void* pvParameters);
// 真正的创建任务函数																 
static void prvInitialiseNewTask(TaskFunction_t pxTaskCode,    // 任务函数
                            const char* const pcName,          // 任务名称
                            const uint32_t ulStackDepth,       // 任务栈深度
                            void* const pvParameters,          // 任务参数
                            TaskHandle_t* const pxCreatedTask, // 任务句柄
                            TCB_t* pxNewTCB)                   // 任务栈控制指针
{
	StackType_t *pxTopOfStack;
	UBaseType_t x;
	
	// 栈顶指针，用于指向分配的任务栈空间的最高地址
	pxTopOfStack = pxNewTCB->pxStack + (ulStackDepth - (uint32_t)1);
	// 8 字节对齐
	pxTopOfStack = (StackType_t*)(((uint32_t)pxTopOfStack) 
	                             & (~((uint32_t)0x0007)));
	// 保存任务名称到TCB中
	for(x = (UBaseType_t)0;x < (UBaseType_t)configMAX_TASK_NAME_LEN;x++)
	{
		pxNewTCB->pcTaskName[x] = pcName[x];
		if(pcName[x] == 0x00)
			break;
	}
	pxNewTCB->pcTaskName[configMAX_TASK_NAME_LEN-1] = '\0';
	
	// 初始化链表项
	vListInitialiseItem(&(pxNewTCB->xStateListItem));
	
	// 设置该链表项的拥有者为 pxNewTCB
	listSET_LIST_ITEM_OWNER(&(pxNewTCB->xStateListItem), pxNewTCB);
	
	// 初始化任务栈
	pxNewTCB->pxTopOfStack = 
	          pxPortInitialiseStack(pxTopOfStack, pxTaskCode, pvParameters);
	
	if((void*)pxCreatedTask != NULL)
	{
	    *pxCreatedTask = (TaskHandle_t)pxNewTCB;
	}
}
```

`prvInitialiseNewTask`做了如下的事情：\
&emsp;(1) 初始化任务栈指针 及 任务名称\
&emsp;(2) 初始化链表并指定这个任务用于这个链表\
&emsp;(3) 初始化任务栈\
前两点比较直白，我们看看(3)初始化任务栈是如何做到的,这就要引入另外一个函数 `pxPortInitialiseStack`

### pxPortInitialiseStack
函数 pxPortInitialiseStack()用于初始化任务栈，就是往任务的栈中写入一些重要的信息，这些信息会在任务切换的时候被弹出到 CPU 寄存器中，以恢复任务的上下文信息，这些信息就包括 xPSR 寄存器的初始值、任务的函数地址（PC 寄存器）、任务错误退出函数地址（LR 寄存
器）、任务函数的传入参数（R0 寄存器）以及为 R1~R12 寄存器预留空间，若使用了浮点单元(CORETEX-M4以上有，STM32F1是M3内核不考虑，那么还会有 EXC_RETURN 的值。同时该函数会返回更新后的栈顶指针。

定义在`port.c`中，实现如下
```
// 初始化栈内存
StackType_t* pxPortInitialiseStack(StackType_t* pxTopOfStack, 
								   TaskFunction_t pxCode, 
								   void* pvParameters)
{
    // 异常发生时，自动加载到CPU的内容
    pxTopOfStack --;
    *pxTopOfStack = portINITIAL_XPSR;
    pxTopOfStack --;
    *pxTopOfStack = ((StackType_t)pxCode) & portSTART_ADDRESS_MASK;
    pxTopOfStack --;
    *pxTopOfStack = (StackType_t)prvTaskExitError;
	
    // r12、r3、r2和r1默认初始化为 0
    pxTopOfStack -= 5;
    *pxTopOfStack = (StackType_t)pvParameters;
	
    // 异常发生时，手动加载到CPU的内容
    pxTopOfStack -= 8;
	
    // 返回栈顶指针，此时pxTopOfStack指向空闲栈
    return pxTopOfStack;
}
```
这里`pxTopOfStack--`的方式就和`prvInitialiseNewTask()`函数中初始化堆栈指针的值对应上`pxTopOfStack = pxNewTCB->pxStack + (ulStackDepth - (uint32_t)1);`，说明任务栈底在高位，它是向上生长的，这点要注意。\
根据M3内核的需求，保存该任务必要的寄存器在任务切换时能顺利切换到这个任务上来,这点也类似于中断的保护现场操作，固定格式。执行完毕后这块栈是如下所示的内容
![内存](.\pic\30.png)

我们总结一下，创建一个任务并初始化它需要做的任务及其流程，如下图所示：
![内存](.\pic\31.jpg)

好了，现在我看了网页是这样的，但对于`pxPortInitialiseStack()`为什么要那样执行还有一定的疑问，为什么必须这样的顺序压入堆栈？参考M3技术手册就能找到答案。`Cortex-M3权威指南中`cp9中断章节第135页解释了原因。节选部分如下图所示

![内存](.\pic\32.png).

由此，我们也看出初始化入栈就和中断的保护现场一样，需要按照中断的顺序进行保护现场操作，流程也和中断一致。那么我们不禁会问，为什么要向中断一样初始化？这就是下面**任务调度器**的内容了。

### 任务调度器

在初始化`TCB_t`的任务模块以后，意味着我们初始化了一个任务，接下来就需要使用它。任务调度的部分主要分为三个部分\
* 初始化调度器
* 启动任务
* 执行任务调度

在此进入任务调度器之前，我们可以在`main.c`中静态的创建任务了。代码如下
```
//静态方式创建任务
//任务句柄
TaskHandle_t Task_Handle;
//任务栈大小
#define TASK_STACK_SIZE 128
StackType_t TaskStack[TASK_STACK_SIZE]; //128字节
//任务控制块
TCB_t TaskTCB;
// 任务 1 入口函数
void Task1_Entry(void *parg)
{
	while(1){
	
	}
}
```
接着，在主函数最后进入主循环前静态的创建它即可
```
int main(){
    //STM32初始化
    ...
    // 创建任务 1
	Task_Handle = xTaskCreateStatic((TaskFunction_t)Task1_Entry,
									 (char *)"Task1",
									 (uint32_t)TASK_STACK_SIZE,
									 (void *)NULL,
									 (StackType_t *)TaskStack,
									 (TCB_t *)&TaskTCB);
    while(1){}
}
```

好了，接下来加该让任务调度器来控制切换任务，以及解释上诉的问题和揭示如何书写一个任务调度器来支持切换多任务

#### 4.1 初始化调度器

再次之前，我们有必要说说什么是`PendSV`和`SysTick`中断及调度器的设计思路

---

**补充**
* 什么是`PendSV`和`SysTick`?

Cotrex-M3指南是这样定义的`PendSV`的
```
PendSV（可悬起的系统调用），它是一种CPU系统级别的异常，它可以像普通外设中断一样被悬起，而不会像SVC服务那样，因为没有及时响应处理，而触发Fault。
```
简而言之，`PendSV`就是一个系统级别的可挂起中断，即它被触发了可以没那么着急的去处理，且不会因为没及时处理而触发一些Fault。

Cotrex-M3指南是这样定义的`SysTick`的
```
SysTick 定时器：系统滴答定时器是一个非常基本的倒计时定时器，用于在每隔一定的时间产生一个中断，即使是系统在睡眠模式下也能工作
```
简而言之，就是`SysTick`是一个可编程优先级的系统级别的定时器中断。

* 为什么使用RTOS调度器要使用`PendSV`和`SysTick`?

由于PendSV的特点就是支持【缓期执行】，所以嵌入式OS可以利用它这个特点，进行任务调度过程的上下文切换。什么又是`上下文切换呢`?具体来讲就是
```
任务调度过程的上下文切换”指的是在嵌入式操作系统（RTOS）中，当进行任务切换时，保存当前任务的状态（上下文），以便之后可以恢复执行。这是操作系统管理多任务并发执行的关键机制之一。
```
这样做的目的是:**任何嵌入式RTOS，都需要尽量不打断外设中断**.

接下来结合两个场景来看看为什么使用这两个中断。我们假设RTOS中有两个任务就绪，需要执行上下文的切换。

(1) 没有外部中断触发的情况下,我们在`SysTick`中断中进行任务上下文切换，且`SysTick`优先级高于一般的中断

![内存](.\pic\33.png).

在Cortex-M3中，如果OS在某个中断活跃时，抢占了该中断，而且又发生了任务调度，并执行了任务，切换到了线程运行模式，将直接触发Fault异常。

![内存](.\pic\35.png).

因此，PendSv中断被提出了，它可以延迟处理系统级的中断(如SVC中断)，这使得其实时性提高。下面是Cotex-M3权威指南提供的一种RTOS设计Pendsv用处。

![内存](.\pic\34.png).

任务执行的流程是:\
(1) 任务A执行一段时间后呼出SVC中断请求，请求调度\
(2) 系统内核接受请求，做好上下文切换工作，挂起一个PendSV中断\
(3) 没有其他的中断在执行，进入这个PendSV服务函数完成任务切换\
(4) 任务B执行一段时间后突发一个中断IRQ\
(5) 系统响应，并进入ISR\
(6) 此时`SysTick`中断IRQ产生\
(7) 系统内核执行必要操作，做好上下文切换，挂起一个PendSV中断\
(8) `SysTick`中断退出后继续执行ISR\
(9) 执行完毕，没有其他中断后，执行挂起的PendSV中断，完成上下文切换\
(10) 切换进入任务A

---

好了，基于上面的思路，接下来就是如何来实现这个调度器了。

* **BaseType_t xPortStartScheduler(void)**\
这个函数用于设置`PendSV`和`SysTick `的中断优先级为最低，并调用`prvStartFirstTask()`。源码如下
```
BaseType_t xPortStartScheduler(void)
{
	// 设置 PendSV 和 SysTick 中断优先级为最低
	portNVIC_SHPR3_REG |= portNVIC_PENDSV_PRI;
	portNVIC_SHPR3_REG |= portNVIC_SYSTICK_PRI;
	
	// 初始化滴答定时器
	
	// 启动第一个任务，不再返回
	prvStartFirstTask();
	
	// 正常不会运行到这里
	return 0;
}
```
这里`portNVIC_SHPR3_REG`为Cortex-M3内核里面的中断控制寄存器，该寄存器的地址定义在`portMarco.h`里面。为
```
#define portNVIC_SHPR3_REG                    ( *( ( volatile uint32_t * ) 0xe000ed20 ) )
	
#define portNVIC_PENDSV_PRI                   ( ( ( uint32_t ) configKERNEL_INTERRUPT_PRIORITY ) << 16UL )
#define portNVIC_SYSTICK_PRI                  ( ( ( uint32_t ) configKERNEL_INTERRUPT_PRIORITY ) << 24UL )

//FreeRTOSConfig.h
configKERNEL_INTERRUPT_PRIORITY 15 //允许的最低优先级 因为我们使用的是 NVIC_PriorityGroup_4 即16个抢占优先级 0个子优先级
```

接下来看看`xPortStartScheduler`中启动第一个任务的函数`prvStartFirstTask`

* **__asm void prvStartFirstTask(void)**

由上面的设计思路，我们可以知道这个函数应该是来启动中断，看是否能执行上下文的切换等相关工作。它实际上是一段汇编代码，用于**启动SVC中断**
```
/ 启动第一个任务,实际上是触发SVC中断
//_asm即为宏定义，__ASM，其允许在Keil编译器中插入ARM指令用
__asm void prvStartFirstTask(void)
{	
	//详细的ARM指令直接看ARM相关汇编书籍
	// 8字节对齐
    PRESERVE8
	
    /* 
	在 cortex-M 中，0xE000ED08 是 SCB_VTOR 这个寄存器的地址，
	里面放的是向量表的向量表的起始地址，即 msp 的地址 
	*/
    ldr r0,=0xE000ED08
    ldr r0,[r0]
    ldr r0,[r0]
	
	// 设置主栈指针msp的值
    msr msp,r0
	
    // 启用全局中断
    cpsie i //使能IRQ中断
    cpsie f //使能快速中断 FIQ中断
    dsb   //数据同步指定，确保执行该指令前所有数据访问已经完成
    isb   //刷新指定流水线
    // 调用SVC启动第一个任务
    svc 0
    nop
    nop
}
```
(1) 为什么进行八字节对齐\
这是因为，栈在任何时候都是需要 4 字节对齐的，而在调用入口得 8 字节对齐，在进行 C 编程的时候，编译器会自动完成的对齐的操作，而对于汇编，就需要开发者手动进行对齐。

(2) 什么是设置主栈指针msp\
程序在运行时都会有一个栈空间来存储一些数据(如局部变量等),保存信息时MCU会自动将SP指针进行调整。对Cortex-M系列的MCU，ARM公司设计的时候设计了两个栈空间，为msp(主堆栈指针)、psp(进程堆栈指针)。FreeRTOS中msp用于给系统空间使用，psp给任务使用，即任务中使用psp，而如中断之类的系统层面则使用msp，当使用不同的堆栈指针时，SP 会等于当前使用的堆栈指针。\
所以这里我们在中断中肯定使用msp。


这段汇编代码大致执行了以下几个工能

(1) 重置中断向量表\
(2) 开启全局中断 \
(3) 调用SVC中断服务函数0\

所以，实际我们还得去看看SVC的中断服务函数中在干嘛

```
// SVC 中断服务函数
__asm void vPortSVCHandler(void)
{
    extern pxCurrentTCB;
	// 8 字节对齐
    PRESERVE8
	/*获取任务栈地址*/
    // 将 pxCurrentTCB 指针的地址加载到寄存器 r3
    ldr r3,=pxCurrentTCB
    // 将 pxCurrentTCB 指针指向的地址（存储 TCB 的地址）加载到寄存器 r1
    ldr r1,[r3]
    // 将 pxCurrentTCB 指针指向的地址里的值（当前运行的 TCB 结构体）加载到寄存器 r0
    // 也即寄存器 r0 中存储了当前运行的任务的栈顶指针地址
    ldr r0,[r1]

	/*模拟出栈 设置psp*/
    // 以寄存器 r0 为基地址，将任务栈中存储的值加载到寄存器 r4 ~ r11 中
    // 同时寄存器 r0 的值会自动更新
    ldmia r0!,{r4-r11}
	//设置psp为任务栈指针
    msr psp,r0
    
    /* 使能中断 */
	isb
	//Base Priority寄存器用于确定中断的优先级。当多个中断同时发生时，处理器会根据Base Priority寄存器的值
	//和各个中断的优先级来决定首先处理哪个中断
	//将 basepri 寄存器清零表示 使能中断 并非屏蔽优先级低于0的中断
    mov r0,#0
    msr basepri,r0

    /*使用psp指针 并跳转到任务函数中*/
    orr r14,#0xd
    bx r14
}
```

(1) SVC服务函数会加载任务(TCB结构体指针)堆栈中的内容到寄存器中，加载的顺序就是`pxPortInitialiseStack()`初始化任务堆栈的顺序相反。

(2) 设置psp为栈指针，这点在之前的msp和psp讨论过

(3) **为什么使用 ?**
```
orr r14,#0xd
bx r14
```
要弄清楚这两条汇编代码，首先要清楚 `r14` 寄存器是干什么用的。通常情况下，r14 为链接寄存器（LR），用于保存函数的返回地址。但是在异常或**中断处理函数**中，r14 为 `EXC_RETURN`，`EXC_RETURN` 各比特位的描述如下图所示\
![内存](.\pic\36.png)

显然，根据计算我们就可以知道`EXC_RETURN`实际只有6个合法值\
![内存](.\pic\37.png)

对于STM32F103来说，没有浮点运算单元，所以只有三个合法值，并且，我们知道，只能是`0xFFFFFFFD`，因此，这就是`orr r14,#0xd`的由来，最后跳转到地址`0xFFFFFFFD`，执行此指令，CPU 会自动从 PSP 指向的栈中出栈 R0、R1、R2、R3、R12、LR、PC、xPSR 寄存器，同样，这些在`pxPortInitialiseStack`中已经压入了任务堆栈中，,至此，进入到真正的第一个启动任务中去。


* **xPortPendSVHandler( )**

该函数是`PendSV`的服务函数，用于实现**任务的上下文切换**。来看看具体的代码实现
```
// PendSV 中断服务函数，真正实现任务切换的函数
__asm void xPortPendSVHandler(void)
{
    /*导入全局变量及函数*/
    extern pxCurrentTCB;
    extern vTaskSwitchContext;
	
    PRESERVE8 //八字节对齐
	
    /*r0为psp，即当前运行任务的任务栈指针 */
    mrs r0,psp
    isb
	
    /* R3 为 pxCurrentTCB 的地址值，即指向当前运行任务控制块的指针 */
    /* R2 为 pxCurrentTCB 的值，即当前运行任务控制块的首地址 */
    ldr r3,=pxCurrentTCB
    ldr r2,[r3]

    /* 保护上文：将 R4~R11 入栈到当前运行任务的任务栈中 */
    stmdb r0!,{r4-r11}
    /* R2 指向的地址为此时的任务栈指针 */
    str r0,[r2]
	
    /* 将 R3、R14 入栈到 MSP 指向的栈中 */
    stmdb sp!,{r3,r14}

    /* 屏蔽受 FreeRTOS 管理的所有中断 */
    mov r0,#configMAX_SYSCALL_INTERRUPT_PRIORITY
    msr basepri,r0
    dsb
    isb

    /* 跳转到函数 vTaskSeitchContext
    * 主要用于更新 pxCurrentTCB，
    * 使其指向最高优先级的就绪态任务
    */    
    bl vTaskSwitchContext

    /*使能所有中断*/
    mov r0,#0
    msr basepri,r0

    /* 将 R3、R14 重新从 MSP 指向的栈中出栈 */
    ldmia sp!,{r3,r14}
	
    /* 注意：R3 为 pxCurrentTCB 的地址值，
    * pxCurrentTCB 已经在函数 vTaskSwitchContext 中更新为最高优先级的就绪态任务
    * 因此 R1 为 pxCurrentTCB 的值，即当前最高优先级就绪态任务控制块的首地址 */
    ldr r1,[r3]
    ldr r0,[r1]

    /* 加载下文：从最高优先级就绪态任务的任务栈中出栈 R4~R11 */
    ldmia r0!,{r4-r11}

    /* 更新 PSP 为任务切换后的任务栈指针 */
    msr psp,r0
    isb

    /* 跳转到切换后的任务运行
    * 执行此指令，CPU 会自动从 PSP 指向的任务栈中，
    * 出栈 R0、R1、R2、R3、R12、LR、PC、xPSR 寄存器，
    * 接着 CPU 就跳转到 PC 指向的代码位置运行，
    * 也就是任务上次切换时运行到的位置
    */
    bx r14
    nop
}
```

上述代码可见 `PendSV`中断的服务函数同之间我们设想的一样，即**完成任务的上下文切换**。它主要做了三件事\
(1) 保护上文，即将当前运行的任务进行入栈保护\
(2) 找到下文，即通过`vTaskSwitchContext()`函数找到需要切换的任务\
(3) 加载下文，入栈下文任务的任务栈，在同SV中断启动第一个任务最后的手法一样，通过`bx r14` 进入下一个任务

(1) (3)均使用汇编指令详细做到，下面来看看(2)怎么找到下一个任务

* **vTaskSwitchContext( )**
```
//task.c
// 任务调度函数
void vTaskSwitchContext(void)
{
	// 两个任务轮流切换
	if(pxCurrentTCB == &Task1TCB)
	{
		pxCurrentTCB = &Task2TCB;
	}
	else
	{
		pxCurrentTCB = &Task1TCB;
	}
}
```

这里我们没有定义如FreeRTOS一样的复杂的任务优先级判断机制来判断当前就绪任务中谁的优先级最高，为了方便，实现简单的两个任务轮流切换。

最后，我们需要一个主动产生任务调度的机会来实现上诉我们任务调度器的工能，即下面提到的`taskYIELD()`。

* **taskYIELD()**
```
/* task.h */
// 主动产生任务调度
#define taskYIELD() portYIELD()

/* portMacro.h */
#define portNVIC_INT_CTRL_REG       (*((volatile uint32_t*)0xE000ED04))
#define portNVIC_PENDSVSET_BIT      (1UL << 28UL)
// 触发 PendSV，产生上下文切换
#define portNVIC_INT_CTRL_REG       (*((volatile uint32_t*)0xE000ED04))
#define portNVIC_PENDSVSET_BIT      (1UL << 28UL)
#define portYIELD() \
{ \
    portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT; \
    __DSB(); \
    __ISB(); \
};
```
实质上，`taskYIELD()`或者说`portYIELD()`主动产生任务调度的方法就是**触发一个PendSV中断**。查阅资料发现\
![](.\pic\38.png)
通过将这个寄存器置为一便可以触发PendSV中断

至此，我们已具备了进行任务初始化和切换的一切条件，可以进行任务创建和调度了。

### 启动流程
在正式启动之前，我们有必要梳理下之间提到的种种关键点，因为它们真的非常重要。

任务的创建和初始化部分是：
```
xTaskCreateStatic() ->
    prvInitialiseNewTask() ->
        初始化栈顶指针
        保存任务名称
        初始化链表项 vListInitialiseItem()
        初始化任务栈 pxPortInitialiseStack()
```

任务的调度部分是：
```
    //启动调度器
    vTaskStartScheduler() ->
        xPortStartScheduler() ->
            设置PendSV和SysTick优先级为最低
            // 启动第一个任务不再返回
            prvStartFirstTask() ->
                配置和触发SVC中断
            //SVC中断服务函数
            vPortSVCHandler() ->
                加载任务栈到寄存器中
                跳转到任务中
            //PendSV中断服务函数
            xPortPendSVHandler() ->
                保存上文
                //找到下文
                vTaskSwitchContext()
                跳转到下文 完成任务切换
    //主动产生任务调度
    taskYIELD() ->
        配置并触发PendSV中断
```

### 试验阶段

为了之后能快速进行调试，我决定先将J-LINK下载的调试功能搞定，以提高调试代码的能力。勾吧 J-LINK下载器是坏的，算了，只能先调通串口通信来进行调试了。所以当务之急是移植正点原子的串口通信到自己的开发板上。
由于我的HAL库和正点原子的HAL库对不上，所以我决定在正点原子串口程序的基础上借尸还魂我的RTOS程序。

终于将串口移植过来了，并且成功实现想要的功能，来填下踩过的坑。

首先说明，本历程是由STM32CUBE生成的，所有没有用到的外设及其库文件是不会加载到MDK工程中的，更不会编译到输出文件中，这样做有助于减小文件的体积，减小
不必要的编译和下载时间。那么，如何手动添加HAL库文件呢？

以本实验要添加串口为例，

(1) MDK工程中添加对应外设HAL库文件

串口需要天见 USART 和 UART两个库文件。目录是\
![](.\pic\40.png)

添加到这个MDK文件下，进行HAL库的统一管理

![](.\pic\39.png)

(2) 编译进工程中

光添加时没有用的，我们需要找到管理HAL库文件的头文件，使其在编译中添加对 xx_USART.h xx_UART.h。通过比较，我们发现每个HAL库文件都会引用`stm32f1xx_hal.h`头文件，而`stm32f1xx_hal.h`文件又会引用`stm32f1xx_hal_conf.h`的头文件。打开看看

![](.\pic\41.png)

所以，显然将对应的xx_USART.h xx_UART.h行的注释注释掉即可。 这样一来，就算添加成功了 欧耶~

接下俩就是编写串口发送的相关程序，这里直接移植正点原子的串口驱动程序即可。注意需要将 正点原子工程中下的`Drivers\SYSTEM`下的所有相关文件进行移植
即可

这里就又有一个坑，牛魔的，正点原子中主程序的头文件引用是这样的

![](.\pic\42.png)

我们移植后有几率报错，那是因为我们将这些文件下的头文件进行了添加，即如下图

![](.\pic\43.png)

C语言引用 使用`include " "`，搜索头文件的顺序是：工程系统目录(一般是CMISI目录) ->  MDK5设置过的Include Paths -> 当前.C文件同级目录

[MDK5中头文件搜索顺序](https://blog.csdn.net/tianzhijiaoxin/article/details/129122089)

所以，我们设置了Include Paths，就直接引用即可，不用正点原子那种相对路径

编译成功即可使用串口调试助手进行验证即可。

### 总结

本节，我学习了关于RTOS任务管理的最基础内容，通过两个任务切换点亮不同的LED验证了可行性。现在对这一节内容进行总结。

#### STM32中断及RTOS管理

STM32中断分为很多不同的大类，统一由NVIC管理。\
(1)首先是中断优先级，我们需要关注的是STM32优先级管理，RTOS采用的是STM32HAL库中的优先级分组4，即只有0~15级抢占优先级，不设置子优先级\
(2)其次是中断优先级配置器，与RTOS相关的中断有三个，分别是 SV PendSv SysTick中断的优先级，因此，我们关注的`SHPR3`这个中断优先级管理寄存器\
(3)然后是中断屏蔽管理，RTOS采用的是Cortex-M内核的`BASEPRI`寄存器进行管理，RTOS用它来实现受RTOS管理的中断和不受它管理的中断\

提到的`PendSV Systick`中断在RTOS中均设置为最低优先级。

#### RTOS 任务

RTOS中的任务主要通过`task.c task.h port.h` 创建、启动、和进行任务切换工作。其中`task.c task.h`中RTOS中任务的核心结构体`TCB_t`，`TCB_t`如何倍初始化，详细见图31。

`port.c`中则是RTOS关于具体MCU来实现任务切换的具体汇编代码

#### RTOS 流程

从这张图来说明RTOS的核心执行流程。

![内存](.\pic\34.png).

上图的执行顺序就是RTOS的设计核心顺序。我们再来复习一下它的执行流程

(1) 线程状态下，我们执行任务A，任务A在某一时刻将会触发 SVC中断来请求任务切换
(2) SVC中断将触发一个PendSV中断
(3) 在没有其他中断的情况下，PendSV中断将进行任务的上下文切换
(4) 进入线程状态，执行任务B
(5) 外部中断发生，系统立即执行响应
(6) ISR的过程中，触发SysTick中断
(7) 系统将进行任务切换，Pend一个PendSV中断准备进行任务上下文切换
(8) SysTick中断结束后，回到ISR继续执行
(9) ISR执行完毕在没有其他中断情况下，立即执行PendSV中断进行任务上下文切换
(10) 回到线程模式后已由任务B切换回任务A

#### 程序的执行流程
```
静态方式定义TASK1 TASK2

int main(){
    GPIO等器件初始化
    链表初始化 prvInitialiseTaskLists
        -> vListInitialise
    静态创建任务TASK1 TASK2 xTaskCreateStatic
        -> prvInitialiseNewTask
            -> 初始化链表项 vListInitialiseItem
            -> 初始化任务栈 pxPortInitialiseStack
    插入到任务到链表中 vListInsertEnd
    启动任务调度器 vTaskStartScheduler
        ->xPortStartScheduler
            -> 启动第一个任务 prvStartFirstTask
                -> SVC中断服务函数 vPortSVCHandler
    while(){

    }
}
```

#### SVC中断服务函数和PendSV 中断服务函数

本例程中的SVC中断用于启动第一个任务，PendSV中断用于任务的上下文切换。

#### 待改进的地方

* 当前的RTOS可以实现\
(1) 任务的静态方式创建\
(2) 任务的手动切换

* 可以改进的地方\
(1) 不支持任务优先级\
(2) 不支持任务的自动切换\
(3) 不支持任务的并行运行\
(4) 无中断临界保护\
(5) 并没有用到SysTick中断

## CP4 中断临界保护区

* 什么是临界段\
临界段(Critical Section)，即一段代码，它在执行过程中不能被其他任务或中断打断,受到整体保护的一段代码范围

* 为什么需要\
临界段的主要目的是保护对共享资源的访问，确保在任意时刻只有一个任务能够访问该资源，从而避免数据不一致或竞争条件的发生。
临界段的设计目的来源于多任务系统对同一资源访问出现的问题，它的设计目的是确保**当前资源被某一任务独享，而不被其他任务打扰**

* 怎么做\
非常简单，任务的切换是通过PendSv中断实现，而外部信号的处理也是通过中断实现，而内部OS的任务切换也是通过SV中断
实现，那么我们关闭中断，即可。即 **关中断 -> 执行临界段代码 -> 开中断**即可。

### 代码实现
任务明确了就可以直接写代码， 我们先实现关中断的代码。如下
```
/*portMarco.h*/
#define portSET_INTERRUPT_MASK_FROM_ISR()       ulPortRaiseBASEPRI()
// 带返回值关中断，将当前中断状态作为返回值返回
static __inline uint32_t ulPortRaiseBASEPRI(void)
{
	uint32_t ulReturn,ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;
	__asm 
	{
		mrs ulReturn,basepri        // 保存中断时 BASEPRI 寄存器的值
		msr basepri,ulNewBASEPRI    // 屏蔽 优先级值 大于等于 11 的中断
		dsb
		isb
	}
	return ulReturn;
}

#define portDISABLE_INTERRUPTS()                vPortRaiseBASEPRI()
// 不带返回值关中断
static __inline void vPortRaiseBASEPRI(void)
{
	uint32_t ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;
	__asm 
	{
		msr basepri,ulNewBASEPRI
		dsb
		isb
	}
}
```
这里，我们实现两种关中断，一种直接关中断，另外一种返回中断管理寄存器`basepri`的值，我们顺便用宏定义对它们进行封装，之后再其他代码中可以很方便的用到，封装后分别是`portSET_INTERRUPT_MASK_FROM_ISR()` 和 `portDISABLE_INTERRUPTS()`

同时，值得注意的是，**临界区中会关闭RTOS能够管理的中断，对高于RTOS管理的中断是不会被屏蔽的**，所以关中断是针对RTOS的系统而言。

接下来，对应写开中断的代码，并将它们封装好即可。
```
/* portMarco.h*/
// 设置 BASEPRI 为 0 开所有中断
#define portENABLE_INTERRUPTS()                 vPortSetBASEPRI(0)
// 设置 BASEPRI 为进入中断时的值则恢复原来的中断状态
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x)    vPortSetBASEPRI(x)
// 开中断
static __inline void vPortSetBASEPRI(uint32_t ulBASEPRI)
{
	__asm volatile
	{
		msr basepri,ulBASEPRI       
	}
}
```
在 之前的PendSV的ISR我们就提到过，将`basepri`中断屏蔽寄存器清0，就是开中断的意思，设置成x，就是屏蔽x级以下的中断，因此这里封装了两个宏定义，用于RTOS不同需求。`portENABLE_INTERRUPTS()`和`portCLEAR_INTERRUPT_MASK_FROM_ISR(x)`

最后，我们将进出临界区的代码进行宏定义，方便调用
```
/*portMarco.h*/
extern void vPortEnterCritical(void); //port.c中实现
extern void vPortExitCritical(void);

#define portENTER_CRITICAL()         vPortEnterCritical()
#define portEXIT_CRITICAL()          vPortExitCritical()
```
我们在`port.c`实现具体的进出临界区代码，去看看。

```
/*port.c*/
// 中断嵌套计数器
static UBaseType_t uxCriticalNesting = 0xAAAAAAAA; //用于确保中断嵌套中正确开关中断寄存器
// 进入临界区
void vPortEnterCritical(void)
{
	portDISABLE_INTERRUPTS();//关中断
	uxCriticalNesting++; //嵌套计数器++ 
	if(uxCriticalNesting==1)
	{
		// configASSERT((portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK) == 0);
	}
}
// 退出临界区
void vPortExitCritical(void)
{
	// configASSERT(uxCriticalNesting);
	uxCriticalNesting--; //嵌套计数器--
	if(uxCriticalNesting == 0) //只有关一个中断 在开一个中断后 才能打开中断
	{
		portENABLE_INTERRUPTS();
	}
}
```
显然，`vPortEnterCritical`和`vPortExitCritical`均是调用了 `portMarco.h`中的开关中断代码实现。同时，为了实现临界区的嵌套，即临界区中嵌套临界区代码，使用`uxCriticalNesting`这个变量做到，只有在`uxCriticalNesting == 0`的时候，说明嵌套完了，可以开中断了，否则说明还在嵌套中。

同时，我们发现，这样的中断嵌套实际非常虚弱，因为带中断临界区的代码中，随时可以打开中断从而破坏临界区，使用`uxCriticalNesting`可以进一步限制这样的事情发生。


好了，我们选在`task.h`中宏定义具体供外部调用临界区的宏定义。
```
/* task.h */
//进入临界段
#define taskENTER_CRITICAL()           portENTER_CRITICAL()
//退出临界段
#define taskEXIT_CRITICAL()            portEXIT_CRITICAL()
//带basepri寄存器返回值的进入临界段函数
#define taskENTER_CRITICAL_FROM_ISR()  portSET_INTERRUPT_MASK_FROM_ISR()
//带basepri寄存器返回值的退出临界段
#define taskEXIT_CRITICAL_FROM_ISR(x)  portCLEAR_INTERRUPT_MASK_FROM_ISR(x)
```
即以后，需要使用临界区时，就使用上面的宏定义即可。

## CP5 阻塞延时

回顾第三章，我们便会发现我们的RTOS系统其实并非是多任务实时进行的，它总是在HAL延时空转后进行任务切换，因此不是严格意义上的多任务系统，且，**大量的系统空转导致CPU利用率下降**。本章，我们将实现RTOS的任务阻塞状态，来实现真正的任务并行执行。

* 什么是阻塞态\
在实时操作系统（RTOS）中，阻塞态（Blocking State）是指任务（Task）或线程（Thread）在等待某个事件发生或某个条件满足时所处的状态。在这种状态下，任务会被挂起，不会消耗CPU资源，直到它等待的事件或条件被触发

* 有什么问题？\
假设，所有任务都进入阻塞态，那按照阻塞态的定义，那就有可能没有任务能回复它们了，因此，我们引入一个在没有任务执行的情况下，执行的任务，叫**空闲任务**

* 如何实现\
(1) 创建空闲任务\
(2) 实现阻塞延时\
(3) 修改任务进度策略\
(4) 提供延时基准

下面来分别看看怎么实现

### 创建空闲任务
我们使用静态的方式来创建空闲任务，空闲任务里面暂时不做仍和操作。
```
/* task.c */
// 空闲任务参数
TCB_t IdleTaskTCB;
#define confgiMINIMAL_STACK_SIZE 128
StackType_t	IdleTasKStack[confgiMINIMAL_STACK_SIZE];

// 空闲任务函数体
void prvIdleTask(void *p_arg)
{
    for(;;){}
}
```
空闲任务需要在一开始就被创建，所以一般我们在启动任务调度器时就创建它。所以在`void vTaskStartScheduler(void)`函数中来实现空闲函数的静态创建。
```
/* task.c */
// 启动任务调度器
void vTaskStartScheduler(void)
{
   
	// 创建空闲任务
	TaskHandle_t xIdleTaskHandle = xTaskCreateStatic((TaskFunction_t)prvIdleTask,
										(char *)"IDLE",
										(uint32_t)confgiMINIMAL_STACK_SIZE,
										(void *)NULL,
										(StackType_t *)IdleTasKStack,
										(TCB_t *)&IdleTaskTCB);
	// 将空闲任务插入到就绪链表中
	vListInsertEnd(&(pxReadyTasksLists), 
				   &(((TCB_t *)(&IdleTaskTCB))->xStateListItem));
	
    //执行其他的操作
    ...
	pxCurrentTCB = &Task1TCB;
	if(xPortStartScheduler() != pdFALSE){}
}
```

### 实现阻塞延时


上述里面说明后，来看看实操应该怎样搞定。

首先，在任务控制结构体中添加一个变量，用来记录任务阻塞延时的时间。
```
/* task.h */
// 任务控制块
typedef struct tskTaskControlBlock
{
	// 省略之前的结构体成员定义
    ...
    // 用于延时
    TickType_t            xTicksToDelay;                        
}tskTCB;
```
阻塞延时函数的实现如下
```
/* task.c */
// 阻塞延时函数
void vTaskDelay(const TickType_t xTicksToDelay)
{
    TCB_t *pxTCB = NULL;

    // 获取当前要延时的任务 TCB
    pxTCB = (TCB_t *)pxCurrentTCB;
    // 记录延时时间
    pxTCB->xTicksToDelay = xTicksToDelay;
    // 主动产生任务调度，让出 MCU 
    taskYIELD();
}

/* task.h */
// 函数声明
void vTaskDelay(const TickType_t xTicksToDelay);
```


### 修改任务调度策略
需要明白的很重要的一点是**任务调度的核心在于找到当前合适的TCB指针**。目前，我们RTOS的内核能发生任务调度的情况之后两种:\
(1) 手动 `taskYIELD()`\
(2) 延时阻塞函数`vTaskDelay`(待实现)

由于引入了阻塞态和空闲任务，我们的理想任务调度策略如下图：\

![](.\pic\45.jpg)

(1)如果发生任务调度时运行的任务为 IdleTask，就按顺序始终尝试去执行未阻塞的 Task1 或 Task2\
(2)如果发生任务调度时运行的任务为 Task1，就按顺序尝试执行未阻塞的 Task2 或 Task1\
(3)如果发生任务调度时运行的任务为 Task2，就按顺序尝试执行未阻塞的 Task1 或 Task2\
(4)如果步骤 1 ~ 3 尝试执行的任务都已阻塞，就执行空闲任务

更具上面的会议精神，我们修改任务调度函数`vTaskSwitchContext`
```
/* task.c */
// 任务调度函数
void vTaskSwitchContext(void)
{
    if(pxCurrentTCB == &IdleTaskTCB)
    {
        if(Task1TCB.xTicksToDelay == 0)
        {
            pxCurrentTCB = &Task1TCB;
        }
        else if(Task2TCB.xTicksToDelay == 0)
        {
            pxCurrentTCB = &Task2TCB;
        }
        else
        {
            return;
        }
    }
    else
    {
        if(pxCurrentTCB == &Task1TCB)
        {
            if(Task2TCB.xTicksToDelay == 0)
            {
                pxCurrentTCB = &Task2TCB;
            }
            else if(pxCurrentTCB->xTicksToDelay != 0)
            {
                pxCurrentTCB = &IdleTaskTCB;
            }
            else
            {
                return;
            }
        }
        else if(pxCurrentTCB == &Task2TCB)
        {
            if(Task1TCB.xTicksToDelay == 0)
            {
                pxCurrentTCB = &Task1TCB;
            }
            else if(pxCurrentTCB->xTicksToDelay != 0)
            {
                pxCurrentTCB = &IdleTaskTCB;
            }
            else
            {
                return;
            }
        }
    }
}
```

### 提供延时时基
阻塞延时的本质是仍旧是延时，凡是涉及到时间的，均需要提供基准，还记得我们之前提到的`SysTick中断`吗，在这里便会用到。通常 MCU 都有一个名为 `SysTick 的滴答定时器`，其会按照某一固定周期产生中断，一般用来为 MCU 提供时间基准，对于 STM32 HAL 库来说，其滴答定时器只用于`HAL_Delay()`延时函数，我们可以在其中断`SysTick_Handler()`函数中对任务的延时时间进行相应的操作，那如何控制滴答定时器产生中断的周期呢？

对于在32cube上配置好的`SysTick`滴答定时器，在对应的库文件中可以见到如下配置
```
/* stm32f1xx_hal.c */
HAL_TickFreqTypeDef uwTickFreq = HAL_TICK_FREQ_DEFAULT;  /* 1KHz */

/* stm32f1xx_hal.h */
typedef enum
{
  HAL_TICK_FREQ_10HZ         = 100U,
  HAL_TICK_FREQ_100HZ        = 10U,
  HAL_TICK_FREQ_1KHZ         = 1U,
  HAL_TICK_FREQ_DEFAULT      = HAL_TICK_FREQ_1KHZ
} HAL_TickFreqTypeDef;
```

较为重要的参数是`HAL_TICK_FREQ_1KHZ`，它用于选择滴答定时器的默认频率，1khz即为1ms.

初始化了滴答定时器，我们需要一个滴答定时器的中断服务函数，这将是RTOS系统的时基和"心跳节拍"，它在阻塞态中将决定处于阻塞态的任务什么时候退出阻塞态。这个函数即 `xPortSysTickHandler`

### xPortSysTickHandler
它的主要用来保证`xTaskIncrementTick`这个函数正常实现阻塞态计时的函数正常运行，实现方式如下

```
//port.c
// SysTick 中断服务函数
void xPortSysTickHandler(void)
{
    // 关中断
    vPortRaiseBASEPRI();
    // 更新任务延时参数
    xTaskIncrementTick();
    // 开中断
    vPortSetBASEPRI(0);
}

/* portMacro.h */
#define xPortSysTickHandler         SysTick_Handler
```
和之前的`PendSv`等中断一样，我们需要屏蔽`f1xx_it.c`中对应的定义好的 `SysTick_Handler `函数来保证编译通过。


### xTaskIncrementTick
这就是实现阻塞延时的具体函数，它的实现方式是：`遍历链表中每个链表项(任务)，如果链表项的延时参数(我们之前在tskTCB结构体中定义的延时参数)不为0，就递减一次，直到为0，即说明改链表项(任务)退出阻塞态，处于就绪态，然后产生任务调度`

25/2/17
md , 之前CP4的有坑导致程序一直不行，初步查明是写cp4的程序中有问题，好好检查下，服了。这提醒我**凡是写过的东西一定要验证其实用性**

25/2/18
找到了问题了，很有意思的一个小问题。首先看看下面的源代码

* 问题描述
```
main.c

//省略
SystemClock_Config();
...

// 使用链表前手动初始化
	prvInitialiseTaskLists();
...
printf("create task success\r\n");
// 将两个任务插入到就绪链表中
vListInsertEnd(&(pxReadyTasksLists),&(((TCB_t *)(&Task1TCB))->xStateListItem));
vListInsertEnd(&(pxReadyTasksLists),&(((TCB_t *)(&Task2TCB))->xStateListItem));
printf("success to insert task to list\r\n");	
...

port.c
// SysTick 中断服务函数
void xPortSysTickHandler(void)
{
    // 关中断
    vPortRaiseBASEPRI();
    // 更新任务延时参数
    xTaskIncrementTick();
    // 开中断
    vPortSetBASEPRI(0);
}
```

上面的实验情况如下

![](.\pic\45.png)

它会卡死在这个地方

* 问题原因

我们加入的串口打印程序使其卡死。目前，具体的开始原因还在研究中

* 解决方法

在关键的地方加入**临界段**保护，这也是临界段的目的，关掉所有中断来执行关键的程序，即我们在任务进入就绪链表的前后加入临界段保护即可
```
printf("create task success\r\n");
	// 将两个任务插入到就绪链表中
	/*进入临界段	*/
	taskENTER_CRITICAL();
	vListInsertEnd(&(pxReadyTasksLists),&(((TCB_t *)(&Task1TCB))->xStateListItem));
	vListInsertEnd(&(pxReadyTasksLists),&(((TCB_t *)(&Task2TCB))->xStateListItem));
	/*退出临界段	*/
	taskEXIT_CRITICAL();
	printf("success to insert task to list\r\n");
```

问题就解决了。

用逻辑分析仪看看，如下图

2025/2/19

研究一下逻辑分析仪，首先说明这个逻辑分析仪不是示波器，一般智能搞搞低频信号，并且需要下载对应的分析仪的PC端上位机使用，
所以第一步是下载购买的逻辑分析仪的支持软件。后面安装软件即可，后续的教程在文件包中有。显示一下最后的结果

![](.\pic\46.png)

可以看见，程序正常运行。说明我们的阻塞功能是成功。

### 补充

在浏览之前的文章时，我发现作者的RTOS时基在最开始进行了替换，即替换成了一个定时器来处理，所以为了同步，我们需要更换时基。即主要工作是**替换SysTick中断为定时器中断为RTOS的时基**

在更换时基后出现了启动不起来的BUG，以后再研究研究，这里我考虑以后做一版完全独立正点原子的串口通信程序后再进行研究，先进行下一章的学习。

2025/2/21

在第六章的途中，仍然会遇见第五章提到的问题，这次仔细研究，确定是了**是因为SysTick中断优先级过低而导致HAL_delay()卡死**，从而导致的问题，所以，必须对我们的
时基进行更换，将SysTick中断的优先级拉到较高，从而保证Hal_delay()和其他相关的延时不会受到影响，而我们设置**通用定时器TIM6**作为RTOS的时基。

(1)首先，使用STM32CUBE生成TIM6的初始化代码。\
![](.\pic\47.png)\
这里，定时器周期的计算公式如下：\
**Tout（溢出时间）=（ARR+1)(PSC+1)/Tclk**

随后，我们设置TIM6和SysTick的优先级。\
![](.\pic\49.png)\

还需要注意一点是，我们需要将TIM6设置为全局中断
![](.\pic\50.png)\

这里还需要说明的是，通用定时TIM6无法选定时钟源，因为它是指定APB1时钟线为它的时钟源，

![](.\pic\48.png)\

因此，TIM6的时钟频率就是72MHZ。注意系数，这里需要它1ms进入一次中断，因此，我们配置**psc = 7199 apr = 9即可**

(2)替换RTOS时钟源
我们将原来的SysTick的中断服务函数换成TIM6的。
```
//port.c
// SysTick 中断服务函数 ->更新为TIM6作为时基
void xPortSysTickHandler(TIM_HandleTypeDef *htim)
{
	if(htim == &htim6)
	{
		//printf("6\r\n");
		// 关中断
		vPortRaiseBASEPRI();
		// 更新系统时基
		xTaskIncrementTick();
		// 开中断
		vPortSetBASEPRI(0);
	}
}

//portMarco.h
#define xPortSysTickHandler         HAL_TIM_PeriodElapsedCallback
```

(3) 启动时基\
在启动调度器后启动时基，使得RTOS正常运行。
```
extern TIM_HandleTypeDef htim6;
// 启动调度器
BaseType_t xPortStartScheduler(void)
{
	// 设置 PendSV 和 SysTick 中断优先级为最低
	portNVIC_SHPR3_REG |= portNVIC_PENDSV_PRI;
	//portNVIC_SHPR3_REG |= portNVIC_SYSTICK_PRI; ->使用TIM6作为系统时基因此不适用
	
	// 初始化RTOS时基
	HAL_TIM_Base_Start_IT(&htim6);
	// 启动第一个任务，不再返回
	prvStartFirstTask();
	
	// 正常不会运行到这里
	return 0;
}
```

这样一来，我们就将RTOS的时基更换为了TIM6，将系统滴答中断优先级拉高，使得系统不会卡死。

## CP6 优先级
是时候来点特权来区分任务的高低贵贱之分了，因此本章将引入优先级。

* 怎样做到\
我们之前的任务均是无条件的插入到`pxReadyTasksLists`这样一个就绪任务链表中，然后调度器`vTaskSwitchContext`将无差别的对齐进行查询，并选择任务执行，所以，我们
优先级的策略是**将任务就绪链表改位任务就绪链表数组**，这样不同优先级的任务在不同的任务链表中，进而修改任务调度的策略，使其在遍历任务链表数组的时候，优先考虑某些链表，从而达到优先级的目的。

### 6.1 修改任务链表

依照之前的想法，我们首先升级就绪任务链表为就绪任务链表数组。

```
//task.c
// 就绪链表
List_t pxReadyTasksLists[configMAX_PRIORITIES];

//FreeRTOSConfig.h
// 设置 RTOS 支持的最大优先级
#define configMAX_PRIORITIES                    5
```

这里，**我们设定RTOS最多有5个优先级，且数字越大优先级越高**


之前，使用过的所有就绪链表函数均需要修改。

* 修改初始化就绪链表数组 `prvInitialiseTaskLists`

由于是数组，所以得写个循环来初始化不同优先级的链表
```
/* task.c */
// 就绪链表始化函数
void prvInitialiseTaskLists(void)
{
	UBaseType_t uxPriority;
	// 初始化就绪任务链表
	for(uxPriority = (UBaseType_t)0U;
	    uxPriority < (UBaseType_t)configMAX_PRIORITIES; uxPriority++)
	{
		vListInitialise(&(pxReadyTasksLists[uxPriority]));
	}
}
```

### 6.2 添加任务方式

之前的任务，直接手动使用`vListInsertEnd()`这样一个函数来直接插入就绪链表，现在，我们想要任务在创建的初期根据优先级，自动插入到就绪链表数组中。

根据上面的需求，我们写一个`prvAddNewTaskToReadyList()`实现这一功能。

当一个任务需要被添加到就绪链表数组中时，它完成以下工作:

(1) 当前系统任务总数 + 1\
(2) 如果当前系统第一次创建任务，则初始化任务相关的链表\
(3) 如果不是第一次创建任务，根据优先级，加入到就绪链表中\
(4) 根据链表优先级，找到当前任务指针` pxCurrentTCB`指向最高优先级任务的TCB\

* prvAddNewTaskToReadyList

```
/* task.c */
// 全局任务计数器
static volatile UBaseType_t uxCurrentNumberOfTasks = (UBaseType_t)0U;

// 添加任务到就绪链表中
static void prvAddNewTaskToReadyList(TCB_t* pxNewTCB)
{
	// 进入临界段
	taskENTER_CRITICAL();
	{
		// 全局任务计数器加一操作
		uxCurrentNumberOfTasks++;
			
		// 如果 pxCurrentTCB 为空，则将 pxCurrentTCB 指向新创建的任务
		if(pxCurrentTCB == NULL)
		{
			pxCurrentTCB = pxNewTCB;
			// 如果是第一次创建任务，则需要初始化任务相关的列表
			if(uxCurrentNumberOfTasks == (UBaseType_t)1)
			{
				// 初始化任务相关的列表
				prvInitialiseTaskLists();
			}
		}
		else 
		// 如果pxCurrentTCB不为空
		// 则根据任务的优先级将 pxCurrentTCB 指向最高优先级任务的 TCB 
		{
			if(pxCurrentTCB->uxPriority <= pxNewTCB->uxPriority)
			{
				pxCurrentTCB = pxNewTCB;
			}
		}
		
		// 将任务添加到就绪列表
		prvAddTaskToReadyList(pxNewTCB);
	}
	// 退出临界段
	taskEXIT_CRITICAL();
}
```

值得注意的是 ，这里 `if(pxCurrentTCB->uxPriority <= pxNewTCB->uxPriority)`，如果，我们有两个相同优先级的新任务，且均大于当前的任务优先级，那么，RTOS启动时候，会执行的是后创建的那个任务。

具体，将任务添加到就绪链表数组的工作就交给`prvAddTaskToReadyList()`处理。

它需要做的工作是：\
(1) 记录当前的优先级在优先级位图上的位置\
(2) 将当前任务按优先级争取插入到就绪链表数组中

什么是优先级位图？\
就本质就是一个数，我们将它的每一位二进制用来表示优先级，并用当前这一位的0/1来表示该优先级是否被占用。\
比如，初始化的位图是 00000 ，假如创建了一个优先级为2的任务，那么第2为上就为1，即00100,表示优先级为2，已有任务准备就绪。

为什么这样做? \
因为这样比较方便，且能快速查到RTOS当前的优先级情况,而且能很快的清除某一优先级的占用状态，但也存在缺陷，同一级优先级任务无法相互切换(在没有阻塞的情况下)。

怎样做？\
* prvAddTaskToReadyList

```
//task.c 
// 根据任务优先级添加任务到对应的就绪链表
#define prvAddTaskToReadyList(pxTCB) \
	taskRECORD_READY_PRIORITY((pxTCB)->uxPriority); \
	vListInsertEnd(&(pxReadyTasksLists[(pxTCB)->uxPriority]), \
	               &((pxTCB)->xStateListItem)); \

// 根据任务优先级置位优先级位图
#define taskRECORD_READY_PRIORITY(uxPriority)	portRECORD_READY_PRIORITY(uxPriority, uxTopReadyPriority)

//portMarco.h
// 设定优先级位图
#define portRECORD_READY_PRIORITY(uxPriority, uxReadyPriorities) (uxReadyPriorities) |= (1UL << (uxPriority))
```

最后，我们在任务被创建的时候，就将任务按优先级添加到对应的链表中。修改后的`xTaskCreateStatic`如下

* `xTaskCreateStatic`
```
#if (configSUPPORT_STATIC_ALLOCATION == 1)
TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,     // 任务函数
                            const char* const pcName,         // 任务名称
                            const uint32_t ulStackDepth,      // 任务栈深度
                            void* const pvParameters,         // 任务参数
														UBaseType_t uxPriority,           // 优先级
                            StackType_t* const puxTaskBuffer, // 任务栈起始指针
                            TCB_t* const pxTaskBuffer)        // 任务栈控制指针
{
	TCB_t* pxNewTCB;
	TaskHandle_t xReturn;
	// 任务栈控制指针和任务栈起始指针不为空 则开始创建任务
	if((pxTaskBuffer != NULL) && (puxTaskBuffer != NULL))
	{
		...
		
		// 创建完任务自动将任务添加到就绪链表
		prvAddNewTaskToReadyList(pxNewTCB);
	}
	...
}
#endif
```

### 6.3 修改TCB即任务运行方式

首先在TCB结构体中加入 优先级成员变量，并修改 `xTaskCreateStatic`和`prvInitialiseNewTask`的声明方式和定义方式\
```
typedef struct tskTaskControlBlock
{
   
	...
	UBaseType_t           uxPriority;                           // 优先级
	
}tskTCB;
typedef tskTCB TCB_t;
```

然后，我们需要修改启动任务`是的空闲任务的相关内容，并且删除手动添加空闲任务至就绪链表中，因为在创建的时候，我们已经做了这一步。

* vTaskStartScheduler()
```
//task.c
//创建空闲任务
	TaskHandle_t xIdleTaskHandle = xTaskCreateStatic((TaskFunction_t)prvIdleTask,
										(char *)"IDLE",
										(uint32_t)confgiMINIMAL_STACK_SIZE,
										(void *)NULL,
										(UBaseType_t)taskIDLE_PRIORITY,	
										(StackType_t *)IdleTasKStack,
										(TCB_t *)&IdleTaskTCB);
	// 启动调度器
	if(xPortStartScheduler() != pdFALSE)//启动调度器失败
	{}
//task.h
// 空闲任务优先级最低
#define taskIDLE_PRIORITY              ((UBaseType_t) 0U)
```

其次，由于引入了优先级，我们的阻塞延时函数也需要修改。同优先级位图那说的一样，**一个任务阻塞就是在优先级位图上清零**，这样在寻找最高优先级任务时不会考虑它。阻塞延时到了在回复即可。

* vTaskDelay()
```
/* task.c */
// 阻塞延时函数
void vTaskDelay(const TickType_t xTicksToDelay)
{
	// 省略未修改程序
    ......
    // 将任务从优先级位图上清除，这样调度时就不会找到该任务
    taskRESET_READY_PRIORITY(pxTCB->uxPriority);
    // 主动产生任务调度，让出 MCU 
    taskYIELD();
}

//portMarco.h
// 清除对应优先级
#define portRESET_READY_PRIORITY(uxPriority, uxReadyPriorities) (uxReadyPriorities) &= ~(1UL << (uxPriority))
```

同时，在系统时基更新函数`xTaskIncrementTick()`中，我们需要在任务延时到了之后回复它的优先级位图。

* xTaskIncrementTick
```
/* task.c */
// 更新任务延时参数
void xTaskIncrementTick(void)
{
	TCB_t *pxTCB = NULL;
	uint8_t i =0;
	uint8_t xSwitchRequired = pdFALSE;
	
	// 更新 xTickCount 系统时基计数器
	const TickType_t xConstTickCount = xTickCount + 1;
	xTickCount = xConstTickCount;
	
	// 扫描就绪列表中所有任务,如果延时时间不为 0 则减 1 
	for(i=0; i<configMAX_PRIORITIES; i++)
	{
		pxTCB = (TCB_t *)listGET_OWNER_OF_HEAD_ENTRY((&pxReadyTasksLists[i]));
		if(pxTCB->xTicksToDelay > 0)
		{
			pxTCB->xTicksToDelay--;
		}
		// 延时时间到，将任务就绪
		else 
		{
			taskRECORD_READY_PRIORITY(pxTCB->uxPriority);
			xSwitchRequired = pdTRUE;
		}
	}
	// 如果就绪链表中有任务从阻塞状态恢复就产生任务调度
	if(xSwitchRequired == pdTRUE){
		// 产生任务调度
		portYIELD();
	}
}
```

注意，这里的`pxTCB = (TCB_t *)listGET_OWNER_OF_HEAD_ENTRY((&pxReadyTasksLists[i]));`说明，我们对任务延时的更新只会更新就绪链表中的第一个任务，假如有相同优先级的多个任务，后续插入的任务延时是不会更新的。

最后，修改`vTaskSwitchContext`任务调度函数，使其始终选择最高优先级的任务运行。

它的工作如下：\
(1) 找到最高优先级链表头\
(2) 将PCB指向它

* vTaskSwitchContext
```
/* task.c */
// 任务调度函数
void vTaskSwitchContext(void)
{
    taskSELECT_HIGHEST_PRIORITY_TASK();
}

//task.c
// 找到就绪列表最高优先级的任务并更新到 pxCurrentTCB
#define taskSELECT_HIGHEST_PRIORITY_TASK() \
{ \
	UBaseType_t uxTopPriority; \
	/* 寻找最高优先级 */ \
	portGET_HIGHEST_PRIORITY(uxTopPriority, uxTopReadyPriority); \
	/* 获取优先级最高的就绪任务的 TCB，然后更新到 pxCurrentTCB */ \
	listGET_OWNER_OF_NEXT_ENTRY(pxCurrentTCB, \
	                            &(pxReadyTasksLists[uxTopPriority])); \
}

// portMacro.h
// 确定当前最高优先级
#define portGET_HIGHEST_PRIORITY(uxTopPriority, uxReadyPriorities) uxTopPriority = (31UL - (uint32_t) __clz((uxReadyPriorities)))

//list.h
// 将链表中 pxIndex 指向下一个链表项，用于获取下一个链表项（任务）
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
```

至此，RTOS已支持简单的优先级了。

### 6.4 实验

由于不在需要手动添加任务进入就绪链表，在假如优先级，我们修改主函数代码如下
```
TaskHandle_t Task1_Handle;
//任务栈大小
#define TASK1_STACK_SIZE 128
StackType_t Task1Stack[TASK1_STACK_SIZE]; //128字节
UBaseType_t Task1Priority = 1; //优先级
TCB_t Task1TCB; //任务控制块
// 任务 1 入口函数
void Task1_Entry(void *parg)
{
	while(1){
		HAL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_PIN);
		vTaskDelay(500);
	}
}

TaskHandle_t Task2_Handle;
//任务栈大小
#define TASK2_STACK_SIZE 128
StackType_t Task2Stack[TASK2_STACK_SIZE]; //128字节
UBaseType_t Task2Priority = 2; //优先级
TCB_t Task2TCB; //任务控制块
// 任务 2 入口函数
void Task2_Entry(void *parg)
{
	while(1){
		HAL_GPIO_TogglePin(LED0_GPIO_PORT, LED0_PIN);
		delay_ms(500); //模拟高优先级任务一直占用
	}
}

int main(void)
{
  ...

  /* USER CODE BEGIN 2 */
	
	delay_init(72);
	usart_init(115200);                     /* 串口初始化为115200 */
	printf("init usart success\r\n");
	printf("This is Task05_v1.2\r\n");
	// 创建任务 1
	Task1_Handle = xTaskCreateStatic((TaskFunction_t)Task1_Entry,
									 (char *)"Task1",
									 (uint32_t)TASK1_STACK_SIZE,
									 (void *)NULL,
										Task1Priority,
									 (StackType_t *)Task1Stack,
									 (TCB_t *)&Task1TCB);
	Task2_Handle = xTaskCreateStatic((TaskFunction_t)Task2_Entry,
									 (char *)"Task2",
									 (uint32_t)TASK2_STACK_SIZE,
									 (void *)NULL,
										Task2Priority,
									 (StackType_t *)Task2Stack,
									 (TCB_t *)&Task2TCB);
	printf("create task success\r\n");
	// 启动任务调度器，永不返回
	vTaskStartScheduler();
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    
  }

}
```

### 6.5 问题

这里出现了一个比较恶心的BUG，值得记录一下。

* 问题描述\
这个问题不是偶发性问题，如果我们按上述的流程写好程序后，我们设置`Task1`的和`Task2`的任务优先级在1，2之间是没有问题的，而一旦有任务设置到3以上的优先级，运行
RTOS会直接进入`HardFault_Handler`中断服务函数，即系统出现了致命的硬件错误。

例如，这段代码设置是不会出现问题的。
```
UBaseType_t Task1Priority = 1; //优先级
TCB_t Task1TCB; //任务控制块

UBaseType_t Task2Priority = 2; //优先级
TCB_t Task2TCB; //任务控制块
```

但这样设置后，就会出现问题
```
UBaseType_t Task1Priority = 3; //优先级 > 3就会出现问题，无论哪个任务的优先级
TCB_t Task1TCB; //任务控制块

UBaseType_t Task2Priority = 2; //优先级
TCB_t Task2TCB; //任务控制块
```

* 问题原因

出现`HardFault_Handler`的原因有很多，主要是\
1. 堆栈溢出 2. 指针操作错误 3.指针越界访问内存。这三类软件上的错误会导致进入这个中断。如何定位错误？这里给出两种方法

(1) 方案1

我们在`stm32f1xxit.c`中的`HardFault_Handler`打上断点\
![](.\pic\52.png)

然后，我们进入debug,是程序运行到断点处。

![](.\pic\53.png)

然后，我们需要找到**哪个函数或者语句出了错误，使其跳转到这里这个中断中**，这里我们就需要知道**PC程序指针**。我们查阅ARM-Cotrex-M3技术手册，
会发现，Cortex-M3在进入一个中断之前，会自动保护现场，进行入栈操作，入栈的顺序如下图\

![](.\pic\54.png)

从下到上，依次入栈。

接下来，我们需要判断当前的栈指针是MSP还是PSP，这需要看R14(LR)寄存器的值。

![](.\pic\55.png)

我们上述图中LR是 `0xffff_fff1`，即SP=MSP，当前使用的是主栈指针。

![](.\pic\56.png)

确定了栈指针和入栈顺序，我们就能定位到入栈时的`PC`寄存器的值了，先点击KEI5上方的`view`找到`memory`，可以查看内存情况，定位`MSP = 0x20000B40`

在内存中找到这一地址。

![](.\pic\57.png)

在`Memory`视图中单击右键，选择`unsige long`更方便查看,（因为寄存器是32位），然后对比入栈顺序，我们发现`PC`是第6个入栈，所以，入栈时候PC的值就是从左往右的第6个数字
，即`08000E49`,，这也是符合STM32 FLASH地址的，因为STM32的FLASH是从0x8000000开始的

接下来，在`view`中开启反汇编窗口，`Disassembly windows`，

![](.\pic\58.png)

右键选择`show Disassembly at address`，，选择地址`0x08000E49`,这样我们就知道了是哪段代码运行调用了硬件错误中断。

![](.\pic\59.png)

可以定位到，是`xTaskIncrementTick`这个函数触发的这个错误。

* 方案2

同理，进入debug模式后，在`HardFault_Handler`中打断点，然后运行

这时候，我们使用`view`中的`Call Stack Windos`，可以看见如下图

![](.\pic\60.png)

我们选中`Call Stack Windos`中的`HardFault_Handler`，右键选择`show caller code`就可以直接看是谁触发了这个中断。

![](.\pic\61.png)

需要注意的是，这个呼出可能不太准确，但触发`HardFault_Handler`就在这个函数中

定位了错误点，我们还可以知道是哪种类型的错误触发了`HardFault_Handler`，在debug模式下，我们找到KEIL5左上角菜单中的`Peripherals`，选择`Core peripherals`中的
`Fault report`，可以看见是哪种类型的错误

![](.\pic\62.png)

可以看见是总先上的不精确访问导致的错误。 NVIC有很多错误状态寄存器(Fault Statuc registers)，
其中有一个就是总线错误状态寄存器(Bus Fault Status register,BFSR)，
从这个寄存器中可以知道错误时由数据/指令访问还是中断堆栈等原因产生的。 它的地址是`0xE000ED29`,查看内存得到

![](.\pic\63.png)

配合它的每一位信息，如下图

![](.\pic\64.png)

我们可以知道，是我们的数据越界了导致，即数据可能指向了非法的地址。

至此，我们基本就可以是**xTaskIncrementTick 函数中的某个数据越界从而导致了硬件错误**。查看代码
```
BaseType_t xTaskIncrementTick(void)
{
	TCB_t *pxTCB = NULL;
	uint8_t i =0;
	uint8_t xSwitchRequired = pdFALSE;
	
	// 更新 xTickCount 系统时基计数器
	const TickType_t xConstTickCount = xTickCount + 1;
	xTickCount = xConstTickCount;
	
	// 扫描就绪列表中所有任务,如果延时时间不为 0 则减 1 
	for(i=0; i<configMAX_PRIORITIES; i++)
	{
		pxTCB = (TCB_t *)listGET_OWNER_OF_HEAD_ENTRY((&pxReadyTasksLists[i]));
		if(pxTCB->xTicksToDelay > 0)
		{
			pxTCB->xTicksToDelay--;
		}
		// 延时时间到，将任务就绪
		else 
		{
			taskRECORD_READY_PRIORITY(pxTCB->uxPriority);
			xSwitchRequired = pdTRUE;
		}
	}
	return xSwitchRequired;
}
```

首先 局部变量`i`和`xSwitchRequired`不可能越界，一般是数组才有可能访问越界，唯一有可能的就是
```
for(i=0; i<configMAX_PRIORITIES; i++)
	{
		pxTCB = (TCB_t *)listGET_OWNER_OF_HEAD_ENTRY((&pxReadyTasksLists[i]));
		if(pxTCB->xTicksToDelay > 0)
		{
			pxTCB->xTicksToDelay--;
		}
		// 延时时间到，将任务就绪
		else 
		{
			taskRECORD_READY_PRIORITY(pxTCB->uxPriority);
			xSwitchRequired = pdTRUE;
		}
	}
```

中的`pxTCB = (TCB_t *)listGET_OWNER_OF_HEAD_ENTRY((&pxReadyTasksLists[i]));`因为只有它是全局数组变量，且我们思考一下，假如这个链表有些优先级没有，那么`pxReadyTasksLists[i]`
不就是没有`TCB_t`的任务项了吗？

我们在看看这个宏定义
```
#define listGET_OWNER_OF_HEAD_ENTRY( pxList )            ( ( &( ( pxList )->xListEnd ) )->pxNext->pvOwner )

struct xLIST_ITEM
{
	...
	// 记录此 链表项 归谁拥有，通常是 TCB (任务控制块)
    void * pvOwner;
    ...
}
```

这样，我们便找到了，不存在优先级的数组被强行转换成`TCB_t`的任务块，而此时的`pvOwner`并没有初始化，而导致了数组访问的越界！

* 问题解决

很好解决，我们在循环中，加入限制，只在这个链表有任务是才循环，否则不处理它，修改的`xTaskIncrementTick`如下
```
for(i=0; i<configMAX_PRIORITIES; i++)
	{
		//在链表不为空的情况下访问
		if( listLIST_IS_EMPTY( &pxReadyTasksLists[i]) == pdFALSE ){
			pxTCB = (TCB_t *)listGET_OWNER_OF_HEAD_ENTRY((&pxReadyTasksLists[i]));
			if(pxTCB->xTicksToDelay > 0)
			{
				pxTCB->xTicksToDelay--;
			}
			
			// 延时时间到，将任务就绪
			else
			{ 
				taskRECORD_READY_PRIORITY(pxTCB->uxPriority);
				xSwitchRequired = pdTRUE;
			}
		}	
	}
```

这样，就解决了问题，程序就能正常运行了~

## CP7 阻塞链表

通过第六章，我们已经注意到了，不能把延时阻塞任务和就绪任务混在一起，否则可能会出现很多问题。因此，我们有必要单独给需要阻塞的任务开一链表，即**阻塞链表**。
有任务阻塞，我们就把任务加入到它之中，阻塞的时间到了，我们就把它给移除即可。

但计算的阻塞的时间上，我们之前就有个很严重的BUG没有处理，请看下面在`xTaskIncrementTick`的这段代码
```
static volatile TickType_t xTickCount = (TickType_t)0U;
BaseType_t xTaskIncrementTick(void){
	...
	// 更新系统时基计数器 xTickCount
	const TickType_t xConstTickCount = xTickCount + 1;
	xTickCount = xConstTickCount;
	...
}
```

细心看，我们发现之前的`xTickCount`会随着程序一直递增，存在**溢出的风险**，但之前为什么没有触发呢？因为`xTickCount`是`uint32_t`类型的，即最大值
有`2^32 - 1`，换算后大概有24天，所以之前没有触发溢出。

在应用阻塞延时上，如何解决这一问题呢？\
参考`FREERTOS`的做法，我们通过**设置两个链表，一个用于正常阻塞，一个用于有溢出的备用阻塞链表**。一旦`xTickCount`发生溢出，我们就叫交换
这两个链表，保证始终在正确的时间到了后有任务能解除阻塞状态。

```
// 任务下次解除阻塞的时间 = 当前滴答定时器计数值 + 要延时的滴答次数
xNextTaskUnblockTime = xConstTickCount + xTicksToWait;
```

为什么交换两个链表就能保证阻塞任务正常运行呢？\
这里我们举个例子来说明。\
假如，有个`Task1`需要阻塞`200`个系统时基，此时的`xTickCount = 300`，`Task2`需要阻塞`500`个系统时基，我们假设`xTickCount`会在`599`后溢出，
那么，根据公式` 任务唤醒时间 = 当前系统时间 + 需要阻塞时间`，我们可以知道，`Task1`会在`xTickCount = 500`时唤醒，而`Task2`会在`xTickCount = 300 + 500 = 800 - 600 -> 200`的时候唤醒。\
因此，我们`Task1`加入`xDelayed_Task_List1`这个阻塞链表，`Task2`加入`pxOverflow_Delayed_Task_List`备用链表，在`xDelayed_Task_List1`中的任务会受到系统的监视，在正确的`xTickCount`时唤醒，而`pxOverflow_Delayed_Task_List`则不用管，因为它不在本轮的`xTickCount`计数范围内。当`xTickCount`发生溢出后，我们交换链表数据，即`Task2`会挂载到`xDelayed_Task_List1`，而`pxOverflow_Delayed_Task_List`不会挂载任何任务。

细心的人一定会发现，这样做似乎仍然是有bug的，为什么？ 按之前的计算，加入`Task2`需要阻塞`5000`个系统时基呢？换算下来，它至少需要2轮以上的溢出才能
本加入到`xDelayed_Task_List1`，进行监视，但它会之间加入`pxOverflow_Delayed_Task_List`，在第1轮溢出后被错误的唤醒。为什么我们不管？\
因为这涉及到数据范围的问题，我们规定了延时设定函数中，能够要求的延时是`TickType_t`类型，和`xTickCount`一个类型，即它不可能要求比`TickType_t`类型范围大的延时数，因此，这个BUG是不可能实现的。

### 7.1 阻塞链表及其备用
根据前面的会议精神，我们这样定义组设链表\
```
/* task.c */
// 阻塞链表和其指针
static List_t xDelayed_Task_List1;
static List_t volatile *pxDelayed_Task_List;
// 溢出阻塞链表和其指针
static List_t xDelayed_Task_List2;
static List_t volatile *pxOverflow_Delayed_Task_List;
```

由于新增了这两个链表，我们在创建任务的时候初始化它们，即在`xTaskCreateStatic` -> `prvAddNewTaskToReadyList` -> `prvInitialiseTaskLists`
初始化\
```
// 就绪链表始化函数
void prvInitialiseTaskLists(void)
{
	...
	// 初始化延时阻塞链表
	vListInitialise(&xDelayed_Task_List1);
	vListInitialise(&xDelayed_Task_List2);
	
	// 初始化指向延时阻塞链表的指针
	pxDelayed_Task_List = &xDelayed_Task_List1;
	pxOverflow_Delayed_Task_List = &xDelayed_Task_List2;

}
```

### 7.2 完善阻塞链表及其备用的功能

* prvAddCurrentTaskToDelayedList

我们首先来解决，如何将一个任务插入到阻塞链表中，我们定这个函数名为`prvAddCurrentTaskToDelayedList`，它需要满足以下的功能:\
(1) 它需要计算出下次唤醒任务的时间`xTimeToWake`\
(2) 它需要判断加入阻塞链表还是备用阻塞链表\
(2) 它需要将任务按阻塞时间，从小到大插入到阻塞链表中\
(3) 计算出当前最早唤醒任务的时间，并更新到`xNextTaskUnblockTime`变量。

```
// 将当前任务添加到阻塞链表中
static void prvAddCurrentTaskToDelayedList(TickType_t xTicksToWait)
{
	TickType_t xTimeToWake;
	// 当前滴答定时器中断次数
	const TickType_t xConstTickCount = xTickCount;
	// 成功从就绪链表中移除该阻塞任务
	if(uxListRemove((ListItem_t *)&(pxCurrentTCB->xStateListItem)) == 0)
	{
		// 将当前任务的优先级从优先级位图中删除
		portRESET_READY_PRIORITY(pxCurrentTCB->uxPriority, uxTopReadyPriority);
	}
	// 计算延时到期时间
	xTimeToWake = xConstTickCount + xTicksToWait;
	// 将延时到期值设置为阻塞链表中节点的排序值
	listSET_LIST_ITEM_VALUE(&(pxCurrentTCB->xStateListItem), xTimeToWake);
	// 如果延时到期时间会溢出
	if(xTimeToWake < xConstTickCount)
	{
		// 将其插入溢出阻塞链表中
		vListInsert((List_t *)pxOverflow_Delayed_Task_List,
		           (ListItem_t *)&(pxCurrentTCB->xStateListItem));
	}
	// 没有溢出
	else
	{
		// 插入到阻塞链表中
		vListInsert((List_t *)pxDelayed_Task_List,
		           (ListItem_t *) &( pxCurrentTCB->xStateListItem));
		
		// 更新下一个任务解锁时刻变量 xNextTaskUnblockTime 的值
		if(xTimeToWake < xNextTaskUnblockTime)
		{
			xNextTaskUnblockTime = xTimeToWake;
		}
	}
}
```

* prvResetNextTaskUnblockTime

我们前面提到了，需要在发生溢出的时候交换两个链表，这里我们实现这一功能。即`prvResetNextTaskUnblockTime( )`函数
```
/* task.c */
// 记录溢出次数
static volatile BaseType_t xNumOfOverflows = (BaseType_t)0;

// 延时阻塞链表和溢出延时阻塞链表交换
#define taskSWITCH_DELAYED_LISTS()\
{\
	List_t volatile *pxTemp;\
	pxTemp = pxDelayed_Task_List;\
	pxDelayed_Task_List = pxOverflow_Delayed_Task_List;\
	pxOverflow_Delayed_Task_List = pxTemp;\
	xNumOfOverflows++;\
	prvResetNextTaskUnblockTime();\
}
```

由于将任务插入溢出(备用)阻塞链表时不会更新 `xNextTaskUnblockTime` 变量，只有在将任务插入阻塞链表中时才会更新`xNextTaskUnblockTime `变量，所以对于溢出阻塞链表中存在的任务没有对应的唤醒时间，因此当心跳溢出切换阻塞链表时候，需要重设 xNextTaskUnblockTime 变量的值。\
上述要求由`prvResetNextTaskUnblockTime( )`实现

```
/* task.c */
// 记录下个任务解除阻塞时间，默认最大值
static volatile TickType_t xNextTaskUnblockTime = (TickType_t)portMAX_DELAY;
// 函数声明
static void prvResetNextTaskUnblockTime(void);

// 重设 xNextTaskUnblockTime 变量值
static void prvResetNextTaskUnblockTime(void)
{
	TCB_t *pxTCB;
	// 切换阻塞链表后，阻塞链表为空
	if(listLIST_IS_EMPTY(pxDelayed_Task_List) != pdFALSE)
	{
		// 下次解除延时的时间为可能的最大值
		xNextTaskUnblockTime = portMAX_DELAY;
	}
	else
	{
		// 如果阻塞链表不为空，下次解除延时的时间为链表头任务的阻塞时间
		(pxTCB) = (TCB_t *)listGET_OWNER_OF_HEAD_ENTRY(pxDelayed_Task_List);
		xNextTaskUnblockTime=listGET_LIST_ITEM_VALUE(&((pxTCB)->xStateListItem));
	}
}
```

这里可以重点说一下这个`xNextTaskUnblockTime`变量，它其实就是**下一个阻塞时间**变量，可能会好奇，加入任务不止一个阻塞量，那这个不就没法表示了吗？实则是不然，只是我们思考，可能加入阻塞量，就和原来一样，在`xTaskIncrementTick`里面遍历阻塞链表，将阻塞链表的每个任务的阻塞阻塞量减去即可。
但FREERTOS的实现是不一样的，其实阻塞链表是按阻塞量的大小，从小到大排序而成，我们没有必要遍历链表，直接用`xNextTaskUnblockTime`表示表头的任务的阻塞量即可，它应该是所有阻塞任务中最早被唤醒的那个。

在`prvAddCurrentTaskToDelayedList`中有这样一段代码
```
static void prvAddCurrentTaskToDelayedList(TickType_t xTicksToWait)
{
	else
	{
		// 插入到阻塞链表中
		vListInsert((List_t *)pxDelayed_Task_List,
		           (ListItem_t *) &( pxCurrentTCB->xStateListItem));
		
		// 更新下一个任务解锁时刻变量 xNextTaskUnblockTime 的值
		if(xTimeToWake < xNextTaskUnblockTime)
		{
			xNextTaskUnblockTime = xTimeToWake;
		}
	}
}
```

发现`xNextTaskUnblockTime`始终是新加入阻塞链表的最小值，并且，我们在回顾一下`vListInsert`的函数如何插入任务到链表项
```
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
```

会发现链表是按照`xItemValue`的值升序插入的，因此，也解释了为什么只用一个变量`xNextTaskUnblockTime`来记录时间即可。

### 7.3 修改内核函数适配新的阻塞链表

* vTaskDelay( )

我们有新的加入阻塞链表的方法了`prvAddCurrentTaskToDelayedList(xTicksToDelay);`，更新这个函数用法
```
/* task.c */
// 阻塞延时函数
void vTaskDelay(const TickType_t xTicksToDelay)
{
	// 将当前任务加入到阻塞链表
	prvAddCurrentTaskToDelayedList(xTicksToDelay);
	
	// 任务切换
	taskYIELD();
}
```

* xTaskIncrementTick( )

最重要的更新，它需要满足以下的功能：\
(1) 判断该次系统时基更新后是否需要切换阻塞链表和备用阻塞链表\
(2) 判断是否需要移除任务从阻塞链表中，如果有，需要完成它\
(3) 支持优先级管理系统

```
BaseType_t xTaskIncrementTick(void)
{
	TCB_t *pxTCB = NULL;
	TickType_t xItemValue;
	BaseType_t xSwitchRequired = pdFALSE;
	
	// 更新系统时基计数器 xTickCount
	const TickType_t xConstTickCount = xTickCount + 1;
	xTickCount = xConstTickCount;

	// 如果 xConstTickCount 溢出，则切换延时列表
	if(xConstTickCount == (TickType_t)0U)
	{
		taskSWITCH_DELAYED_LISTS();
	}
	
	// 最近的延时任务延时到期
	if(xConstTickCount >= xNextTaskUnblockTime)
	{
		for(;;)
		{
			// 延时阻塞链表为空则跳出 for 循环
			if(listLIST_IS_EMPTY(pxDelayed_Task_List) != pdFALSE)
			{
				// 设置下个任务解除阻塞时间为最大值，也即永不解除阻塞
				xNextTaskUnblockTime = portMAX_DELAY;
				break;
			}
			else
			{
				// 依次获取延时阻塞链表头节点
				pxTCB=(TCB_t *)listGET_OWNER_OF_HEAD_ENTRY(pxDelayed_Task_List);
				// 依次获取延时阻塞链表中所有节点解除阻塞的时间
				xItemValue = listGET_LIST_ITEM_VALUE(&(pxTCB->xStateListItem));
				
				// 当阻塞链表中所有延时到期的任务都被移除则跳出 for 循环
				if(xConstTickCount < xItemValue)
				{
					xNextTaskUnblockTime = xItemValue;
					break;
				}
				
				// 将任务从延时列表移除，消除等待状态
				(void)uxListRemove(&(pxTCB->xStateListItem));
				
				// 将解除等待的任务添加到就绪列表
				prvAddTaskToReadyList(pxTCB);
#if(configUSE_PREEMPTION == 1)
				// 如果解除阻塞状态的任务优先级比当前任务优先级高，则需要进行任务调度
				if(pxTCB->uxPriority >= pxCurrentTCB->uxPriority)
				{
					xSwitchRequired = pdTRUE;
				}
#endif
			}
		}
	}
	return xSwitchRequired;
}
```

至此，我们的RTOS就支持阻塞链表的功能了~

### 7.4 实验

修改主函数的程序如下，验证其可行性。

```
TaskHandle_t Task1_Handle;
#define TASK1_STACK_SIZE                    128
StackType_t Task1Stack[TASK1_STACK_SIZE];
TCB_t Task1TCB;
UBaseType_t Task1Priority = 2;

TaskHandle_t Task2_Handle;
#define TASK2_STACK_SIZE                    128
StackType_t Task2Stack[TASK2_STACK_SIZE];
TCB_t Task2TCB;
UBaseType_t Task2Priority = 3;


/ 任务 1 入口函数
void Task1_Entry(void *parg)
{
	for(;;)
	{
		HAL_GPIO_TogglePin(LED0_GPIO_PORT, LED0_PIN);
		//vTaskDelay(500);
		my_delay(500000); //验证是否支持优先级
	}
}
// 任务 2 入口函数
void Task2_Entry(void *parg)
{
	for(;;)
	{
		HAL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_PIN);
		vTaskDelay(300);
		//my_delay(500000);
	}
}
```

下载，验证，用逻辑分析仪查看，发现可行。

![](.\pic\65.png)

### 一点思考

* 为什么RTOS的SYSTICK和PENDSV优先级如此之底？

通过以上章节的学习，我们可以清晰的认识到，我们的`RTOS`的心跳并不是有规律的跳动的，这将导致我们无法在`RTOS`中完成精度较高的阻塞延时。因为我们的RTOS系统可能会因为外设等优先级较高的中断处理耗时从而并非`1ms`就能进入一次`sys`中断，同样，我们的任务切换可能也会因为`pendsv`的优先级问题导致不能流畅的切换。为什么不把她两设高一点？

我在[RTOS系列文章（3）](https://blog.csdn.net/u012351051/article/details/124830031)找到了一些答案和思考。

对于CPU来说，嵌入式OS也只是一个稍微复杂的程序而已，只是因为我们把操作系统看的太重要了，总感觉操作系统非常神秘，应该是最高级的，这其实是一个错误的认知，操作系统说白了，就是为了方便程序员写出稳定的程序框架，操作系统并不是CPU的一部分，它只是有一些高级的骚操作而已，在CPU眼里，跟裸机程序是一样的。
所以，既然操作系统也是程序，那么操作系统的优先级肯定不能超过外部中断的优先级，因为外部中断一般是硬件中断，优先级都需要很高。

操作系统的心跳节拍本质上讲，就是一个保证操作系统正常运行的节拍而已，就像人的心跳一样，有的人60次/分，有的人70次/分，有的人80次/分，没有唯一答案，你肯定也不会想着要让自己的心跳整齐划一的一直是66次/分。有个大概正常稳定的范围就可以了。嵌入式OS的时钟节拍，压根就不是精准的，也不需要太准，大概准就可以了，因为只要有周期性的节拍，就能保证周期性的调度。软件定时器也不是特别准的，一般用于【短时间】、对时间要求不严苛的场景。如果非要对时间要求的特别准，还是需要用硬件定时器实现的

总结下来就是：\
（1）`os`对于CPU就是一个大型裸机程序，没必要搞高优先级待遇\
(2) 嵌入式的设计一定要以硬件为主，因此`os`的设计应给硬件优先级让路\
(3) 嵌入式`os`的周期不要求精准，只要求能稳定的进行调度即可。

## CP8 时间片轮询

### 8.1 思考

在之前的程序上，我们修改以下`Task2`和`Task1`的优先级，将他们改成相同优先级试试,并且，我们模拟它们都没有阻塞，看看效果。
```
...
UBaseType_t Task1Priority = 2;

...
UBaseType_t Task2Priority = 2;

// 任务 1 入口函数
void Task1_Entry(void *parg)
{
	for(;;)
	{
		HAL_GPIO_TogglePin(LED0_GPIO_PORT, LED0_PIN);
		//vTaskDelay(500);
		my_delay(300000);
	}
}
// 任务 2 入口函数
void Task2_Entry(void *parg)
{
	for(;;)
	{
		HAL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_PIN);
		//vTaskDelay(300);
		my_delay(500000);
	}
}
```

![](.\pic\66.png)

我们发现，任务1的LED似乎不会翻转，这是怎么回事？这就是我们的任务优先级并不支持**时间片轮询**策略。即两个相同任务的优先级没有办法进行切换。这是为什么呢？

想要进行任务切换，就要`xPortSysTickHandler`即系统滴答定时器服务函数的`xTaskIncrementTick`使得它满足切换条件。
```
BaseType_t xTaskIncrementTick(void)
{
	...
	BaseType_t xSwitchRequired = pdFALSE;
	
	
	// 最近的延时任务延时到期
	if(xConstTickCount >= xNextTaskUnblockTime)
	{
		...
				// 如果解除阻塞状态的任务优先级比当前任务优先级高，则需要进行任务调度
				if(pxTCB->uxPriority >= pxCurrentTCB->uxPriority)
				{
					xSwitchRequired = pdTRUE;
				}
	}
		
	}	
	return xSwitchRequired;
}
```

我们会发现，只有进行阻塞时才会更新这个`xSwitchRequired`这个值，所以这是不完美，我们需要想办法让他支持同优先级无阻塞状态下的任务切换。

方法很简单，我们只需要**让它判断下当前任务优先级下的就绪链表是否有 > 1个的任务即可**,这就表明同等优先级下，有两个任务时就绪的，它们需要pendsv里面切换。 修改如下
```
BaseType_t xTaskIncrementTick(void)
{
	...
	if(xConstTickCount >= xNextTaskUnblockTime){
		...
	}
#if((configUSE_PREEMPTION == 1) && (configUSE_TIME_SLICING == 1))
	// 支持时间片轮询
	if(listCURRENT_LIST_LENGTH(&(pxReadyTasksLists[pxCurrentTCB->uxPriority])) > 1)
	{
		xSwitchRequired = pdTRUE;
	}
#endif
	return xSwitchRequired;
}

//FreeRTOSconfig.h
//支持时间片轮询
#define configUSE_TIME_SLICING									1
```

这样就可以了。但这真的奏效吗？它只是表明可以进行任务切换了，但不一定切换任务啊。我们再去任务切换函数`vTaskSwitchContext`中看看，是否实现了任务切换。
```
//list.h
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
//task.c
// 找到就绪列表最高优先级的任务并更新到 pxCurrentTCB
#define taskSELECT_HIGHEST_PRIORITY_TASK() \
{ \
	UBaseType_t uxTopPriority; \
	/* 寻找最高优先级 */ \
	portGET_HIGHEST_PRIORITY(uxTopPriority, uxTopReadyPriority); \
	/* 获取优先级最高的就绪任务的 TCB，然后更新到 pxCurrentTCB */ \
	listGET_OWNER_OF_NEXT_ENTRY(pxCurrentTCB, \
	                            &(pxReadyTasksLists[uxTopPriority])); \
}
// 任务调度函数
void vTaskSwitchContext(void)
{
	taskSELECT_HIGHEST_PRIORITY_TASK();
}
```

我们可以看到，加入进行任务切换，在`pendsv`服务函数中的`vTaskSwitchContext`运行了`listGET_OWNER_OF_NEXT_ENTRY`,可以看到
```
( pxConstList )->pxIndex = ( pxConstList )->pxIndex->pxNext;
```

说明它会自动更新链表指针，使其指向当前链表项的下一项，从而实现同优先级任务的切换。因此时奏效的。

### 8.2 实验验证

![](.\pic\67.png)

RTOS已经支持同优先级任务的时间片轮询操作了。
