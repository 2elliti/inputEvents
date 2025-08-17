#include <inttypes.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define command_line_increment 32
#define pid_path_size 1024
static char pid_path[pid_path_size];

// function for reading cmd by pid.
void get_command_from_pid(pid_t pid, char **buffer) {
  int written = snprintf(pid_path, pid_path_size, "/proc/%" PRIdMAX "/cmdline", (intmax_t)pid);
  if (written == pid_path_size) {
    *buffer = NULL;
    return;
  }
  FILE *pid_file = fopen(pid_path, "r");
  if (!pid_file) {
    *buffer = NULL;
    return;
  }

  size_t size_buffer = command_line_increment;
  *buffer = malloc(size_buffer);
  char *current_buffer = *buffer;
  current_buffer[0] = '\0';

  size_t total_read = 0;
  do {
    size_t num_read = fread(current_buffer, 1, command_line_increment, pid_file);
    total_read += num_read;
    if (num_read == command_line_increment) {
      size_buffer += command_line_increment;
      *buffer = realloc(*buffer, size_buffer);
      current_buffer = &((*buffer)[total_read]);
    }
  } while (!feof(pid_file) && !ferror(pid_file));
  if (ferror(pid_file)) {
    fclose(pid_file);
    free(*buffer);
    *buffer = NULL;
    return;
  }
  fclose(pid_file);

  for (size_t i = 0; total_read && i < total_read - 1; ++i) {
    if ((*buffer)[i] == '\0')
      (*buffer)[i] = ' ';
  }
}


int main(){
	char **buffer = malloc(sizeof(char *)*pid_path_size);
	get_command_from_pid(4047,buffer);
	printf("CMD: %s\n", *buffer);
}
