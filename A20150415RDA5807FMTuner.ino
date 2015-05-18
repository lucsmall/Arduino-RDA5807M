// Very simple test code for the RDA5807M (RRD-102 module) FM radio receiver IC
// Luc Small
// lucsmall.com
// 20150518

// Find this code at: https://github.com/lucsmall/Arduino-RDA5807M

// See these videos for more:
// "Talking about my Arduino sketch for the RDA5807M FM Radio IC"
// http://youtu.be/9TegkAu96nE

// "Connecting the RDA5807M FM Radio IC to an Arduino"
// https://youtu.be/2j1hX4uzkQ8

// "Making my Arduino controlled RDA5807M FM Radio"
// https://youtu.be/4pZmkeqg5h8

#include <Wire.h>

// Select the frequency we want to tune to by way
// of selecting the channel for the desired frequency
uint16_t channel = 209;
   // assuming band starts at 87.0MHz (per settings below)
   // and channel spacing of 100kHz (0.1MHz) (per settings below)
   // then channel can be derived as follows:
   //
   // channel = (<desired freq in MHz> - 87.0) / 0.1 
   //
   // which is the same as:
   //
   // <10 x desired freq in MHz> - 870
   //
   // some examples:
   //
   // channel 145 dec = 101.5 MHz
   // channel 153 dec = 102.3 MHz
   // channel 177 dec = 104.7 MHz
   // channel 193 dec = 106.3 MHz
   // channel 209 dec = 107.9 MHz


// address of the RDA5807 on two wire bus
#define RDA5807M_ADDRESS  0b0010000 // 0x10

#define BOOT_CONFIG_LEN 12
#define TUNE_CONFIG_LEN 4

// These bytes set our initial configuration
// We don't bother to tune to a channel at this stage.
// But instead initiate a reset.
uint8_t boot_config[] = {
  /* register 0x02 */
  0b11000000,
    // DHIZ audio output high-z disable
    // 1 = normal operation
    
    // DMUTE mute disable 
    // 1 = normal operation
    
    // MONO mono select
    // 0 = stereo
    
    // BASS bass boost
    // 0 = disabled
    
    // RCLK NON-CALIBRATE MODE 
    // 0 = RCLK is always supplied
    
    // RCLK DIRECT INPUT MODE 
    // 0 = ??? not certain what this does
    
    // SEEKUP
    // 0 = seek in down direction
    
    // SEEK
    // 0 = disable / stop seek (i.e. don't seek)
    
  0b00000011,
    // SKMODE seek mode: 
    // 0 = wrap at upper or lower band limit and contiue seeking
    
    // CLK_MODE clock mode
    //  000 = 32.768kHZ clock rate (match the watch cystal on the module) 
    
    // RDS_EN radio data system enable
    // 0 = disable radio data system
    
    // NEW_METHOD use new demodulate method for improved sensitivity
    // 0 = presumably disabled 
    
    // SOFT_RESET
    // 1 = perform a reset
    
    // ENABLE power up enable: 
    // 1 = enabled 

  /* register 0x03 */
  // Don't bother to tune to a channel at this stage
  0b00000000, 
    // CHAN channel select 8 most significant bits of 10 in total
    // 0000 0000 = don't boher to program a channel at this time

  0b00000000,
    // CHAN two least significant bits of 10 in total 
    // 00 = don't bother to program a channel at this time
    
    // DIRECT MODE used only when test
    // 0 = presumably disabled
    
    // TUNE commence tune operation 
    // 0 = disable (i.e. don't tune to selected channel)
    
    // BAND band select
    // 00 = select the 87-108MHz band
    
    // SPACE channel spacing
    // 00 = select spacing of 100kHz between channels
    
  /* register 0x04 */
  0b00001010, 
    // RESERVED 15
    // 0
    
    // PRESUMABLY RESERVED 14
    // 0
    
    // RESERVED 13:12
    // 00
    
    // DE de-emphasis: 
    // 1 = 50us de-emphasis as used in Australia
    
    // RESERVED
    // 0
    
    // SOFTMUTE_EN
    // 1 = soft mute enabled
    
    // AFCD AFC disable
    // 0 = AFC enabled
    
  0b00000000, 
    // Bits 7-0 are not specified, so assume all 0's
    // 0000 0000
  
  /* register 0x05 */
  0b10001000, 
    // INT_MODE
    // 1 = interrupt last until read reg 0x0C
    
    // RESERVED 14:12 
    // 000
    
    // SEEKTH seek signal to noise ratio threshold
    // 1000 = suggested default 
  
  0b00001111, 
    // PRESUMABLY RESERVED 7:6
    // 00
    
    // RESERVED 5:4
    // 00
    
    // VOLUME
    // 1111 = loudest volume
  
  /* register 0x06 */
  0b00000000, 
    // RESERVED 15
    // 0
    
    // OPEN_MODE open reserved registers mode
    // 00 = suggested default
    
    // Bits 12:8 are not specified, so assume all 0's
    // 00000 
   
  0b00000000, 
    // Bits 7:0 are not specified, so assume all 0's
    // 00000000
    
  /* register 0x07 */
  0b01000010, 
    // RESERVED 15 
    // 0
    
    // TH_SOFRBLEND threshhold for noise soft blend setting
    // 10000 = using default value
    
    // 65M_50M MODE 
    // 1 = only applies to BAND setting of 0b11, so could probably use 0 here too
    
    // RESERVED 8
    // 0    
  
  0b00000010, 
    // SEEK_TH_OLD seek threshold for old seek mode
    // 000000
    
    // SOFTBLEND_EN soft blend enable
    // 1 = using default value
    
    // FREQ_MODE
    // 0 = using defualt value  
};

