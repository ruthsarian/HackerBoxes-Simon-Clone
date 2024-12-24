/* Hackerboxes Simon Clone v1.0 : ruthsarian@gmail.com
 * -- 
 * Received the Hackerboxes Simon clone PCB in a Hackerboxes "surprise surplus" box. Without referring
 * to any Haxerboxes documentation, I reversed the PCB, populated it with components I had on hand, and 
 * wrote this firmware.
 * 
 * This PCB was part of Hackerboxes #0044
 * https://hackerboxes.com/products/hackerbox-0044-pcb-123
 * 
 * Hackerboxes PCB Notes
 * 
 *   MCU Pinout on PCB:
 *     1=BZR   8=VCC
 *     2=BT1   7=BT2
 *     3=LED   6=BT4
 *     4=GND   5=BT3
 *
 *   Buzzer is attached to pin 1, the ATTiny85's reset pin. To use the buzzer, the
 *   reset line must be disabled. Unless you have a special programmer, without the
 *   reset line you can't reprogram the ATTiny85 unless you install a bootloader.
 *
 *   Addressable RGB LEDs 
 *     PCB Pinout: 
 *       FLAT, 1=DOUT, 2=GND, 3=PWR, 4=DIN
 *
 *     Order on PCB follows IC PIN ordering convention
 *       1  4
 *       2  3
 *
 *     PCB Expects APA106 RGB LEDs
 *       I am using some "WS2811" RGB LEDs with a pinout that has the POWER and GROUND pins
 *       reversed compared to the PCB. I had to twist those leads to make them fit.
 *       
 *       My RGB LEDs also likely have a clone WS2811 chip with an issue that they require a
 *       few hundred microseconds for changes to latch. Therefore I needed to build a delay
 *       into the leds.show() function. 
 * 
 * USBasp Programmer notes
 *   https://www.amazon.com/s?k=usbasp
 *   https://www.ebay.com/sch/i.html?_nkw=usbasp&_sacat=0&_blrs=spell_auto_correct
 *
 *   Some versions of AVRDUDE (used by Arduino IDE to program the ATTiny85) need the USBasp
 *   device to use the libusb driver. You'll need to use Zadig in order to set the driver.
 *   https://www.instructables.com/USBASP-Installation-in-Windows-10/
 *
 *   USBasp Connector to ATTiny85 Breakout Board
 *     VCC  = green   = 8 5V
 *     GND  = black   = 4 GND
 *     DO   = blue    = 5 P0
 *     DI   = white   = 6 P1
 *     SCK  = grey    = 7 P2
 *     RST  = purple  = 1 P5
 * 
 * ATTiny85 Breakout Board (DigiSpark) Notes
 *   https://www.ebay.com/sch/i.html?_nkw=attiny85+dip-8+usb&_sacat=0
 *   
 *   LED_BUILTIN = P1
 *
 * ATTiny85 datasheet:
 *   https://ww1.microchip.com/downloads/en/devicedoc/atmel-2586-avr-8-bit-microcontroller-attiny25-attiny45-attiny85_datasheet.pdf
 *   
 * Arduino Core Notes 
 *   To program the ATTiny85 you need to install a "core" (aka 'board' via 'board manager')
 * 
 *   Install ATTinyCore (includes tinyNeoPixel)
 *     https://github.com/SpenceKonde/ATTinyCore
 *     https://github.com/SpenceKonde/tinyNeoPixel
 * 
 *   tone() is available
 *     ATTinyCore puts millis() on Timer0
 *     We are free to use Timer1 for tone generation
 *     https://github.com/SpenceKonde/ATTinyCore/issues/211
 *
 *   Arduino Board Configuration (with Bootloader)
 *     Board: ATTiny85 (Micronucleus / DigiSpark)
 *     Clock: 16MHz
 *     Timer1 Clock: CPU
 *     LTO: enabled
 *     millis()/micros(): enabled
 *     Burn Bootloader Method: upgrade (via USB)
 *     Reset Pin: GPIO
 *     BOD Level: disabled
 *     Programmer: micronucleus
 *   
 *   Burn Bootloader: even if you're not using one
 *     When programming the ATTiny85 with the ArduinoIDE for the first time, I recommend 
 *     doing a "Burn Bootloader" operation so the settings Arduino assumes the ATTiny85
 *     is configured for are actually in place. Otherwise your code may not run correctly.
 *     
 *     If you are not using a bootloader, then do not solder the buzzer to the PCB.
 *    
 * Bootloader Notes
 * 
 *   It was necessary to install a bootloader on the ATTiny85 in order to disable the RESET pin and allow
 *   it to be used as I/O and still be able to program the ATTiny85. I chose to use Micronucleus because
 *   it's what I've encountered before with DigiSpark boards.
 *   
 *   Micronucleus Bootloader: https://github.com/micronucleus/micronucleus
 *   
 *   To build and install the bootloader, I used Windows Subsystem for Linux (WSL)
 *   This required I attach the USBasp device to WSL in order for WSL to be able to interact with it.
 *   How this can be done is documented here: https://learn.microsoft.com/en-us/windows/wsl/connect-usb
 *
 *     1. start WSL
 *     2. start Powershell as Administrator
 *     3. in powershell, get BUSID for USBasp
 *        usbipd list
 *     4. attach USBasp to WSL
 *        usbipd bind --busid <BUSID>
 *        usbipd attach --wsl --busid <BUSID>
 *     5. in WSL, confirm device is attached
 *        lsusb
 *     6. compile Micronucleus firmware
 *        git clone https://github.com/micronucleus/micronucleus
 *        cd micronucleus/firmware
 *        sudo make CONFIG=t85_default
 *        sudo make CONFIG=t85_default fuse
 *        sudo make CONFIG=t85_default flash
 *        sudo make CONFIG=t85_default disablereset
 *
 *   Fuses can also be set manually with AVRDUDE
 *     sudo avrdude -c USBasp -p attiny85  -U lfuse:w:0xe1:m -U hfuse:w:0xdd:m -U efuse:w:0xfe:m -B 20 -F
 * 
 * Fuse Reset Notes
 *   During development, the ATTiny85 became unresponsive; the bootloader was no longer working.
 *   In order to recover the ATTiny85 I needed to first reset its fuses so I could reload the
 *   bootloader. This requires using high voltage (12V) serial programming.
 *   
 *   I loaded a fuse reset sketch onto an Arduino Nano, wired up the Nano to the ATTiny85 on a
 *   breadboard, and added 12V from a bench power supply to the reset line. This reset the fuses
 *   and let me reload the bootloader onto the ATTiny85 and I was back in action.
 *   
 *   The sketch and related resources for the fuse reset are below. 
 *     https://www.electronics-lab.com/recover-bricked-attiny-using-arduino-as-high-voltage-programmer/
 *       -> https://www.rickety.us/2010/03/arduino-avr-high-voltage-serial-programmer/
 *          https://www.rickety.us/wp-content/uploads/2010/03/hv_serial_prog.pde
 *            -> https://mightyohm.com/blog/2008/09/arduino-based-avr-high-voltage-programmer/
 *               https://mightyohm.com/blog/wp-content/uploads/2008/09/hvfuse.pde
 *               https://mightyohm.com/blog/products/hv-rescue-shield-2-x/
 *               https://www.amazon.com/MightyOhm-AVR-HV-Rescue-Shield/dp/B08M27NN8Y
 *     https://sites.google.com/site/wayneholder/attiny-fuse-reset
 *     https://github.com/shariramani/AVR-High-voltage-Serial-Fuse-Reprogrammer
 *     
 */

