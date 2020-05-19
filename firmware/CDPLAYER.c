//file name: CDPLAYER.c

#include <pic.h>
#include "delays.h"
#include "init.h"
#include "font8x8.h"
#include "font8x3.h"
#include "KS0108.h"
#include "atapi_cdrom.h"

__CONFIG(HS&WDTDIS&LVPDIS&PWRTDIS&BORDIS&WRTEN&UNPROTECT&DUNPROT);

char ir_int=0, irbyte=0, irbytereceive=0, irtemp=0;//must be globals

void interrupt intrpt(void)// endast till för IR-mottagning (endast en typ av fjärrkontroll)
{
	if (INTF)
	{
		INTF=0;
		if (INTEDG)
		{
			TMR1ON=0;
			INTEDG=0;
			TMR1H=0;TMR1L=0;
			TMR1ON=1;
		}
		else // INTEDG==0
		{
			TMR1ON=0;
			INTEDG=1;
			irtemp = ((TMR1H<<8)+TMR1L)/100;
			if (irtemp>100) ir_int=0;
			else if (ir_int==0) ir_int++;
			else if (ir_int==9){ irbyte=irbytereceive; ir_int=0;}
			else if ((irtemp>=4)&&(irtemp<=6)) // noll
			{
				irbytereceive = irbytereceive >> 1;
				irbytereceive = irbytereceive & 0x7F;
				ir_int++;
			}
			else if ((irtemp>=14)&&(irtemp<=16)) // ett
			{
				irbytereceive = irbytereceive >> 1;
				irbytereceive = irbytereceive | 0x80;
				ir_int++;
			}

			TMR1H=0;TMR1L=0;
		}
	}
}

void main(void)
{
	char loop=0,unload=2,resume=0,tmp;
	init_PCB();
	initglcd();
	clearglcd();
	read_idpd();	
	print_dataio();

	start_stop_unit(0x01);
	wait1s();
	prevent_allow_eject(0x01);

	read_toc();
	start_stop_unit(0);
	print_toc();
	print_track();
	while(1)
	{
		if(irbyte)
		{
			if (irbyte==0x14) // STOP/EJECT
			{
				if (play) 
				{
					start_stop_unit(0);
					play=0;
				}
				else
				{
					prevent_allow_eject(0x00);
					start_stop_unit(unload);
					if (unload==2) unload = 3;
					else
					{
						unload = 2;
						start_stop_unit(0x01);
						read_toc();
						start_stop_unit(0);
						print_toc();
						track=1;
						print_track();
					}
					prevent_allow_eject(0x01);
				}
				wait1s();
			}
			else if(irbyte==0x04) // PLAY/PAUSE/RESUME
			{
				if (play)
				{
					pause_resume(resume);
					if (resume) resume=0;
					else resume=1;
				}
				else 
				{
					print_track();
					tmp=(track-1)*3;
					play_audio_msf(toc[tmp],toc[tmp+1],toc[tmp+2],toc[tmp+3],toc[tmp+4],toc[tmp+5]);
					play=1;
				}
				wait1s();
			}
			else if(irbyte==0x08) // prev track
			{
				prev_track();
				wait_xms(255);
			}
			else if(irbyte==0x0C) // next track
			{
				next_track();
				wait_xms(255);
			}
			part=2;
			writeglcdcmd(0x40,3);
			writeglcdcmd(0xBF,3);
			writehex(irbyte);
			irbyte=0;
			ir_int=0;

		} //if((irtemp>100)&&(irtemp<130))

	} //while(1)
}
