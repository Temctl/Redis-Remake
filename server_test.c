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
    if (valread <= 0)
      break;
    int command_size = 0;
    int key_size = 0;
    int target_size = 0;
    char *buffer = malloc(buffer_size);
    int total = 0;
    while (total < buffer_size) {
      valread = read(new_socket, &buffer[total], buffer_size - total);
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

    printf("key %s", command);
    printf("%d\n", command_size);

    int index = fnv1a_hash(key) % 1024;
    printf("cominghere %d\n", strcmp(command, "get"));
    if (strcmp(command, "set") == 0) {
      struct slot *temp = malloc(sizeof(struct slot));
      temp->key = malloc(strlen(key) + 1);
      strcpy(temp->key, key);
      temp->value = malloc(strlen(target) + 1);
      strcpy(temp->value, target);
      table[index] = temp;
      printf("%s\n", table[index]->value);
      printf("%s\n", table[index]->key);

    } else if (strcmp(command, "get") == 0) {
      printf("bbb");
      struct slot *result_value = table[index];
      int not_found = 1;
      while (not_found) {
        printf("what");
        if (strcmp(result_value->key, key)) {
          continue;
        }
        result_value = table[index]->next_slot;
      }
      send(new_socket, table[index]->key, sizeof(table[index]->key), 0);
      not_found = 0;
    } else {
      send(new_socket, "command not found", sizeof("command not found"), 0);
    }
    // printf("you: ");
    // fgets(buffer, sizeof(buffer), stdin);
    // send(new_socket, buffer, sizeof(buffer), 0);
  }
  close(new_socket);
  close(server_fd);
  return 0;
}
