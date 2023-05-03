 uint8_t gclk_pin = 9;

 void gsclock_init(Print &out) {
     setup_GenericClock7();
     setup_D9_10MHz();
     setup_D2_10MHz();
     gsclock_set_frequency_MHz(3.0);
   }
  void setup_GenericClock7() {
   // Set up the generic clock (GCLK7)
   GCLK->GENCTRL[7].reg =
     // Divide clock source by divisor 1
     GCLK_GENCTRL_DIV(1) |
     // Set the duty cycle to 50/50 HIGH/LOW
     GCLK_GENCTRL_IDC |
     // Enable GCLK7
     GCLK_GENCTRL_GENEN |
     GCLK_GENCTRL_SRC_DPLL0;
   // Wait for synchronization
   while (GCLK->SYNCBUSY.bit.GENCTRL7) {}
 }
 void setup_D9_10MHz() {
  
   MCLK->APBBMASK.reg |= MCLK_APBBMASK_TC3;
   GCLK->PCHCTRL[26].reg =
     // Enable the peripheral channel
     GCLK_PCHCTRL_CHEN |
     // Connect generic clock 7
     GCLK_PCHCTRL_GEN_GCLK7;

   // Enable the peripheral multiplexer on pin D9
   PORT->Group[g_APinDescription[9].ulPort].
   PINCFG[g_APinDescription[9].ulPin].bit.PMUXEN = 1;
   PORT->Group[g_APinDescription[9].ulPort].
   PMUX[g_APinDescription[9].ulPin >> 1].reg |= PORT_PMUX_PMUXO(4);

   TC3->COUNT8.CTRLA.reg =
     // Set prescaler to 2
     // 120MHz/2 = 60MHz
     TC_CTRLA_PRESCALER_DIV2 |
     // Set the reset/reload to trigger on prescaler clock
     TC_CTRLA_PRESCSYNC_PRESC;
   TC3->COUNT8.WAVE.reg = TC_WAVE_WAVEGEN_MFRQ;
   
   TC3->COUNT8.CC[0].reg = 2;
   // Wait for synchronization
   while (TC3->COUNT8.SYNCBUSY.bit.CC1) {}

   // Enable timer TC3
   TC3->COUNT8.CTRLA.bit.ENABLE = 1;
   // Wait for synchronization
   while (TC3->COUNT8.SYNCBUSY.bit.ENABLE) {}
 }

 void set_D9_period_reg(uint8_t period_reg) {
   TC3->COUNT8.CC[0].reg = period_reg;
   while (TC3->COUNT8.SYNCBUSY.bit.CC1) {}
 }

 uint8_t get_D9_period_reg() {
   return TC3->COUNT8.CC[0].reg;
 }

 void set_D2_period_reg(uint8_t period_reg) {
   TC1->COUNT8.CC[0].reg = period_reg;
   while (TC1->COUNT8.SYNCBUSY.bit.CC1) {}
 }

 uint8_t get_D2_period_reg() {
   return TC1->COUNT8.CC[0].reg;
 }

 float gsclock_set_frequency_MHz(float frequency_MHz) {
   const float frequency_MHz_min = 0.117;
   const float frequency_MHz_max = 30.0;
   if (frequency_MHz < frequency_MHz_min) {
     frequency_MHz = frequency_MHz_min;
   }
   if (frequency_MHz > frequency_MHz_max) {
     frequency_MHz = frequency_MHz_max;
   }
   float frequency_MHz_result = -1;
   // initialise to 1MHz
   uint8_t period_reg = 29;
   float req_raw = ((60 / 2) / frequency_MHz) - 1;
   period_reg = static_cast<int>(req_raw);
   set_D9_period_reg(period_reg);
   set_D2_period_reg(period_reg);
   // calculate actual used frequency
   frequency_MHz_result = (60.0 / 2) / (period_reg + 1);
   return frequency_MHz_result;
 }

 float gsclock_get_frequency_MHz() {
   // uint8_t period_reg = get_D9_period_reg();
   uint8_t period_reg = get_D2_period_reg();
   float frequency_MHz_result = (60.0 / 2) / (period_reg + 1);
   return frequency_MHz_result;
 }

 #include <SPI.h>
 #include <slight_TLC5957.h>
 
 boolean infoled_state = 0;
 const byte infoled_pin = 13;
 uint8_t chip_count = 12;
 uint8_t pixel_count = 16*12;
 uint8_t latch = 7;
 uint8_t gsclk = 9;
 uint8_t spi_clock = SCK;
 uint8_t spi_mosi = MOSI;
 uint8_t spi_miso = MISO;

 slight_TLC5957 tlc = slight_TLC5957(pixel_count, latch, gsclk, spi_clock, spi_mosi, spi_miso);

 void setup_D2_10MHz() {
   MCLK->APBAMASK.reg |= MCLK_APBAMASK_TC1;
   GCLK->PCHCTRL[9].reg =
     // Enable the peripheral channel
     GCLK_PCHCTRL_CHEN |
     // Connect generic clock 7
     GCLK_PCHCTRL_GEN_GCLK7;

   // Enable the peripheral multiplexer on pin D2
   PORT->Group[g_APinDescription[2].ulPort].
   PINCFG[g_APinDescription[2].ulPin].bit.PMUXEN = 1;
   PORT->Group[g_APinDescription[2].ulPort].
   PMUX[g_APinDescription[2].ulPin >> 1].reg |= PORT_PMUX_PMUXO(4);

   TC1->COUNT8.CTRLA.reg =
     // Set prescaler to 2
     // 120MHz/2 = 60MHz
     TC_CTRLA_PRESCALER_DIV2 |
     // Set the reset/reload to trigger on prescaler clock
     TC_CTRLA_PRESCSYNC_PRESC;
   TC1->COUNT8.WAVE.reg = TC_WAVE_WAVEGEN_MFRQ;
   TC1->COUNT8.CC[0].reg = 2;
   // Wait for synchronization
   while (TC1->COUNT8.SYNCBUSY.bit.CC1) {}

   // Enable timer TC1
   TC1->COUNT8.CTRLA.bit.ENABLE = 1;
   // Wait for synchronization
   while (TC1->COUNT8.SYNCBUSY.bit.ENABLE) {}
 }
 
 uint8_t step = 0;
 uint16_t value_low = 0;
 uint16_t value_high = 65535;

 void tlc_init(Print &out) {
   out.println(F("setup tlc:")); {
     setup_D9_10MHz();
     tlc.begin();
   }
   out.println(F("\t finished."));
 }

 void bills_test_pattern_1() {
   int cells = 16;
     if (step == 0) {
       for (int i = 0 ; i < cells ; i++) {
         tlc.set_pixel_16bit_value(2 * i, value_low, value_low, value_low);
         tlc.set_pixel_16bit_value(2 * i + 1, value_high, value_low, value_low); // red
       }
       tlc.show();
       step += 1;
     } else if (step == 1) {
       for (int i = 0 ; i < cells ; i++) {
         tlc.set_pixel_16bit_value(2 * i, value_low, value_low, value_low);
         tlc.set_pixel_16bit_value(2 * i + 1, value_low, value_high, value_low); // green 1st half
       }
       tlc.show();
       step += 1;
     } else if (step == 2) {
       for (int i = 0 ; i < cells ; i++) {
         tlc.set_pixel_16bit_value(2 * i, value_low, value_low, value_high); // green 2nd half
         tlc.set_pixel_16bit_value(2 * i + 1, value_low, value_low, value_low);
       }
       tlc.show();
       step += 1;
     } else if (step == 3) {
       for (int i = 0 ; i < cells ; i++) {
         tlc.set_pixel_16bit_value(2 * i, value_low, value_low, value_high); // green full
         tlc.set_pixel_16bit_value(2 * i + 1, value_low, value_high, value_low);
       }
       tlc.show();
       step += 1;
     } else if (step == 4) {
       for (int i = 0 ; i < cells ; i++) {
         tlc.set_pixel_16bit_value(2 * i, value_low, value_low, value_low);
         tlc.set_pixel_16bit_value(2 * i + 1, value_low, value_low, value_high); //blue
       }
       tlc.show();
       step += 1;
     } else if (step == 5) {
       for (int i = 0 ; i < cells ; i++) {
         tlc.set_pixel_16bit_value(2 * i, value_high, value_low, value_low); //UV 1st half
         tlc.set_pixel_16bit_value(2 * i + 1, value_low, value_low, value_low); 
       }
       tlc.show();
       step += 1;
     } else if (step == 6) {
       for (int i = 0 ; i < cells ; i++) {
         tlc.set_pixel_16bit_value(2 * i, value_low, value_high, value_low); //UV 2nd half
         tlc.set_pixel_16bit_value(2 * i + 1, value_low, value_low, value_low);
       }
       tlc.show();
       step += 1;
     } else if (step == 7) {
       for (int i = 0 ; i < cells ; i++) {
         tlc.set_pixel_16bit_value(2 * i, value_high, value_high, value_low); //UV full
         tlc.set_pixel_16bit_value(2 * i + 1, value_low, value_low, value_low);
       }
       tlc.show();
       //step += 1;
       step = 0;
     } else {
       step = 0;
     }
     //step += 1;
     tlc.show();
 }
  void staticRed() { //If button is pressed in GUI
         int cells = 8; // discovery 4/28/23 (see excel sheet in teams) so for red we want 12-23, 36-47 etc. 
       for (int i = 0 ; i < cells ; i++) {
         tlc.set_pixel_16bit_value(12 + 24 * i, 65000, 0, 0); // R G B Red full
         tlc.set_pixel_16bit_value(13 + 24* i, 65000, 0, 0); 
         tlc.set_pixel_16bit_value(14 + 24* i, 65000, 0, 0); 
         tlc.set_pixel_16bit_value(15 + 24 * i, 65000, 0, 0); 
         tlc.set_pixel_16bit_value(16 + 24* i, 65000, 0, 0); 
         tlc.set_pixel_16bit_value(17 + 24 * i, 65000, 0, 0); 
         tlc.set_pixel_16bit_value(18 + 24* i, 65000, 0, 0); 
         tlc.set_pixel_16bit_value(19 + 24 * i, 65000, 0, 0); 
         tlc.set_pixel_16bit_value(20 + 24* i, 65000, 0, 0); 
         tlc.set_pixel_16bit_value(21 + 24 * i, 65000, 0, 0); 
         tlc.set_pixel_16bit_value(22 + 24 * i, 65000, 0, 0); 
         tlc.set_pixel_16bit_value(23 + 24 * i, 65000, 0, 0); 
         }
       tlc.show();
 }
  void staticBlue() {
         int cells = 8; // discovery 4/28/23 so for green we want 1-11, 24-35, 48-59, etc. 
       for (int i = 0 ; i < cells ; i++) {
         tlc.set_pixel_16bit_value(12 + 24 * i, 0, 0, 65000); // R G B Blue full
         tlc.set_pixel_16bit_value(13 + 24* i, 0, 0, 65000); 
         tlc.set_pixel_16bit_value(14 + 24* i, 0, 0, 65000); 
         tlc.set_pixel_16bit_value(15 + 24 * i, 0, 0, 65000); 
         tlc.set_pixel_16bit_value(16 + 24* i, 0, 0, 65000); 
         tlc.set_pixel_16bit_value(17 + 24 * i, 0, 0, 65000); 
         tlc.set_pixel_16bit_value(18 + 24* i, 0, 0, 65000); 
         tlc.set_pixel_16bit_value(19 + 24 * i, 0, 0, 65000); 
         tlc.set_pixel_16bit_value(20 + 24* i, 0, 0, 65000); 
         tlc.set_pixel_16bit_value(21 + 24 * i, 0, 0, 65000); 
         tlc.set_pixel_16bit_value(22 + 24 * i, 0, 0, 65000); 
         tlc.set_pixel_16bit_value(23 + 24 * i, 0, 0, 65000); 
         }
       tlc.show();
  }
  void staticGreen() {
     int cells = 8; // discovery 4/28/23 so for green we want 1-11, 24-35, 48-59, etc. 
       for (int i = 0 ; i < cells ; i++) {
         tlc.set_pixel_16bit_value(0 + 24 * i, 0, 0, 65000); // green full
         tlc.set_pixel_16bit_value(1 + 24* i, 0, 0, 65000); // green full
         tlc.set_pixel_16bit_value(2 + 24* i, 0, 0, 65000); // green full
         tlc.set_pixel_16bit_value(3 + 24 * i, 0, 0, 65000); // green full
         tlc.set_pixel_16bit_value(4 + 24* i, 0, 0, 65000); // green full
         tlc.set_pixel_16bit_value(5 + 24 * i, 0, 0, 65000); // green full
         tlc.set_pixel_16bit_value(6 + 24* i, 0, 0, 65000); // green full
         tlc.set_pixel_16bit_value(7 + 24 * i, 0, 0, 65000); // green full
         tlc.set_pixel_16bit_value(8 + 24* i, 0, 0, 65000); // green full
         tlc.set_pixel_16bit_value(9 + 24 * i, 0, 0, 65000); // green full
         tlc.set_pixel_16bit_value(10 + 24 * i, 0, 0, 65000); // green full
         tlc.set_pixel_16bit_value(11 + 24 * i, 0, 0, 65000); // green full
         }
       tlc.show();
 }
   void uv() {
     int cells = 8; // discovery 4/28/23 so for UV we also want 1-11, 24-35, 48-59, etc. 
       for (int i = 0 ; i < cells ; i++) {
         tlc.set_pixel_16bit_value(0 + 24 * i, 65000, 65000, 0); // UV full
         tlc.set_pixel_16bit_value(1 + 24* i, 65000, 65000, 0); 
         tlc.set_pixel_16bit_value(2 + 24* i, 65000, 65000, 0); 
         tlc.set_pixel_16bit_value(3 + 24 * i, 65000, 65000, 0); 
         tlc.set_pixel_16bit_value(4 + 24* i, 65000, 65000, 0); 
         tlc.set_pixel_16bit_value(5 + 24 * i, 65000, 65000, 0); 
         tlc.set_pixel_16bit_value(6 + 24* i, 65000, 65000, 0); 
         tlc.set_pixel_16bit_value(7 + 24 * i, 65000, 65000, 0); 
         tlc.set_pixel_16bit_value(8 + 24* i, 65000, 65000, 0); 
         tlc.set_pixel_16bit_value(9 + 24 * i, 65000, 65000, 0); 
         tlc.set_pixel_16bit_value(10 + 24 * i, 65000, 65000, 0); 
         tlc.set_pixel_16bit_value(11 + 24 * i, 65000, 65000, 0); 
         }
       tlc.show();
   }
 void expo(){
     int cells = 4, l=12;  
       for (int i = 0 ; i < cells ; i++) {
        for (l; l<(24+i*24); l++){ //This goes from 12-23 the first time, 36-47, 60-71,84-95 
         tlc.set_pixel_16bit_value(l , 65000, 65000, 65000); // R G B Blue full    + 24 * i
         tlc.show();
         //delay(50);
        }
        l=l+12;
       }      
 }
 void Smile(){
         tlc.set_pixel_16bit_value(86, 10000, 10000, 0); // R G B Blue full     well #65 
         tlc.set_pixel_16bit_value(110, 10000, 10000, 0); // R G B Blue full    well #67  
         tlc.set_pixel_16bit_value(86, 10000, 10000, 0); // R G B Blue full     well #65 
         tlc.set_pixel_16bit_value(110, 10000, 10000, 0); // R G B Blue full    well #67    
         tlc.show();
 }
 //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // main
 //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 void setup() {
   pinMode(infoled_pin, OUTPUT);
   digitalWrite(infoled_pin, HIGH);
   #if defined (__AVR_ATmega32U4__)
     // wait for arduino IDE to release all serial ports after upload.
     delay(1000);
   #endif
   Serial.begin(115200);
   // for ATmega32U4 devices:
   #if defined (__AVR_ATmega32U4__)
     // Wait for Serial Connection to be Opend from Host or
     // timeout after 6second
     uint32_t timeStamp_Start = millis();
     while ( (! Serial) && ( (millis() - timeStamp_Start) < 2000 ) ) {
       // nothing to do
     }
   #endif
   Serial.println();
   gsclock_init(Serial);
   tlc_init(Serial);
   delay(1000);
 }

 void loop() {
   //expo();
   //staticBlue();
   //uv();
    if (Serial.available()) {
      char val = Serial.read();
      //int duration=Serial.read();
      
      if (val == 'R') {        //from the processing code if a button is pressed. in this case it is Red_On
            //while timeentered>0
            tlc.set_pixel_all_16bit_value(value_low, value_low, value_low);
            staticRed();
                //timeentered--;
      }
      if (val == 'U') {
          tlc.set_pixel_all_16bit_value(value_low, value_low, value_low);
          uv();
      }
      if (val == 'G') {
        tlc.set_pixel_all_16bit_value(value_low, value_low, value_low);
        staticGreen();
      }

      if (val == 'B') {
        tlc.set_pixel_all_16bit_value(value_low, value_low, value_low);
        staticBlue();
      }
      if (val == 't') {
        for (int i=0; i<10; i++){
          bills_test_pattern_1();
          delay(400);
        }
      }
      if (val == 'P') {
        tlc.set_pixel_all_16bit_value(value_low, value_low, value_low);
        expo();
      }
      if (val == 'S') {
        tlc.set_pixel_all_16bit_value(value_low, value_low, value_low);
        Smile();
      }
      if (val == 'o') {
        tlc.set_pixel_all_16bit_value(value_low, value_low, value_low);
        tlc.show();
      }
    }
    delay(2000);
 }
