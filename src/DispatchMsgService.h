﻿/***********************************************************
* 负责分发消息服务模块，其实就是把外部收到的消息，转化为内部事件，也就是data->msg->event的解码过程
* 然后再把事件投递到线池的消息队列，由线程池调用其process方法对事件进行处理，最终调用每个event的handler方法
* 来处理event,此时每个event handler需要subscrive该event后才会被调用到.
***********************************************************/

#ifndef BRK_SERVICE_DISPATCH_EVENT_SERVICE_H_
#define BRK_SERVICE_DISPATCH_EVENT_SERVICE_H_

#include <map>
#include <vector>
#include <queue>
//forward_list是C++11新增的单链表存储相同个数的同类型元素，单链表耗用的内存空间更少，
//空间利用率更高，并且对于实现某些操作单链表的执行效率也更高。
#include <forward_list> 
#include "ievent.h"
#include "eventtype.h"
#include "iEventHandler.h"
#include "threadpool/thread_pool.h"
#include "NetworkInterface.h"


class DispatchMsgService
{
protected:
	DispatchMsgService();

public:
	virtual ~DispatchMsgService();

	virtual BOOL open();//打开服务，线程池在此创建
	virtual void close();

	virtual void subscribe(u32 eid, iEventHandler* handler);
	virtual void unsubscribe(u32 eid, iEventHandler* handler);

	//把事件投递到线程池中进行处理
	virtual i32 enqueue(iEvent* ev);

	//线程池回调函数,C语言的线程池，回调C++方法，定义成静态因为C++，会传递this指针这样会导致错误
	static void svc(void* argv);

	//对具体的事件进行分发处理
	virtual iEvent* process(const iEvent* ev);

	static DispatchMsgService* getInstance();
	static void addClientNumber();
	static void addUserToList(struct bufferevent* user);
	static void decUserFromList(struct bufferevent* user);
	static std::forward_list<struct bufferevent*> getUserList();

	iEvent* parseEvent(const char *message, u32 len, i32 eid);
	
	//处理响应事件
	void handleAllResponseEvent(NetworkInterface * interface);
	//发送响应信息
	void sendPesponseMessage(iEvent* ev, EventID Eid, NetworkInterface* interface);

protected:
	thread_pool_t* tp;

	static DispatchMsgService* DMS_;//单例
	static int clientNumber;                
	static std::forward_list<struct bufferevent*> m_userList;//用户事件列表
	
	typedef std::vector<iEventHandler*> T_EventHandlers;//用户事件管理容器
	typedef std::map<u32, T_EventHandlers> T_EventHandlersMap;//事件对应的处理器，一个事件可能有多个处理器,但是当前并没有需要多个处理器的业务
	T_EventHandlersMap subscribers_;   //订阅者
	
	bool svr_exit_;

	static std::queue<iEvent *> response_events;//响应事件队列
	static pthread_mutex_t queue_mutext;//响应事件锁
	static NetworkInterface* NTIF_;  //网络监听服务
};



#endif // !BRK_SERVICE_DISPATCH_EVENT_SERVICE_H_


