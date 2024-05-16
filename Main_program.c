#include <16F877A.h>
#device ADC=16
#FUSES NOWDT                    // No Watch Dog Timer
#FUSES NOBROWNOUT               // No brownout reset
#FUSES NOLVP                    // No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O
#use delay(crystal=20000000)
#define LCD_RS_PIN      PIN_D1
#define LCD_RW_PIN      PIN_D2
#define LCD_ENABLE_PIN  PIN_D3
#define LCD_DATA4       PIN_D4
#define LCD_DATA5       PIN_D5
#define LCD_DATA6       PIN_D6
#define LCD_DATA7       PIN_D7
#include <lcd.c>
#include <stdlib.h>

int countchar=0;
char UART_Buffer;
char str[25];
int pos_d=0,pos_e=0;
char ax[6],ay[6],pwmval[4],angleval[4];
int16 angleint;
int32 ovrflow=0;
int16 overflowcount;
int allowovf;

#byte TRISC=0x87  
#byte TRISD=0x88

#byte PORTC=0x07
#bit C0=PORTC.0
#bit C1=PORTC.1

#byte TXREG=0x19

#byte TXSTA=0x98
#bit TRMT=TXSTA.1 
#bit BRGH=TXSTA.2 
#bit SYNC=TXSTA.4 
#bit TXEN=TXSTA.5 

#byte RCSTA=0x18  
#bit CREN=RCSTA.4 
#bit SPEN=RCSTA.7

#byte SPBRG=0x99 
#byte RCREG=0x1A

#byte PIE1=0x8C
#bit RCIE=PIE1.5

#byte INTCON=0x0B
#bit PEIE=INTCON.6
#bit GIE=INTCON.7

#byte INTCON=0x0B

#byte T1CON=0x10
#bit TMR1ON=T1CON.0

#byte TMR1L=0x0E
#byte TMR1H=0x0F

#byte T2CON=0x12
#byte TMR2=0x11
#byte PR2=0x92
#byte PIR1=0x0C
#byte CCPR1L=0x15
#byte CCPR1H=0x16
#byte CCP1CON=0x17

#byte TMRO=0x01
#byte OPREG=0x81

#int_timer0
void ngatt0()
{
   if(allowovf==1)
   {
      overflowcount+=1; // Counting overflow time (if allow)
   }
   TMRO=0;
}

#int_timer1
void ngatt1()
{
   ovrflow+=1;
   TMR1L=0;
   TMR1H=0;
}

#int_rda
void uart_rcv()
{
   UART_buffer=RCREG;  
   if(UART_buffer=='d') pos_d=countchar; // Get position of 'd' and 'e' character
   else if(UART_buffer=='e') pos_e=countchar;
   if(UART_buffer>=32) // Sort for character with decimal value >=32
   {
      str[countchar]=UART_buffer;
      countchar+=1;  
   }   
   if(pos_e>0) // Extract coordinates, pwm value and desired speed string
   {
      for(int i=1;i<6;i++) ax[i-1]=str[i]; 
      for(i=1;i<6;i++) ay[i-1]=str[i+6]; 
      for(i=13;i<pos_d;i++) pwmval[i-13]=str[i]; 
      for(i=pos_d+1;i<pos_e;i++) angleval[i-pos_d-1]=str[i]; 
   }  
}

int16 round(float number) 
{
    return (int16)(number+0.5); // Round number to the closest integer
}


void uart_send(char data) 
{
   while(!TRMT);
   TXREG=data;
}

void uart_init()
{
   BRGH=1; // High speed, asynchronous mode
   SPBRG=129; // Baudrate 9600 bps
   SYNC=0;
   SPEN=1;  
   RCIE=1;   
   PEIE=1; 
   GIE=1;    
   CREN=1; // Enable data reception
   TXEN=1; // Enable UART transmission 
}