#include <tinyNeoPixel_Static.h>

#define SEED_PIN      PIN_PB2
#define BUZZER_PIN    PIN_PB5
#define LED_DATA_PIN  PIN_PB4
#define NUMLEDS       4
#define DEBOUNCE_MAX  64

// my particular variant of addressable RGB LEDs have a 'bug' that requires a delay between calls to show()
// see note under "Refresh Rate" section: https://github.com/SpenceKonde/tinyNeoPixel
#define SHOW_LEDS     delayMicroseconds(400);leds.show

byte pixels[NUMLEDS * 3];
tinyNeoPixel leds = tinyNeoPixel(NUMLEDS, LED_DATA_PIN, NEO_RGB, pixels);

uint32_t led_colors[NUMLEDS+1] = {
  leds.Color(  0,  64,   0),    // green
  leds.Color( 64,  64,   0),    // yellow
  leds.Color(  0,   0,  64),    // blue
  leds.Color( 64,   0,   0),    // red
  leds.Color(  0,   0,   0)     // off
};

uint8_t button_pins[NUMLEDS] = {
  PIN_PB3,                      // top-left      green
  PIN_PB2,                      // bottom-left   yellow
  PIN_PB0,                      // bottom-right  blue
  PIN_PB1                       // top-right     red
};

