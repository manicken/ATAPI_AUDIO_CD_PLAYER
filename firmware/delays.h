void wait10us(void);
void wait1us(void);
void wait_xms(char millisec);
void wait1s(void);
void wait500ms(void);

void wait10us(void)
{
	asm("NOP");
	asm("NOP");
	asm("NOP");
	asm("NOP");
	asm("NOP");
	asm("NOP");
	asm("NOP");
	asm("NOP");
	asm("NOP");
	asm("NOP");
}
void wait1us(void)
{
	asm("NOP");
	asm("NOP");
	asm("NOP");
	asm("NOP");
}
void wait_xms(char millisec)
{
	OPTION=OPTION & 0xC0;
	OPTION=OPTION | 0x04; //prescaler divide by 256
   do
   {
      TMR0=0;
      while (TMR0<157);
      millisec--;
   }
   while (millisec > 0);
}
void wait1s(void)
{
	wait_xms(250);
	wait_xms(250);
	wait_xms(250);
	wait_xms(250);
}	
void wait500ms(void)
{
	wait_xms(250);
	wait_xms(250);
}
