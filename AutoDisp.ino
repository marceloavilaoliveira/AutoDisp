//----------------------------------------------------------------------------//
// Filename    : AutoDisp.ino                                                 //
// Description : An Automatic M&Ms Dispenser                                  //
// Version     : 1.0.0                                                        //
// Author      : Marcelo Avila de Oliveira <marceloavilaoliveira@gmail.com>   //
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// DEFINITIONS                                                                //
//----------------------------------------------------------------------------//

// TURN ON DEBUG MODE
// #define DEBUG_BUTTON
// #define DEBUG_NFC
// #define DEBUG_PROX

//----------------------------------------------------------------------------//
// LIBRARIES                                                                  //
//----------------------------------------------------------------------------//

// SERVO LIBRARY
#include <Servo.h>

// SOUND LIBRARY
#include <Pitches.h>

// NFC LIBRARIES
#include <SPI.h>
#include "PN532_SPI.h"
#include "PN532.h"
#include "NfcAdapter.h"
PN532_SPI interface(SPI, 10);
NfcAdapter nfc = NfcAdapter(interface);

//----------------------------------------------------------------------------//
// CONSTANTS                                                                  //
//----------------------------------------------------------------------------//

// MOTORS
Servo lever_motor;
Servo body_motor;

// PINS
const int body_pin = 9;
const int lever_pin = 3;
const int speaker_pin = 4;
const int prox_pin = A0;
const int button_pin = 2;
const int led_r_pin = 7;
const int led_g_pin = 5;
const int led_b_pin = 6;

// POSITIONS
const int body_min = 180;
const int body_max = 0;
const int lever_min = 0;
const int lever_max = 100;

// NFC
String const nfc_id = "EB C8 66 D6";

// MISCELANEOUS
const int motor_step = 15;

// TIME
const unsigned long timeout = 10000;

// MATH
float percent_to_bright_factor = 100 * log10(2) / log10(255);

// SOUND
const int sound_num[3] = { 20, 18, 64 };

const int sound_notes[3][64] = {
    {
        // STAR WARS
        NOTE_D3, NOTE_D3, NOTE_D3, NOTE_G3, NOTE_D4, NOTE_C4, NOTE_B3, NOTE_A3,
        NOTE_G4, NOTE_D4, NOTE_C4, NOTE_B3, NOTE_A3, NOTE_G4, NOTE_D4, NOTE_C4,
        NOTE_B3, NOTE_C4, NOTE_A3, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        // IMPERIAL MARCH
        NOTE_A3, NOTE_A3, NOTE_A3, NOTE_F3, NOTE_C4, NOTE_A3, NOTE_F3, NOTE_C4,
        NOTE_A3, NOTE_E4, NOTE_E4, NOTE_E4, NOTE_F4, NOTE_C4, NOTE_GS3, NOTE_F3,
        NOTE_C4, NOTE_A3, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        // MARIO BROS
        NOTE_E7, NOTE_E7, 0, NOTE_E7, 0, NOTE_C7, NOTE_E7, 0,
        NOTE_G7, 0, 0,  0, NOTE_G6, 0, 0, 0, 
        NOTE_C7, 0, 0, NOTE_G6, 0, 0, NOTE_E6, 0,
        0, NOTE_A6, 0, NOTE_B6, 0, NOTE_AS6, NOTE_A6, 0, 
        NOTE_G6, NOTE_E7, NOTE_G7, NOTE_A7, 0, NOTE_F7, NOTE_G7, 0,
        NOTE_E7, 0, NOTE_C7, NOTE_D7, NOTE_B6, 0, 0, NOTE_C7,
        0, 0, NOTE_G6, 0, 0, NOTE_E6, 0, 0,
        NOTE_A6, 0, NOTE_B6, 0, NOTE_AS6, NOTE_A6, 0, NOTE_G6
    },
};

const int sound_durat[3][64] = {
    {
        // STAR WARS
        10, 10, 10, 2, 2, 10, 10, 10,
        2, 4, 10, 10, 10, 2, 4, 10,
        10, 10, 2, 4, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        // IMPERIAL MARCH
        3, 3, 3, 6, 8, 3, 6, 8,
        2, 3, 3, 3, 6, 8, 3, 6,
        8, 2, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        // MARIO BROS
        12, 12, 12, 12, 12, 12, 12, 12,
        12, 12, 12, 12, 12, 12, 12, 12, 
        12, 12, 12, 12, 12, 12, 12, 12, 
        12, 12, 12, 12, 12, 12, 12, 12, 
        9, 9, 9, 12, 12, 12, 12, 12,
        12, 12, 12, 12, 12, 12, 12, 12,
        12, 12, 12, 12, 12, 12, 12, 12,
        12, 12, 12, 12, 12, 12, 12, 9
    },
};

