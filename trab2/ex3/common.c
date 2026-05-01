#include "common.h"
#include <stdint.h>

void fatal_system_error(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

// size em numero de elementos
int receive_data(int fd, void *buffer, size_t size) {
  size_t bytes_read = 0;
  while (bytes_read < size) {
    int n = read(fd, buffer + bytes_read, size - bytes_read);
    if (n <= 0)
      return EXIT_FAILURE;
    bytes_read += n;
  }
  return EXIT_SUCCESS;
}

int send_data(int fd, void *buffer, size_t size) {
  size_t bytes_written = 0;
  while (bytes_written < size) {
    int n = write(fd, buffer + bytes_written, size - bytes_written);
    if (n <= 0)
      return EXIT_FAILURE;
    bytes_written += n;
  }
  return EXIT_SUCCESS;
}

/* Envia um bloco no formato: [4 bytes dimensão][dados] */
int send_block(int fd, void *buffer, uint32_t size) {
  if (send_data(fd, &size, sizeof(size)) == EXIT_FAILURE)
    return EXIT_FAILURE;
  return send_data(fd, buffer, size);
}

int send_status(int clientfd, uint8_t status, char *msg) {
  if (send_data(clientfd, &status, sizeof(status)) == EXIT_FAILURE)
    return EXIT_FAILURE;
  if (status != OK)
    return send_block(clientfd, msg, strlen(msg));
	return EXIT_SUCCESS;
}

/* Cria o socket INET/TCP do servidor */
int create_inet_socket(int port) {
  int sockfd;
  struct sockaddr_in addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  int opt = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    fatal_system_error("bind");

  return sockfd;
}

typedef struct {
  uint16_t *v; // pointer posicao inicial
  unsigned long dim;
} ThreadArgs;

void *thread_func(void *_args) {
  ThreadArgs *args = (ThreadArgs *)_args;
  ThreadReturn *ret = calloc(1, sizeof(*ret)); // inicializa a 0
  *ret = (ThreadReturn){
      .sum = 0,
      .bigger = args->v[0],
      .smaller = args->v[0],
  };
  for (unsigned long j = 0; j < args->dim; ++j) {
    ret->sum += args->v[j];
    if (args->v[j] > ret->bigger)
      ret->bigger = args->v[j];
    if (args->v[j] < ret->smaller)
      ret->smaller = args->v[j];
  }
	//printf("smaller:%d, bigger:%d, sum:%ld\n", ret->smaller, ret->bigger, ret->sum);
  return ret;
}

 ThreadReturn values_processing(uint16_t *values, uint32_t dim, int nthreads){
  int resto = dim % nthreads;
  dim = dim / nthreads;

  ThreadArgs args[nthreads];
  pthread_t th[nthreads];
	//printf("dim per th:%d, nthreads: %d\n", dim, nthreads);
  for (int i = 0; i < nthreads; i++) {
    args[i].v = values + (i*dim);
    args[i].dim = (i == nthreads - 1) ? dim + resto : dim;
    if(pthread_create(&th[i], NULL, thread_func, &args[i]) != 0)
			fatal_system_error("criar thread processamento");
	}

  long sum = 0;
  int bigger = values[0];
  int smaller = values[0];

  for (int i = 0; i < nthreads; i++) {
    ThreadReturn *ret;
    pthread_join(th[i], (void **)&ret);
    sum += ret->sum;
    if (ret->smaller < smaller)
      smaller = ret->smaller;
    if (ret->bigger > bigger)
      bigger = ret->bigger;
    free(ret);
  }

  printf("The smaller is %d\n", smaller);
  printf("The bigger  is %d\n", bigger);
  printf("The sum is %ld\n", sum);

  free(values);
	return (ThreadReturn){
		.sum = sum,
		.bigger = bigger,
		.smaller = smaller
	};
}
