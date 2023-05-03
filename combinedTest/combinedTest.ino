uint8_t gclk_pin = 9;


void gsclock_init(Print &out) {
    out.println(F("init gsclock:")); {
    out.println(F("  init gsclock timer."));
    setup_GenericClock7();
    setup_D9_10MHz();
    setup_D2_10MHz();
    out.println(F("  set gsclock to 3MHz."));
    gsclock_set_frequency_MHz(3.0);
    // out.println(F("  set gsclock to 30MHz."));
    // gsclock_set_frequency_MHz(30.0);
  }
  out.println(F("  finished."));
}

// Set-up the CC (counter compare), channel 0 register
// this sets the period
// 750 / (2 * (CC0 + 1))  = outputfrequence
// (750 / 2) / (CC0 + 1)  = outputfrequence
// (750 / 2) / (2 + 1)  = 125
// 750kHz / (2 * (255 + 1))  = 1,4648kHz
// tests
//       750.00kHz =   1,33us
//   0 = 375.00kHz =   2.67us
//   1 = 187.50kHz =   5.35us
//   2 = 125.00kHz =   8.02us
//   3 =  93.75kHz =  10.67us
//   4 =  74.90kHz =  13.40us
//   5 =  61.80kHz =  16.20us
//  10 =  33.60kHz =  29.80us
//  64 =   5.74kHz = 174.00us
// 128 =   2.89kHz = 346.00us
// 255 =   1.46kHz = 687.00us
//
// (clockfreq / 2) / (CC0 + 1)  = outfreq  | * (CC0 + 1)
// (clockfreq / 2) = outfreq * (CC0 + 1)   | / outfreq
// (clockfreq / 2) / outfreq  = CC0 + 1    | -1
// ((clockfreq / 2) / outfreq) -1  = CC0
// (750 / 2) / 93.75  = CC0 + 1
// ((750 / 2) / 93.75) - 1  = CC0
// --------------------------
// ((60 / 2) / 2) -1  = CC0
// (60MHz / 2) / (0 + 1)  = 30MHz
// (60MHz / 2) / (255 + 1)  = 0,117MHz = 117kHz
//
//       60.0MHz
//   0 = 30.0MHz
//   1 = 15.0MHz
//   2 = 10.0MHz
//   3 =  7.5MHz
//   4 =  6.0MHz
//   5 =  5.0MHz
//   9 =  3.0MHz
//  10 =  2.7MHz
//  14 =  2.0MHz
//  29 =  1.0MHz
//  59 =  0.5MHz = 500kHz
//  74 =  0.4MHz
//  99 =  0.3MHz
// 149 =  0.2MHz
// 255 =  0.11MHz


void setup_GenericClock7() {
  // Set up the generic clock (GCLK7)
  GCLK->GENCTRL[7].reg =
    // Divide clock source by divisor 1
    GCLK_GENCTRL_DIV(1) |
    // Set the duty cycle to 50/50 HIGH/LOW
    GCLK_GENCTRL_IDC |
    // Enable GCLK7
    GCLK_GENCTRL_GENEN |
    // // Select 48MHz DFLL clock source
    // GCLK_GENCTRL_SRC_DFLL;
    // // Select 100MHz DPLL clock source
    // GCLK_GENCTRL_SRC_DPLL1;
    // Select 120MHz DPLL clock source
    GCLK_GENCTRL_SRC_DPLL0;
  // Wait for synchronization
  while (GCLK->SYNCBUSY.bit.GENCTRL7) {}
}

