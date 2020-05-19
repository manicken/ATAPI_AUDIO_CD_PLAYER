// redefines
#define		CDRD 		RE0
#define		CDWR 		RE1
#define 	CDDATAHIGH 	PORTC
#define 	CDDATALOW	PORTD
#define 	CDDIRHIGH 	TRISC
#define 	CDDIRLOW 	TRISD
#define		CDADDR		PORTA
// globals
unsigned char data_high;
unsigned char data_low;
unsigned char bank2 dataio2[96];
unsigned char bank3 dataio[96];
unsigned char tracks;
unsigned char track=1,play=0,tmp=0,infosel=0;
// prototypes
void read_ata_reg( unsigned char addr);
void write_ata_reg( unsigned char addr);
void read_atapi(void);
void write_atapi(unsigned char high,unsigned char low,unsigned char alternative);
void read_idpd(void);
unsigned char read_bsy_drq(void);
void write_ata_reg_chk(unsigned char data,unsigned char reg,unsigned status);
void write_zeros(void);
void init_packet_command(void);
void end_packet_command(void);
void device_master(void);
void start_stop_unit(unsigned char un_load);
void read_toc(void);
void read_cdtext(void);
void mode_sense(unsigned char mode);
void prevent_allow_eject(unsigned char prevent);
void play_audio_msf(unsigned char bmin,unsigned char bsek,unsigned char bframe,unsigned char emin,unsigned char esek,unsigned char eframe);
void pause_resume(unsigned char resume);
void clear_dataio(void);
void clear_dataio2(void);
void print_track(void);
void print_dataio2(void);
void print_dataio(void);
void prev_track(void);
void next_track(void);

unsigned char read_error(void);
