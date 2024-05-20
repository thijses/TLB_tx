
/*
see 'thijsUIpulses_tunes_example.ino' for a better/more-legible example
in this sketch, i've connected 2 buttons to select a song

*/


#include "thijsUIpulses.h"
#include "buzzer_tunes.h"

UIpulse* tuneList[] = {
  tunes::asabranca,
  tunes::babyelephantwalk,
  tunes::bloodytears,
  tunes::brahmslullaby,
  tunes::cannonind,
  tunes::cantinaband,
  tunes::doom,
  tunes::furelise,
  tunes::gameofthrones,
  tunes::greenhill,
  tunes::greensleeves,
  tunes::happybirthday,
  tunes::harrypotter,
  tunes::imperialmarch,
  tunes::jigglypuffsong,
  tunes::keyboardcat,
  tunes::merrychristmas,
  tunes::miichannel,
  tunes::minuetg,
  tunes::nevergonnagiveyouup,
  tunes::nokia,
  tunes::odetojoy,
  tunes::pacman,
  tunes::pinkpanther,
  tunes::princeigor,
  tunes::professorlayton,
  tunes::pulodagaita,
  tunes::silentnight,
  tunes::songofstorms,
  tunes::startrekintro,
  tunes::starwars,
  tunes::supermariobros,
  tunes::takeonme,
  tunes::tetris,
  tunes::badinerie,
  tunes::thegodfather,
  tunes::thelick,
  tunes::thelionsleepstonight,
  tunes::vampirekiller,
  tunes::zeldaslullaby,
  tunes::zeldatheme
};

const uint16_t tuneLengthList[] = {
  sizeof(tunes::asabranca)/sizeof(UIpulse),
  sizeof(tunes::babyelephantwalk)/sizeof(UIpulse),
  sizeof(tunes::bloodytears)/sizeof(UIpulse),
  sizeof(tunes::brahmslullaby)/sizeof(UIpulse),
  sizeof(tunes::cannonind)/sizeof(UIpulse),
  sizeof(tunes::cantinaband)/sizeof(UIpulse),
  sizeof(tunes::doom)/sizeof(UIpulse),
  sizeof(tunes::furelise)/sizeof(UIpulse),
  sizeof(tunes::gameofthrones)/sizeof(UIpulse),
  sizeof(tunes::greenhill)/sizeof(UIpulse),
  sizeof(tunes::greensleeves)/sizeof(UIpulse),
  sizeof(tunes::happybirthday)/sizeof(UIpulse),
  sizeof(tunes::harrypotter)/sizeof(UIpulse),
  sizeof(tunes::imperialmarch)/sizeof(UIpulse),
  sizeof(tunes::jigglypuffsong)/sizeof(UIpulse),
  sizeof(tunes::keyboardcat)/sizeof(UIpulse),
  sizeof(tunes::merrychristmas)/sizeof(UIpulse),
  sizeof(tunes::miichannel)/sizeof(UIpulse),
  sizeof(tunes::minuetg)/sizeof(UIpulse),
  sizeof(tunes::nevergonnagiveyouup)/sizeof(UIpulse),
  sizeof(tunes::nokia)/sizeof(UIpulse),
  sizeof(tunes::odetojoy)/sizeof(UIpulse),
  sizeof(tunes::pacman)/sizeof(UIpulse),
  sizeof(tunes::pinkpanther)/sizeof(UIpulse),
  sizeof(tunes::princeigor)/sizeof(UIpulse),
  sizeof(tunes::professorlayton)/sizeof(UIpulse),
  sizeof(tunes::pulodagaita)/sizeof(UIpulse),
  sizeof(tunes::silentnight)/sizeof(UIpulse),
  sizeof(tunes::songofstorms)/sizeof(UIpulse),
  sizeof(tunes::startrekintro)/sizeof(UIpulse),
  sizeof(tunes::starwars)/sizeof(UIpulse),
  sizeof(tunes::supermariobros)/sizeof(UIpulse),
  sizeof(tunes::takeonme)/sizeof(UIpulse),
  sizeof(tunes::tetris)/sizeof(UIpulse),
  sizeof(tunes::badinerie)/sizeof(UIpulse),
  sizeof(tunes::thegodfather)/sizeof(UIpulse),
  sizeof(tunes::thelick)/sizeof(UIpulse),
  sizeof(tunes::thelionsleepstonight)/sizeof(UIpulse),
  sizeof(tunes::vampirekiller)/sizeof(UIpulse),
  sizeof(tunes::zeldaslullaby)/sizeof(UIpulse),
  sizeof(tunes::zeldatheme)/sizeof(UIpulse),
};
String tuneNameList[] = {
  "asabranca",
  "babyelephantwalk",
  "bloodytears",
  "brahmslullaby",
  "cannonind",
  "cantinaband",
  "doom",
  "furelise",
  "gameofthrones",
  "greenhill",
  "greensleeves",
  "happybirthday",
  "harrypotter",
  "imperialmarch",
  "jigglypuffsong",
  "keyboardcat",
  "merrychristmas",
  "miichannel",
  "minuetg",
  "nevergonnagiveyouup",
  "nokia",
  "odetojoy",
  "pacman",
  "pinkpanther",
  "princeigor",
  "professorlayton",
  "pulodagaita",
  "silentnight",
  "songofstorms",
  "startrekintro",
  "starwars",
  "supermariobros",
  "takeonme",
  "tetris",
  "badinerie",
  "thegodfather",
  "thelick",
  "thelionsleepstonight",
  "vampirekiller",
  "zeldaslullaby",
  "zeldatheme",
};

