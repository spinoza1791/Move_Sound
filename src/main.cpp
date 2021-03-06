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

uint8_t volume_left = 0x05;
uint8_t volume_right = 0x05;
long rndnum;
int shaken = 0;
int org_playing = 0;

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

const int xInput = A2;
const int yInput = A1;
const int zInput = A0;

int xRaw;
int yRaw;
int zRaw;
int last_xRaw_high;
int last_xRaw_low;
int last_yRaw_high;
int last_yRaw_low;
int last_zRaw_high;
int last_zRaw_low;
const int accel = 15;
int wait_for_shake = 0;
int max_wait = 10;

// initialize minimum and maximum Raw Ranges for each axis
int RawMin = 0;
int RawMax = 1023;

// Take multiple samples to reduce noise
const int sampleSize = 10;

int ReadAxis(int axisPin)
{
  long reading = 0;
  analogRead(axisPin);
  delay(1);
  for (int i = 0; i < sampleSize; i++)
  {
  reading += analogRead(axisPin);
  }
  return reading/sampleSize;
}

void setup() {
  Serial.begin(9600);
  pinMode(8, INPUT_PULLUP);

  // Wait for serial port to be opened, remove this line for 'standalone' operation
  //while (!Serial) { delay(1); }
  delay(500);
  //Serial.println("\n\nAdafruit VS1053 Feather Test");

  if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }

  Serial.println(F("VS1053 found"));

  musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working

  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }
  Serial.println("SD OK!");

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(volume_left,volume_right);

#if defined(__AVR_ATmega32U4__)
  // Timer interrupts are not suggested, better to use DREQ interrupt!
  // but we don't have them on the 32u4 feather...
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int
#else
  // If DREQ is on an interrupt pin we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
#endif

  // Play a file in the background, REQUIRES interrupts!
  Serial.println(F("Playing full track 001"));
  musicPlayer.startPlayingFile(wait_snd1);
  randomSeed(analogRead(2));

  //Read raw values
  xRaw = ReadAxis(xInput);
  yRaw = ReadAxis(yInput);
  zRaw = ReadAxis(zInput);

}

void loop() {
  delay(100);

  last_xRaw_high = xRaw + accel;
  last_xRaw_low = xRaw - accel;
  last_yRaw_high = yRaw + accel;
  last_yRaw_low = yRaw - accel;
  last_zRaw_high = zRaw + accel;
  last_zRaw_low = zRaw - accel;

  //Read raw values
  xRaw = ReadAxis(xInput);
  yRaw = ReadAxis(yInput);
  zRaw = ReadAxis(zInput);
  Serial.print("X, Y, Z  :: ");
  Serial.print(xRaw);
  Serial.print(", ");
  Serial.print(yRaw);
  Serial.print(", ");
  Serial.println(zRaw);

  if (xRaw < last_xRaw_low || xRaw > last_xRaw_high) {
    Serial.println("X has accelerated");
    shaken = 1;
    wait_for_shake = 0;
  }
  else if (yRaw < last_yRaw_low || yRaw > last_yRaw_high) {
    Serial.println("Y has accelerated");
    shaken = 1;
    wait_for_shake = 0;
  }
  else if (zRaw < last_zRaw_low || zRaw > last_zRaw_high) {
    Serial.println("Z has accelerated");
    shaken = 1;
    wait_for_shake = 0;
  }
  else {
    Serial.println("no acceleration detected");
    shaken = 0;
  }

//delay(200);
//Serial.println(shaken);
//Serial.println(org_playing);
Serial.println(wait_for_shake);

  if (shaken == 1 && org_playing == 0) {
    Serial.println("Stopping Wait and Starting Random Org");
    shaken = 0;
    musicPlayer.stopPlaying();
    rndnum = random(1, 4);
    if (rndnum == 1) {
      musicPlayer. startPlayingFile(org_snd1);
    }
    if (rndnum == 2) {
      musicPlayer. startPlayingFile(org_snd2);
    }
    if (rndnum == 3) {
      musicPlayer. startPlayingFile(org_snd3);
    }
    org_playing = 1;
  }

  if (shaken == 0 && org_playing == 1) {
    wait_for_shake++;
  }
  //Check if not shaken for 2 sec
  if (wait_for_shake > max_wait) {
    musicPlayer.stopPlaying();
    musicPlayer. startPlayingFile(wait_snd1);
    org_playing = 0;
    wait_for_shake = 0;
  }

  // Check if sounds have ended and reset flags if so
  if (musicPlayer.stopped()) {
    org_playing = 0;
    if (shaken == 0) {
      musicPlayer. startPlayingFile(wait_snd1);
    }
  }
}