uint16_t tones[NUMLEDS+1] = {
  330,  // E4   green
  554,  // C5#  yellow
  659,  // E5   blue
  880,  // A5   red
  49    // G1   error
};

int16_t current_tone = 0;

// light an LED and play its tone
void start_node(uint8_t node) {
  leds.setPixelColor(node, led_colors[node]);
  SHOW_LEDS();
  current_tone += tones[node];
  tone(BUZZER_PIN, current_tone);
}

// turn off an LED and stop any tone
void stop_node(uint8_t node) {

  // remove node from current tone
  current_tone -= tones[node];

  // in theory, we should never go below zero, but...
  // this is to catch any screwups
  if (current_tone < 0) {
    current_tone = 0;
  }

  // set new tone
  if (current_tone == 0) {
    noTone();
  } else {
    tone(BUZZER_PIN, current_tone);
  }

  // turn off LED
  leds.setPixelColor(node, led_colors[NUMLEDS]);
  SHOW_LEDS();
}

// turn on the LED and play the tone for the given node for the given length of time.
void play_node(uint8_t node, uint16_t play_time) {
  start_node(node);
  delay(play_time);
  stop_node(node);
}

void clear_nodes() {
  noTone();
  current_tone = 0;
  leds.clear();
  SHOW_LEDS();
}

// game states
typedef enum {
  GAME_OVER,
  GAME_INIT,
  SAY,
  LISTEN,
  WIN,
  LOSE
} StateOfGame;

