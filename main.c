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
	
	char device_name[256];
	ioctl(evfd, EVIOCGNAME(sizeof(device_name)), device_name);

	if (HasKeyEvents(evfd) && HasSpecificKey(evfd, KEY_B)) {
		printf("%s\n", device_name);
		return evfd;
	}
	

	return -1;
}

void read_events(int fd){
	struct input_event ev;

	while(true){
		ssize_t bytes = read(fd,&ev,sizeof(struct input_event));
		if(bytes < 0){
			perror("read");
			exit(1);
		}
		

		if(bytes == sizeof(struct input_event)){
			if(ev.type == EV_KEY){
				if(ev.value == 1){
					printf("%d\n",ev.code);
				}
			}
		}
	}

}

int main(){
	char root[50] = "/dev/input/";

	DIR *d;
	struct dirent *dir;
	char *target_device;

	d = opendir(root);
	int device_fd;

	if(d){
		while(dir = readdir(d)){
			if(dir->d_type == DT_CHR && ((device_fd = interrogate(dir,root)) > 0 )){
				printf("%s has descriptor: %d\n", dir->d_name, device_fd);
				//break;
			}
		}

	}


	//read_events(device_fd);
	

}
