﻿#include "NetworkInterface.h"
#include "DispatchMsgService.h"
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



//ConnectSession 必须是C类型的成员变量,因为new一个对象之后，如果memset将这个对象内部嵌套的其他类对象例如string，初始化为0，那么就会将这个对象给覆盖，是不好的行为
static ConnectSession* session_init(int fd, struct bufferevent* bev)
{
	ConnectSession* temp = nullptr; //bufferevent

	temp = new ConnectSession();

	if (!temp)
	{
		fprintf(stderr, "malloc failed. reason: %m\n");
		return nullptr;
	}
	memset(temp, '\0', sizeof(ConnectSession));
	temp->bev = bev;//bufferevent
	temp->fd = fd;
	return temp;
}

void session_free(ConnectSession *cs)
{
	if (cs)
	{		
		if (cs->read_buf)
		{
			delete[] cs->read_buf;
			cs->read_buf = nullptr;
		}
	
		if (cs->write_buf)
		{
			delete[] cs->write_buf;
			cs->write_buf = nullptr;
		}
		delete cs;
		cs = nullptr;
	}
}

NetworkInterface::NetworkInterface()
{
	base_ = nullptr;
	listener_ = nullptr;
}

NetworkInterface::~NetworkInterface()
{
	LOG_DEBUG("网络接口服务已关闭\n");
	close();
}

bool NetworkInterface::start(int port)
{
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	//创建事件管理池
	base_ = event_base_new();
	//创建端口事件监听
	//LEV_OPT_REUSEABLE：允许端口被重复使用。

	//LEV_OPT_CLOSE_ON_FREE：释放监听器时，自动关闭底层套接字。
	listener_ = evconnlistener_new_bind(base_, NetworkInterface::listener_cb, base_,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
		512, (struct sockaddr*)&sin,
		sizeof(struct sockaddr_in));
	return true;
}

void NetworkInterface::close()
{
	if (base_)
	{
		event_base_free(base_);
		base_ = nullptr;
	}
	if (listener_)
	{
		evconnlistener_free(listener_);
		listener_ = nullptr;
	}
}

void NetworkInterface::session_reset(ConnectSession * cs)
{
	if (cs)
	{
		LOG_DEBUG("正在释放资源......");
		if (cs->read_buf)
		{
			delete cs->read_buf;
			cs->read_buf = nullptr;
		}
		if (cs->write_buf)
		{
			delete cs->write_buf;
			cs->write_buf = nullptr;
		}

		cs->session_stat = SESSION_STATUS::SS_REQUEST;
		cs->req_stat = MESSAGE_STATUS::MS_READ_HEADER;

		cs->message_len = 0;
		cs->read_message_len = 0;
		cs->read_header_len = 0;
		LOG_DEBUG("释放资源完成！");
	}
}

void NetworkInterface::listener_cb(evconnlistener * listener, evutil_socket_t fd, sockaddr * sock, int socklen, void * arg)
{
	//事件base
	struct event_base* base = (struct event_base*)arg;
	
	//为这个客户端分配一个bufferevent
	struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	
	ConnectSession* cs = session_init(fd, bev);
	
	cs->session_stat = SESSION_STATUS::SS_REQUEST;//
	cs->req_stat = MESSAGE_STATUS::MS_READ_HEADER;
	/*inet_ntoa 是一个用于处理网络地址的 C 语言函数，其全称是 "Internet Network Address to ASCII String"。
	它的作用是将一个网络字节序的 IP 地址（通常是 struct in_addr 类型）转换为一个点分十进制的字符串表示形式，方便阅读。*/
	strcpy(cs->remote_ip, inet_ntoa(((sockaddr_in*)sock)->sin_addr));
	LOG_DEBUG("接收到共享单车用户连接请求， ip :%s[%p]\n", cs->remote_ip, cs->bev);
	LOG_DEBUG("用户已成功连接服务器！ ip :%s[%p]\n", cs->remote_ip, cs->bev);
	printf("接收到共享单车用户连接请求， ip :%s[%p]\n", cs->remote_ip, cs->bev);
	printf("用户已成功连接服务器！ ip :%s[%p]\n", cs->remote_ip, cs->bev);
	DispatchMsgService::addUserToList(cs->bev);
	//DispatchMsgService::addClientNumber();
	bufferevent_setcb(bev, handle_request, handle_response, handle_error, cs);
	bufferevent_enable(bev, EV_READ | EV_PERSIST);

	//设置超时值，10秒没有写或者读时，即超时时，调用出错函数 handle_error
	bufferevent_settimeout(bev, 1000, 1000);//超时值应设置在配置文件中
}

