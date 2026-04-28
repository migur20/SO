#include "common.h"
#include <limits.h>
#include <stdint.h>
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
  if (argc != 2) {
    printf("Uso: %s <ip_servidor>\n", argv[0]);
    return 1;
  }

  int sockfd;
  struct sockaddr_in addr;

  /* Cria o socket TCP */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  /* Define o IP e o porto do servidor */
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(argv[1]);
  addr.sin_port = htons(SERVER_PORT);

  /* Liga ao servidor */
  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    fatal_system_error("connect");

  random_init();
  int dim = 10000;
  uint16_t *values = vector_create_uint16_t(dim);
  vector_random_init_uint16_t(values, dim);

  write(sockfd, &dim, sizeof(dim));
  write(sockfd, values, dim * sizeof(values[0]));

  uint8_t status;
  if (read(sockfd, &status, sizeof(status)) < 0)
    fatal_system_error("read status");

  if (status == 1)
    fatal_system_error("Pedido inválido: pedido não respeita o protocolo");
  else if (status == 2)
    fatal_system_error("Erro de processamento: falha interna do servidor "
                       "ocorrida após a validação do pedido");

	//Pedido aceite e procesado corretamente

  close(sockfd);
}
