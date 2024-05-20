
/*
this tab contains functions/classes for handling user-feedback.
stuff like:
 - blinking LEDs
 - buzzer sounds
 - vibration

TODO:
 - 2-dimensional UIpulse array support (a list of lists might actually help cut down tune sizes, AND make beep-based menus easier)
 - fix large tune compilation time (by uploading/compiling it differently)
*/

// #pragma once  // i'll just use the ifndef method
#ifndef THIJS_UI_PULSES_H
#define THIJS_UI_PULSES_H

#include "Arduino.h"
//#include "esp32-hal-ledc.h" // needed for ledcwrite?

struct UIpulse { // a handy little storage-object for UI pulses (LED blinks, buzzer sounds, vibrations, etc.)
  uint32_t interval;
  float duty;
  float PWMfreq;
  float PWMduty;
  UIpulse(uint32_t pulseInterval, float pulseDuty=0.5, float PWMfreq=0.0, float PWMduty=1.0) : 
    interval(pulseInterval), duty(pulseDuty), PWMfreq(PWMfreq), PWMduty(PWMduty) {}
};

struct UIpulseHandler {
public:
  const uint8_t pin; // the GPIO pin the hardware is connected to
  const bool pinActiveState; // whether the hardware is active HIGH, or active LOW
  uint8_t _ledCchannel; // automatically or manually set in the constructor
  //// UI pulsation (low frequancy, intermittent)
  int8_t UIpulsesLeft = 0; // pulses left (including current pulse)
  uint32_t _UIpulseEndTimer; // (millis) time when the current pulse will be completed
  uint32_t _UIpulseActiveTimer; // (millis) time when the current pulse will need to become inactive (inactive period still left)
  bool _UIpulseActive = false; // just for speed, not really required per-se
  //// automated pulse lists (mostly for audio purposes)
  UIpulse* currentPulseListPtr = NULL; // a pointer to the next entry of the pulseList to use
  uint16_t currentPulseListLength = 0; // the total length of the pulseList (needs to be stored for loopPulseList to work)
  uint16_t currentPulseListLeft = 0; // how many entries of the pulseList are still unused 
  bool loopPulseList = false; // loop the current array of UIpulse objects
//private: // stuff the user really doesn't need to interact with
  UIpulse currentPulse = UIpulse(0,0.0,0.0,0.0); // the currently active UIpulse object. NOTE: this is a copy (to avoid memory management), but it could have also been a pointer, i guess
  const uint8_t _ledCchannelsMax = 16; // 8 on S2 and S3, 6 on C3  TODO: get from compiler clues
  const uint8_t _PWMresBits = 8; // the ESP32's ledC resolution depends on the frequency. See page 379 of the datasheet for more detail. 8 is fine, as PWM is usally not too significant
  const uint8_t _maxPWMval = (1<<_PWMresBits)-1; // hopefully this works (and is somethat efficient)
  const float _maxPMWfreq = 1000000.0;//40000000.0; // (Hz) if desired freq is above this value, constrain it (and give an error message)
  const float _lowFreqThresh = 10.0; // (Hz) if desired freq is below this value, skip hardware PWM. TODO: find ESP32/atmega328p minimum hardware-PWM frequencies
  uint32_t _lowFreqTimer; // timer for low frequency signals (unlike UIpulse__Timers, this is the time when the pulse STARTED!)
  uint32_t _lowFreqInterval = 0; // it's more computationally efficient to store this value ( used if it's nonzero)
  bool _lowFreqPinActive = false; // just for speed, not really required per-se