void pwm_init()
{
   PR2=0xff;                            
   CCP1CON=0b00001100; // Set up CCP1 as PWM mode
   T2CON=0b00000111; // Set up timer 2 with prescaler is 16, 1:1 postcale    
   TMR2=0; 
}

void timer1_init()
{
   INTCON=0b11100000; // Enable global, peripheral and TMR0 overflow interrupts
   PIE1=0b00000001;
   T1CON=0;
}

void timer0_init()
{
   OPREG=0b00000110; // Prescaler 1:128
}

void main()
{   
   TRISC=0b11000001; 
   TRISD=0x00;
   lcd_init(); // Initialize 
   lcd_putc('\f');
   timer0_init();
   timer1_init();
   uart_init();
   pwm_init();
   
   int32 time_count,sec_count;
   float encoder_read;
   int done1st=0,sent=0;
   
   float kp=0,ki=0.2,kd=0; // Define PID parameters
   double setpoint=0,volt=0,in_speed=0,tsamp=0.02,tcal; //Sampling time value for the first iteration only!
   double integral=0,last_error=0,error=0,derivative=0;  
   int16 pwmvalue;
   CCPR1L=0;
   C1=0;
   lcd_gotoxy(1,1);
   printf(lcd_putc,"Desired:"); 
   lcd_gotoxy(1,2);
   printf(lcd_putc,"Feedback:"); 
   while(TRUE)
   {
      if(angleint==0) angleint=atol(angleval);
      else // Desired speed received!
      {                                          
         if(done1st!=0) tsamp=tcal; // Sampling time for next iterations
         allowovf=1; 
         TMRO=0; // Start counting sampling time
         overflowcount=0;
         setpoint=(float)angleint;
         error=setpoint-in_speed; // PID calculation
         derivative=(error-last_error)/tsamp;        
         integral+=error*tsamp;          
         last_error=error;        
         volt=kp*error+ki*integral+kd*derivative; 
         if (volt<0) C1=1; // If the voltage<0, the motor direction is changed
         else C1=0;
         if (volt>12) volt=12; // Create upper and lower limit
         if (volt<-12) volt=-12;
         pwmvalue = round(255*abs(volt)/12.0); //Scale to PWM value (0->255)
         if(pwmvalue>255) pwmvalue=255;                
         CCPR1L=(unsigned int)pwmvalue;
         TMR1L=0;
         TMR1H=0; // Calulate pulse frequency from the encoder input
         while(C0==1);
         while(C0==0);
         TMR1ON=1;  
         while(C0==1);    
         while(C0==0);
         TMR1ON=0; 
         time_count=make16(TMR1H,TMR1L)+ ovrflow*65536;
         encoder_read=(5000000.0/time_count)/24*60; // Pulse per Rev=24     
         in_speed=round(encoder_read); // Speed from encoder=Freq/Pulse per Rev*60(s)    
         done1st=1; // Done 1st iteration
         ovrflow=0;            
         allowovf=0;
         sec_count=TMRO+256*overflowcount; // Stop counting sampling time    
         tcal=sec_count*128*1.0/5000000; // Change to second           
         lcd_gotoxy(9,1);
         printf(lcd_putc,"%ld     ",angleint);
         lcd_gotoxy(10,2);
         printf(lcd_putc,"%ld     ",round(encoder_read)); 
         if(sent==0) // Send the received value to the terminal 1 time only!
         {            
            for(int i=0;i<5;i++) uart_send(ax[i]);
            uart_send('\n');
            uart_send('\r');
            for(i=0;i<5;i++) uart_send(ay[i]);
            uart_send('\n');
            uart_send('\r');
            for(i=0;i<pos_d-13;i++) uart_send(pwmval[i]);
            uart_send('\n');
            uart_send('\r');
            for(i=0;i<pos_e-pos_d-1;i++) uart_send(angleval[i]);
            uart_send('\n');
            uart_send('\r');
            sent=1;
         }
         delay_ms(10);
      }
   }
}