void setup_D9_10MHz() {
  // Activate timer TC3
  // check for correct mask at MCLK – Main Clock - 15.7 Register Summary
  // page178f
  // http://ww1.microchip.com/downloads/en/DeviceDoc/60001507C.pdf#page=178&zoom=page-width,-8,539
  // CLK_TC3_APB
  MCLK->APBBMASK.reg |= MCLK_APBBMASK_TC3;

  // for PCHCTRL numbers have a look at
  // Table 14-9. PCHCTRLm Mapping page168ff
  // http://ww1.microchip.com/downloads/en/DeviceDoc/60001507C.pdf#page=169&zoom=page-width,-8,696
  // GCLK->PCHCTRL[GCLK_TC3].reg =
  GCLK->PCHCTRL[26].reg =
    // Enable the peripheral channel
    GCLK_PCHCTRL_CHEN |
    // Connect generic clock 7
    GCLK_PCHCTRL_GEN_GCLK7;

  // Enable the peripheral multiplexer on pin D9
  PORT->Group[g_APinDescription[9].ulPort].
  PINCFG[g_APinDescription[9].ulPin].bit.PMUXEN = 1;

  // Set the D9 (PORT_PA19) peripheral multiplexer to
  // peripheral (odd port number) E(6): TC3, Channel 1
  // check if you need even or odd PMUX!!!
  // http://forum.arduino.cc/index.php?topic=589655.msg4064311#msg4064311
  PORT->Group[g_APinDescription[9].ulPort].
  PMUX[g_APinDescription[9].ulPin >> 1].reg |= PORT_PMUX_PMUXO(4);

  TC3->COUNT8.CTRLA.reg =
    // Set prescaler to 2
    // 120MHz/2 = 60MHz
    TC_CTRLA_PRESCALER_DIV2 |
    // Set the reset/reload to trigger on prescaler clock
    TC_CTRLA_PRESCSYNC_PRESC;

  // Set-up TC3 timer for
  // Match Frequency Generation (MFRQ)
  // the period time (T) is controlled by the CC0 register.
  // (instead of PER or MAX)
  // WO[0] toggles on each Update condition.
  TC3->COUNT8.WAVE.reg = TC_WAVE_WAVEGEN_MFRQ;
  // Wait for synchronization
  // while (TC3->COUNT8.SYNCBUSY.bit.WAVE)

  // Set-up the CC (counter compare), channel 0 register
  // this sets the period
  //
  // (clockfreq / 2) / (CC0 + 1)  = outfreq  | * (CC0 + 1)
  // (clockfreq / 2) = outfreq * (CC0 + 1)   | / outfreq
  // (clockfreq / 2) / outfreq  = CC0 + 1    | -1
  // ((clockfreq / 2) / outfreq) -1  = CC0
  //
  // ((60 / 2) / 2) -1  = CC0
  //
  // MAX: (60MHz / 2) / (0 + 1)  = 30MHz
  // MIN: (60MHz / 2) / (255 + 1)  = 0,117MHz = 117kHz
  //
  //       60.0MHz
  //   0 = 30.0MHz
  //   1 = 15.0MHz
  //   2 = 10.0MHz
  //   3 =  7.5MHz
  //   4 =  6.0MHz
  //   5 =  5.0MHz
  //   9 =  3.0MHz
  //  14 =  2.0MHz
  //  29 =  1.0MHz
  //  59 =  0.5MHz
  //  74 =  0.4MHz
  //  99 =  0.3MHz
  // 149 =  0.2MHz
  // 255 =  0.11MHz
  // start with 10MHz
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// THE END
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/******************************************************************************

    TLC5957_minimal.ino
        minimal usage for slight_TLC5957 library.
        this sketch just lights up all pixels after each other.
        debugout on usbserial interface: 115200baud

    hardware:
        Board:
            ItsyBitsy M4 (or compatible Port-Pin-Mapping)
            LED on pin 13
            TLC5957

    libraries used:
        ~ slight_TLC5957
            written by stefan krueger (s-light),
                git@s-light.eu, http://s-light.eu, https://github.com/s-light/
            license: MIT

    written by stefan krueger (s-light),
        git@s-light.eu, http://s-light.eu, https://github.com/s-light/

******************************************************************************/
#include <SPI.h>
#include <slight_TLC5957.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Debug Output

boolean infoled_state = 0;
const byte infoled_pin = 13;

// TLC5957

// possible options and there defaults:
// slight_TLC5957(
//     uint8_t chip_count = 16,
//     uint8_t latch = A5,
//     uint8_t gsclk = 2,
//     uint8_t spi_clock = SCK,
//     uint8_t spi_mosi = MOSI,
//     uint8_t spi_miso = MISO
// );
//uint8_t pixel_count = 2*16;
// use default pins
//slight_TLC5957 tlc = slight_TLC5957(pixel_count);

uint8_t chip_count = 1;
uint8_t pixel_count = 40;
uint8_t latch = 7;
uint8_t gsclk = 9;
uint8_t spi_clock = SCK;
uint8_t spi_mosi = MOSI;
uint8_t spi_miso = MISO;

slight_TLC5957 tlc = slight_TLC5957(pixel_count, latch, gsclk, spi_clock, spi_mosi, spi_miso);



void setup_D2_10MHz() {
  // Activate timer TC1
  // check for correct mask at MCLK – Main Clock - 15.7 Register Summary
  // page178f
  // http://ww1.microchip.com/downloads/en/DeviceDoc/60001507C.pdf#page=178&zoom=page-width,-8,539
  // CLK_TC1_APB
  MCLK->APBAMASK.reg |= MCLK_APBAMASK_TC1;

  // for PCHCTRL numbers have a look at
  // Table 14-9. PCHCTRLm Mapping page168ff
  // http://ww1.microchip.com/downloads/en/DeviceDoc/60001507C.pdf#page=169&zoom=page-width,-8,696
  // GCLK->PCHCTRL[GCLK_TC1].reg =
  GCLK->PCHCTRL[9].reg =
    // Enable the peripheral channel
    GCLK_PCHCTRL_CHEN |
    // Connect generic clock 7
    GCLK_PCHCTRL_GEN_GCLK7;

  // Enable the peripheral multiplexer on pin D2
  PORT->Group[g_APinDescription[2].ulPort].
  PINCFG[g_APinDescription[2].ulPin].bit.PMUXEN = 1;

  // Set the D2 (PORT_PA07) peripheral multiplexer to
  // peripheral (odd port number) E(6): TC1, Channel 1
  // check if you need even or odd PMUX!!!
  // http://forum.arduino.cc/index.php?topic=589655.msg4064311#msg4064311
  PORT->Group[g_APinDescription[2].ulPort].
  PMUX[g_APinDescription[2].ulPin >> 1].reg |= PORT_PMUX_PMUXO(4);

  TC1->COUNT8.CTRLA.reg =
    // Set prescaler to 2
    // 120MHz/2 = 60MHz
    TC_CTRLA_PRESCALER_DIV2 |
    // Set the reset/reload to trigger on prescaler clock
    TC_CTRLA_PRESCSYNC_PRESC;

  // Set-up TC1 timer for
  // Match Frequency Generation (MFRQ)
  // the period time (T) is controlled by the CC0 register.
  // (instead of PER or MAX)
  // WO[0] toggles on each Update condition.
  TC1->COUNT8.WAVE.reg = TC_WAVE_WAVEGEN_MFRQ;
  // Wait for synchronization
  // while (TC1->COUNT8.SYNCBUSY.bit.WAVE)

  // Set-up the CC (counter compare), channel 0 register
  // this sets the period
  //
  // (clockfreq / 2) / (CC0 + 1)  = outfreq  | * (CC0 + 1)
  // (clockfreq / 2) = outfreq * (CC0 + 1)   | / outfreq
  // (clockfreq / 2) / outfreq  = CC0 + 1    | -1
  // ((clockfreq / 2) / outfreq) -1  = CC0
  //
  // ((60 / 2) / 2) -1  = CC0
  //
  // MAX: (60MHz / 2) / (0 + 1)  = 30MHz
  // MIN: (60MHz / 2) / (255 + 1)  = 0,117MHz = 117kHz
  //
  //       60.0MHz
  //   0 = 30.0MHz
  //   1 = 15.0MHz
  //   2 = 10.0MHz
  //   3 =  7.5MHz
  //   4 =  6.0MHz
  //   5 =  5.0MHz
  //   9 =  3.0MHz
  //  14 =  2.0MHz
  //  29 =  1.0MHz
  //  59 =  0.5MHz
  //  74 =  0.4MHz
  //  99 =  0.3MHz
  // 149 =  0.2MHz
  // 255 =  0.11MHz
  // start with 10MHz
  TC1->COUNT8.CC[0].reg = 2;
  // Wait for synchronization
  while (TC1->COUNT8.SYNCBUSY.bit.CC1) {}

  // Enable timer TC1
  TC1->COUNT8.CTRLA.bit.ENABLE = 1;
  // Wait for synchronization
  while (TC1->COUNT8.SYNCBUSY.bit.ENABLE) {}
}


unsigned long animation_timestamp = 0;
const uint16_t animation_interval = 1000; //ms

uint8_t step = 0;

uint16_t value_low = 0;
uint16_t value_high = 65535;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// TLC5957

void tlc_init(Print &out) {
  out.println(F("setup tlc:")); 
      { setup_D9_10MHz();
        tlc.begin();
      }
  out.println(F("\t finished."));
    }

void pixel_check() {
  if (step >= tlc.pixel_count) {
    step = 0;
    Serial.println("step wrap around.");
    //tlc.set_pixel_all_16bit_value(value_low, value_low, value_low);
  }
  //tlc.set_pixel_16bit_value(step, value_low, value_low, value_high);
  tlc.set_pixel_16bit_value(step, value_high, value_high, value_high);
  step += 1;
  Serial.print("step:");
  Serial.println(step);
  tlc.show();
}

void channel_check() {
  if (step >= tlc.channel_count) {
    step = 0;
    Serial.println("step wrap around.");
    tlc.set_pixel_all_16bit_value(value_low, value_low, value_low);
  }
  tlc.set_pixel_all_16bit_value(value_low, value_low, value_low);
  tlc.set_channel(step, value_high);
  step += 1;
  Serial.print("step:");
  Serial.println(step);
  tlc.show();
}

void staticColor() {
    int cells = 96; //6 LEDS in X Cells (2-UV + 1-Green + 3-RGB)
      for (int i = 0 ; i < cells ; i++) {
        tlc.set_pixel_16bit_value(4 * i, value_low, value_low, value_high); // green full
        tlc.set_pixel_16bit_value(4 * i + 1, value_low, value_low, value_high); // green full
        tlc.set_pixel_16bit_value(4 * i + 2, value_low, value_low, value_high); // green full
        tlc.set_pixel_16bit_value(4 * i + 3, value_low, value_low, value_high); // green full
      }
      tlc.show();
}

//function where stuff happens

void update_animation() {
  if ((millis() - animation_timestamp) > animation_interval) {
    animation_timestamp = millis();
//    tlc.set_pixel_all_16bit_value(value_high, value_high, value_high);
//    tlc.set_channel(1, value_high);
//    tlc.set_pixel_all_16bit_value(value_high, value_high, value_high);
//    tlc.set_pixel_16bit_value(8, value_high, value_high, value_high); 
//    tlc.show();
    staticColor();
    
  }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// main
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LC709203F.h>

Adafruit_LC709203F lc;

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR   0x3C 
#define OLED_RESET -1 
Adafruit_SSD1306 display(OLED_WIDTH,OLED_HEIGHT,&Wire,OLED_RESET);

void setup() {
  pinMode(infoled_pin, OUTPUT);
  digitalWrite(infoled_pin, HIGH);
  lc.begin();
  lc.setThermistorB(3950);
  lc.setPackSize(LC709203F_APA_500MAH);
  lc.setAlarmVoltage(3.8);
  
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.display();
  delay(1000); // Pause for 2 second
  display.clearDisplay(); display.setTextColor(WHITE); display.setCursor(0,0); display.setTextSize(1);
  display.println("SD405-Sp22-09");
  display.setTextSize(2);
  display.println("LPA-Light"); display.println("Plate"); display.println("Apparatus");

  display.display();

  delay(2500);
  
  Serial.println();
  gsclock_init(Serial);
  tlc_init(Serial);
  Serial.println(F("wait 1s."));
  delay(1000);
  Serial.println(F("Loop:"));
}

void loop() {
  update_animation();
  
  display.clearDisplay(); display.setTextSize(2); display.setTextColor(WHITE); display.setCursor(0,0);
  display.print(lc.cellVoltage()); //data dump line
  display.clearDisplay(); display.setCursor(0,0);
  display.println("Power: "); 
  display.setTextSize(1); display.setCursor(0, 17); delay(250);
  display.println("Voltage: "); display.print(lc.cellVoltage()) ; display.println(" V");
  display.println("Charge Percent: "); display.print(lc.cellPercent()); display.println(" %");
  display.println("Temperature: "); display.print(lc.getCellTemperature()); display.println(" C");
  display.display();
  delay(2000);
}