//----------------------------------------------------------------------------//
// VARIABLES                                                                  //
//----------------------------------------------------------------------------//

// STATUS
int lever_pos = lever_min;
int body_pos = body_min;
boolean button_state = HIGH;
boolean button_state_prev = HIGH;
float prox_sensor;
// 0 = SLEEPING
// 1 = WAITING
// 2 = AUTHORIZED
int state = 0;

//----------------------------------------------------------------------------//
// FUNCTIONS (SETTINGS)                                                       //
//----------------------------------------------------------------------------//

void setup() {
    // INITIALIZE PINS
    pinMode(speaker_pin, OUTPUT);
    pinMode(button_pin, INPUT);
    pinMode(led_r_pin, OUTPUT);
    pinMode(led_g_pin, OUTPUT);
    pinMode(led_b_pin, OUTPUT);
    pinMode(prox_pin, INPUT);

    // INITIATE SERIAL COMMUNICATION
    Serial.begin(9600);

    // INITIATE NFC COMMUNICATION
    nfc.begin();

    // INITIATE RANDOM NUMBER GENERATOR
    randomSeed(analogRead(0));

    // RESET
    reset();
}

void reset() {
    set_leds(4, 0);
    reset_motors();

    return;
}

void motors_attach_detach(int mode, int num) {
    // MODE:
    // 0 = ATTACH
    // 1 = DETACH
    //
    // NUM:
    // 0 = LEVER
    // 1 = BODY
    if (mode == 0) {
        if (num == 0) {
            lever_motor.attach(lever_pin);
        } else {
            body_motor.attach(body_pin);
        }
    } else {
        if (num == 0) {
            lever_motor.detach();
        } else {
            body_motor.detach();
        }
    }
}
//----------------------------------------------------------------------------//
// FUNCTIONS (SOUND)                                                          //
//----------------------------------------------------------------------------//

void play_tone(int note, int duration) {
    // NOTE:
    // 0 = BASS
    // 1 = MID-BASS
    // 2 = MID-TREBLE
    // 3 = TREBLE
    //
    // DURATION:
    // 0 = SHORT
    // 1 = MID-SHORT
    // 2 = MID-LONG
    // 3 = LONG

    if (note < 0 || note > 3 || duration < 0 || duration > 3) {
        return;
    }

    switch (note) {
        case 0:
            note = NOTE_C2;
            break;
        case 1:
            note = NOTE_C4;
            break;
        case 2:
            note = NOTE_C6;
            break;
        case 3:
            note = NOTE_C8;
            break;
    }

    switch (duration) {
        case 0:
            duration = 100;
            break;
        case 1:
            duration = 200;
            break;
        case 2:
            duration = 400;
            break;
        case 3:
            duration = 800;
            break;
    }

    tone(speaker_pin, note, duration);
    delay(duration);
    noTone(speaker_pin);

    return;
}

void play_sound(int sound) {
    // SOUND:
    // 0 = STAR WARS
    // 1 = IMPERIAL MARCH
    // 2 = MARIO BROS

    if (sound < 0 || sound > 2) {
        return;
    }

    for (int thisNote = 0; thisNote < sound_num[sound]; thisNote++) {
        // to calculate the note duration, take one second 
        // divided by the note type.
        // e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
        int noteDuration = 1000/sound_durat[sound][thisNote];
        tone(speaker_pin, sound_notes[sound][thisNote], noteDuration);

        // to distinguish the notes, set a minimum time between them.
        // the note's duration + 30% seems to work well:
        int pauseBetweenNotes = noteDuration * 1.30;

        delay(pauseBetweenNotes);

        // stop the tone playing:
        noTone(speaker_pin);
    }

    return;
}

//----------------------------------------------------------------------------//
// FUNCTIONS (LIGHT)                                                          //
//----------------------------------------------------------------------------//

int percent_to_bright(int percent) {
    // PERCENT:
    // 0..100
    // RETURN BRIGHT
    // 255..0

    return 256 - pow(2, percent / percent_to_bright_factor);
}

void set_leds(int color, int bright) {
    // COLOR:
    // 0 = GREEN
    // 1 = YELLOW  
    // 2 = RED
    // 3 = CYAN
    // 4 = BLUE
    // 5 = MAGENTA
    // 6 = WHITE
    //
    // BRIGHT:
    // 0 = MAX
    // ..
    // 255 = OFF

    if (color < 0 || color > 6 || bright < 0 || bright > 255) {
        return;
    }

    int led_r_bright = 255;
    int led_g_bright = 255;
    int led_b_bright = 255;

    switch (color) {
        case 0:
            // GREEN
            led_g_bright = bright;
            break;
        case 1:
            // YELLOW
            led_r_bright = bright;
            led_g_bright = bright;
            break;
        case 2:
            // RED
            led_r_bright = bright;
            break;
        case 3:
            // CYAN
            led_g_bright = bright;
            led_b_bright = bright;
            break;
        case 4:
            // BLUE
            led_b_bright = bright;
            break;
        case 5:
            // MAGENTA
            led_r_bright = bright;
            led_b_bright = bright;
            break;
        case 6:
            // WHITE
            led_r_bright = bright;
            led_g_bright = bright;
            led_b_bright = bright;
            break;
    }

    analogWrite(led_r_pin, led_r_bright);
    analogWrite(led_g_pin, led_g_bright);
    analogWrite(led_b_pin, led_b_bright);

    return;
}

