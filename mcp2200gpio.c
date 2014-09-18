/** Copyright (c) 2014 Andras Biro <bbandi86@gmail.com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* - Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* - Redistributions in binary form must reproduce the above
* copyright notice, this list of conditions and the following
* disclaimer in the documentation and/or other materials provided
* with the distribution.
* - Neither the name of University of Szeged nor the names of its
* contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>

#include "mcp2200io.h"

/*
 * This is based on Microchip's TB3066 MCP2200 HID Interface Command Description
 * http://ww1.microchip.com/downloads/en/DeviceDoc/93066A.pdf
 */
int main(int argc, char **argv){
	#define M_ON 0
	#define M_OFF 1
	#define M_SPIKE 2
	#define M_HOLE 3
	char path[255]="";
	long vid=0, pid=0, busnum=0, devnum=0;
	int portnum=-1, mode=-1;
	struct option long_options[] =
		{
			{"devicefile",    required_argument, 0, 'f'},
			{"vid",    required_argument, 0, 'v'},
			{"pid",    required_argument, 0, 'p'},
			{"location",    required_argument, 0, 'l'},
			{"num",    required_argument, 0, 'n'},
			{"mode",    required_argument, 0, 'm'},
			{"help",    no_argument,       0, 'h'},
			{0, 0, 0, 0}
		};
	char c;
	int option_index;
	while ((c = getopt_long (argc, argv, "hf:v:p:l:n:m:",long_options,&option_index)) != -1){
		switch (c)
		{
			case 'h':
				printf("Usage: mcp2200gpio <arguments>\
				\narguments:\
				\n-h, --help        Prints this help\
				\n-f, --devicefile  Selects the HID device to use\
				\n-v, --vid         Selects the vendor ID of the device to use\
				\n-p, --pid         Selects the product ID of the device to use\
				\n-l, --location    Selects the USB location of the device to use\
				\n-n, --number      Selects the GPIO port to manipulate (0..7)\
				\n-m, --mode        Selects the GPIO manipulation mode (on/off/spike/hole)\
				\n example:  mcp2200gpio -f /dev/usb/hiddev0 -n 4 -m hole\n");
				exit(1);
			break;
			case 'f':
				strncpy(path,optarg,255);
			break;
			case 'v':
				vid = strtol(optarg, NULL, 0);
			break;
			case 'p':
				pid = strtol(optarg, NULL, 0);
			break;
			case 'l':{
				char *split;
				busnum = strtoul(optarg, &split, 0);
				split++;//jump over separator
				devnum = strtoul(split, NULL, 0);
			}break;
			case 'n':
				portnum=atoi(optarg);
			break;
			case 'm':
				if(strcmp(optarg,"on")==0){
					mode=M_ON;
				} else if(strcmp(optarg,"off")==0){
					mode=M_OFF;
				} else if(strcmp(optarg,"spike")==0){
					mode=M_SPIKE;
				} else if(strcmp(optarg,"hole")==0){
					mode=M_HOLE;
				} else {
					perror("Invalid mode. Valid modes are: on/off/spike/hole\n");
					exit(1);
				}
			break;
		}
	}
	if(strcmp(path,"")==0 && vid==0 && busnum==0){
		fprintf(stderr,"No device selected\n");
		exit(1);
	}
	if(portnum==-1){
		fprintf(stderr,"No GPIO port selected\n");
		exit(1);
	}
	if(mode==-1){
		fprintf(stderr,"No mode selected\n");
		exit(1);
	}

	if( mcp2200searchdevice(vid, pid, busnum, devnum, path) <= 0 )
		exit(1);
	
	unsigned char command[16];
	command[0] = CMD_SET_CLEAR_OUT;
	if(mode==M_ON||mode==M_SPIKE){
		command[11]=1<<portnum;
		command[12]=0;
	}else{
		command[11]=0;
		command[12]=1<<portnum;
	}
	mcp2200writecommand(path, command, -1, NULL);
	return 0;
}