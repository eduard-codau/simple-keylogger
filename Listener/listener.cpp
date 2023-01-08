#define _BSD_SOURCE
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <unistd.h>


int main() {

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    std::cerr << "Failed to create socket" << std::endl;
    return 1;
  }

  sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(6666);
  server_address.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
    std::cerr << "Failed to bind socket" << std::endl;
    close(sock);
    return 1;
  }

  while (true) {
    sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    char buffer[1024];
    ssize_t num_bytes = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&client_address, &client_address_len);
    if (num_bytes < 0) {
      std::cerr << "Failed to receive key" << std::endl;
    } else {
      std::string message(buffer, num_bytes);
      std::cout << "Received key from " << inet_ntoa(client_address.sin_addr) << ": " << message << std::endl;
    }
  }

  close(sock);
  return 0;
}

