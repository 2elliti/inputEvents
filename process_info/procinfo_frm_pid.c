#include <inttypes.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define pid_path_size 1024
static char pid_path[pid_path_size];

struct process_cpu_usage {
  double total_user_time;   // Seconds
  double total_kernel_time; // Seconds
  size_t virtual_memory;    // Bytes
  size_t resident_memory;   // Bytes
//  nvtop_time timestamp;
};

bool get_process_info(pid_t pid, struct process_cpu_usage *usage) {
  double clock_ticks_per_second = sysconf(_SC_CLK_TCK);
  size_t page_size = (size_t)sysconf(_SC_PAGESIZE);
  int written = snprintf(pid_path, pid_path_size, "/proc/%" PRIdMAX "/stat", (intmax_t)pid);
  if (written == pid_path_size) {
    return false;
  }
  FILE *stat_file = fopen(pid_path, "r");
  if (!stat_file) {
    return false;
  }
  //nvtop_get_current_time(&usage->timestamp);
  unsigned long total_user_time;   // in clock_ticks
  unsigned long total_kernel_time; // in clock_ticks
  unsigned long virtual_memory;    // In bytes
  long resident_memory;            // In page number?

  int retval = fscanf(stat_file,
                      "%*d %*[^)]) %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u "
                      "%*u %lu %lu %*d %*d %*d %*d %*d %*d %*u %lu %ld",
                      &total_user_time, &total_kernel_time, &virtual_memory, &resident_memory);
  fclose(stat_file);
  if (retval != 4)
    return false;
  usage->total_user_time = total_user_time / clock_ticks_per_second;
  usage->total_kernel_time = total_kernel_time / clock_ticks_per_second;
  usage->virtual_memory = virtual_memory;
  usage->resident_memory = (size_t)resident_memory * page_size;
  return true;
}


int main(){
	
	struct process_cpu_usage usage;
	get_process_info(4047,&usage);

}
