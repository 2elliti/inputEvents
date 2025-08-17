#include<stdio.h>
#include<dirent.h>
#include<stdbool.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<linux/input.h>
#include<stdint.h>
#include<poll.h>
#define test_bit(bit, array) ((array[bit / 8] >> (bit % 8)) & 1)

bool is_numlock_on = true;

typedef struct {
	int fd;
	char *name;
	char *path;
} keyboard_devices;

char *get_device_name(int fd){
	const size_t buflen = 256;
	char *device_name = (char *)malloc(sizeof(char) * buflen);
	if(ioctl(fd, EVIOCGNAME(buflen), device_name) < 0){
		perror("EVIOCGNAME");
		free(device_name);
		return NULL;
	}
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

	if((evfd = open(path,O_RDONLY)) < 0){
		free(path);	
		return -1;
	}
	
	if (HasKeyEvents(evfd) && HasSpecificKey(evfd, KEY_B)) {
		free(path);
		return evfd;
	}
	
	close(evfd);
	free(path);
	return -1;
}

void print_key_devices(keyboard_devices *devstr){
	for(int index = 0; devstr[index].path != NULL; index++){
		printf("Device name: %s, Device fd: %d, Device path: %s\n", devstr[index].name, devstr[index].fd, devstr[index].path);
	}
}

ssize_t get_devicelist_size(keyboard_devices * device_list){
	if(device_list == NULL) return -1;
	ssize_t count = 0;
	for(int index = 0; device_list[index].path != NULL; index++){
		count++;
	}

	return count;
}

void set_capslock_status(int fd){
	unsigned char status_led[LED_MAX/8 + 1];
	
	if(ioctl(fd,EVIOCGLED(sizeof(status_led)), status_led) < 0){
		perror("EVIOCGLED");
		return;
	}
	
	is_numlock_on = test_bit(LED_NUML, status_led);	
	printf("Current LED Status: %X\n", status_led);
	printf("Current Num Lock Status: %d\n", test_bit(LED_NUML, status_led));
}

void print_event_interface_version(int fd){
	uint32_t version;
	ioctl(fd, EVIOCGVERSION, &version);
	printf("Event Interface Version: %d.%d.%d\n", version >> 16, (version >> 8) & 0xff, (version & 0xff));
}

void listen_input_devices(keyboard_devices *device_list){
	ssize_t list_size;
	struct pollfd pfd;
	int poll_status;
	if((list_size = get_devicelist_size(device_list)) < 0){
		fprintf(stderr, "No Device list found.\n");
		exit(1);
	}
	
	printf("Number of keyboard devices detected: %d.\n", list_size);
	if(list_size == 0){
		fprintf(stderr, "Path for device file is malfuctioned\n");
		exit(1);
	}
	
	if(list_size > 1){
		printf("Multiple keyboard devices detected. Defaulting to first device\n");
	}

	printf("Picked Device Name: %s, Device Path: %s.\n",device_list[0].name, device_list[0].path);
	int fd = device_list[0].fd;
	print_event_interface_version(fd);
	
	// For storing event.
	struct input_event ev;

	set_capslock_status(fd);
	
	pfd.fd = fd;
	pfd.events = POLLIN;

	poll_status = poll(&pfd, 1, -1);		
	if(poll_status == -1){
		perror("poll");
		exit(1);
	}

	while(true){
		// Check for event file
		if(pfd.revents & POLLIN){
			if(read(fd,&ev, sizeof(struct input_event)) != sizeof(struct input_event)) continue;	
			if((ev.type == EV_KEY) && (ev.value == 1)){
				printf(" Pressed key code: %d\n", ev.code);
				if(ev.code == KEY_NUMLOCK){
					if(is_numlock_on)is_numlock_on = false;
					else is_numlock_on = true;
				}

				if(is_numlock_on){
					switch(ev.code){
						case KEY_KP8 : {
							printf("Volume up!!\n");
							// Increase volume by x percentage.
							break;
						}
						case KEY_KP2 : {
							printf("Volume down!!\n");
							// Decrease volume by x  percentage.
							break;
						}
						case KEY_KP5 : {
							printf("volume mute!!\n");
							break;
						}
						case KEY_RIGHTALT : {
							printf("Spaceeee Barr!!!\n");
							struct input_event space_ev;
							space_ev.code = KEY_SPACE;
							space_ev.value = 1;
							write(fd, &space_ev, sizeof(struct input_event));
							//usleep(200000);
							space_ev.value = 0;
							write(fd, &space_ev, sizeof(struct input_event));
							break;
						}
					}
				}
			}
		}
		

	}
	// audio connection destroy	

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
				
				(keyboard_input_devices + device_num)->path = (char *)malloc(sizeof(char )*(strlen(dir->d_name)+strlen(root))+1);
				strcpy((keyboard_input_devices + device_num)->path,root);
				strcat((keyboard_input_devices + device_num)->path,dir->d_name);
				device_num++; 
			}
		}
		keyboard_input_devices[device_num].path = NULL;

	}
	
	print_key_devices(keyboard_input_devices);

	listen_input_devices(keyboard_input_devices);	

}
