#include <arpa/inet.h>
#include <bits/pthreadtypes.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
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
