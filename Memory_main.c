#include <stdio.h>
#include <avr/io.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <xc.h>
#include <avr/interrupt.h>
#define Read_Status_Register 0x05
#define write_Status_Register 0x01
#define Write_Enable 0x06
#define  Write_Disable 0x04
#define Read_Memory 0x03
#define write_Memory 0x02
// Function to initialize SPI
void SPI0_Init() {
    SPI0_CTRLA = 0x00;
    SPI0_CTRLA |=(1<<SPI_MASTER_bp)|(1<<SPI_ENABLE_bp);
    SPI0_CTRLB = 0x00;
    SPI0_CTRLB |= (SPI_MODE_0_gc | SPI_SSD_bm);
    SPI0_INTCTRL = 0x00;
    SPI0_INTCTRL |=(1<<SPI_IE_bp);
    SPI0_INTFLAGS = 0x00;
    SPI0_INTFLAGS |= (1<<SPI_IF_bp);
    }
void SPI0_Enable()
{
    SPI0.CTRLA |= SPI_ENABLE_bm;
}

void SPI0_Disable()
{
    SPI0.CTRLA &= ~SPI_ENABLE_bm;
}
uint8_t SPI0_ReadData()
{
 SPI0.DATA = 0xFF; //send dummy data
 while (!(SPI0.INTFLAGS & SPI_IF_bm)); /* Waits until data are exchanged*/
 return SPI0.DATA;
}
void SPI0_WriteData(uint8_t data)
{
 SPI0.DATA = data; //send data
 while (!(SPI0.INTFLAGS & SPI_IF_bm)); /* Waits until data are exchanged*/
}
uint8_t SPI0_ExchangeData(uint8_t data)
{
 SPI0.DATA = data; //send data
 while (!(SPI0.INTFLAGS & SPI_IF_bm)); /* Waits until data are exchanged*/
 return SPI0.DATA;
}
void SPI0_WaitDataready()
{
    while (!(SPI0.INTFLAGS & SPI_RXCIF_bm));
}
uint8_t memory_Read_Status_Register ()
{
  uint8_t read;
  PORTA_OUTCLR = PIN7_bm; //SS low
  SPI0_WriteData(Read_Status_Register);
  read = SPI0_ReadData();
//  read = SPI0_ExchangeData(Read_Status_Register);
  PORTA_OUTSET = PIN7_bm; //SS high
  _delay_us(100);
  return read;
}
void write_enable_memory ()
{
    if((memory_Read_Status_Register() & 0x01)==0) //not busy
   {
   PORTA_OUTCLR = PIN7_bm; //SS low
   SPI0_WriteData(Write_Enable); //06h
   PORTA_OUTSET = PIN7_bm; //SS high   
    }
   _delay_us(100);
}

void memory_write (uint8_t address ,uint8_t data)
{
    if((memory_Read_Status_Register() & 0x01)==0) //not busy
   {
   write_enable_memory();
   
   PORTA_OUTCLR = PIN7_bm; //SS low
   SPI0_WriteData(address);
   SPI0_WriteData(data);
   PORTA_OUTSET = PIN7_bm; //SS high
   }
   _delay_us(100);
}


//page programming//  size represents the number of bytes(up to 256)
void memory_write_longDATA (uint32_t address , char block[] , uint8_t size)
{
    Write_Enable;
    PORTA_OUTCLR = PIN7_bm; //SS low
    SPI0_WriteData(write_Memory); //02h
    SPI0_WriteData((address>>16) & 0xFF); 
    // specifying the 24bit address
    SPI0_WriteData((address>>8) & 0xFF);
    SPI0_WriteData(address & 0xFF);
    
    for (uint8_t i=0 ; i<size ; i++)
    {
      SPI0_WriteData(block[i]);  
    }
     PORTA_OUTSET = PIN7_bm; //SS high
}
void memory_read_longDATA(uint32_t address,char block[] , uint8_t size) {
    volatile uint8_t x=0;
    PORTA_OUTCLR = PIN7_bm; // SS low
    SPI0_WriteData(Read_Memory); // 0x03
    SPI0_WriteData((address >> 16) & 0xFF); // specifying the 24-bit address
    SPI0_WriteData((address >> 8) & 0xFF);
    SPI0_WriteData(address & 0xFF);
    for (uint8_t i = 0; i < size; i++) {
        x = SPI0_ReadData();
        block[i] = x;
    }
    PORTA_OUTSET = PIN7_bm; // SS high
    _delay_us(100);
}


void main(void) {
    // Initialize SPI
    SPI0_Init();
    _delay_us(50);
    PORTA_DIRSET = PIN7_bm;  //SS IS OUTPUT
    PORTA_DIRSET = PIN6_bm;  //clk IS OUTPUT
    PORTA_DIRCLR = PIN5_bm; //MISO
    PORTA_DIRSET = PIN4_bm ; //MOSI
    SPI0.DATA=0x00;
    
      /// clk init
    ccp_write_io((void*) &(CLKCTRL_MCLKCTRLA) ,( CLKCTRL_CLKSEL_OSCHF_gc ));
    ccp_write_io((void*) &(CLKCTRL_MCLKCTRLB) ,( CLKCTRL_PEN_bm | CLKCTRL_PDIV_64X_gc ));
    ccp_write_io((void*) &(CLKCTRL_MCLKCTRLC) ,(CLKCTRL_CFDSRC_CLKMAIN_gc | CLKCTRL_CFDEN_bm ));
    ccp_write_io((void*) &(CLKCTRL_OSCHFCTRLA) ,(CLKCTRL_RUNSTDBY_bm | CLKCTRL_FRQSEL_1M_gc )); 
    uint8_t x;
    while (1) {
   write_enable_memory();
// reading ID of the memory   
   PORTA_OUTCLR = PIN7_bm; //SS low
   SPI0_WriteData(0x9f);
   SPI0_ReadData();
   SPI0_ReadData();
   SPI0_ReadData();
   PORTA_OUTSET = PIN7_bm; //SS high 
//.... 
   //sending and receiving character B
//  char send[20] = "B";
//  char receive[3] ;
//        uint32_t address = 0x7E0000; // Adjust the address according to your memory layout
//        memory_write_longDATA(address, send, 1);
//        _delay_us(100);
//       memory_read_longDATA(address,receive, 3); // Assuming you want to read 3 bytes
//        _delay_us(100);

  //x= memory_Read_Status_Register(); 
    }
}