#include<stdio.h>
#include<dirent.h>
#include<stdbool.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include <sys/ioctl.h>
#include <linux/input.h>

bool is_numlock_on = true;

typedef struct {
	int fd;
	char *name;
	char *path;
} keyboard_devices;

char *get_device_name(int fd){
	char *device_name = (char *)malloc(sizeof(char) * 256);
	ioctl(fd, EVIOCGNAME(sizeof(device_name)), device_name);
	return device_name;
}

bool HasKeyEvents(int device_fd) {
  unsigned long evbit = 0;
  ioctl(device_fd, EVIOCGBIT(0, sizeof(evbit)), &evbit);
  return evbit & (1 << EV_KEY);
}

bool HasSpecificKey(int device_fd, unsigned int key) {
  size_t nchar = KEY_MAX/8 + 1;
  unsigned char bits[nchar];
  ioctl(device_fd, EVIOCGBIT(EV_KEY, sizeof(bits)), &bits);
  return bits[key/8] & (1 << (key % 8));
}

ssize_t interrogate(struct dirent *dir,char *root){
	int evfd;
	char *path = (char *)malloc(sizeof(char *)*(strlen(dir->d_name)+strlen(root))+1);
	strcpy(path,root);
	strcat(path,dir->d_name);

	if((evfd = open(path,O_RDONLY)) < 1){
		perror("error while opening\n");
		exit(1);
	}
	
	if (HasKeyEvents(evfd) && HasSpecificKey(evfd, KEY_B)) {
		return evfd;
	}

	free(path);
	return -1;
}

void print_key_devices(keyboard_devices *devstr){
	for(int index = 0; devstr[index].path != NULL; index++){
		printf("Device name: %s, Device fd: %d, Device path: %s\n", devstr[index].name, devstr[index].fd, devstr[index].path);
	}
}

int main(){
	char root[50] = "/dev/input/";

	DIR *d;
	struct dirent *dir;

	// Allocate enough devices. 11 devices are pretty enough.	
	keyboard_devices *keyboard_input_devices = (keyboard_devices *)calloc(11, sizeof(keyboard_devices));

	d = opendir(root);
	int device_fd;

	int device_num = 0;
	if(d){
		while(dir = readdir(d)){
			if(dir->d_type == DT_CHR && ((device_fd = interrogate(dir,root)) > 0 )){
				(keyboard_input_devices + device_num)->fd = device_fd;
				(keyboard_input_devices + device_num)->name = get_device_name(device_fd);
				
				(keyboard_input_devices + device_num)->path = (char *)malloc(sizeof(char *)*(strlen(dir->d_name)+strlen(root))+1);
				strcpy((keyboard_input_devices + device_num)->path,root);
				strcat((keyboard_input_devices + device_num)->path,dir->d_name);
				device_num++; 
			}
		}
		keyboard_input_devices[device_num].path = NULL;

	}
	
	print_key_devices(keyboard_input_devices);	

}
