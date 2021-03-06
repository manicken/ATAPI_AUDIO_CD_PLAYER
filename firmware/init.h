void init_PCB(void)
{
	INTCON=0x00;
	ADCON1=0x06;
	OPTION=0x80;
	// CONTROL SIGNALS
	PORTA = 0x18;
	TRISA = 0x20;
	PORTB = 0x00;
	TRISB = 0xC1;
	PORTE = 0x03;
	TRISE = 0x04;
	// DATA SIGNALS
	PORTC = 0x00;
	TRISC = 0xFF;
	PORTD = 0x00;
	TRISD = 0xFF;
	PEIE=0;
	GIE=1;
	INTE=1;
	T1CON=0x20;
}
