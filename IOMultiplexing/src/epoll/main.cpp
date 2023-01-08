#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>

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
  // 创建一个epoll对象
  int epoll_fd = epoll_create(1);
  // 添加服务端socket到epoll中
  struct epoll_event ep_ev{};
  ep_ev.events = EPOLLIN | EPOLLPRI;
  ep_ev.data.fd = server_sock_fd;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_sock_fd, &ep_ev);
  // 回调事件数组
  struct epoll_event events[MAX_CONNECTIONS];
  while (true) {
	// epoll
	int ret = epoll_wait(epoll_fd, events, MAX_CONNECTIONS, -1);
	if (ret == -1) {
	  std::cout << "epoll error" << std::endl;
	  return 0;
	} else if (ret == 0) {
	  continue;
	}
	// 遍历所有事件
	for (int i = 0; i < ret; i++) {
	  if (events[i].data.fd == server_sock_fd) {
		// 新的客户端连接
		if (events[i].events & (EPOLLIN | EPOLLPRI)) {
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
			// 设置边缘模式
			ep_ev.events = EPOLLIN | EPOLLPRI | EPOLLET;
			ep_ev.data.fd = client_fd;
			// 设置非阻塞模式
			int flags = fcntl(client_fd, F_GETFL, 0);
			if (flags < 0) {
			  std::cout << "set no block error, fd: " << client_fd << std::endl;
			  continue;
			}
			if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
			  std::cout << "set no block error, fd: " << client_fd << std::endl;
			  continue;
			}
			// 客户端连接添加到epoll中
			epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ep_ev);
		  }
		}
	  } else {
		if (events[i].events & (EPOLLIN | EPOLLPRI)) {
		  while (true) {
			char recv_buf[10];
			memset(recv_buf, 0, sizeof(recv_buf));
			ssize_t recv_ret = recv(events[i].data.fd, recv_buf, sizeof(recv_buf) - 1, 0);
			if (recv_ret > 0) {
			  std::cout << "recv data: " << recv_buf << std::endl;
			} else {
			  if (errno == EAGAIN || errno == EWOULDBLOCK) {
				break;
			  }
			  std::cout << "recv data error!" << std::endl;
			  epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
			  close(events[i].data.fd);
			}
		  }
		}
	  }
	}
  }
}