// all of the game logic is here
//
// there are 4 "difficulty" levels. which level is used depends on which button is pressed
// to start a new game. each difficulty level increases the number of items you have to
// remember by 4. 
//
// Q: why didn't i put this code straight into loop()?
// A: because i had other code in loop() while developing this project (blinking an LED) to
//    act as a visual clue that the board was still running. in the final version, that code
//    is no longer used, but maybe some sort of watchdog could be added in later? or maybe
//    some extra functionality that should be outside of the game itself? so i'm leaving
//    the game logic here, in its own function, separate from loop()
// 
void game() {
  static StateOfGame game_state = GAME_OVER;

  static const uint8_t difficulty_level_count[] = { 8, 12, 16, 20 };
  static uint8_t game_key[20];
  static uint8_t current_level = 0;
  static uint8_t listen_level = 0;
  static uint8_t difficulty = 0;

  static uint8_t debounce[NUMLEDS] = {0};
  static uint8_t last_button_press = 0;
  static uint32_t play_timeout = 0;

  uint8_t current_button_press = 0;
  uint8_t i, j;

  // detect if a button has been pressed with debounce mitigation
  //
  // concept is everytime we call this routine, check each button. if the button
  // is detected as pressed, increment its debounce value by 1, otherwise decrement
  // it by 1.
  // 
  // if we have a debounce value > 1/2 DEBOUNCE_MAX then consider the button currently pressed
  // this technique handles debounce well enough, but it's imprecise. how well it performs 
  // depends on how often the buttons are checked and how big DEBOUNCE_MAX is. i picked 64
  // for DEBOUNCE_MAX out of thin air and it seems to work. you can tweak this value yourself
  // as needed.
  //
  // or just write a proper debounce mitigation routine
  //
  // or use a button library that has proper debounce mitigation already baked in and is well
  // developed.
  for(i=0;i<NUMLEDS;i++) {
    if (digitalRead(button_pins[i]) == LOW) {
      if (debounce[i] < DEBOUNCE_MAX) {
        debounce[i]++;
      }
    } else {
      if (debounce[i] > 0) {
        debounce[i]--;
      }
    }
    if (debounce[i] > (DEBOUNCE_MAX >> 1)) {    // 1/2 DEBOUNCE_MAX
      current_button_press = i + 1;
      break;
    }
  }

  // game management
  switch(game_state) {

    // wait for a key press selecting the difficulty to start to the next game
    case GAME_OVER:
      if (current_button_press != 0) {
        difficulty = current_button_press - 1;
        game_state = GAME_INIT;
      }
      break;

    // generate the game key and start the game
    case GAME_INIT:

      // wait until the difficulty button press is released
      if (current_button_press == 0) {

        // seed the RNG
        randomSeed(micros());

        // generate the game key
        for(i=0;i<difficulty_level_count[difficulty];i++) {
          game_key[i] = random(NUMLEDS);
        }

        // initialize variables for a new game
        current_level = 0;
        listen_level = 0;
        last_button_press = 0;
        game_state = SAY;
      }
      break;

    // play the current sequence
    case SAY:

      // wait before playback
      delay(500);

      // play all the nodes for the current level
      for(i=0; i<=current_level; i++) {

        // an attempt at crafting a formula to speed up the game as it progresses
        play_node(game_key[i], 1000 - ((current_level >> 2) * 160));
        delay(250);
      }

      // change to LISTEN state
      game_state = LISTEN;

      // reset last_button_press
      last_button_press = 0;

      // start a timeout timer
      play_timeout = millis() + 5000;
      break;

    // listen for the user's sequence
    case LISTEN:

      // wait until the button is released
      if (current_button_press != 0) {

        // this little block of code allows the LEDs to light and tones to sound
        // as buttons are pressed and released
        if (current_button_press != last_button_press) {
          if (last_button_press > 0) {
            stop_node(last_button_press - 1);
          }
          start_node(current_button_press - 1);
        }
        last_button_press = current_button_press;

      // button has been released, evaluate
      } else if (last_button_press != 0) {

        // stop the node from playing before we evaluate the keypress
        stop_node(last_button_press - 1);

        // check if correct button was pressed
        if (game_key[listen_level] == (last_button_press - 1)) {

          // time to add a new level?
          if (listen_level == current_level) {
            current_level++;

            // have we reached the end of the game?
            if (current_level == difficulty_level_count[difficulty]) {
              game_state = WIN;
  
            // not yet, move to next level
            } else {
              listen_level = 0;
              game_state = SAY;
            }
          } else {
            listen_level++;
          }

        // wrong button pressed, you lose
        } else {
          game_state = LOSE;
        }

        // reset last_button_press
        last_button_press = 0;

      // waiting for a button to be pressed
      } else {

        // if they take too long, the game is over
        if (play_timeout < millis()) {
          game_state = LOSE;
        }
      }
      break;

    // put on a show
    case WIN:
      for (i=0;i<4;i++) {
        // light 'em if you got 'em
        for(j=0;j<NUMLEDS;j++) {
          leds.setPixelColor(j, led_colors[j]);
        }
        SHOW_LEDS();  
        delay(500);

        // blink
        leds.clear();
        SHOW_LEDS();  
        delay(500);
      }
      game_state = GAME_OVER;
      break;

    // try again buddy
    case LOSE:
      delay(250);

      // show the correct node
      leds.clear();
      leds.setPixelColor(game_key[listen_level], led_colors[game_key[listen_level]]);
      SHOW_LEDS();  

      // play the error tone while showing the correct node
      tone(BUZZER_PIN, tones[NUMLEDS]);

      // hold 2 seconds
      delay(2000);

      // clear the board
      leds.clear();
      SHOW_LEDS();
      noTone();

      // reset the game
      game_state = GAME_OVER;
      break;
  }
}

void intro() {
  uint8_t i;

  // turn on all LEDs and wait 2 seconds
  for(i=0;i<NUMLEDS;i++) {
    leds.setPixelColor(i, led_colors[i]);
  }
  SHOW_LEDS();
  delay(2000);
  leds.clear();
  SHOW_LEDS();

  // pause for a half second
  delay(500);

  // play tones for each color
  for (i=0;i<NUMLEDS;i++) {
    play_node(i, 500);
  }
  delay(500);

  // dramatic ending
  tone(BUZZER_PIN, tones[NUMLEDS]);
  delay(2000);
  noTone();
}

void setup() {
  uint8_t i;

  // initialize LEDs
  pinMode(LED_DATA_PIN, OUTPUT);
  leds.clear();
  SHOW_LEDS();

  // initialize buttons
  for(i=0;i<NUMLEDS;i++) {
    pinMode(button_pins[i], INPUT_PULLUP);
  }

  // initialize buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // just a little display on power up
  intro();
}

void loop() {
  game();
}
