#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <cassert>
#include <cerrno>  // 引入 errno 头文件

#define BUF_SIZE 1024

void distributeMessage(int* conns, int connAmount, int nowConn, const char* clientIp, const char* msg) {
    char final_msg[100];
    std::memset(final_msg, '\0', sizeof(final_msg));

    // from *.*.*.* : msg\n 
    std::strcat(final_msg, "from ");
    std::strcat(final_msg, clientIp);
    std::strcat(final_msg, ": ");
    std::strcat(final_msg, msg);
    std::strcat(final_msg, "\n");

    for (int i = 0; i <= connAmount && i != nowConn; i++) {
        send(conns[i], final_msg, BUF_SIZE - 1, 0);
    }
}

int main(int argc, char* argv[]) {

    if (argc <= 2) {
        std::cout << "error, argument amount error" << std::endl;
        return 1;
    }
    // 获取IP和端口
    char* ip = argv[1];
    int port = std::atoi(argv[2]);

    // 把当前套接字的信息用sockaddr_in保存
    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    address.sin_family = AF_INET;
    // 创建用于监听的套接字
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    // 设置sock能为 time_wait 状态的IP:端口
    int resue = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &resue, sizeof(resue));

    // 绑定本机的IP端口
    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    if (ret == -1) {
        std::cout << "error code is: " << errno << ", error is: " << strerror(errno) << std::endl;
        return 1;
    }

    // 设置监听
    ret = listen(sock, 5);
    assert(ret != -1);

    int maxClient = 2;
    int clientAmount = 0;
    struct sockaddr_in clients[maxClient];

    int acceptSocket[maxClient];
    int acceptConn[maxClient];

    while (clientAmount != maxClient) {
        // 创建用于连接客户端的套接字
        struct sockaddr_in* client = &clients[clientAmount];
        socklen_t client_addrlength = sizeof(*client);
        // accept
        int conn = accept(sock, (struct sockaddr*)client, &client_addrlength);
        if (conn != -1) {
            char newSocketIp[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(client->sin_addr), newSocketIp, INET_ADDRSTRLEN);
            std::cout << "New connection established - " << sock << ", IP address: " << newSocketIp << std::endl;
            acceptSocket[clientAmount] = sock;
            acceptConn[clientAmount] = conn;
            clientAmount++;
        }
    }

    for (int i = 0;; i++) {
        char recvBuff[BUF_SIZE];
        std::memset(recvBuff, '\0', BUF_SIZE);
        recv(acceptConn[i % maxClient], recvBuff, BUF_SIZE - 1, MSG_DONTWAIT);

        if (recvBuff[0] != '\0') {
            char newMsgIp[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(clients[i % maxClient].sin_addr), newMsgIp, INET_ADDRSTRLEN);
            std::cout << "Received message from " << newMsgIp << ": " << recvBuff << std::endl;
            distributeMessage(acceptConn, clientAmount - 1, i % maxClient - 1, newMsgIp, recvBuff);
        }
        if (!std::strcmp(recvBuff, "exit")) {
            return 1;
        }
    }

    return 0;
}
