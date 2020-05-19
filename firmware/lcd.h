// lcd.c
// drivers for the alphanumeric LDC
// för PIC16F877
 
//definitions

#define RS	     RE0
#define RW	     RE1
#define E	     RE2
#define Busy_flag    RD7
#define Function_set 0x20
#define DL_8_bits    0x10  //interface data length (DL) 8 bits
#define Display_ON   0x0C     
#define Display_OFF  0x08
#define Cursor_ON    0x0A
#define Cursor_OFF   0x08     
#define Blink_ON     0x09
#define BLINK_OFF    0x08     
#define N_1_line  0x00   //display with 1 row
#define N_2_lines 0x08   //display with 2 rows

//definitions of instructions

#define Clear_display           0x01
#define Return_home             0x02
#define Auto_increment_position 0x06
#define Display_shift_left      0x18
#define Move_cursor_left	0x10
#define Move_cursor_right	0x14
#define Shift_display_left	0x18
#define Shift_display_right	0x1C

void read_busyflag(void)
{
   TRISD = 0xFF; //inport
   RS = 0; //RS=0
   RW = 1; //RW=1, läsning
   E = 1; //Enable=1
   asm("NOP");
   asm("NOP");
   while (Busy_flag==1);     //wait intil Busy flag (BF) = 0
   asm("NOP");
   asm("NOP");
   E = 0;
   RW = 0;
   TRISD = 0x00; //utport
}

void write_to_LCD(unsigned char data)
{
   PORTD = data;
   E=1;          // sätt Enable = 1
   asm("NOP");
   asm("NOP");
   asm("NOP");
   E=0;         // sätt Enable = 0
}

void write_cmd(unsigned char code)
{
   read_busyflag();
   RS=0;
   write_to_LCD(code);
}

void write_char(unsigned char character)
{
   read_busyflag();
   RS=1;
   write_to_LCD(character);
}

void wait_15ms(void)
{
   char millisec = 15;
   OPTION=0x84; //prescaler divide by 32
   do
   {
      TMR0=0;
      while (TMR0<156);
   }
   while (--millisec > 0);
}

void write_char_on_pos(unsigned char character, unsigned char position, unsigned char rad)
{
   unsigned char adress;
   adress = position-1;
   if (rad==2)
   {
      adress |= 0x40;
   }
   write_cmd(adress | 0x80);
   write_char(character);
}

void write_str_on_pos(const unsigned char *string_pointer, unsigned char position, unsigned char rad)
{
   unsigned char adress;
   adress = position-1;
   if (rad==2)
   {
      adress |= 0x40;
   }
   write_cmd(adress | 0x80);
   while (*string_pointer)
   {
      write_char(*string_pointer);;
      string_pointer++;
   }
}
void initLCD(void) //Initializing For 8 bit data interfacing
{
   TRISA=0x08;
   TRISD=0;
   ADCON1=6;
   RW = 0;
   wait_15ms();
   RS = 0;
   write_to_LCD(Function_set | DL_8_bits);  
   wait_15ms();
   write_to_LCD(Function_set | DL_8_bits);
   wait_15ms();
   write_to_LCD(Function_set | DL_8_bits);
   write_cmd(Function_set | DL_8_bits | N_2_lines);
   write_cmd(Display_OFF);
   write_cmd(Clear_display);
   write_cmd(Auto_increment_position);
}