void NetworkInterface::handle_request(bufferevent * bev, void * arg)
{
	ConnectSession *cs = (ConnectSession*)(arg);
	if (cs->session_stat != SESSION_STATUS::SS_REQUEST)
	{
		LOG_WARN("NetworkInterface::handle_request wrong session stat[%d]\n", cs->session_stat);
		return;
	}
	
	//char read_message[1024];
	//i32 len = bufferevent_read(bev, read_message, 1024);
	//read_message[len] = '\0';
	//DispatchMsgService::addClientNumber();
	//LOG_DEBUG("read message from client[%p]: %s", bev, read_message);
	//len = bufferevent_write(bev, read_message, 1024);
	
	//if (cs->req_stat == MESSAGE_STATUS::MS_READ_HEADER)//读取头部
	if(cs->req_stat == MESSAGE_STATUS::MS_READ_HEADER)
	{
		
		i32 len = bufferevent_read(bev, cs->header + cs->read_header_len, MESSAGE_HEADER_LEN - cs->read_header_len);
		cs->read_header_len += len;
		cs->header[cs->read_header_len] = '\0';

		LOG_DEBUG("recv from client cs->header[%s], lenth[%d]", cs->header, cs->read_header_len);
		printf("recv from client cs->header[%s], lenth[%d]", cs->header, cs->read_header_len);
		//printf("recv from client cs->header<<<< %s \n", cs->header);
		//printf("read_header_len:%d\n", cs->read_header_len);

		if (cs->read_header_len == MESSAGE_HEADER_LEN)//读取头部长度
		{
			if (strncmp(cs->header, MESSAGE_HEADER_ID, strlen(MESSAGE_HEADER_ID)) == 0)
			{
				//FBEB	   事件ID	   数据长度N	 数据内容
				//4字节    2个字节      4个字节
				cs->eid = *((u16*)(cs->header + 4));		 //获取事件ID
				if (cs->eid > 34) {
					LOG_ERROR("cs->eid==%d", cs->eid);

					printf("cs->eid==%d", cs->eid);
				}
				//cs->message_len = *((u32 *)(cs->header + 6));
				cs->message_len = *((u32*)(cs->header + 6));//获取数据长度
				//LOG_DEBUG("NetworkInterface::handle_request erron: message_len:%d\n", cs->message_len);
				if (cs->message_len < 1 || cs->message_len > MAX_MESSAGE_LEN)
				{
					LOG_ERROR("NetworkInterface::handle_request erron: message_len:%d\n", cs->message_len);
					printf("NetworkInterface::handle_request erron: message_len:%d\n", cs->message_len);
					bufferevent_free(bev);
					session_free(cs);
					return;
				}
				LOG_DEBUG("cs->message_len: %d", cs->message_len);
				LOG_DEBUG("cs->read_header_len: %d", cs->read_header_len);

				printf("cs->message_len: %d", cs->message_len);
				printf("cs->read_header_len: %d", cs->read_header_len);
				cs->req_stat = MESSAGE_STATUS::MS_READ_MESSAGE;
				cs->read_buf = new char[cs->message_len];
				//cs->read_buf = NULL;
				cs->read_message_len = 0;
			}
			else
			{
				LOG_ERROR("NetworkInterface::handle_request -Invalid request from:%s\n", cs->remote_ip);
				//printf("NetworkInterface::handle_request -Invalid request from:%s\n", cs->remote_ip);
				//直接关闭请求，不给予任何相应，防止客户端恶意试探
				bufferevent_free(bev);//
				session_free(cs);
				return;
			}
			
		}
		
	}
	//读取消息流的数据部分，并判断缓冲区是否还有数据需要读
	if (cs->req_stat == MESSAGE_STATUS::MS_READ_MESSAGE && evbuffer_get_length(bufferevent_get_input(bev)) > 0)
	{
		//i32 len = bufferevent_read(bev, cs->read_buf + cs->read_message_len, cs->message_len - cs->read_message_len);
		i32 len = bufferevent_read(bev, cs->read_buf + cs->read_message_len, cs->message_len - cs->read_message_len);
		cs->read_message_len += len;
		LOG_DEBUG("读取信息：NetworkInterface::handle_request: bufferevnet_read: %d byte\n, read_message_len: %d, message_len:%d\n", len, cs->read_message_len, cs->message_len);
		printf("读取信息：NetworkInterface::handle_request: bufferevnet_read: %d byte\n, read_message_len: %d, message_len:%d\n", len, cs->read_message_len, cs->message_len);
		//printf("NetworkInterface::handle_request: bufferevnet_read: %d byte, read_message_len: %d, message_len:%d\n", len, cs->read_message_len, cs->message_len);

		if (cs->read_message_len == cs->message_len)
		{
			LOG_DEBUG("cs->read_message_len:%d", cs->read_message_len);
			LOG_DEBUG("cs->message_len:%d\n", cs->message_len);
			printf("cs->read_message_len:%d", cs->read_message_len);
			printf("cs->message_len:%d\n", cs->message_len);
			//printf("!!!cs->read_buf: %s\n", cs->read_buf);
			cs->session_stat = SESSION_STATUS::SS_RESPONSE;
			//解析收到的数据，生成响应的请求事件			
			iEvent *ev = DispatchMsgService::getInstance()->parseEvent(cs->read_buf, cs->read_message_len, cs->eid);
			//释放cs->read_buf的空间
			delete[] cs->read_buf;
			cs->read_buf = nullptr;
			cs->read_message_len = 0;

			if (ev)
			{			
				ev->set_args(cs);//保存事件对应的连接
				DispatchMsgService::getInstance()->enqueue(ev);//投入线程池处理
			}
			//ev->set_args(cs);
			else
			{
				LOG_ERROR("NetworkInterface::handle_request ev is null, remote ip:%s, eid=%d\n", cs->remote_ip, cs->eid);
				printf("NetworkInterface::handle_request ev is null, remote ip:%s, eid=%d\n", cs->remote_ip, cs->eid);
				//直接关闭请求，不给予任何相应，防止客户端恶意试探
				bufferevent_free(bev);
				session_free(cs);
				return;
			}
		}
		/*
		   如果 read_message_len < message_len，函数并未终止，而是依赖 Libevent 的事件触发机制：

           每当新的数据到达，Libevent 会触发 handle_request 回调，继续读取剩余数据。

          因此，数据读取是逐步完成的，无需手动循环等待。*/
	
	}
		
}

