#include <arpa/inet.h>
#include <bits/pthreadtypes.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int sock;
//
// void *receiver(void *arg) {
//   char buffer[1024];
//
//   while (1) {
//     memset(buffer, 0, sizeof(buffer));
//     int valread = read(sock, buffer, sizeof(buffer));
//     if (valread <= 0)
//       break;
//     printf("server: %s", buffer);
//     fflush(stdout);
//   }
//   return NULL;
// }
//
// void *sender(void *arg) {
//   char buffer[1024];
//
//   while (1) {
//     printf("you: ");
//     fgets(buffer, sizeof(buffer), stdin);
//     send(sock, buffer, sizeof(buffer), 0);
//   }
//   return NULL;
// }
//
int main() {
  int new_socket;
  char buffer[1024];
  struct sockaddr_in address;
  int addrln = sizeof(address);
  uint32_t buffer_size;
  sock = socket(AF_INET, SOCK_STREAM, 0);

  address.sin_family = AF_INET;
  address.sin_port = htons(8080);
  inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);

  connect(sock, (struct sockaddr *)&address, sizeof(address));

  while (1) {
    printf("command > ");
    fgets(buffer, sizeof(buffer), stdin);
    printf("strlen %d\n", (int)strlen(buffer));
    printf("sizeof %d\n", (int)sizeof(buffer));
    uint32_t size = htonl(strlen(buffer));
    if (send(sock, &size, sizeof(size), 0) < 0) {
      perror("send (length) failed");
    }

    if (send(sock, buffer, strlen(buffer), 0) <
        0) { // Use strlen(buffer), not sizeof
      perror("send (message) failed");
    }
    printf("bbbbb");
    size_t total_read = 0;
    while (total_read < sizeof(buffer_size)) {
      ssize_t bytes_read = read(sock, (char *)&buffer_size + total_read,
                                sizeof(buffer_size) - total_read);
      if (bytes_read <= 0) {
        // Handle error or disconnect
        return -1;
      }
      total_read += bytes_read;
    }
    int valread;
    printf("fdsfdsfds %d\n", valread);
    printf("buffersize test raw : %u\n", buffer_size);
    fflush(stdout);
    buffer_size = ntohl(buffer_size);
    printf("%d\n", buffer_size);
    fflush(stdout);
    if (valread <= 0)
      break;
    char *buffer_value = malloc(buffer_size + 1);
    int total = 0;
    while (total < buffer_size) {
      printf("%d\n", total);
      printf("%d\n", buffer_size);
      valread = read(sock, &buffer_value[total], buffer_size - total);
      printf("vlaread %d\n", valread);
      fflush(stdout);
      total += valread;
    }
    buffer_value[total] = '\0';
    printf("server: %s\n", buffer_value);
    fflush(stdout);
  }
  // pthread_t rec_thread, send_thread;
  // pthread_create(&rec_thread, NULL, receiver, NULL);
  // pthread_create(&send_thread, NULL, sender, NULL);
  //
  // pthread_join(rec_thread, NULL);
  // pthread_join(send_thread, NULL);
  //
  close(sock);
  return 0;
}
