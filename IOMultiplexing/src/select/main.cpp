#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>

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
  // 客户端socket集合
  std::vector<int> client_fds;
  while (true) {
	// 文件描述符读集合
	fd_set reads;
	// 标志位清零
	FD_ZERO(&reads);
	// 添加服务端socket
	FD_SET(server_sock_fd, &reads);
	// 最大文件描述符
	int max_fd = server_sock_fd;
	// 添加客户端socket到待检测的socket集合中
	for (auto it : client_fds) {
	  FD_SET(it, &reads);
	  max_fd = std::max(max_fd, it);
	}
	// select
	int ret = select(max_fd + 1, &reads, nullptr, nullptr, nullptr);
	if (ret == -1) {
	  std::cout << "select error!" << std::endl;
	  return 0;
	} else if (ret == 0) {
	  continue;
	}

	// 监听到socket读事件
	if (FD_ISSET(server_sock_fd, &reads)) {
	  // 新的客户端连接
	  struct sockaddr_in client_addr{};
	  socklen_t client_len = sizeof(client_addr);
	  memset(&client_addr, 0, client_len);
	  int client_fd = accept(server_sock_fd, (struct sockaddr *)&client_addr, &client_len);
	  if (client_fd == -1) {
		std::cout << "client socket error!" << std::endl;
		break;
	  } else {
		char ip[64];
		std::cout << "accept client socket, IP: "
				  << inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip, sizeof(ip))
				  << " Port: " << ntohs(client_addr.sin_port) << std::endl;
		client_fds.push_back(client_fd);
	  }
	} else {
	  // 接收client的socket数据
	  for (auto it = client_fds.begin(); it != client_fds.end();) {
		if (FD_ISSET(*it, &reads)) {
		  char recv_buf[1024];
		  memset(recv_buf, 0, sizeof(recv_buf));
		  ssize_t recv_ret = recv(*it, recv_buf, 1024, 0);
		  if (recv_ret <= 0) {
			std::cout << "recv data error" << std::endl;
			close(*it);
			it = client_fds.erase(it);
		  } else {
			std::cout << "recv data: " << recv_buf << std::endl;
			it++;
		  }
		} else {
		  it++;
		}
	  }
	}
  }
  // 关闭所有客户端连接
  for (auto it : client_fds)
	close(it);
  // 关闭server监听
  close(server_sock_fd);
  return 0;
}
