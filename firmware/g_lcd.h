#define LCDWR 		RE1
#define LCDRD	 	RE0
#define LCDCE	    RE2
#define LCDCD	    RC2
#define LCDRST	    RC5
#define LCDFS		RC7
#define GLCDDATA	PORTB
#define GLCDDIR		TRISB
#define dat			0
#define cmd			1

void cursor_point_set( char column, char row);
void offset_reg_set( char cg_ram_adr);
void addr_point_set( char adr_high, char adr_low);

void text_home_addr_set( char adr_high, char adr_low);
void text_area_set( char columns, char fontselect);
void graph_home_addr_set( char adr_high, char adr_low);
void graph_area_set( char columns);
void init_glcd(void);

void write_char_on_pos(char data,char col,char row); 
void write_str_on_pos(const unsigned char *string_pointer, unsigned char position, unsigned char rad);
void write2glcd( char data,char cmd_data);
void writeauto( char data);
void writepulse(void);
char readstatus(void);
void automodeoff(void);

void debug(char number,char pos);
void write_char_on_pos(char data,char col,char row);

void rst_glcd(void);
void use_chargen_SRAM(unsigned char table);
//void write_new_chargen_to_EEPROM(unsigned char plats);
//mode select and div funk

void init_glcd(void)
{
	write2glcd(0x94,cmd);
	write2glcd(0x80,cmd);

	text_home_addr_set(0x00,0x00);
	text_area_set(0x1E,0); //(hex=dec) 1E=30, 20=32, 28=40
	graph_home_addr_set(0x03,0x00);
	graph_area_set(0x1E);
	offset_reg_set(0x03);
	write2glcd(0xA0,cmd);
	cursor_point_set(0,1);
}
void text_home_addr_set(char adr_high,char adr_low)
{
	write2glcd(adr_low,dat);
	write2glcd(adr_high,dat);
	write2glcd(0x40,cmd);
}
void text_area_set(char columns,char fontselect)
{
	if (fontselect==0) LCDFS=0;
	if (fontselect==1) LCDFS=1;
	write2glcd(columns,dat);
	write2glcd(0x00,dat);
	write2glcd(0x41,cmd);
}
void graph_home_addr_set(char adr_high,char adr_low)
{
	write2glcd(adr_low,dat);
	write2glcd(adr_high,dat);
	write2glcd(0x42,cmd);
}
void graph_area_set(char columns)
{
	write2glcd(columns,dat);
	write2glcd(0x00,dat);
	write2glcd(0x43,cmd);
}
void cursor_point_set(char column,char row)
{
	write2glcd(column,dat);
	write2glcd(row,dat);
	write2glcd(0x21,cmd);
}
void offset_reg_set(char cg_ram_adr)
{
	write2glcd(cg_ram_adr,dat);
	write2glcd(0x00,dat);
	write2glcd(0x22,cmd);
}
void addr_point_set(char adr_high,char adr_low)
{
	write2glcd(adr_low,dat);
	write2glcd(adr_high,dat);
	write2glcd(0x24,cmd);
}
void clrglcd(void)
{
	char count=0,count2=0;
	addr_point_set(0x00,0x00);
	write2glcd(0xB0,cmd);
	count=0;
	while (count<32)
	{
		count2=0;
		while (count2<255)
		{
			writeauto(0x00);
			count2++;
		}
		count++;
	}
	automodeoff();
}
void use_chargen_SRAM(unsigned char table)
{
	int count=0;
	write2glcd(0x88,cmd);
	addr_point_set(0x10,0x00);
	write2glcd(0xB0,cmd);
	read_ext_eeprom_start(table,0x00);
	while (count<4096)
	{
		writeauto(i2c_read(1)); //external CG-RAM write from ext i2c EEPROM
		count++;
	}
	i2c_stop();
	automodeoff();
}

// interface funks

void rst_glcd(void)
{
	LCDRD=1;
	LCDWR=1;
	LCDCE=1;
	LCDCD=1;
	LCDRST=1;
	wait_xms(50);
	LCDRST=0;
	wait_xms(1);
	LCDRST=1;
	wait_xms(50);
}
void writepulse(void)
{
	LCDCE=0;
	LCDWR=0;
	LCDWR=1;
	LCDCE=1;
}
void write2glcd(char data,char cmd_data)
{
	while((readstatus() & 0x03) != 0x03){}
	GLCDDIR=0x00;
	GLCDDATA=data;
	if (cmd_data==0x00){LCDCD=0;}
	if (cmd_data==0x01){LCDCD=1;}
	writepulse();
	GLCDDIR=0xFF;
}
char readstatus(void)
{
	char data;
	GLCDDIR=0xFF;
	LCDCD=1;
	LCDCE=0;
	LCDRD=0;
	LCDRD=0;	
	data=GLCDDATA;
	LCDRD=1;
	LCDCE=1;
	return data;
}
void writeauto(char data)
{
	while((readstatus() & 0x08) != 0x08){}
	GLCDDIR=0x00;
	LCDCD=0;
	GLCDDATA=data;
	writepulse();
	GLCDDIR=0xFF;
}
void automodeoff(void)
{
	while((readstatus() & 0x08) != 0x08){}
	GLCDDIR=0x00;
	LCDCD=1;
	GLCDDATA=0xB2;
	writepulse();
	GLCDDIR=0xFF;
}
void write_char_on_pos(char data,char col,char row)
{
	addr_point_set(0x00,col+30*row);
	write2glcd(data,dat);
	write2glcd(0xC4,cmd);
}
void write_str_on_pos(const unsigned char *string_pointer, unsigned char position, unsigned char rad)
{

	addr_point_set(0x00,position+30*rad);
	write2glcd(0xB0,cmd);
	while (*string_pointer)
	{
		writeauto(*string_pointer);
		string_pointer++;
	}
	automodeoff();
}
void debug(char number,char pos)
{
	addr_point_set(0x00,210+pos);
	write2glcd(number,dat);
	write2glcd(0xC4,cmd);
}
/*void write_new_chargen_to_EEPROM(unsigned char plats)
{
	unsigned char page=0,byte;
	while (page<16)
	{
		write_ext_eeprom_start(plats+(page&0x0E)/2,(page&0x01)*0x80);
		byte=0;
		while (byte<128)
		{
			i2c_write(chargen[page*128+byte]);
			byte++;
		}
		i2c_stop();
		wait_xms(6);
		page++;
	}
}
*/
