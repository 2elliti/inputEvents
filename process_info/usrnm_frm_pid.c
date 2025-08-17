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

// Get username from pid

void get_username_from_pid(pid_t pid, char **buffer) {
  int written = snprintf(pid_path, pid_path_size, "/proc/%" PRIdMAX, (intmax_t)pid);
  if (written == pid_path_size) {
    *buffer = NULL;
    return;
  }
  struct stat folder_stat;
  int starval = stat(pid_path, &folder_stat);
  if (starval == -1) {
    *buffer = NULL;
    return;
  }
  uid_t user_id = folder_stat.st_uid;
  struct passwd *user_info = getpwuid(user_id);
  if (user_info == NULL) {
    *buffer = NULL;
    return;
  }
  size_t namelen = strlen(user_info->pw_name) + 1;
  *buffer = malloc(namelen * sizeof(**buffer));

  strncpy(*buffer, user_info->pw_name, namelen);
}


int main(){
	char *path = "/proc/4047";
	//strcpy(pid_path, path);
	//printf("print: %s\n", pid_path);
	char **buffer = malloc(sizeof(char *)*pid_path_size);
	
	get_username_from_pid(4047,buffer);

	printf("USER: %s\n", *buffer);
}
