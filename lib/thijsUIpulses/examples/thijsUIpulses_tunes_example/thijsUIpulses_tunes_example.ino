
/*

some example usage:
LED example:
UIpulseHandler LEDhandler(27, HIGH);
void setup() {
LEDhandler.init();
LEDhandler.startPulsing(5, 1000, 0.5, 0.0, 1.0); // 5 half-second pulses of fully-lit LED
//LEDhandler.startPulsing(3, 1000, 0.5, 500.0, 0.25); // 3 half-second pulses of dimmly list LED
}
void loop() { bool stillGoing = LEDhandler.loop(); } // required to complete the pulses

buzzer example:
UIpulseHandler buzzerHandler(26, HIGH);
void setup() {
buzzerHandler.init();
//buzzerHandler.startPulsing(5, 1000, 0.5, 500.0, 0.5); // 5 half-second pulses at 500Hz (at 50% duty cycle, which should be max volume)
buzzerHandler.startPulsing(3, 100, 0.4, 200, 0.2); // 3 quick (100ms interval, 40ms long sound) pulses at 200Hz at a lower volume (20% duty cycle is just under half volume, i think)
}
void loop() { bool stillGoing = buzzerHandler.loop(); } // required to complete the pulses

vibration motor example:
UIpulseHandler vibrMotorHandler(25, HIGH);
void setup() {
vibrMotorHandler.init();
vibrMotorHandler.startPulsing(2, 500, 0.5, 500, 0.75); // 2 quarter-second pulses at 75% power (PWM freq is 500, but you should change it if you hear coil-whine/resonance)
}
void loop() { bool stillGoing = vibrMotorHandler.loop(); } // required to complete the pulses

buzzer tune example: (grab/make buzzer_tunes.h or auto-generate it (see "scraping songs from github" folder in library))
UIpulseHandler buzzerHandler(26, HIGH);
void setup() {
buzzerHandler.init();
#define playTune(songName)  buzzerHandler.startPulseList(tunes::songName, sizeof(tunes::songName) / sizeof(UIpulse));  Serial.print("playing: ");Serial.println(#songName);
playTune(imperialmarch); buzzerHandler.finishPulses();
// in this example, tunes::imperialmarch is an array of UIpulse objects
// here is an example of an pulse-array:
UIpulse exampleArray[] = { // duration: 3 secs,  size:32 bytes
    {1000, 0.75, 392, 0.5},
    {2000, 1.0, 262, 0.5}};
buzzerHandler.startPulseList(exampleArray, sizeof(exampleArray) / sizeof(UIpulse), false); // pass the array, how long the array is and whether you want to loop it
buzzerHandler.finishPulses(); // this just calls 'while(buzzerHandler.loop())', you can also just use buzzerHandler.loop() in void loop() like in the previous examples
// a looping example
#define loopTune(songName)  buzzerHandler.startPulseList(tunes::songName, sizeof(tunes::songName) / sizeof(UIpulse), true);  Serial.print("looping: ");Serial.println(#songName);
loopTune(thelick);  buzzerHandler.finishPulses();
}


*/


#include "thijsUIpulses.h"
#include "buzzer_tunes.h"

#define outputPin 27 // any (output-capable) GPIO pin

//buzzer example:
UIpulseHandler buzzerHandler(outputPin, HIGH, -1); // input parameters are: GPIO pin, active state, ledCchannel (-1 for automatic selection)

void setup() {
  Serial.begin(115200);
  buzzerHandler.init();
  #define playTune(songName)  buzzerHandler.startPulseList(tunes::songName, sizeof(tunes::songName) / sizeof(UIpulse));  Serial.print("playing: ");Serial.println(#songName);
  playTune(imperialmarch); buzzerHandler.finishPulses();
  
  delay(1000);
  
  // in the example above, tunes::imperialmarch is an array of UIpulse objects
  // here is an example of an pulse-array:
  UIpulse exampleArray[] = { // duration: 3 secs,  size:32 bytes
      {1000, 0.75, 392, 0.5},
      {2000, 1.0, 262, 0.5}};
  buzzerHandler.startPulseList(exampleArray, sizeof(exampleArray) / sizeof(UIpulse), false); // pass the array, how long the array is and whether you want to loop it
  buzzerHandler.finishPulses(); // this just calls 'while(buzzerHandler.loop())', you can also just use buzzerHandler.loop() in void loop() like in the other examples
  
  delay(1000);
  
  // a looping example
  #define loopTune(songName)  buzzerHandler.startPulseList(tunes::songName, sizeof(tunes::songName) / sizeof(UIpulse), true);  Serial.print("looping: ");Serial.println(#songName);
  loopTune(thelick);  buzzerHandler.finishPulses();
}
void loop() {}// you can also just use buzzerHandler.loop() in void loop() like in the other examples