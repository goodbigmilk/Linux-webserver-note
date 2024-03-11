#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024

static const char* status_line[2] = { "200 OK", "500 Server Error" };

int main() {

	//if (argc <= 3) {
	//	printf("Wrong number of parameters\n");
	//	return 1;
	//}

	// 指定ip,端口,文件名
	char* ip = "192.168.10.110";
	int port = 9999;
	char* file_name = "/home/milk/a.txt";

	// 以只读方式，打开文件
	int filefd = open(file_name, O_RDONLY);
	assert(filefd > 0);
	// 创建stat类型的结构体stat_buf用于保存文件信息
	struct stat stat_buf;
	// 读取filedf描述符对应文件的信息,存入stat_buf中
	fstat(filefd, &stat_buf);


	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;

	// 创建描述符并且绑定
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(sockfd >= 0);
	int retu = bind(sockfd, (struct sockaddr*)&address, sizeof(address));
	assert(retu != -1);

	// 设置监听，最大监听数为5
	retu = listen(sockfd, 5);
	assert(sockfd != -1);

	// 创建接收信息的客户端的标识符
	struct sockaddr_in client;
	socklen_t client_addrlength = sizeof(client);
	int connfd = accept(sockfd, (struct sockaddr*)&client, &client_addrlength);
	if (connfd >= 0) {
		// 使用sendfile把文件发送给客户端
		sendfile(connfd, filefd, NULL, stat_buf.st_size);
		close(connfd);
	}
	close(sockfd);
	return 0;
}