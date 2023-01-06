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
  };
  // 绑定socket信息
  struct sockaddr_in server_addr{};
  socklen_t server_len = sizeof(server_addr);
  memset(&server_addr, 0, server_len);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(9808);
  if (bind(server_sock_fd, (struct sockaddr *) &server_addr, server_len) == -1) {
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

  return 0;
}