void sign_leds(int color, int type, int ord) {
    // COLOR:
    // 0 = GREEN
    // 1 = YELLOW  
    // 2 = RED
    // 3 = CYAN
    // 4 = BLUE
    // 5 = MAGENTA
    // 6 = WHITE
    //
    // TYPE:
    // 0 = OK
    // -----------------------|           |-----------------------|
    //                        |           |                       |
    //                        |-----------|                       |
    // 1 = WARNING
    //
    // --------|        |--------|        |--------|        |--------|
    //         |        |        |        |        |        |        |
    //         |--------|        |--------|        |--------|        |
    //
    // 2 = ERROR
    //
    // --|    |--|    |--|    |--|    |--|    |--|    |--|    |--|
    //   |    |  |    |  |    |  |    |  |    |  |    |  |    |  |
    //   |----|  |----|  |----|  |----|  |----|  |----|  |----|  |
    //
    // 3 = SIGN OF LIFE 1
    //
    //     /----\
    //   /        \
    // /            \
    //
    // 4 = SIGN OF LIFE 2
    //
    //   /--\
    //  /    \
    // /      \
    //
    // ORD:
    // 0 = OFF-ON-OFF
    //
    // --|    |- .. -|    |--|      /--\
    //   |    |      |    |  | OR  /    \
    //   |----|      |----|  |    /      \
    //
    // 1 = ON-OFF-ON
    //
    //   |----|      |----|  |    \      /
    //   |    |      |    |  | OR  \    /
    // --|    |- .. -|    |--|      \--/

    if (color < 0 || color > 6 || type < 0 || type > 4 || ord < 0 || ord > 1) {
        return;
    }

    int time_1;
    int time_2;
    int repeat;

    switch (type) {
        case 0:
            // OK - TOTAL TIME = 2 * 600 + 1 * 300 = 1500 OR 2 * 300 + 1 * 600 = 1200
            time_1 = 600;
            time_2 = 300;
            repeat = 2;
            break;
        case 1:
            // WARNING - TOTAL TIME = 7 * 240 = 1680
            time_1 = 240;
            time_2 = 240;
            repeat = 4;
            break;
        case 2:
            // ERROR - TOTAL TIME = 8 * 60 + 7 * 120 = 1320 OR 8 * 120 + 7 * 60 = 1380
            time_1 = 60;
            time_2 = 120;
            repeat = 8;
            break;
        case 3:
            // SIGN OF LIFE 1 - TOTAL TIME = 200 * 12 + 300 = 2700
            time_1 = 12;
            time_2 = 300;
            break;
        case 4:
            // SIGN OF LIFE 2 - TOTAL TIME = 200 * 4 + 100 = 900
            time_1 = 4;
            time_2 = 100;
            break;
    }

    if (type == 3 || type == 4) {
        // SIGN OF LIFE 1 AND 2
        if (ord == 0) {
            for (int percent = 0; percent <= 100; percent++) {
                set_leds(color, percent_to_bright(percent));
                delay(time_1);
            }
        } else {
            for (int percent = 100; percent >= 0; percent--) {
                set_leds(color, percent_to_bright(percent));
                delay(time_1);
            }
        }
        delay(time_2);
        if (ord == 0) {
            for (int percent = 100; percent >= 0; percent--) {
                set_leds(color, percent_to_bright(percent));
                delay(time_1);
            }
        } else {
            for (int percent = 0; percent <= 100; percent++) {
                set_leds(color, percent_to_bright(percent));
                delay(time_1);
            }
        }
    } else {
        // OK, WARNING, ERROR
        if (ord == 0) {
            for (int rep = 0; rep < repeat; rep++) {
                set_leds(color, 0);
                delay(time_1);
                set_leds(6, 255);
                if (rep < repeat - 1) {
                    delay(time_2);
                }
            }
        } else {
            for (int rep = 0; rep < repeat; rep++) {
                set_leds(6, 255);
                delay(time_2);
                set_leds(color, 0);
                if (rep < repeat - 1) {
                    delay(time_1);
                }
            }
        }
    }

    return;
}

//----------------------------------------------------------------------------//
// FUNCTIONS (CHECK)                                                          //
//----------------------------------------------------------------------------//

