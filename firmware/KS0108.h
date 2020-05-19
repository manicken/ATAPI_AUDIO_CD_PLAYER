#define gLCDCS1		RB4
#define gLCDCS2		RB5
#define gLCDRS		RB1
#define gLCDRW		RB2
#define gLCDE		RB3
#define gLCDDATA	PORTC
#define gLCDDATADIR	TRISC

static volatile bit	gLCDBSY	@ (unsigned)&gLCDDATA*8+7;
char page=0;
char part=1;
char Yaddress=0;

void busycheck(void);
void writeglcddata(char data);//,char chipselect);
void writeglcdcmd(char data, char chipselect);
void writechar(char character);
void clearglcd(void);
void initglcd(void);
void glcdSetDot(char x, char y);
void glcdClearDot(char x, char y);
char readglcddata(void);
char setbit(char byte);
void writehex(char hex);
void writedec_s(char dec);
void writesmall(char character);
void writestime(char min, char sec);
void writedec(unsigned int heltal);
void glcdPutStr(char *data);


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

void writeglcddata(char data)//,char chipselect)
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

void writeglcdcmd(char data, char chipselect)
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
void writechar(char character)
{
	char loop=0;
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

void glcdSetDot(char x, char y) // set dot
{
    char temp;
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

void glcdClearDot(char x, char y) // clear dot
{
    char temp;
	if (x<128) writeglcdcmd(0x40+x,1);
	else writeglcdcmd(0x40+x,2);
	writeglcdcmd(0xB8+(y/8),3);

    temp = readglcddata();  // read back current value

	if (x<128) writeglcdcmd(0x40+x,1);
	else writeglcdcmd(0x40+x,2);

    writeglcddata(temp & !(1 << (y % 8)));
}
char readglcddata(void)
{
	char data=0;
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

void glcdCircle(char xcenter, char ycenter, char radius) // draw circle
{
	int tswitch, y, x = 0;
	char d;

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
void glcdRectangle(char x, char y, char bx, char ay) // draw rectangle
{
	char j;
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
void writehex(char hex)
{
	if ((hex & 0xF0) < 0xA0) writechar(0x30 + (hex & 0xF0)/16);
	else writechar(0x37 + (hex & 0xF0)/16);
	if ((hex & 0x0F) < 0x0A) writechar(0x30 + (hex & 0x0F));
	else writechar(0x37 + (hex & 0x0F));
}
void writedec_s(char dec)
{
	char dtemp;
	dtemp = dec/10; // tiotal
	writesmall(dtemp);
	writeglcddata(0x80);
	dtemp = dec%10; // ental
	writesmall(dtemp);
	writeglcddata(0x00);
}
void writesmall(char character)
{
	writeglcddata(chargen2[character*3]);
	writeglcddata(chargen2[character*3+1]);
	writeglcddata(chargen2[character*3+2]);
}
void writestime(char min, char sec)
{
	writesmall(min/10);
	writeglcddata(0x80);
	writesmall(min%10);
	writeglcddata(0x80);
	writeglcddata(0x94);
	writeglcddata(0x80);
	writesmall(sec/10);
	writeglcddata(0x80);
	writesmall(sec%10);
}
void writedec(unsigned int heltal)
{
	char dtemp;
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
void glcdPutStr(char *data)
{
	while (*data)
	{
		writechar(*data);
		data++;
	}
}

