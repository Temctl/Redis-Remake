#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int new_socket;

// void *receiver(void *arg) {
//   char buffer[1024];
//
//   while (1) {
//     memset(buffer, 0, sizeof(buffer));
//     int valread = read(new_socket, buffer, sizeof(buffer));
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
//     send(new_socket, buffer, sizeof(buffer), 0);
//   }
//   return NULL;
// }

struct slot {
  char *key;
  char *value;
  struct slot *next_slot;
};

struct slot *table[1024];

uint32_t fnv1a_hash(const char *key) {
  uint32_t hash = 2166136261u;
  while (*key) {
    hash ^= (unsigned char)*key++;
    hash *= 16777619;
  }
  return hash;
}

int main() {
  int server_fd;
  uint32_t buffer_size;
  struct sockaddr_in address;
  int addrln = sizeof(address);

  server_fd = socket(AF_INET, SOCK_STREAM, 0);

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(8080);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Bind failed");
  }

  listen(server_fd, 1);
  new_socket =
      accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrln);
  printf("connected\n");
  while (1) {
    int valread = read(new_socket, &buffer_size, sizeof(buffer_size));
    buffer_size = ntohl(buffer_size);
    printf("buffersize test : %d\n", buffer_size);
    if (valread <= 0)
      break;
    int command_size = 0;
    int key_size = 0;
    int target_size = 0;
    char *buffer = malloc(buffer_size);
    int total = 0;
    while (total < buffer_size) {
      valread = read(new_socket, &buffer[total], buffer_size - total + 1);
      total = total + valread + 1;
    }
    int current_part = 0;
    int current_byte_loc = 0;
    for (int i = 0; i < buffer_size; i++) {
      if (buffer[i] == ' ') {
        current_byte_loc = 0;
        current_part++;
        continue;
      } else if (buffer[i] == '\0' || buffer[i] == '\n') {
        continue;
      }
      if (current_part == 0) {
        command_size += 1;
        current_byte_loc++;
      } else if (current_part == 1) {
        key_size += 1;
        current_byte_loc++;
      } else {
        target_size += 1;
        // target[current_byte_loc] = buffer[i];
        current_byte_loc++;
      }
    }
    if (key_size == 0) {
      send(new_socket, "key not sent", sizeof("key not sent"), 0);
      continue;
    }
    char command[command_size + 1];
    char key[key_size + 1];
    char target[target_size + 1];
    current_part = 0;
    current_byte_loc = 0;
    for (int i = 0; i < buffer_size; i++) {
      if (buffer[i] == ' ') {
        current_byte_loc = 0;
        current_part++;
        continue;
      }
      if (current_part == 0) {
        command[current_byte_loc] = buffer[i];
        current_byte_loc++;
      } else if (current_part == 1) {
        key[current_byte_loc] = buffer[i];
        current_byte_loc++;
      } else {
        target[current_byte_loc] = buffer[i];
        current_byte_loc++;
      }
    }
    command[command_size] = '\0';
    key[key_size] = '\0';
    target[target_size] = '\0';

    int index = fnv1a_hash(key) % 1024;
    if (strcmp(command, "set") == 0) {
      struct slot *temp = malloc(sizeof(struct slot));
      temp->key = malloc(strlen(key) + 1);
      strcpy(temp->key, key);
      temp->value = malloc(strlen(target) + 1);
      strcpy(temp->value, target);
      table[index] = temp;
      char succ_mess[19] = "stored succesfully\n";
      uint32_t return_size = htonl(strlen(succ_mess));
      printf("here too %d,   sizeof : %d\n", ntohl(return_size),
             (int)sizeof(return_size));
      printf("raw buffer<ise : %u\n", return_size);
      if (send(new_socket, &return_size, sizeof(return_size), 0) < 0) {
        printf("error");
        perror("send (length) failed");
      }
      printf("fdsfdsfds");
      if (send(new_socket, "stored succesfully\n",
               strlen("stored succesfully\n"),
               0) < 0) { // Use strlen(buffer), not sizeof
        perror("send (message) failed");
      }
      printf("toooo");
      fflush(stdout);
    } else if (strcmp(command, "get") == 0) {
      printf("bbb\n");
      struct slot *result_value = table[index];
      while (result_value != NULL) {
        printf("what");
        if (strcmp(result_value->key, key)) {
          break;
        }
        result_value = table[index]->next_slot;
      }
      printf("%s\n", table[index]->key);
      uint32_t return_size = htonl(strlen(table[index]->value));
      if (send(new_socket, &return_size, sizeof(return_size), 0) < 0) {
        perror("send (length) failed");
      }

      if (send(new_socket, table[index]->value, strlen(table[index]->value),
               0) < 0) { // Use strlen(buffer), not sizeof
        perror("send (message) failed");
      }
    } else {
      uint32_t return_size = htonl(strlen("command not found\n"));
      printf("here too\n");
      if (send(new_socket, &return_size, sizeof(return_size), 0) < 0) {
        printf("error");
        perror("send (length) failed");
      }
      printf("fdsfdsfds");
      if (send(new_socket, "command not found\n", strlen("command not found\n"),
               0) < 0) { // Use strlen(buffer), not sizeof
        perror("send (message) failed");
      }
    }
    // printf("you: ");
    // fgets(buffer, sizeof(buffer), stdin);
    // send(new_socket, buffer, sizeof(buffer), 0);
  }
  close(new_socket);
  close(server_fd);
  return 0;
}