void NetworkInterface::handle_response(bufferevent * bev, void * arg)
{
	LOG_DEBUG("NetworkInterface::handle_response。。。");
	
	//ConnectSession* cs = (ConnectSession*)(arg);
	//bufferevent_write(bev, cs->read_buf, 100);
	//LOG_DEBUG("cs->read_buf:%s\n", cs->read_buf);
}

//超时，连接关闭，读写出错等异常情况指定调用的出错回调函数
void NetworkInterface::handle_error(bufferevent * bev, short event, void * arg)
{
	LOG_DEBUG("NetworkInterface::handle_error。。。\n");

	ConnectSession *cs = (ConnectSession *)(arg);
	if (event & BEV_EVENT_EOF)
	{
		LOG_DEBUG("client[%s][%p] connection closed.\n", cs->remote_ip, cs->bev);
		//DispatchMsgService::decUserFromList(bev);
	}
	else if ((event & BEV_EVENT_TIMEOUT) && (event & BEV_EVENT_READING))
	{
		LOG_WARN("client[%s][%p] reading timeout", cs->remote_ip, cs->bev);
		//DispatchMsgService::decUserFromList(bev);
		
	}
	else if ((event & BEV_EVENT_TIMEOUT) && (event & BEV_EVENT_WRITING))
	{
		LOG_WARN("NetworkInterface::writting timeout ...\
				 client ip: %s\n", cs->remote_ip);
		//DispatchMsgService::decUserFromList(bev);
	}
	else if (event & BEV_EVENT_ERROR)
	{
		LOG_WARN("NetworkInterface::other some error ...\
				 client ip: %s\n", cs->remote_ip);
		//DispatchMsgService::decUserFromList(bev);
	}
	DispatchMsgService::decUserFromList(bev);
	//关闭该客户端bev, 和cs读写缓冲区
	bufferevent_free(bev);
	session_free(cs);
}

void NetworkInterface::network_event_dispatch()
{
	event_base_loop(base_, EVLOOP_NONBLOCK);//有事件立即处理，没有返回，继续下一个
	
	//printf("event_base_loop(base_, EVLOOP_NONBLOCK); before---\n");
	DispatchMsgService::getInstance()->handleAllResponseEvent(this);
}

void NetworkInterface::send_response_message(ConnectSession * cs)
{
	if (cs->response == nullptr)
	{
		bufferevent_free(cs->bev);
		if (cs->request)//没有响应事件那么请求事件也不需要存在
		{
			delete cs->request;
			cs->request = nullptr;
		}
		session_free(cs);
	}
	else
	{
		//崩溃了这里，解决了，bev出现了问题，
		//bufferevent_write(cs->bev, cs->write_buf, cs->message_len + MESSAGE_HEADER_LEN);
		if (cs->bev)
		{			
			LOG_DEBUG("序列化数据成功!");
			printf("序列化数据成功!\n");
 			if (cs->eid == EEVENTID_BROADCAST_MSG_RSP)
			{
				
				std::forward_list<struct bufferevent*> userList= DispatchMsgService::getUserList();
				LOG_DEBUG("正在广播响应中......");
				printf("正在广播响应中......\n");
				for (auto iter = userList.begin(); iter != userList.end(); iter++)
				{
					LOG_DEBUG("broadcast response to client: %p", *iter);
					printf("broadcast response to client: %p\n", *iter);
					bufferevent_write(*iter, cs->write_buf, cs->message_len + MESSAGE_HEADER_LEN);
				}								
				LOG_DEBUG("广播响应完成！");
				printf("广播响应完成！\n");

			}
			else 
			{
				LOG_DEBUG("正在回应客户端cs->bev[%p],message[%s]......", cs->bev, cs->write_buf);
				printf("正在回应客户端cs->bev[%p],message[%s]......\n", cs->bev, cs->write_buf);
				bufferevent_write(cs->bev, cs->write_buf, cs->message_len + MESSAGE_HEADER_LEN);
				printf("回应客户端成功\n");
			}
		}
		
		else
			LOG_ERROR("cs->bev is empty! cann't send to client\n, NetworkInterface.cpp, line[304]");
		if(cs)	
			session_reset(cs);//响应操作完成，重置连接状态
	}
}
