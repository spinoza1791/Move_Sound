// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>

// These are the pins used
#define VS1053_RESET   -1     // VS1053 reset pin (not used!)
#define VS1053_CS       6     // VS1053 chip select pin (output)
#define VS1053_DCS     10     // VS1053 Data/command select pin (output)
#define CARDCS          5     // Card chip select pin
// DREQ should be an Int pin *if possible* (not possible on 32u4)
#define VS1053_DREQ     9     // VS1053 Data request, ideally an Interrupt pin

const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
const char *org_snd1 = "/org_001.mp3";
const char *org_snd2 = "/org_002.mp3";
const char *org_snd3 = "/org_003.mp3";
const char *wait_snd1 = "/wait_001.mp3";

uint8_t volume_left = 0x00;
uint8_t volume_right = 0x00;
long randNumber;
int org_playing = 0;
int wait_playing = 0;

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(2));
  if (! musicPlayer.begin()) { // initialise the music player
     //Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }
  musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working
  if (!SD.begin(CARDCS)) {
    //Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }
  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(volume_left,volume_right);
  // If DREQ is on an interrupt pin we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
}

void loop() {
  delay(10);
  // Check if sounds have ended and reset flags if so
  if (musicPlayer.stopped()) {
    org_playing = 0;
    wait_playing = 0;
  }

  if (org_playing < 1) {
    wait_playing = 0;
    randNumber = random(2);
    Serial.print (F("randNumber:    ")); Serial.print(randNumber); Serial.println(F("/n"));
    musicPlayer.stopPlaying();
    if (randNumber == 0) {
      musicPlayer. startPlayingFile(org_snd1);
      org_playing = 1;
    }
    if (randNumber == 1) {
     musicPlayer. startPlayingFile(org_snd2);
     org_playing = 2;
    }
    if (randNumber == 2) {
    musicPlayer. startPlayingFile(org_snd3);
    org_playing = 3;
    }
  }

  if (wait_playing == 0) {
    musicPlayer.stopPlaying();
    musicPlayer. startPlayingFile(wait_snd1);
    wait_playing = 1;
    org_playing = 0;
  }
}
