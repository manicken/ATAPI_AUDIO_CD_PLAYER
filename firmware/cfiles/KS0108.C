#include <pic.h>
#include "KS0108.h"
#include "font8x8.h"
#include "font8x3.h"

void busycheck(void)
{
	gLCDDATADIR=0xFF;
	gLCDE=0;
	gLCDRS=0;
	gLCDRW=1;
	gLCDCS2=0;
	gLCDCS1=1;
	gLCDE=1;
	asm("NOP");
	while (gLCDBSY);
	gLCDE=0;
	gLCDCS1=0;
	gLCDCS2=1;
	gLCDE=1;
	asm("NOP");
	while (gLCDBSY);
	gLCDE=0;
	gLCDCS2=0;
	gLCDRW=0;
}

void writeglcddata(unsigned char data)//,unsigned char chipselect)
{
	if(Yaddress==64)
	{
		if (part==1) part=2;
		else if (part==2)
		{
			page++;
			writeglcdcmd(page+0xB8,3);
			part=1;
		}
		if (page==8)
		{
			page=0;
			writeglcdcmd(0xB8,3);
			part=1;
		}
		Yaddress=0;
	}
	busycheck();
	gLCDDATADIR=0x00;
	gLCDRS=1;
	gLCDRW=0;
	gLCDCS1=0;
	gLCDCS2=0;
	if (part==1) gLCDCS1=1;
	else if(part==2) gLCDCS2=1;
	gLCDDATA=data;
	gLCDE=1;
	asm("NOP");
	asm("NOP");
	asm("NOP");
	gLCDE=0;
	gLCDCS1=0;
	gLCDCS2=0;
	Yaddress++;
	if((Yaddress==64) && (page==7) && (part==2))
	{
		Yaddress=0;
		writeglcdcmd(0xB8,3);
		page=0;
		part=1;
	}
}

void writeglcdcmd(unsigned char data,char chipselect)
{
	if ((data&0xC0)==0x40) Yaddress=data&0x3F;
	if ((data&0xF8)==0xB8) page=data&0x07;
	if (chipselect!=3) part=chipselect;
	busycheck();
	gLCDRS=0;
	gLCDRW=0;
	gLCDCS1=0;
	gLCDCS2=0;
	if (chipselect==1) gLCDCS1=1;
	else if(chipselect==2) gLCDCS2=1;
	else if(chipselect==3){gLCDCS1=1; gLCDCS2=1;}
	gLCDDATADIR=0x00;
	gLCDDATA=data;
	gLCDE=1;
	asm("NOP");
	asm("NOP");
	asm("NOP");
	gLCDE=0;
	gLCDCS1=0;
	gLCDCS2=0;
}
void writechar(unsigned char character)
{
	unsigned char loop=0;
	while (loop<8)
	{
		writeglcddata(chargen[loop+character*8]);
		loop++;	
	}
}

void clearglcd(void)
{
	unsigned int loop;
	writeglcdcmd(0x40,3); // set Y adress to zero
	writeglcdcmd(0xB8,3); // set page to zero
	loop=0;
	while (loop<1024)
	{
		writeglcddata(0x00);
		loop++;
	}
	writeglcdcmd(0x40,3); // set Y adress to zero
	writeglcdcmd(0xB8,3); // set page to zero
}

void initglcd(void)
{
	gLCDDATA=0x00;
	gLCDDATADIR=0xFF;
	PORTB=0x00;
	TRISB=0xC1;
	writeglcdcmd(0x3F,3); // set display to on
	writeglcdcmd(0x40,3); // set Y adress to zero (0x40-0x7F)
	writeglcdcmd(0xB8,3); // set page to zero (0xB8-0xBF)
	writeglcdcmd(0xC0,3); // set display start line to zero (0xC0-0xFF)
}

void glcdSetDot(unsigned char x, unsigned char y) // set dot
{
    unsigned char temp;
	if (x<64) writeglcdcmd(0x40+x,1);
	else writeglcdcmd(x,2);
	writeglcdcmd(0xB8+(y/8),3);

    temp = readglcddata();  // read back current value

	if (x<64) writeglcdcmd(0x40+x,1);
	else writeglcdcmd(x,2);
	y=y%8;
	if (y==7) temp |= 0x80;
	else if (y==6) temp |= 0x40;
	else if (y==5) temp |= 0x20;
	else if (y==4) temp |= 0x10;
	else if (y==3) temp |= 0x08;
	else if (y==2) temp |= 0x04;
	else if (y==1) temp |= 0x02;
	else if (y==0) temp |= 0x01;
    writeglcddata(temp);// | (1 << (y % 8)));
}