void check_timeout() {
    // if (millis() > timeout) {
    //     xxxxx;
    // }
    // sign_leds(4, 3, 1);

    return;
}

void check_button() {
    button_state = digitalRead(button_pin);

    #ifdef DEBUG_BUTTON
        Serial.print("button_state = ");
        Serial.println(button_state);
    #endif
    
    if (button_state != button_state_prev) {
        button_state_prev = button_state;

        if (button_state == LOW) {
            play_tone(2, 1);
            set_leds(0, 0);
            delay(500);
            set_leds(1, 0);
            delay(500);
            set_leds(2, 0);
            delay(500);
            set_leds(3, 0);
            delay(500);
            set_leds(4, 0);
            delay(500);
            set_leds(5, 0);
            delay(500);
            set_leds(6, 0);
            delay(500);
            set_leds(0, 0);
            sign_leds(0, 4, 1);
            set_leds(4, 0);
            sign_leds(4, 4, 1);
       }
    }

    return;
}

void check_nfc() {
    if (nfc.tagPresent()) {
        NfcTag tag = nfc.read();

        #ifdef DEBUG_NFC
            tag.print();
        #endif

        String nfc_id_read = tag.getUidString();

        if (nfc_id.compareTo(nfc_id_read) == 0) {
            set_leds(0, 0);
            play_tone(3, 2);
            move_body(1);
        } else {
            play_tone(0, 2);
        }
    }
}

void check_prox() {
    prox_sensor = 0;
    int num_check = 5;
    for (int i = 0; i < num_check; i++)
    {
        prox_sensor = prox_sensor + analogRead(prox_pin);
    }
    prox_sensor = prox_sensor / num_check;

    #ifdef DEBUG_PROX
        Serial.print("prox_sensor = ");
        Serial.println(prox_sensor);
    #endif

    if (prox_sensor > 500) {
        set_leds(1, 0);
        delay(500);
        move_lever();
        set_leds(4, 0);
        delay(1000);
        move_body(0);
    }
}

//----------------------------------------------------------------------------//
// FUNCTIONS (MOVE)                                                           //
//----------------------------------------------------------------------------//

void reset_motors() {
    lever_pos=lever_min;
    body_pos=body_min;

    motors_attach_detach(0, 0);
    lever_motor.write(lever_min);
    delay(500);
    motors_attach_detach(1, 0);

    motors_attach_detach(0, 1);
    body_motor.write(body_min);
    delay(500);
    motors_attach_detach(1, 1);
}

void move(int time, int pos1, int new_pos1, int pos2, int new_pos2) {
    boolean continue1 = true;
    boolean continue2 = true;
    boolean moved = false;

    if (new_pos1 != 999) {
        motors_attach_detach(0, 0);
        lever_motor.write(pos1);
    } else {
        continue1 = false;
    }
    if (new_pos2 != 999) {
        motors_attach_detach(0, 1);
        body_motor.write(pos2);
    } else {
        continue2 = false;
    }

    do {
        if (continue1) {
            if (pos1 == new_pos1) {
              continue1 = false;
            } else {
              if (pos1 < new_pos1) {
                pos1 += motor_step;
              } else {
                pos1 -= motor_step;
              }
              lever_motor.write(pos1);
              moved = true;
            }
        }
        if (continue2) {
            if (pos2 == new_pos2) {
              continue2 = false;
            } else {
              if (pos2 < new_pos2) {
                pos2 += motor_step;
              } else {
                pos2 -= motor_step;
              }
              body_motor.write(pos2);
              moved = true;
            }
        }

        if (moved) {
            delay(time);
            moved = false;
        }
    } while (continue1 || continue2);

    motors_attach_detach(1, 0);
    motors_attach_detach(1, 1);
}

void move_body(int pos) {
    // POS:
    // 0 = FRONT
    // 1 = BACK

//    if (pos == 0) {
//        move(70, 999, 999, body_max, body_min);
//    } else {
//        move(70, 999, 999, body_min, body_max);
//    }

    if (pos == 0) {
        motors_attach_detach(0, 1);
        body_motor.write(body_min);
        delay(600);
        motors_attach_detach(1, 1);
    } else {
        motors_attach_detach(0, 1);
        body_motor.write(body_max);
        delay(600);
        motors_attach_detach(1, 1);
    }
}

void move_lever() {
    motors_attach_detach(0, 0);
    lever_motor.write(lever_max);
    delay(300);
    lever_motor.write(lever_min);
    delay(300);
    motors_attach_detach(1, 0);
}

//----------------------------------------------------------------------------//
// MIS                                                                        //
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
// MAIN                                                                       //
//----------------------------------------------------------------------------//

void loop() {
    check_timeout();
    check_button();
    check_nfc();
    check_prox();
}