// After reset, we can tune the device
// We only need program the first 4 bytes in order to do this
uint8_t tune_config[] = {
  /* register 0x02 */
  0b11000000, 
    // DHIZ audio output high-z disable
    // 1 = normal operation
    
    // DMUTE mute disable 
    // 1 = normal operation
    
    // MONO mono select
    // 0 = stereo
    
    // BASS bass boost
    // 0 = disabled
    
    // RCLK NON-CALIBRATE MODE 
    // 0 = RCLK is always supplied
    
    // RCLK DIRECT INPUT MODE 
    // 0 = ??? not certain what this does
    
    // SEEKUP
    // 0 = seek in down direction
    
    // SEEK
    // 0 = disable / stop seek (i.e. don't seek)
    
   0b00000001, 
    // SKMODE seek mode: 
    // 0 = wrap at upper or lower band limit and contiue seeking
    
    // CLK_MODE clock mode
    //  000 = 32.768kHZ clock rate (match the watch cystal on the module) 
    
    // RDS_EN radio data system enable
    // 0 = disable radio data system
    
    // NEW_METHOD use new demodulate method for improved sensitivity
    // 0 = presumably disabled 
    
    // SOFT_RESET
    // 0 = don't reset this time around
    
    // ENABLE power up enable: 
    // 1 = enabled 

   /* register 0x03 */
   /* Here's where we set the frequency we want to tune to */
   (channel >> 2), 
    // CHAN channel select 8 most significant bits of 10 in total   

   ((channel & 0b11) << 6 ) | 0b00010000
    // CHAN two least significant bits of 10 in total 
    
    // DIRECT MODE used only when test
    // 0 = presumably disabled
    
    // TUNE commence tune operation 
    // 1 = enable (i.e. tune to selected channel)
    
    // BAND band select
    // 00 = select the 87-108MHz band
    
    // SPACE channel spacing
    // 00 = select spacing of 100kHz between channels  
};

void setup()
{
 
  Serial.begin(9600);
  Serial.println("\n\nA20150415 RDA5807M FM Tuner Example\n\n");
  
  Wire.begin(); // join i2c bus (address optional for master)
  
  Serial.print("Sending boot configuration (device should reset)...");
  Wire.beginTransmission(RDA5807M_ADDRESS);
  // Write the boot configuration bytes to the RDA5807M
  Wire.write(boot_config, BOOT_CONFIG_LEN);
  Wire.endTransmission();    // stop transmitting  
  Serial.println("Done.");

  Serial.print("Tuning to channel...");
  Wire.beginTransmission(RDA5807M_ADDRESS);
  // Write the tuning configuration bytes to the RDA5807M
  Wire.write(tune_config, TUNE_CONFIG_LEN);
  Wire.endTransmission();    // stop transmitting 
  Serial.println("Done.");
}

void loop()
{
  Serial.print(".");
  delay(1000);
}
