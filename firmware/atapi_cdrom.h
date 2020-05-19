// redefines
#define		CDRD 		RE0
#define		CDWR 		RE1
#define 	CDDATAHIGH 	PORTC
#define 	CDDATALOW	PORTD
#define 	CDDIRHIGH 	TRISC
#define 	CDDIRLOW 	TRISD
#define		CDADDR		PORTA
// globals
char bank2 toc[96];
char bank3 dataio[96];
char tracks;
char track=1, play=0, infosel=0;
// prototypes
void read_ata_reg(char addr);
void write_ata_reg(char addr);
void read_atapi(void);
void write_atapi(char high, char low, char alternative);
void read_idpd(void);
char read_bsy_drq(void);
void write_ata_reg_chk(char data, char reg, char status);
void write_zeros(void);
void init_packet_command(void);
void end_packet_command(void);
void device_master(void);
void start_stop_unit(char un_load);
void read_toc(void);
void read_cdtext(void);
void mode_sense(char mode);
void prevent_allow_eject(char prevent);
void play_audio_msf(char bmin, char bsek, char bframe, char emin, char esek, char eframe);
void pause_resume(char resume);
void clear_dataio(void);
void clear_toc(void);
void print_track(void);
void print_toc(void);
void print_dataio(void);
void prev_track(void);
void next_track(void);

char read_error(void);

void write_ata_reg_chk(char data, char reg, char status)
{
	CDDATALOW=data;
	write_ata_reg(reg);
	asm("NOP");
	while(read_bsy_drq()!=status);
}
void read_ata_reg(char addr)
{
	CDDIRHIGH=0xFF;
	CDDIRLOW=0xFF;
	CDADDR=addr & 0x1F;
	wait10us();
	CDRD = 0;
	wait10us();
	CDDATALOW=CDDATALOW;
	CDRD = 1;
	wait10us();
	CDADDR=0x18;
}
void write_ata_reg(char addr)
{
	CDDIRLOW=0x00;
	CDADDR=addr & 0x1F;
	wait10us();
	CDWR = 0;
	asm("NOP");
	CDWR = 1;
	wait10us();
	CDDIRLOW=0xFF;
	CDADDR=0x18;
}
void read_atapi(void)
{
	CDDIRLOW=0xFF;
	CDDIRHIGH=0xFF;
	CDADDR=0x10;
	wait_xms(0x01);
	CDRD=0;
	wait_xms(0x01);
	CDDATAHIGH=CDDATAHIGH;
	CDDATALOW=CDDATALOW;
	CDRD=1;
	wait_xms(0x01);
	CDADDR=0x18;
}
void write_atapi(char high, char low, char alternative)
{
	CDDIRLOW=0x00;
	CDDIRHIGH=0x00;
	CDADDR=0x10;
	CDDATAHIGH=high;
	CDDATALOW=low;
	wait10us();
	CDWR=0;
	asm("NOP");
	CDWR=1;
	wait10us();
	CDDIRLOW=0xFF;
	CDDIRHIGH=0xFF;
	if (alternative) read_ata_reg(0x0E); // Alternative status
	CDADDR=0x18;
}
char read_bsy_drq(void)
{
	char data;
	read_ata_reg(0x17);
	CDDIRLOW=0x00;
	data=CDDATALOW;
	CDDIRLOW=0xFF;
	return (data & 0x88);
}
char read_error(void)
{
	char data;
	read_ata_reg(0x17);
	CDDIRLOW=0x00;
	data=CDDATALOW;
	CDDIRLOW=0xFF;
	return (data & 0x01);
}
void write_zeros(void)
{
	CDDIRLOW=0x00;
	CDDIRHIGH=0x00;
	CDADDR=0x10;
	CDDATAHIGH=0x00;
	CDDATALOW=0x00;
	wait10us();
	CDWR=0;
	asm("NOP");
	CDWR=1;
	wait10us();
	CDDIRLOW=0xFF;
	CDDIRHIGH=0xFF;
	read_ata_reg(0x0E); // Alternative status
}

