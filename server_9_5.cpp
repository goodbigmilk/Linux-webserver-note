#include <sys/socket.h>
#include <sys/types.h>
#include <cassert>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <unistd.h>
#include <cstdio>
#include <fcntl.h>
#include <cerrno>

// 设置为非阻塞并返回 原配置 方便复原 
int setnonblocking(int fd) {

	int oldopt = fcntl(fd, F_GETFL);
	int newopt = oldopt | O_NONBLOCK;
	fcntl(fd, F_SETFL, newopt);
	// 注意返回的是 为设置为阻塞前的
	return oldopt;
}

int unblock_connect(const char* ip, int port, int time) 
{

	struct sockaddr_in address;
	address.sin_port = htons(port);
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);

	// 创建套接字描述符并设置为非阻塞
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	int oldsockopt = setnonblocking(sockfd);
	int ret = connect(sockfd, (struct sockaddr*)&address, sizeof(address));
	// 连接成功  
	if (ret == 0) 
	{
		printf("connect with server immediately\n");
		// 恢复socket属性 (一般情况下，当我们需要进行非阻塞 connect 时，会将套接字设置为非阻塞模式，
		// 以便可以立即返回并继续执行后续的逻辑。但是在部分代码中，可能会希望在连接成功后将套接字恢复为原来的阻塞模式，
		// 这样可以确保后续的网络操作或者数据传输不受影响，同时也符合代码的健壮性和可维护性。)
		fcntl(sockfd, F_SETFL, oldsockopt);

		return sockfd;
	}
	else if (errno != EINPROGRESS) {
		printf("unblock connect not support\n");
		return -1;
	}
	//  EINPROGRESS表示连接正在进行 
	fd_set readfds;
	fd_set writefds;
	struct timeval timeout;

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_SET(sockfd, &writefds);

	timeout.tv_sec = time;
	timeout.tv_usec = 0;

	ret = select(sockfd + 1, nullptr, &writefds, nullptr, &timeout);
	if (ret <= 0) {
		printf("connect time error\n");
		close(sockfd);
		return -1;
	}

	if (!FD_ISSET(sockfd, &writefds)) {
		printf("no events on sockfd found\n");
		close(sockfd);
		return -1;
	}

	int error = 0;
	socklen_t length = sizeof(error);

	// 获取连接的错误 
	if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &length) < 0) {
		printf("get socket option failed\n");
		close(sockfd);
		return -1;
	}

	if (error != 0) {
		printf("connection failed after select with the error: %d \n", error);
		close(sockfd);
		return -1;
	}

	printf("connect ready after select with the socket: %d \n", sockfd);
	fcntl(sockfd, F_SETFL, oldsockopt);
	return sockfd;
}

int main(int argc, char* argv[]) {
	const char* ip = "192.168.9.35";
	int port = atoi(argv[1]);

	int sockfd = unblock_connect(ip, port, 10);
	if (sockfd < 0) {
		return 1;
	}


	return 0;
}