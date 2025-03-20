#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>

#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <string>
#include "bike.pb.h"

#define SERVER_PORT 666
#define SERVER_IP "127.0.0.1"

typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;

using namespace std;
using namespace tutorial;

// 全局变量：存储icode（假设用于登录）
static int icode = 0;

// 函数声明
int tcp_connect_server(const char *server_ip, int port);
void cmd_msg_cb(int fd, short events, void *arg);
void server_msg_cb(struct bufferevent *bev, void *arg);
void event_cb(struct bufferevent *bev, short event, void *arg);

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        return -1;
    }

    int sockfd = tcp_connect_server(argv[1], atoi(argv[2]));
    if (sockfd == -1) {
        fprintf(stderr, "Failed to connect to server.\n");
        return -1;
    }

    printf("Connected to the server successfully!\n");

    // 初始化libevent事件基础设施
    struct event_base *base = event_base_new();
    struct bufferevent *bev = bufferevent_socket_new(base, sockfd, BEV_OPT_CLOSE_ON_FREE);

    // 创建监听标准输入的事件
    struct event *ev_cmd = event_new(base, STDIN_FILENO, EV_READ | EV_PERSIST, cmd_msg_cb, (void *)bev);
    event_add(ev_cmd, NULL);

    // 设置缓冲区事件回调
    bufferevent_setcb(bev, server_msg_cb, NULL, event_cb, (void *)ev_cmd);
    bufferevent_enable(bev, EV_READ | EV_PERSIST);

    // 进入事件循环
    event_base_dispatch(base);

    // 清理资源
    event_free(ev_cmd);
    bufferevent_free(bev);
    event_base_free(base);

    printf("Finished!\n");
    return 0;
}

int tcp_connect_server(const char *server_ip, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

void cmd_msg_cb(int fd, short events, void *arg) {
    char cmd[1024];
    char msg[1024];

    int ret = read(fd, cmd, sizeof(cmd));
    if (ret <= 0) {
        perror("read failed");
        exit(-1);
    }
    cmd[ret - 1] = '\0'; // 去掉换行符

    printf("Read command: [%s]\n", cmd);

    struct bufferevent *bev = (struct bufferevent *)arg;

    if (strcmp(cmd, "sendMr") == 0) {
        // 构造 mobile_request 消息
        mobile_request mr;
        mr.set_username("123456466546");

        int len = mr.ByteSizeLong();
        memcpy(msg, "FBEB", 4);       // 魔术字
        *(u16 *)(msg + 4) = 1;        // 消息类型
        *(i32 *)(msg + 6) = len;      // 消息长度
        mr.SerializeToArray(msg + 10, len);

        bufferevent_write(bev, msg, len + 10);
    } else if (strcmp(cmd, "sendLr") == 0) {
        // 构造 login_request 消息
        login_request lr;
        lr.set_username("123456466546");
        lr.set_userpwd("password123");

        int len = lr.ByteSizeLong();
        memcpy(msg, "FBEB", 4);       // 魔术字
        *(u16 *)(msg + 4) = 2;        // 消息类型
        *(i32 *)(msg + 6) = len;      // 消息长度
        lr.SerializeToArray(msg + 10, len);

        bufferevent_write(bev, msg, len + 10);
    } else {
        printf("Unknown command!\n");
    }
}

void server_msg_cb(struct bufferevent *bev, void *arg) {
    char msg[1024];
    size_t len = bufferevent_read(bev, msg, sizeof(msg));
    if (len <= 0) {
        printf("Failed to read message from server.\n");
        return;
    }

    msg[len] = '\0';
    printf("Received message from server: [%s], length: %ld\n", msg, len);

    if (strncmp(msg, "FBEB", 4) == 0) {
        u16 code = *(u16 *)(msg + 4);
        i32 msg_len = *(i32 *)(msg + 6);

        if (code == 1) { // mobile_response
            mobile_response mr;
            if (mr.ParseFromArray(msg + 10, msg_len)) {
                printf("Mobile response - resCode: %d, verCode: %d, data: %s\n",
                       mr.rescode(), mr.vercode(), mr.data().c_str());
            }
        } else if (code == 2) { // login_response
            login_response lr;
            if (lr.ParseFromArray(msg + 10, msg_len)) {
                printf("Login response - resCode: %d, userName: %s\n",
                       lr.rescode(), lr.username().c_str());
            }
        } else {
            printf("Unknown message code: %d\n", code);
        }
    }
}

void event_cb(struct bufferevent *bev, short events, void *arg) {
    if (events & BEV_EVENT_EOF) {
        printf("Connection closed by server.\n");
    } else if (events & BEV_EVENT_ERROR) {
        printf("An error occurred with the connection.\n");
    }

    // 清理资源
    struct event *ev_cmd = (struct event *)arg;
    event_free(ev_cmd);
    bufferevent_free(bev);
}