void read_idpd(void)
{
	char count=0;
	int reads=0;
	clear_dataio();
	device_master();
	write_ata_reg_chk(0x0A,0x0E,0x00);
	write_ata_reg_chk(0x00,0x11,0x00);
	write_ata_reg_chk(0x00,0x12,0x00);
	write_ata_reg_chk(0x00,0x14,0x00);
	write_ata_reg_chk(0x00,0x15,0x00);

	CDDATALOW=0xA1; // Identify Packet Device (ATA command)
	write_ata_reg(0x17);
	wait_xms(10);
	read_ata_reg(0x0E); // Alternative status
	while(read_bsy_drq()!=0x08);
	while(reads < 256)
	{
		read_atapi();
		CDDIRLOW=0x00;
		CDDIRHIGH=0x00;
		if ((CDDATAHIGH!=0x00) && (CDDATALOW!=0x00))
		{
			dataio[count]=CDDATAHIGH;
			dataio[count+1]=CDDATALOW;
			count=count+2;
		}
		CDDIRLOW=0xFF;
		CDDIRHIGH=0xFF;
		reads++;
	}
	read_ata_reg(0x0E);
	read_ata_reg(0x17);
}
void init_packet_command(void)
{
	device_master();
	CDDATALOW=0x0A; // disable interrupt
	write_ata_reg(0x0E); // device control
	CDDATALOW=0x00; // PIO timings
	write_ata_reg(0x11); // features
	CDDATALOW=0x00; 
	write_ata_reg(0x12); // sector count
	write_ata_reg(0x14); // ATAPI Byte Count LSB
	CDDATALOW=0x60; 
	write_ata_reg(0x15); // ATAPI Byte Count MSB
	CDDATALOW=0xA0; // packet command
	write_ata_reg(0x17); // Command
	asm("NOP");
	asm("NOP");
	while(read_bsy_drq()!=0x08);
}
void end_packet_command(void)
{
	read_ata_reg(0x0E); // Alternative status
	while(read_bsy_drq()!=0x00);
}
void device_master(void)
{
	while(read_bsy_drq()!=0x00);
	CDDATALOW=0x00; // select master drive
	write_ata_reg(0x16); // Device/Head
	asm("NOP");
	asm("NOP");
	while(read_bsy_drq()!=0x00);
}
void start_stop_unit(char un_load)
{
	init_packet_command();
	write_atapi(0x01,0x1B,1); // return status immediately
	write_zeros();
	write_atapi(0x00,un_load,1);
	write_zeros();
	write_zeros();
	write_zeros();
	end_packet_command();
}
void read_toc(void)
{
	char reads,temp;
	clear_toc();
	do{
		CDDIRLOW=0xFF;
		init_packet_command();
		write_atapi(0x02,0x43,1);
		write_zeros();
		write_zeros();
		write_atapi(0xFF,0x00,1);
		write_atapi(0x00,0xFF,1);
		write_zeros();
		read_ata_reg(0x0E); // Alternative status
		read_ata_reg(0x17);
	
		while((read_bsy_drq()!=0x08) && (read_error()!=0x01));
		wait_xms(10);
		reads=0;
		read_atapi(); // TOC Data Lenght
		read_atapi(); // First/Last Track Number
		CDDIRHIGH=0x00;
		tracks = CDDATAHIGH; // Last Track
		CDDIRHIGH=0xFF;
		while((reads<=tracks)&&(reads<=32))
		{
			temp=reads*3;
			read_atapi(); // Reserved/ADR_Control
			read_atapi(); // Track Number/Reserved
			read_atapi(); // Reserved/Min
			CDDIRHIGH=0x00;
			toc[temp]=CDDATAHIGH; // Min
			CDDIRHIGH=0xFF;
			read_atapi(); // Sek/Frame 
			CDDIRLOW=0x00;
			CDDIRHIGH=0x00;
			toc[temp+1]=CDDATALOW; // Sek
			toc[temp+2]=CDDATAHIGH;// Frame
			CDDIRLOW=0xFF;
			CDDIRHIGH=0xFF;
			reads++;
		}
		while(reads<=tracks)
		{
			temp=reads*3;
			read_atapi(); // Reserved/ADR_Control
			read_atapi(); // Track Number/Reserved
			read_atapi(); // Reserved/Min
			read_atapi(); // Sek/Frame
			reads++;
		}
		read_ata_reg(0x0E);
		read_ata_reg(0x17);
		CDDIRLOW=0x00;

	}while (CDDATALOW & 0x01);
	CDDIRLOW=0xFF;
}
void read_cdtext(void)// denna funktion är ej klar (dvs den funkar inte) avkodning av cd-text är komplext
{
	char reads,temp;
	clear_dataio();
//	do{
	init_packet_command();
	write_atapi(0x02,0x43,1);
	write_atapi(0x00,0x05,1);
	write_zeros();
	write_atapi(0xFF,0x00,1);
	write_atapi(0x00,0xFF,1);
	write_zeros();
	read_ata_reg(0x0E); // Alternative status
	read_ata_reg(0x17);
	
	while((read_bsy_drq()!=0x08) && (read_error()!=0x01));
	wait_xms(10);
	reads=0;
	read_atapi(); // TOC Data Lenght
	read_atapi(); // First/Last Track Number
	while(reads<=47)
	{
		temp=reads*2;
		read_atapi(); // Sek/Frame 
		dataio[temp]=CDDATALOW; // Sek
		dataio[temp+1]=CDDATAHIGH;// Frame
		reads++;
	}
	read_ata_reg(0x0E);
	read_ata_reg(0x17);
	print_dataio();
//	}while (data_low & 0x01);
}
void mode_sense(char mode)// denna funktion är ej klar (dvs den funkar inte)
{
	char reads,temp,length;
	init_packet_command();
	write_atapi(0x00,0x5A,1);
	write_atapi(0x00,mode,1);
	write_zeros();
	write_atapi(0xFF,0x00,1);
	write_atapi(0x00,0xFF,1);
	write_zeros();
	read_ata_reg(0x0E); // Alternative status
	read_ata_reg(0x17);
	while((read_bsy_drq()!=0x08));
	wait_xms(10);
	reads=0;
	read_atapi(); // 
	temp=2*reads;
	dataio[temp]=CDDATALOW;
	dataio[temp+1]=CDDATAHIGH;
	reads++;
	while(reads<dataio[1]+1)
	{
		temp=2*reads;
		read_atapi();  
		dataio[temp]=CDDATALOW;
		dataio[temp+1]=CDDATAHIGH;
		reads++;
	}
	read_ata_reg(0x0E);
	read_ata_reg(0x17);
}
void prevent_allow_eject(char prevent)
{
	init_packet_command();
	write_atapi(0x00,0x1E,1);
	write_zeros();
	write_atapi(0x00,prevent,1);
	write_zeros();
	write_zeros();
	write_zeros();
	end_packet_command();
}
void play_audio_msf(char bmin, char bsek, char bframe, char emin, char esek, char eframe)
{
	init_packet_command();
	write_atapi(0x00,0x47,1);
	write_atapi(bmin,0x00,1);
	write_atapi(bframe,bsek,1);
	write_atapi(esek,emin,1);
	write_atapi(0x00,eframe,1);
	write_zeros();
	end_packet_command();
}
void pause_resume(char resume)
{
	init_packet_command();
	write_atapi(0x00,0x4B,1);
	write_zeros();
	write_zeros();
	write_zeros();
	write_atapi(0x00,resume,1);
	write_zeros();
	end_packet_command();
}	
void clear_dataio(void)
{
	char count=0;
	while (count < 96)
	{
		dataio[count]=0;
		count++;
	}
}
void clear_toc(void)
{
	char count=0;
	while (count < 96)
	{
		toc[count]=0;
		count++;
	}
}
void print_track(void)
{
	writeglcdcmd(0x6C,3);
	part=2;
	writeglcdcmd(0xBF,3);
	writedec_s(track);
	writeglcddata(0x30);
	writeglcddata(0x08);
	writeglcddata(0x06);
	writeglcddata(0x00);
	writedec_s(tracks);
}
void print_toc(void)
{
	char min,sec,tmp;
	clearglcd();
	writeglcdcmd(0x40,3); // set Y adress to zero (0x40-0x7F)
	part=1;
	writeglcdcmd(0xB8,3); // set page to zero (0xB8-0xBF)
	tmp=0;
	while ((tmp<(tracks*3))&&(tmp<93))
	{

		if (toc[tmp+4]<toc[tmp+1])
		{
			min = toc[tmp+3]-toc[tmp]-1;
			sec = 60-(toc[tmp+1]-toc[tmp+4]);
			writestime(min,sec);
		}
		else
		{
			min = toc[tmp+3]-toc[tmp];
			sec = toc[tmp+4]-toc[tmp+1];
			writestime(min,sec);
		}
		writeglcddata(0x00);
		min=((tmp/3)+1);
		if ((min==7)||(min==14)||(min==21)||(min==28)||(min==35)||(min==42))
		{
			writeglcddata(0x00);
			writeglcddata(0x00);
		}
		tmp=tmp+3;
	}
}
void print_dataio(void)
{
	char tmp;
	writeglcdcmd(0x40,3); // set Y adress to zero (0x40-0x7F)
	part=1;
	writeglcdcmd(0xB8,3); // set page to zero (0xB8-0xBF)
	tmp=0;
	while (tmp<96)
	{
		if (dataio[tmp]!=0) writechar(dataio[tmp]);
		tmp++;
	}
}
void next_track(void)
{
	char tmp;
	track++;
	if (track > tracks) track = 1;
	print_track();
	if (play)
	{
		tmp=(track-1)*3;
		play_audio_msf(toc[tmp], toc[tmp+1], toc[tmp+2], toc[tmp+3], toc[tmp+4], toc[tmp+5]);
	}
}
void prev_track(void)
{
	char tmp;
	track--;
	if (track == 0) track = tracks;
	print_track();
	if (play)
	{
		tmp=(track-1)*3;
		play_audio_msf(toc[tmp], toc[tmp+1], toc[tmp+2], toc[tmp+3], toc[tmp+4], toc[tmp+5]);
	}
}