void glcdClearDot(unsigned char x, unsigned char y) // clear dot
{
    unsigned char temp;
	if (x<128) writeglcdcmd(0x40+x,1);
	else writeglcdcmd(0x40+x,2);
	writeglcdcmd(0xB8+(y/8),3);

    temp = readglcddata();  // read back current value

	if (x<128) writeglcdcmd(0x40+x,1);
	else writeglcdcmd(0x40+x,2);

    writeglcddata(temp & !(1 << (y % 8)));
}
unsigned char readglcddata(void)
{
	unsigned char data=0;
	if(Yaddress==64)
	{
		if (part==1) part=2;
		else if (part==2)
		{
			page++;
			writeglcdcmd(page+0xB8,3);
			part=1;
		}
		if (page==8)
		{
			page=0;
			writeglcdcmd(0xB8,3);
			part=1;
		}
		Yaddress=0;
	}
	busycheck();
	gLCDRS=1;
	gLCDRW=1;
	gLCDCS1=0;
	gLCDCS2=0;
	if (part==1) gLCDCS1=1;
	else if(part==2) gLCDCS2=1;
	gLCDE=1;
	asm("NOP");
	asm("NOP");
	asm("NOP");
	gLCDE=0;
	busycheck();
	gLCDRS=1;
	gLCDRW=1;
	gLCDCS1=0;
	gLCDCS2=0;
	if (part==1) gLCDCS1=1;
	else if(part==2) gLCDCS2=1;
	gLCDE=1;
	asm("NOP");
	asm("NOP");
	data=gLCDDATA;
	gLCDE=0;
	gLCDCS1=0;
	gLCDCS2=0;
	Yaddress++;
	return data;
}

void glcdCircle(unsigned char xcenter, unsigned char ycenter, unsigned char radius) // draw circle
{
	int tswitch, y, x = 0;
	unsigned char d;

	d = ycenter - xcenter;
	y = radius;
	tswitch = 3 - 2 * radius;
	while (x <= y)
	{
		glcdSetDot(xcenter + x, ycenter + y); 
		glcdSetDot(xcenter + x, ycenter - y);
		glcdSetDot(xcenter - x, ycenter + y);
		glcdSetDot(xcenter - x, ycenter - y);
		glcdSetDot(ycenter + y - d, ycenter + x);
		glcdSetDot(ycenter + y - d, ycenter - x);
		glcdSetDot(ycenter - y - d, ycenter + x);
		glcdSetDot(ycenter - y - d, ycenter - x);

		if (tswitch < 0) tswitch += (4 * x + 6);
		else
		{
			tswitch += (4 * (x - y) + 10);
			y--;
		}
		x++;
	}
}
void glcdRectangle(unsigned char x, unsigned char y, unsigned char bx, unsigned char ay) // draw rectangle
{
	unsigned char j;
	for (j = 0; j < ay; j++)
	{
		glcdSetDot(x, y + j);
		glcdSetDot(x + bx - 1, y + j);
	}
	for (j = 0; j < bx; j++)
	{
		glcdSetDot(x + j, y);
		glcdSetDot(x + j, y + ay - 1);
	}
}
void writehex(unsigned char hex)
{
	if ((hex & 0xF0) < 0xA0) writechar(0x30 + (hex & 0xF0)/16);
	else writechar(0x37 + (hex & 0xF0)/16);
	if ((hex & 0x0F) < 0x0A) writechar(0x30 + (hex & 0x0F));
	else writechar(0x37 + (hex & 0x0F));
}
void writedec_s(unsigned char dec)
{
	unsigned char dtemp;
	dtemp = dec/10; // tiotal
	writesmall(dtemp);
	writeglcddata(0x80);
	dtemp = dec%10; // ental
	writesmall(dtemp);
	writeglcddata(0x00);
}
void writesmall(unsigned char character)
{
	writeglcddata(chargen2[character*3]);
	writeglcddata(chargen2[character*3+1]);
	writeglcddata(chargen2[character*3+2]);
}
void writestime(unsigned char min,unsigned char sec)
{
	writesmall(min/10);
	writesmall(min%10);
	writeglcddata(0x80);
	writeglcddata(0x94);
	writeglcddata(0x80);
	writesmall(sec/10);
	writesmall(sec%10);
}
void writedec(unsigned int heltal)
{
	unsigned char dtemp;
	dtemp=heltal/10000; // tiotusental
	writechar(0x30 + dtemp);
	dtemp = heltal%10000/1000; // tusental
	writechar(0x30 + dtemp);
	dtemp = heltal%10000%1000/100; // hundratal
	writechar(0x30 + dtemp);
	dtemp = heltal%10000%1000%100/10; // tiotal
	writechar(0x30 + dtemp);
	dtemp = heltal%10000%1000%100%10; // ental
	writechar(0x30 + dtemp);
}
void glcdPutStr(unsigned char *data)
{
	while (*data)
	{
		writechar(*data);
		data++;
	}
}

