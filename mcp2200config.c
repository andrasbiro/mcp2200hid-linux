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
/*
 * The CMD_CONFIGURE part is based on Microchip's TB3066 MCP2200 HID Interface Command Description
 * http://ww1.microchip.com/downloads/en/DeviceDoc/93066A.pdf
 * 
 * I mostly figured out the script configuration with usb sniffer, but I couldn't understand
 * how the manufacturer/product description lenght was coded into the 3rd byte
 * For that, thanks for noe at stackoverflow: http://stackoverflow.com/users/3540589/noe
 * http://stackoverflow.com/questions/12746744/mcp2200-linux-settings/23106926#23106926
 */
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>

#include "mcp2200io.h"

int main(int argc, char **argv){
	char path[255]="";
	long vid=0, pid=0, busnum=0, devnum=0;
	int gpio=-1, alt=-1, baud=-1, def=-1,altopts=-1;
	char prod[64]="";
	char manuf[64]="";
	struct option long_options[] =
		{
			{"devicefile",    required_argument, 0, 'f'},
			{"vid",    required_argument, 0, 'v'},
			{"pid",    required_argument, 0, 'p'},
			{"location",    required_argument, 0, 'l'},
			{"gpio",    required_argument, 0, 'g'},
			{"alt",    required_argument, 0, 'a'},
			{"default",    required_argument, 0, 'd'},
			{"altopts",    required_argument, 0, 'o'},
			{"baud",    required_argument, 0, 'b'},
			{"manuf",    required_argument, 0, 'm'},
			{"prod",    required_argument, 0, 'r'},
			{"help",    no_argument,       0, 'h'},
			{0, 0, 0, 0}
		};
	char c;
	int option_index;
	while ((c = getopt_long (argc, argv, "hf:v:p:l:g:a:d:o:b:m:r:",long_options,&option_index)) != -1){
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
				\n-g, --gpio        Sets GPIO bitmap for pin assignment (1=input/0=output; LSB=GPIO0)\
				\n-d, --default     Sets default GPIO value bitmap (1=logic high/0=logic low; LSB=GPIO0)\
				\n-a, --alt         Sets alternativ configuration pin settings\
				\n-o, --altopts     Sets alternative function options\
				\n-b, --baud        Sets default baudrate\
				\n-m, --manuf       Sets manufacturer string\
				\n-r, --prod        Sets product string\
				\n example:   -f /dev/usb/hiddev0 -g 0x2e -a 0x8c -b57600 -m\"my manufacturer\" -r\"my product\"\
				\n For the alternative pin bytes, see Microchip's TB3066 MCP2200 HID Interface Command Description\
				\n");
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
			case 'g':
				gpio = strtol(optarg, NULL, 0);
			break;
			case 'a':
				alt = strtol(optarg, NULL, 0);
			break;
			case 'd':
				def = strtol(optarg, NULL, 0);
			break;
			case 'o':
				altopts = strtol(optarg, NULL, 0);
			break;
			case 'b':
				baud = strtol(optarg, NULL, 0);
				baud = 12000000UL/baud - 1;
			break;
			case 'm':
				strncpy(manuf, optarg, 64);
			break;
			case 'r':
				strncpy(prod, optarg, 64);
			break;
		}
	}
	if(strcmp(path,"")==0 && vid==0 && busnum==0){
		fprintf(stderr,"No device selected\n");
		exit(1);
	}
	if( mcp2200searchdevice(vid, pid, busnum, devnum, path) <= 0 )
		exit(1);
	
	unsigned char command[16];
	//set all config options for the current one
	if( (baud == -1 || alt == -1 || gpio == -1 || def == -1 || altopts == -1) && (baud != -1 || alt != -1 || gpio != -1 || def != -1 || altopts != -1)){
		memset(command, 0, 16);
		command[0] = CMD_CONFIGURE;
		mcp2200writecommand(path, command, 0, command);
		
		if( gpio == -1 ){
			gpio = command[4];
		}
		if( alt == -1 ){
			alt = command[5];
		}
		if( def == -1 ){
			def = command[6];
		}
		if( altopts == -1 ){
			altopts = command[7];
		}
		if( baud == -1 ){
			baud = command[8]<<8|command[9];
		}
	}
	if( baud != -1 ){
		memset(command, 0, 16);
		command[0] = CMD_CONFIGURE;
		command[4] = gpio;
		command[5] = alt;
		command[6] = def;
		command[7] = altopts;
		command[8] = (baud>>8)&0xff; //high byte
		command[9] = baud&0xff; //low byte
		mcp2200writecommand(path, command, -1, NULL);
	}
	if( strcmp(manuf,"")!=0 ){
		int currentChar = 0;
		while(currentChar < 63){
			command[0] = CMD_SECRET_CONFIGURE;
			command[1] = CFG_MANU;
			command[2] = (currentChar+1)>>2;
			if(currentChar == 0){
				command[3] = strlen(manuf)*2+2;
				command[4] = 0x03;
			}else{
				if( currentChar <= strlen(manuf) ){
					command[3] = manuf[currentChar++];
				}else{
					command[3] = ' ';
					currentChar++;
				}
				command[4] = 0x00;
			}
			if( currentChar <= strlen(manuf) ){
				command[5] = manuf[currentChar++];
			}else{
				command[5] = ' ';
				currentChar++;
			}
			command[6] = 0x00;
			if( currentChar <= strlen(manuf) ){
				command[7] = manuf[currentChar++];
			}else{
				command[7] = ' ';
				currentChar++;
			}
			command[8] = 0x00;
			if( currentChar <= strlen(manuf) ){
				command[9] = manuf[currentChar++];
			}else{
				command[9] = ' ';
				currentChar++;
			}
			command[10] = 0x00;
			command[11] = 0xff;
			command[12] = 0xff;
			command[13] = 0xff;
			command[14] = 0xff;
			command[15] = 0xff;
			mcp2200writecommand(path, command, -1, NULL);
		}
	}
	if( strcmp(prod,"")!=0 ){
		int currentChar = 0;
		while(currentChar < 63){
			command[0] = CMD_SECRET_CONFIGURE;
			command[1] = CFG_PROD;
			command[2] = (currentChar+1)>>2;
			if(currentChar == 0){
				command[3] = strlen(prod)*2+2;
				command[4] = 0x03;
			}else{
				if( currentChar <= strlen(prod) ){
					command[3] = prod[currentChar++];
				}else{
					command[3] = ' ';
					currentChar++;
				}
				command[4] = 0x00;
			}
			if( currentChar <= strlen(prod) ){
				command[5] = prod[currentChar++];
			}else{
				command[5] = ' ';
				currentChar++;
			}
			command[6] = 0x00;
			if( currentChar <= strlen(prod) ){
				command[7] = prod[currentChar++];
			}else{
				command[7] = ' ';
				currentChar++;
			}
			command[8] = 0x00;
			if( currentChar <= strlen(prod) ){
				command[9] = prod[currentChar++];
			}else{
				command[9] = ' ';
				currentChar++;
			}
			command[10] = 0x00;
			command[11] = 0xff;
			command[12] = 0xff;
			command[13] = 0xff;
			command[14] = 0xff;
			command[15] = 0xff;
			mcp2200writecommand(path, command, -1, NULL);
		}
	}
	//TODO VID/PID configuration
	
	return 0;
}