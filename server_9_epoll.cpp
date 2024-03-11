#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// 定义的宏
// 监听最大个数
#define LISTEN_BACKLOG (5)
// buf缓冲区大小
#define BUF_SIZE (2048)

#define ONCE_READ_SIZE (1500)

#define EPOLL_SIZE (100);

#define MAX_EVENTS (10)

// 用于打印服务器ip和端口
void usage(void) {
    printf("*********************************\n");
    printf("./server 本端ip 本端端口\n");
    printf("*********************************\n");
}

// 用flag获取描述符详细，并且设置成 非阻塞模式
void setnonblocking(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

int main(int argc, char* argv[])
{
    struct sockaddr_in local;
    struct sockaddr_in peer;
    socklen_t addrlen = sizeof(peer);
    int new_fd = 0;
    int ret = 0;

    // 初始化 写  读  缓冲区
    char send_buf[BUF_SIZE] = { 0 };
    char recv_buf[BUF_SIZE] = { 0 };

    if (argc != 3) {
        usage();
        return -1;
    }

    // 初始化服务器 描述符
    char* ip = argv[1];
    unsigned short port = atoi(argv[2]);
    printf("ip:port->%s:%u\n", argv[1], port);

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket error");
        return -1;
    }

    memset(&local, 0, sizeof(struct sockaddr_in));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = inet_addr(ip);
    local.sin_port = htons(port);

    ret = bind(sock_fd, (struct sockaddr*)&local, sizeof(struct sockaddr));
    if (ret == -1) {
        close(sock_fd);
        perror("bind error");
        return -1;
    }

    // 设置监听
    ret = listen(sock_fd, LISTEN_BACKLOG);
    if (ret == -1) {
        close(sock_fd);
        perror("listen error");
        return -1;
    }

    // 创建一个 epoll实例
    int epoll_size = EPOLL_SIZE;
    int efd = epoll_create(epoll_size);
    if (efd == -1) {
        perror("epoll create error");
        return -1;
    }

    // 创建 epoll事件对象和数组
    struct epoll_event ev;
    struct epoll_event events[MAX_EVENTS];

    // 先把 要监听的套接字描述符与epoll事件关联，然后把他加入到epoll实例对象中进行监听
    ev.data.fd = sock_fd;
    ev.events = EPOLLIN;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, sock_fd, &ev) == -1) {
        perror("epoll ctl ADD error");
        return -1;
    }

    int timeout = 1000;
    while (1) 
    {
        int nfds = epoll_wait(efd, events, MAX_EVENTS, timeout);
        if (nfds == -1) 
        {
            perror("epoll wait error");
            return -1;
        }
        else if (nfds == 0) 
        {
            printf("epoll wait timeout\n");
            continue;
        }
        else 
        {

        }

        for (int i = 0; i < nfds; i++) 
        {
            // 当前处理的监测到可执行的描述符 设为fd
            int fd = events[i].data.fd;
            printf("events[%d] events:%08x\n", i, events[i].events);

            // 如果fd == sock_fd 表示有客户端的连接请求
            if (fd == sock_fd) 
            {
                new_fd = accept(sock_fd, (struct sockaddr*)&peer, &addrlen);
                if (new_fd == -1) 
                {
                    perror("accept error");
                    continue;
                }
                // 把客户端的对象设置 非阻塞，边缘模式 然后加入到 epoll实体对象的监测中
                setnonblocking(new_fd);
                ev.data.fd = new_fd;
                ev.events = EPOLLIN | EPOLLET;
                if (epoll_ctl(efd, EPOLL_CTL_ADD, new_fd, &ev) == -1) 
                {
                    perror("epoll ctl ADD new fd error");
                    close(new_fd);
                    continue;
                }
            }
            else 
            {
                // 如果是可读请求
                if (events[i].events & EPOLLIN) 
                {
                    printf("fd:%d is readable\n", fd);
                    memset(recv_buf, 0, BUF_SIZE);
                    unsigned int len = 0;
                    // 处理可读
                    // recv的返回值 >0 表示读取了多少字节 =0 表示读取完毕,客户端关闭了连接 =-1 有错
                    while (1) 
                    {
                        ret = recv(fd, recv_buf + len, ONCE_READ_SIZE, 0);
                        if (ret == 0) 
                        {
                            printf("remove fd:%d\n", fd);
                            epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
                            close(fd);
                            break;
                        }
                        else if ((ret == -1) && ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))) 
                        {
                            printf("fd:%d recv errno:%d done\n", fd, errno);
                            break;
                        }
                        else if ((ret == -1) && !((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))) 
                        {
                            printf("remove fd:%d errno:%d\n", fd, errno);
                            epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
                            close(fd);
                            break;
                        }
                        else 
                        {
                            printf("once read ret:%d\n", ret);
                            len += ret;
                        }
                    }
                    printf("recv fd:%d, len:%d, %s\n", fd, len, recv_buf);
                } 
                else if (events[i].events & EPOLLOUT)  // 处理可写
                {
                    printf("fd:%d is sendable\n", fd);
                }
                else if ((events[i].events & EPOLLERR) || ((events[i].events & EPOLLHUP))) 
                {
                    printf("fd:%d error\n", fd);
                }
            }
        }
    }
    return 0;
}