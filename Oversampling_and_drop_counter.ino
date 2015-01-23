/* pH electrode and drop counter interface with Arduino
 * 
 * Copyright 2013 by An-Phong Le, Florida Southern College
 * ale@flsouthern.edu
 * 
 * This code is released under a Creative Commons Attribution -
 * NonCommercial - ShareAlike 3.0 Unported License
 *
 * The oversampling code is based on code posted at http://arduino.cc/forum/index.php?topic=109899.0
 * and http://arduino.cc/forum/index.php?PHPSESSID=6ee2c372307a000563aa3cccc4240173&/topic,91042.msg683916.html#msg683916
 * and http://www.sparkyswidgets.com/Blog/tabid/93/EntryId/18/Oversampling-to-enhance-ADC-resolution.aspx
 *
 * The debounce code for the drop counter is based on code posted at http://arduino.cc/forum/index.php/topic,2378.0.html
 */
 
#define BUFFER_SIZE 256 // For 14-bit ADC data
 
 volatile uint32_t result[BUFFER_SIZE];
 volatile uint32_t CurrentReading = 0;
 volatile int i = 0;
 volatile uint32_t sum = 0;
 
#define LEDsignalPin 13
volatile int state = HIGH;
int checkval = 0;
volatile int val = 0;
 
const int Preset_Drop_Delay_Time = 40;
volatile unsigned long dropdelaytime = 0;

 
 /*
  * ADC and Timer setup function
  * 
  * Argument 1:  uint8_t channel must be between 0 and 7 (analog pins)
  * Argument 2:  int frequency must be integer from 1 to 600 
  * 
  */
  
  void setup()
  {
    Serial.begin(115200);
    attachInterrupt(0, blink, RISING);  // digital pin 2 = interrupt 0 for drop counter
    pinMode(LEDsignalPin, OUTPUT);
    pinMode(0, INPUT);
    setupADC(0, 30);  // Recording 14-bit data from analog pin 0 at 30 Hz
  }
 
  void setupADC(uint8_t channel, int frequency)
  {
    cli();
    ADMUX = channel | _BV(REFS0);
    ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) | _BV(ADATE) | _BV(ADIE);
    ADCSRB |= _BV(ADTS2) | _BV(ADTS0);  //Compare Match B on counter 1
    TCCR1A = _BV(COM1B1);
    TCCR1B = _BV(CS11)| _BV(CS10) | _BV(WGM12);
    uint32_t clock = 250000;
    uint16_t counts = clock/(BUFFER_SIZE*frequency);
    OCR1B = counts;
  
    TIMSK1 = _BV(OCIE1B);
    ADCSRA |= _BV(ADEN) | _BV(ADSC);
    sei();
  }
  
  ISR(ADC_vect)
{
  result[i] = ADC;
  i=++i&(BUFFER_SIZE-1);
  for(int j=0;j<BUFFER_SIZE;j++)
  {
    sum+=result[j];
  }
  if(i==0)
  {
   /****DEAL WITH DATA HERE*****/
    sum = sum>>4;  // bit shift result back to 14-bit value
    CurrentReading = sum;
  }
  sum=0;
  TCNT1=0;
}
ISR(TIMER1_COMPB_vect)
{
}

void blink()
{
  if(abs(millis() - dropdelaytime) > Preset_Drop_Delay_Time)
  {
    val++;
    
    if (state == LOW){
      state = HIGH;
    } else {
      state = LOW;
    }
    dropdelaytime = millis();
    digitalWrite(LEDsignalPin, state);
  }
}

void loop()
{
  if (Serial.available()) {
    Serial.println(Serial.read());
  }
  if (checkval != val)
  {
     checkval = val;
     Serial.print(val);
     Serial.print(",");
     Serial.println(CurrentReading);
  }
}
