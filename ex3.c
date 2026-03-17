#define _XOPEN_SOURCE 600 // for clock_gettime
#include <stdio.h>
#include <time.h>

int main(void) {
  struct timespec t_start;
  struct timespec t_end;
  // Get the start time
  clock_gettime(CLOCK_MONOTONIC, &t_start);
  // code to be timed goes here
	
	//TODO ...
	
  // Get the end time
  clock_gettime(CLOCK_MONOTONIC, &t_end);
  // Calculate the elapsed time in seconds
  double elapsed_time = (t_end.tv_sec - t_start.tv_sec) +
                        (t_end.tv_nsec - t_start.tv_nsec) * 1e-9;
  // Print the elapsed time in seconds and microseconds
  printf("Total elapsed time = %9.6lfs (%9.6lfus)\n", elapsed_time,
         elapsed_time * 1e6);
  return 0;
}
