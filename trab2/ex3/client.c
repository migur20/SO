#include "common.h"
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
void random_init() {
  // Initialize the random number generator with the current time as the seed,
  // which ensures that we get a different sequence of random numbers each
  // time we run the program.
  // srandom(time(NULL));

  // Set a fixed seed for reproducibility, i.e. will generate the same sequence
  // of random numbers every time the program is run, which is useful for
  // debugging and testing
  srandom(2026);
}

long random_get_value(long min, long max) {
  return min + random() % (max - min + 1);
}

//
// vector functions for uint16_t type
//
uint16_t *vector_create_uint16_t(unsigned long dim) {
  return malloc(dim * sizeof(uint16_t));
}

void vector_init_uint16_t(uint16_t values[], unsigned long dim) {
  for (unsigned long i = 0; i < dim; ++i) {
    values[i] = i + 1;
  }
}

void vector_random_init_uint16_t(uint16_t values[], unsigned long dim) {
  for (unsigned long i = 0; i < dim; ++i) {
    values[i] = random_get_value(SHRT_MIN, SHRT_MAX);
  }
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Uso: %s <ip_servidor> <port> <number-of-data>\n", argv[0]);
    return 1;
  }
  char *ip_address = argv[1];
  int port = atoi(argv[2]);
  uint32_t dim = atoi(argv[3]);

  int sockfd;
  struct sockaddr_in addr;

  /* Cria o socket TCP */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  /* Define o IP e o porto do servidor */
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip_address);
  addr.sin_port = htons(port);

  /* Liga ao servidor */
  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    fatal_system_error("connect");

  random_init();
	
	uint16_t *values;

  printf("Creating a vector of %u (%.2f MB; %.2f GB) values\n", dim,
         dim / 1e6, dim / 1e9);
  printf("This will require approximately %.2f MB (%.2f GB) of "
         "memory\n",
         dim * sizeof(*values) / 1e6, dim * sizeof(*values) / 1e9);

  values = vector_create_uint16_t(dim);
  // vector_random_init_uint16_t(values, dim);
  vector_init_uint16_t(values, dim);

  // Envio dos dados e dimensao no formato : [dimensao em bytes(4 bytes)][dados]
  if (send_block(sockfd, values, dim * sizeof(*values)) == EXIT_FAILURE)
    fatal_system_error("write values e dim");

  // Recebe status do servidor
  uint8_t status;
  if (receive_data(sockfd, &status, sizeof(status)) == EXIT_FAILURE)
    fatal_system_error("read status do servidor");

  printf("Status = %s\n", status_to_string(status));

  // Tratamento do status
  if (status != 0) {
    // Receber mensagem de erro no formato : [dimensao 4 bytes][dados]
    uint32_t msg_dim;
    if (receive_data(sockfd, &msg_dim, sizeof(msg_dim)) == EXIT_FAILURE)
      fatal_system_error("read tamanho mensagem de erro");
    char msg[msg_dim + 1];
    if (receive_data(sockfd, msg, msg_dim) == EXIT_FAILURE)
      fatal_system_error("read mensagem de erro");
    msg[msg_dim] = '\0';
    printf("%s\n", msg);
    // TODO : provavelmemte melhor voltar a tentar caso o erro seja no servidor
    exit(EXIT_FAILURE);
  }

  // Pedido aceite e procesado corretamente
  //[2 bytes smaller][2 bytes bigger][8 bytes soma]
  uint16_t smaller, bigger;
  uint64_t sum;
  if (receive_data(sockfd, &smaller, sizeof(smaller)) == EXIT_FAILURE)
    fatal_system_error("read smaller");
  printf("smaller: %d\n", smaller);

  if (receive_data(sockfd, &bigger, sizeof(bigger)))
    fatal_system_error("read bigger");
  printf("bigger: %d\n", bigger);

  if (receive_data(sockfd, &sum, sizeof(sum)))
    fatal_system_error("read sum");
  printf("sum: %ld\n", sum);

  close(sockfd);
  return EXIT_SUCCESS;
}
