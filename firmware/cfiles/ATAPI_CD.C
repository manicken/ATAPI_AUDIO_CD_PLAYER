void write_ata_reg_chk(unsigned char data,unsigned char reg,unsigned status)
{
	CDDATALOW=data;
	write_ata_reg(reg);
	asm("NOP");
	while(read_bsy_drq()!=status);
}
void read_ata_reg(unsigned char addr)
{
	CDDIRHIGH=0xFF;
	CDDIRLOW=0xFF;
	CDADDR=addr & 0x1F;
	wait10us();
	CDRD = 0;
	wait10us();
	data_low = CDDATALOW;
	CDRD = 1;
	wait10us();
	CDADDR=0x18;
}

void write_ata_reg(unsigned char addr)
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
	data_high=CDDATAHIGH;
	data_low=CDDATALOW;
	CDRD=1;
	wait_xms(0x01);
	CDADDR=0x18;
}
void write_atapi(unsigned char high,unsigned char low,unsigned char alternative)
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

void read_idpd(void)
{
	unsigned char count=0;
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
		if ((data_high!=0x00) && (data_low!=0x00))
		{
			dataio[count]=data_high;
			dataio[count+1]=data_low;
			count=count+2;
		}
		reads++;
	}
	read_ata_reg(0x0E);
	read_ata_reg(0x17);
}
unsigned char read_bsy_drq(void)
{
	read_ata_reg(0x17);
	return (data_low & 0x88);
}
unsigned char read_error(void)
{
	read_ata_reg(0x17);
	return (data_low & 0x01);
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
void start_stop_unit(unsigned char un_load)
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
	unsigned char reads,temp;
	clear_dataio2();
	do{
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
	tracks = data_high; // Last Track
	while(reads<=tracks)
	{
		temp=reads*3;
		read_atapi(); // Reserved/ADR_Control
		read_atapi(); // Track Number/Reserved
		read_atapi(); // Reserved/Min
		dataio2[temp]=data_high; // Min
		read_atapi(); // Sek/Frame 
		dataio2[temp+1]=data_low; // Sek
		dataio2[temp+2]=data_high;// Frame
		reads++;
	}
	read_ata_reg(0x0E);
	read_ata_reg(0x17);
	}while (data_low & 0x01);
}
void read_cdtext(void)
{
	unsigned char reads,temp;
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
		dataio[temp]=data_low; // Sek
		dataio[temp+1]=data_high;// Frame
		reads++;
	}
	read_ata_reg(0x0E);
	read_ata_reg(0x17);
	print_dataio();
//	}while (data_low & 0x01);
}
void mode_sense(unsigned char mode)
{
	unsigned char reads,temp,length;
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
	dataio[temp]=data_low;
	dataio[temp+1]=data_high;
	reads++;
	while(reads<dataio[1]+1)
	{
		temp=2*reads;
		read_atapi();  
		dataio[temp]=data_low;
		dataio[temp+1]=data_high;
		reads++;
	}
	read_ata_reg(0x0E);
	read_ata_reg(0x17);
}
void prevent_allow_eject(unsigned char prevent)
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
void play_audio_msf(unsigned char bmin,unsigned char bsek,unsigned char bframe,unsigned char emin,unsigned char esek,unsigned char eframe)
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
void pause_resume(unsigned char resume)
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
	unsigned char count=0;
	while (count < 96)
	{
		dataio[count]=0;
		count++;
	}
}
void clear_dataio2(void)
{
	unsigned char count=0;
	while (count < 96)
	{
		dataio2[count]=0;
		count++;
	}
}
void print_track(void)
{
	writeglcdcmd(0x40,3);
	part=1;
	writeglcdcmd(0xBF,3);
//	writechar(track/10+0x30);
//	writechar(track%10+0x30);
	writedec_s(track);
	writechar('/');
//	writechar(tracks/10+0x30);
//	writechar(tracks%10+0x30);
	writedec_s(tracks);
}
void print_dataio2(void)
{
	unsigned char min,sec;
	writeglcdcmd(0x40,3); // set Y adress to zero (0x40-0x7F)
	part=1;
	writeglcdcmd(0xB8,3); // set page to zero (0xB8-0xBF)
	tmp=0;
	while (tmp<(tracks)*3)
	{

		if (dataio2[tmp+4]<dataio2[tmp+1])
		{
			min = dataio2[tmp+3]-dataio2[tmp]-1;
			sec = 60-(dataio2[tmp+1]-dataio2[tmp+4])
			writestime(min,sec);
		}
		else
		{
			min = dataio2[tmp+3]-dataio2[tmp];
			sec = dataio2[tmp+4]-dataio2[tmp+1];
			writestime(min,sec);
		}
		writeglcddata(0x00);
		tmp=tmp+3;
	}
}
void print_dataio(void)
{
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
	track++;
	if (track > tracks) track = 1;
	print_track();
	if (play)
	{
		tmp=(track-1)*3;
		play_audio_msf(dataio2[tmp],dataio2[tmp+1],dataio2[tmp+2],dataio2[tmp+3],dataio2[tmp+4],dataio2[tmp+5]);
	}
}
void prev_track(void)
{
	track--;
	if (track == 0) track = tracks;
	print_track();
	if (play)
	{
		tmp=(track-1)*3;
		play_audio_msf(dataio2[tmp],dataio2[tmp+1],dataio2[tmp+2],dataio2[tmp+3],dataio2[tmp+4],dataio2[tmp+5]);
	}
}