  void _loopLowFreqPWM() {
    uint32_t rightNow = millis();
    uint32_t dTime = rightNow - _lowFreqTimer;
    if(dTime > _lowFreqInterval) { // if one full PWM period has passed
      _lowFreqTimer = rightNow; // restart PWM timer
      _setPinActive();
      _lowFreqPinActive = true; // note the fact that the pin is active
    } else if(_lowFreqPinActive) {  if(dTime > (_lowFreqInterval * currentPulse.PWMduty)) { // only perform the second (inefficient) check if the first (quick) check passes
      _setPinInactive();
      _lowFreqPinActive = false; // this means you won't have to re-check if the pin has already been set inactive (saving a little time)
    } }
  }
  inline void _resetLowFreqTimers() { _lowFreqTimer = 0; _lowFreqInterval = 0; _lowFreqPinActive = false; } // reset stuff related to the low-freq-PWM
  inline void _resetAllTimers() { _UIpulseEndTimer = 0; _UIpulseActiveTimer = 0; _resetLowFreqTimers(); } // reset all (internal) timers, also stops low frequency pulsing
  inline void _resetPulseList() { currentPulseListPtr = NULL; currentPulseListLength = 0; currentPulseListLeft = 0; }
  uint16_t _incrementPulseList() {
    if((currentPulseListPtr) && (currentPulseListLeft) && (currentPulseListLength)) { // if there is a list
      startPulsing(*currentPulseListPtr, 1);
      currentPulseListLeft--; // one less entry left
      if(currentPulseListLeft > 0) {
        currentPulseListPtr += 1; // NOTE: this will increment the actual address of the pointer in sizeof(UIpulse) steps
      } else { // currentPulseListLeft == 0
        if(loopPulseList) {
          currentPulseListPtr -= currentPulseListLength-1; // NOTE: this will decrement the actual address of the pointer in sizeof(UIpulse) steps
          currentPulseListLeft = currentPulseListLength;
        } else {
          _resetPulseList();
        }
      }
    } else { // if either one is NULL/0 then calling this function is silly
      _resetPulseList();
      Serial.println("UIpulseHandler._incrementPulseList() called needlessly");
    }
    return(currentPulseListLeft);
  }
  inline void _writePWMduty(uint8_t PWMval) {
    ledcWrite(_ledCchannel, pinActiveState ? PWMval : (_maxPWMval-PWMval)); // has some inconsistancies at low frequencies
  }
  inline void _changePWMfreq(float desiredFreq) {
    float returnedFreq = ledcChangeFrequency(_ledCchannel, desiredFreq, _PWMresBits);
    if(returnedFreq == 0.0) { Serial.print("UIpulseHandler._changePWMfreq() error: "); Serial.println(returnedFreq); }
  }
  inline void _setPinActive() { /* digitalWrite(pin, pinActiveState); */ _changePWMfreq(1000); _writePWMduty(_maxPWMval); } // a bit of a hack, but it's fast enough for now
  inline void _setPinInactive() { /* digitalWrite(pin, !pinActiveState); */ _changePWMfreq(1000); _writePWMduty(0); } // the frequency is basically arbetrary, but if it's too low, it will mess up the signal
public:
  UIpulseHandler(uint8_t pin, bool activeState=HIGH, int8_t ledCchannel=-1) : pin(pin), pinActiveState(activeState) {
    static uint8_t _ledCchannelsAlreadyUsed = 0; // on the ESP32 any pin can do PWM, but there are 16 channels (or 8,6,8 on the S2,C3,S3 respectively)
    if(ledCchannel >= 0) { // if the user specified a ledC channel to use
      _ledCchannel = ledCchannel;
      //if(_ledCchannel == _ledCchannelsAlreadyUsed) { _ledCchannelsAlreadyUsed++; } // a terrible no-good addition, which could perhaps maybe technically sort of help(?)
      //else { _ledCchannelsAlreadyUsed = ledCchannel; } // even worse
      // note: if you're going to set manually specify the channel of one handler, you should also manually set all subsequent handler's channels
    } else { // if the ledCchannel is to be automatically inferred
      _ledCchannel = _ledCchannelsAlreadyUsed;
      _ledCchannelsAlreadyUsed++; // increment this static class variable, sothat the next constructor will know
    }
  }
  void init() {
    pinMode(pin, OUTPUT);
    ledcSetup(_ledCchannel, 1000, _PWMresBits); // initialize to some random frequency (which will change once any pulse starts)
    ledcAttachPin(pin, _ledCchannel); // make sure the PWM is ready to go
    _setPinInactive();
  }
  void stopPulsing() { // stops the pulse being ACTIVE, does NOT stop the timers (so the inactive portion of the pulse will still continue
    _setPinInactive();
    _UIpulseActive = false; // prevent the loop() checks from repeatedly stopping the pulse
  }
  void abortPulsing() { // completely abort the pulsing actions (also used to stop indefinite pulses (UIpulsesLeft = -1))
    stopPulsing(); // stop the current pulse if it was still active
    _resetAllTimers();
    UIpulsesLeft = 0;
    //_resetPulseList();
  }
  void startPulseList(UIpulse* pulseList, uint16_t listLength, bool loopList=false) {
    currentPulseListPtr = pulseList; currentPulseListLength = listLength; currentPulseListLeft = listLength; loopPulseList = loopList; // set all the list elements
    _incrementPulseList(); // this will call startPulsing()
  }
  void _startPulsing() { // the function for starting the PWM signals and timers and such
    uint32_t rightNow = millis();
    _UIpulseActiveTimer = rightNow + (currentPulse.duty * currentPulse.interval); // set the timer for when the signal should go inactive
    _UIpulseEndTimer = rightNow + currentPulse.interval; // set the timer for when the pulse is done
    if(currentPulse.PWMduty >= 1.0) { // if the frequency doesn't matter, because the user just wanted a regular active signal
      _setPinActive();
      //_resetLowFreqTimers(); // (super extra precaution) this is not low-freq PWM, this is just a completely high signal.
    } else if(currentPulse.PWMfreq < _lowFreqThresh) { // if low-freq PWM is needed
      if(currentPulse.PWMfreq <= 0.0) { // if a negative or 0 frequency was entered (and currentPulse.PWMduty is below 1.0)
        Serial.print("UIpulseHandler: PWM freq too low!, "); Serial.print(currentPulse.PWMfreq); Serial.print("  "); Serial.println(currentPulse.PWMduty);
        currentPulse.PWMfreq = 1000.0 / (currentPulse.interval * currentPulse.duty); // set PWMfreq to be the full pulse length (and then low-freq PWM code will shorten it again, based on PWMduty)
      }
      _lowFreqInterval = (1000.0 / currentPulse.PWMfreq);
      _lowFreqTimer = rightNow; // restart PWM timer
      _setPinActive();
      _lowFreqPinActive = true; // note the fact that the pin is active
    } else { // if hardware PWM can be used
      if(currentPulse.PWMfreq >= _maxPMWfreq) {
        Serial.print("UIpulseHandler: PWM freq too high!, "); Serial.print(currentPulse.PWMfreq); Serial.print(" -> "); Serial.println(_maxPMWfreq);
        currentPulse.PWMfreq = _maxPMWfreq; // constrain freq
      }
      _changePWMfreq(currentPulse.PWMfreq);
      _writePWMduty(currentPulse.PWMduty * _maxPWMval); // set the signal to active
    }
    _UIpulseActive = true;
  }
  void startPulsing(UIpulse newPulse, int8_t pulseCount=-1) {
    if((newPulse.interval == 0) || (newPulse.duty <= 0.0) || (newPulse.PWMduty <= 0.0)) {
      Serial.print("UIpulseHandler: startPulsing bad pulse entered: ");
      Serial.print(newPulse.interval); Serial.print('\t');
      Serial.print(newPulse.duty, 7); Serial.print('\t');
      Serial.print(newPulse.PWMduty, 7); Serial.println();
      abortPulsing();
      return;
    }
//    if((UIpulsesLeft != 0) || (rightNow < _UIpulseEndTimer)) { // if there is a pulse currently ongoing.... do something ,idk
    currentPulse = newPulse; UIpulsesLeft = pulseCount;
    currentPulse.duty = constrain(currentPulse.duty, 0.0, 1.0);  currentPulse.PWMduty = constrain(currentPulse.PWMduty, 0.0, 1.0); // make extra sure the values are acceptable
    _startPulsing(); // set the actual pins/timers
  }
  void startPulsing(int8_t pulseCount, uint32_t pulseInterval, float pulseDuty, float PWMfreq=0.0, float PWMduty=1.0) {
    startPulsing(UIpulse(pulseInterval, pulseDuty, PWMfreq, PWMduty), pulseCount);
  }
  void setPWMfreq(float desiredFreq) { // useful for audio applications
    if(desiredFreq < 0.0) { Serial.println("UIpulseHandler: gonna ignore negative frequency!"); return; }
    uint32_t rightNow = millis();
    if((rightNow < _UIpulseActiveTimer) || (UIpulsesLeft != 0)) { // all of this only makes sense if the pulse is still going on, or if it's going to repeat  TODO: check if this makes sense
      if(desiredFreq == 0.0) { // not the cleanest check when handling floats, but with 0.0 it is not terribly unreasonable
        // the user doesn't want PWM at all, just a solid HIGH signal. The same thing can be accomplished with a duty cycle of 1.0
        //if(currentPulse.PWMduty < 1.0) { Serial.println("UIpulseHandler: please set PWM duty to 1.0 instead of freq to 0.0");
        currentPulse.PWMduty = 1.0;
        desiredFreq = _lowFreqThresh;
        if(rightNow < _UIpulseActiveTimer) { // if the current pulse is still active, overwrite it with the new values
          _changePWMfreq(desiredFreq); _writePWMduty(currentPulse.PWMduty * _maxPWMval);  // TODO!
        }
      } else if(desiredFreq < _lowFreqThresh) {
        if(rightNow < _UIpulseActiveTimer) {
          if(_lowFreqInterval == 0) { // if low freq PWM is NOT already ongoing
            _lowFreqTimer = rightNow; // restart PWM timer
            _setPinActive();
            _lowFreqPinActive = true; // note the fact that the pin is active
          }
          _lowFreqInterval = (1000.0 / desiredFreq);
        }
      } else {
        if(desiredFreq >= _maxPMWfreq) {
          Serial.print("UIpulseHandler: PWM freq too high!, "); Serial.print(desiredFreq); Serial.print(" -> "); Serial.println(_maxPMWfreq);
          desiredFreq = _maxPMWfreq; // constrain freq
        }
        //if(_lowFreqInterval > 0) { // hardware PWM will overwrite the digitalWrite from the low freq (manual) PWM
        _resetLowFreqTimers(); // just in case it was at a low freq before
        if(rightNow < _UIpulseActiveTimer) { // if the current pulse is still active, overwrite it with the new values
          _changePWMfreq(desiredFreq); _writePWMduty(currentPulse.PWMduty * _maxPWMval);
        } // note: write duty cycle as well, because the previous frequency may have been below _lowFreqThresh
      }
    }
  }
  void setPWMduty(float desiredDuty) { // useful for limiting power
    if(desiredDuty < 0.0) { Serial.println("UIpulseHandler: gonna ignore negative duty cycle!"); return; }
    currentPulse.PWMduty = constrain(desiredDuty, 0.0, 1.0);
    if(millis() < _UIpulseActiveTimer) { _writePWMduty(currentPulse.PWMduty * _maxPWMval); } // TODO!
  }
  bool loop() { // returns whether a UI interaction is currently active
//    Serial.println("loop");
    uint32_t rightNow = millis();
    if((UIpulsesLeft != 0) || (rightNow < _UIpulseEndTimer)) { // if there are actually pulses ongoing
      if(rightNow >= _UIpulseEndTimer) { // if it's time to end the pulse entirely
        if(UIpulsesLeft > 0) { UIpulsesLeft--; } // only if the counter is positive (and non-zero)
        if(UIpulsesLeft != 0) { // if it's -1 or above 0, do the same pulse again
          _startPulsing(); // activate the pulse
        } else { // if UIpulsesLeft == 0
          if((currentPulseListPtr) && (currentPulseListLeft) && (currentPulseListLength)) { // if there are still entries in the pulseList
            _incrementPulseList(); // move to the next entry in the list (will call startPulsing())
          } else {
            abortPulsing();
          }
        }
      } else { // pulse is still ongoing (could be either active or inactive, but definitely ongoing)
        if(_UIpulseActive) {
          if(rightNow >= _UIpulseActiveTimer) { // if it's time to set the pulse to the inactive state
            stopPulsing(); // set pin to inactive
            _resetLowFreqTimers();
            // note: UIpulseActive is set to false in stopPulsing()
          } else if(_lowFreqInterval > 0) { // if low-frew PWM needs to be handled
            _loopLowFreqPWM(); // 
          } // note: hardware PWM will just continue working untill stopped, no intervention required there
        } // if the pulse was already deactivated, just do nothing
      }
    }
    return((UIpulsesLeft != 0) || (rightNow < _UIpulseEndTimer)); // return if there are any pulses left, it's pulsing indefinitely or a pulse is still still ongoing
  }
  void finishPulses() { // a quick 'n dirty function to wait for pulses to finish
//    if(UIpulsesLeft < 0) { // prevent an infinite loop
//      Serial.print("UIpulseHandler.finishPulses(), not gonna wait forever! "); Serial.print(UIpulsesLeft); Serial.println(" < 0");
//      return;
//    }
    while(loop()); // loop untill it returns false (at which point, all the pulses are done)
  }
};

