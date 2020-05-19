/******************************************************************************************/
// Hardware I2C single master routines for PIC16F877
// for HI-TEC PIC C COMPILER.
//
// i2c_init  - initialize I2C functions
// i2c_start - issue Start condition
// i2c_repStart- issue Repeated Start condition
// i2c_stop  - issue Stop condition
// i2c_read(x) - receive unsigned char - x=0, don't acknowledge - x=1, acknowledge
// i2c_write - write unsigned char - returns ACK
//
// modified from CCS i2c_877 demo
//
// by
//
// Michael Alon - MICROCHIP FAE
// My E-mail michael@elina-micro.co.il
// www.elina-micro.co.il
// Tel: 972-3-6498543/4
// Fax: 972-3-6498745
// Elina Micro LTD .ISRAEL
//
/******************************************************************************************/
void i2c_init(void)
{
	TRISC3=1;           // set SCL and SDA pins as inputs
	TRISC4=1;

	SSPCON = 0x28;      // set I2C master mode
	SSPCON2 = 0x00;
	SSPSTAT=0x00;
	SSPADD = 0x32;     // SSPADD=((Fosc/Bitrate)/4)-1

	STAT_CKE=0;     // use I2C levels      worked also with '0'
	STAT_SMP=1;     // disable slew rate control  worked also with '0'

	PSPIF=0;      // clear SSPIF interrupt flag
	BCLIF=0;      // clear bus collision flag
}

void i2c_waitForIdle(void)
{
	while (( SSPCON2 & 0x1F ) | STAT_RW ) {}; // wait for idle and not writing
}

void i2c_start(void)
{
	i2c_waitForIdle();
	SEN=1;
}

void i2c_repStart(void)
{
	i2c_waitForIdle();
	RSEN=1;
}

void i2c_stop(void)
{
	i2c_waitForIdle();
	PEN=1;
}

int i2c_read( unsigned char ack )
{
	unsigned char i2cReadData;

	i2c_waitForIdle();
	RCEN=1;
	i2c_waitForIdle();
	i2cReadData = SSPBUF;
	i2c_waitForIdle();
	if ( ack ) ACKDT=0;
	else	   ACKDT=1;
	ACKEN=1;               // send acknowledge sequence
	
	return( i2cReadData );
}

/******************************************************************************************/

unsigned char i2c_write( unsigned char i2cWriteData )
{
	i2c_waitForIdle();
	SSPBUF = i2cWriteData;

	return ( ! ACKSTAT  ); // function returns '1' if transmission is acknowledged
}

/******************************************************************************************/


/******************************************************************************************/
void write_ext_eeprom_start(unsigned char high,unsigned char low)
{
	i2c_start();
	i2c_write(0xa0);
	i2c_write(high);
	i2c_write(low);
}
void write_ext_eeprom_stop(void)
{
	i2c_stop();
	wait_xms(6);
}
void write_ext_eeprom(unsigned char high,unsigned char low, unsigned char data)
{
	i2c_start();
	i2c_write(0xa0);
	i2c_write(high);
	i2c_write(low);
	asm("NOP");
	asm("NOP");
	asm("NOP");
	i2c_write(data);
	i2c_stop();
	wait_xms(6);
}

/******************************************************************************************/

unsigned char read_ext_eeprom(unsigned char high,unsigned char low)
{
	unsigned char data;

	i2c_start();
	i2c_write(0xa0);
    i2c_write(high);
    i2c_write(low);
    i2c_repStart();
    i2c_write(0xa1);
    data=i2c_read(0);
    i2c_stop();
    return(data);
}
void read_ext_eeprom_start(unsigned char high,unsigned char low)
{
    i2c_start();
    i2c_write(0xa0);
    i2c_write(high);
    i2c_write(low);
    i2c_repStart();
    i2c_write(0xa1);
}