//buzzer example:
UIpulseHandler buzzerHandler(26, HIGH, -1); // input parameters are: GPIO pin, active state, ledCchannel (-1 for automatic selection)

int8_t tuneListIndex = -1; // the first tune played will either be the first- or last one

const uint8_t speedSwitchPins[2] = {34, 35}; // pins for the 3-pos switch
uint32_t debounceTimers[2];
const uint32_t debouceInterval = 250;

void startTune(uint8_t tuneIndex) {
  Serial.print("playing: "); Serial.println(tuneNameList[tuneIndex]);
  buzzerHandler.startPulseList(tuneList[tuneListIndex], tuneLengthList[tuneListIndex]);
}

void setup() {
  Serial.begin(115200);
  buzzerHandler.init();

  pinMode(speedSwitchPins[0], INPUT_PULLUP); // pins 34 and up don't have an internal pullup
  pinMode(speedSwitchPins[1], INPUT_PULLUP);
}
void loop() {
  buzzerHandler.loop();
  if((!digitalRead(speedSwitchPins[0])) && (!digitalRead(speedSwitchPins[1]))) { // if both buttons are press simultaneously
    buzzerHandler.abortPulsing(); // shhhhh
    Serial.println("!interrupted tune!");
  } else if(!digitalRead(speedSwitchPins[0])) { // if the forward button is pressed
    if((millis() - debounceTimers[0]) > debouceInterval) {
      tuneListIndex++;  tuneListIndex = (tuneListIndex >= (sizeof(tuneList)/sizeof(UIpulse*))) ? 0 : tuneListIndex;
      startTune(tuneListIndex);
    }
    debounceTimers[0] = millis();
  }
  else if(!digitalRead(speedSwitchPins[1])) { // if the backward button is pressed
    if((millis() - debounceTimers[1]) > debouceInterval) {
      tuneListIndex--;  tuneListIndex = (tuneListIndex >= 0) ? tuneListIndex : (sizeof(tuneList)/sizeof(UIpulse*) - 1);
      startTune(tuneListIndex);
    }
    debounceTimers[1] = millis();
  }
}// you can also just use buzzerHandler.loop() in void loop() like in the other examples
