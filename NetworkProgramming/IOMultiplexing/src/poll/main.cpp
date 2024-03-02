#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <sys/poll.h>

const constexpr unsigned int MAX_CONNECTIONS = 1024;

int main() {
  // 创建socket连接
  int server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock_fd == -1) {
	std::cout << "create server socket error!" << std::endl;
	return 0;
  } else {
	std::cout << "create server socket successful!" << std::endl;
  }
  // 绑定socket信息
  struct sockaddr_in server_addr{};
  socklen_t server_len = sizeof(server_addr);
  memset(&server_addr, 0, server_len);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(9808);
  if (bind(server_sock_fd, (struct sockaddr *)&server_addr, server_len) == -1) {
	std::cout << "server bind error!" << std::endl;
	return 0;
  } else {
	std::cout << "server bind successful!" << std::endl;
  }
  // 监听端口，最多32个
  if (listen(server_sock_fd, 32) == -1) {
	std::cout << "server listen error!" << std::endl;
	return 0;
  } else {
	std::cout << "server listen successful!" << std::endl;
  }
  // 初始化文件描述符集合，添加服务端socket到集合中
  struct pollfd fds[MAX_CONNECTIONS];
  for (auto & fd : fds) {
	fd.fd = -1;
	fd.events = POLLIN | POLLPRI;
  }
  fds[0].fd = server_sock_fd;
  // 文件描述符集合有效长度
  int nfds = 0;
  while (true) {
	// poll
	int ret = poll(fds, nfds + 1, -1);
	if (ret == -1) {
	  std::cout << "poll error!" << std::endl;
	  return 0;
	} else if (ret == 0) {
	  continue;
	}
	// 监听到socket读事件
	if (fds[0].revents & (POLLIN | POLLPRI)) {
	  // 新的客户端连接
	  struct sockaddr_in client_addr{};
	  socklen_t client_len = sizeof(client_addr);
	  memset(&client_addr, 0, client_len);
	  int client_fd = accept(server_sock_fd, (struct sockaddr *)&client_addr, &client_len);
	  if (client_fd == -1) {
		std::cout << "client socket error!" << std::endl;
		continue;
	  } else {
		char ip[64];
		std::cout << "accept client socket, IP: "
				  << inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip, sizeof(ip))
				  << " Port: " << ntohs(client_addr.sin_port) << "Fds: " << client_fd << std::endl;
		for (auto & fd : fds) {
		  if (fd.fd == -1) {
			fd.fd = client_fd;
			break;
		  }
		}
		nfds = std::max(nfds, client_fd);
	  }
	} else {
	  // 接收client的socket数据
	  for (int i = 0; i < nfds; i++) {
		if (fds[i].revents & (POLLIN | POLLPRI)) {
		  char recv_buf[1024];
		  memset(recv_buf, 0, sizeof(recv_buf));
		  ssize_t recv_ret = recv(fds[i].fd, recv_buf, 1024, 0);
		  if (recv_ret <= 0) {
			std::cout << "recv data error!" << std::endl;
			close(fds[i].fd);
			fds[i].fd = -1;
		  } else {
			std::cout << "recv data: " << recv_buf << std::endl;
		  }
		}
	  }
	}
  }
}
