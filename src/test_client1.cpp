#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <string>
#include <arpa/inet.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include "bike.pb.h" // Protobuf 头文件

#define EEVENTID_LOGIN_REQ 5 // 登录请求
#define EEVENTID_LOGIN_RSP 6 // 登录响应

#define SERVER_PORT 6666
#define SERVER_IP "192.168.31.129"
#define BATCH_SIZE 10000 // 每批处理 10,000 个连接

typedef unsigned short u16;
typedef unsigned int u32;

std::mutex log_mutex; // 用于线程安全日志输出

void sendLoginRequest(const std::string &username, const std::string &password, int clientID);
void handleServerResponse(struct bufferevent *bev);
void onEventCallback(struct bufferevent *bev, short events, void *arg);
int tcp_connect_server(const char *server_ip, int port);

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <total_clients>\n";
        return -1;
    }

    int totalClients = std::stoi(argv[1]);  // 获取总的客户端数量
    int numBatches = (totalClients + BATCH_SIZE - 1) / BATCH_SIZE; // 计算批次数量
    std::vector<std::thread> threads;

    std::cout << "Total clients: " << totalClients << "\n";
    std::cout << "Processing in " << numBatches << " batches of " << BATCH_SIZE << "...\n";

    for (int batch = 0; batch < numBatches; ++batch) {
        int startID = batch * BATCH_SIZE + 1;
        int endID = std::min((batch + 1) * BATCH_SIZE, totalClients);

        std::cout << "Starting batch " << (batch + 1) << " (" << startID << " to " << endID << ")...\n";

        // 创建当前批次的线程
        for (int i = startID; i <= endID; ++i) {
            threads.emplace_back(sendLoginRequest, "testuser", "testuser", i);
        }

        // 等待当前批次完成
        for (auto &t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
        threads.clear(); // 清空线程向量，为下一批次释放资源

        std::cout << "Batch " << (batch + 1) << " completed.\n";
    }

    std::cout << "All clients finished.\n";
    return 0;
}

void sendLoginRequest(const std::string &username, const std::string &password, int clientID) {
    int sockfd = tcp_connect_server(SERVER_IP, SERVER_PORT);
    if (sockfd < 0) {
        std::lock_guard<std::mutex> lock(log_mutex);
        std::cerr << "Client " << clientID << ": Failed to connect to server.\n";
        return;
    }

    struct event_base *base = event_base_new();
    struct bufferevent *bev = bufferevent_socket_new(base, sockfd, BEV_OPT_CLOSE_ON_FREE);

    if (!bev) {
        std::lock_guard<std::mutex> lock(log_mutex);
        std::cerr << "Client " << clientID << ": Failed to create bufferevent.\n";
        close(sockfd);
        return;
    }

    bufferevent_setcb(bev, [](struct bufferevent *bev, void *arg) {
        handleServerResponse(bev);
    }, NULL, onEventCallback, NULL);
    bufferevent_enable(bev, EV_READ | EV_PERSIST);

    // 构造 Protobuf 消息
    tutorial::login_request lr;
    lr.set_username(username);
    lr.set_userpwd(password);

    int len = lr.ByteSizeLong();
    char msg[1024] = {0};

    memcpy(msg, "FBEB", 4);
    *(u16 *)(msg + 4) = EEVENTID_LOGIN_REQ;
    *(u32 *)(msg + 6) = len;
    lr.SerializeToArray(msg + 10, len);

    {
        std::lock_guard<std::mutex> lock(log_mutex);
        std::cout << "Client " << clientID << ": Sending login request...\n";
    }

    if (bufferevent_write(bev, msg, len + 10) != 0) {
        std::lock_guard<std::mutex> lock(log_mutex);
        std::cerr << "Client " << clientID << ": Failed to send login request.\n";
        bufferevent_free(bev);
        event_base_free(base);
        close(sockfd);
        return;
    }

    event_base_dispatch(base); // 进入事件循环
    bufferevent_free(bev);
    event_base_free(base);
}

void handleServerResponse(struct bufferevent *bev) {
    char response[1024];
    size_t len = bufferevent_read(bev, response, sizeof(response));
    if (len <= 0) {
        std::lock_guard<std::mutex> lock(log_mutex);
        std::cerr << "Failed to read response from server.\n";
        return;
    }

    if (strncmp(response, "FBEB", 4) == 0) {
        u16 eventID = *(u16 *)(response + 4);
        u32 dataLen = *(u32 *)(response + 6);

        if (eventID == EEVENTID_LOGIN_RSP) {
            tutorial::login_response lr;
            if (lr.ParseFromArray(response + 10, dataLen)) {
                std::lock_guard<std::mutex> lock(log_mutex);
                std::cout << "Login Response Received:\n";
                std::cout << "  Username: " << lr.username() << "\n";
                std::cout << "  ResCode: " << lr.rescode() << "\n";
            } else {
                std::cerr << "Failed to parse login response.\n";
            }
        } else {
            std::cerr << "Unknown Event ID: " << eventID << "\n";
        }
    } else {
        std::cerr << "Invalid response header.\n";
    }
}

void onEventCallback(struct bufferevent *bev, short events, void *arg) {
    if (events & BEV_EVENT_EOF) {
        std::lock_guard<std::mutex> lock(log_mutex);
        std::cout << "Server closed the connection.\n";
    }
    if (events & BEV_EVENT_ERROR) {
        std::lock_guard<std::mutex> lock(log_mutex);
        std::cerr << "A network error occurred.\n";
    }

    bufferevent_free(bev);
    struct event_base *base = bufferevent_get_base(bev);
    if (base) {
        event_base_loopexit(base, NULL);
    }
}

int tcp_connect_server(const char *server_ip, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return -1;
    }

    return sockfd;
}
