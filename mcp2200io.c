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

#include <string.h>
#include <fcntl.h>
#include <linux/hiddev.h>
#include <linux/input.h>
#include "mcp2200io.h"
#include <dirent.h>
#include <stdio.h>

void mcp2200writecommand(char* path, unsigned char* command, int waitForResponse, unsigned char* response){
	struct hiddev_report_info resp;  
	struct hiddev_usage_ref_multi comm;
	int i;
	int fd=-1;
	fd=open(path, O_RDONLY);
	resp.report_type=HID_REPORT_TYPE_OUTPUT;
	resp.report_id=HID_REPORT_ID_FIRST;
	resp.num_fields=1;
	comm.uref.report_type=HID_REPORT_TYPE_OUTPUT;
	comm.uref.report_id=HID_REPORT_ID_FIRST;
	comm.uref.field_index=0;
	comm.uref.usage_index=0;
	comm.num_values=16;
	for(i=0;i<16;i++)
		comm.values[i] = command[i];
	
	ioctl(fd, HIDIOCSUSAGES, &comm); 
	ioctl(fd, HIDIOCSREPORT, &resp);
	if(waitForResponse >= 0){
		if(waitForResponse > 0)
 			usleep(waitForResponse);
		ioctl(fd,HIDIOCGUSAGES, &comm); 
		ioctl(fd,HIDIOCGREPORT, &resp);
	}
	close(fd);
}

int mcp2200searchdevice(long vid, long pid, long busnum, long devnum, char* device){
	DIR           *d;
	struct dirent *dir;
	d = opendir("/dev/usb/");
	if (d)
	{
		char temppath[255];
		while ((dir = readdir(d)) != NULL)
		{
			if( strncmp(dir->d_name,"hiddev", 6)==0 ){
				int fd=-1;
				strcpy(temppath, "/dev/usb/");
				strcat(temppath, dir->d_name);
				fd=open(temppath, O_RDONLY);
				if(fd<0){
					fprintf(stderr,"Can't open device: %s\n",temppath);
					return -1;
				}
				struct hiddev_devinfo dinfo;
				ioctl(fd, HIDIOCGDEVINFO, &dinfo);
				close(fd);
				if(vid != -1){//check VID
					if( vid != dinfo.vendor)
						continue;
				}
				if(pid != -1){//check PID
					if( pid != dinfo.product)
						continue;
				}
				if(busnum != -1){//check busnum
					if( busnum != dinfo.busnum)
						continue;
				}
				if(devnum != -1){//check busnum
					if( devnum != dinfo.devnum)
						continue;
				}
				if(strcmp(device, "") != 0){
					fprintf(stderr, "Ambiguous match, please be more specific: %s and %s", device, temppath);
					return -1;
				}
				strcpy(device, temppath);
			}
		}
		closedir(d);
		return 1;
	}
}