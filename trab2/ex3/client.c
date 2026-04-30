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

  // Envio dos dados e dimensao no formato : [dimensao 4 bytes][dados]
  write(sockfd, &dim, sizeof(dim));
  write(sockfd, values, dim * sizeof(values[0]));

  // Recebe status do servidor
  uint8_t status;
  if (read(sockfd, &status, sizeof(status)) < 0)
    fatal_system_error("read status");

  // Tratamento do status
  if (status != 0){
		// Receber mensagem de erro no formato : [dimensao 4 bytes][dados]
		int bytes_read;
		uint32_t size;
		if((bytes_read = read(sockfd, &size, sizeof(size))))
			fatal_system_error("read tamanho msg erro");
		char msg[size];
		if((bytes_read = read(sockfd, msg, size)))
			fatal_system_error("read msg erro");
		write(STDOUT_FILENO, msg, bytes_read);
		// TODO : provavelmemte melhor voltar a tentar caso o erro seja no servidor
		exit(EXIT_FAILURE);
	}

  // Pedido aceite e procesado corretamente
  //[2 bytes min][2 bytes max][8 bytes soma]
  uint16_t min, max;
  uint64_t sum;
  if (read(sockfd, &min, sizeof(min)) == -1)
    fatal_system_error("read min");
	printf("min: %d\n", min);

  if (read(sockfd, &max, sizeof(max)) == -1)
    fatal_system_error("read max");
	printf("max: %d\n", max);

  if (read(sockfd, &sum, sizeof(sum)) == -1)
    fatal_system_error("read sum");
	printf("sum: %ld\n", sum);

  close(sockfd);
	return EXIT_SUCCESS;
}