#endif // THIJS_UI_PULSES_H


/*

some example usage:
LED example:
UIpulseHandler LEDhandler(27, HIGH);
void setup() {
LEDhandler.init();
LEDhandler.startPulsing(5, 1000, 0.5, 0.0, 1.0); // 5 half-second pulses of fully-lit LED
//LEDhandler.startPulsing(3, 1000, 0.5, 500.0, 0.25); // 3 half-second pulses of dimmly list LED
}
void loop() { bool stillGoing = LEDhandler.loop() } // required to complete the pulses

buzzer example:
UIpulseHandler buzzerHandler(26, HIGH);
void setup() {
buzzerHandler.init();
//buzzerHandler.startPulsing(5, 1000, 0.5, 500.0, 0.5); // 5 half-second pulses at 500Hz (at 50% duty cycle, which should be max volume)
buzzerHandler.startPulsing(3, 100, 0.4, 200, 0.2); // 3 quick (100ms interval, 40ms long sound) pulses at 200Hz at a lower volume (20% duty cycle is just under half volume, i think)
}
void loop() { bool stillGoing = buzzerHandler.loop() } // required to complete the pulses

vibration motor example:
UIpulseHandler vibrMotorHandler(25, HIGH);
void setup() {
vibrMotorHandler.init();
vibrMotorHandler.startPulsing(2, 500, 0.5, 500, 0.75); // 2 quarter-second pulses at 75% power (PWM freq is 500, but you should change it if you hear coil-whine/resonance)
}
void loop() { bool stillGoing = vibrMotorHandler.loop() } // required to complete the pulses

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
