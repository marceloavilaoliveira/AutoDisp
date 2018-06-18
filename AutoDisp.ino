//----------------------------------------------------------------------------//
// Filename    : AutoDisp.ino                                                 //
// Description : Automatic Dispenser                                          //
// Version     : 1.1.0                                                        //
// Author      : Marcelo Avila de Oliveira <marceloavilaoliveira@gmail.com>   //
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// DEFINITIONS                                                                //
//----------------------------------------------------------------------------//

// TURN ON DEBUG MODE
// #define DEBUG_TEST
// #define DEBUG_TIMEOUT
// #define DEBUG_PROX

//----------------------------------------------------------------------------//
// LIBRARIES                                                                  //
//----------------------------------------------------------------------------//

// SERVO LIBRARY
#include <Servo.h>

// SOUND LIBRARY
#include <Pitches.h>

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
const int led_r_pin = 5;
const int led_g_pin = 6;
const int led_b_pin = 7;

// POSITIONS
const int body_min = 180;
const int body_max = 0;
const int lever_min = 0;
const int lever_max = 55;

// TIME
const unsigned long timeout_value = 10000;

// MATH
float percent_to_bright_factor = 100 * log10(2) / log10(255);

//----------------------------------------------------------------------------//
// VARIABLES                                                                  //
//----------------------------------------------------------------------------//

// STATUS
boolean lock = true;
boolean front = true;
boolean lever_ok = false;

// TIME
unsigned long timeout = 0;

//----------------------------------------------------------------------------//
// FUNCTIONS (SETTINGS)                                                       //
//----------------------------------------------------------------------------//

void setup() {
    // INITIALIZE PINS
    pinMode(speaker_pin, OUTPUT);
    pinMode(led_r_pin, OUTPUT);
    pinMode(led_g_pin, OUTPUT);
    pinMode(led_b_pin, OUTPUT);
    pinMode(prox_pin, INPUT);

    // INITIATE SERIAL COMMUNICATION
    Serial.begin(9600);

    // INITIATE RANDOM NUMBER GENERATOR
    randomSeed(analogRead(0));

    // RESET
    reset();
}

void reset() {
    // LEDS
    set_leds(2, 25);

    // MOTOR LEVER
    motors_attach_detach(0, 0);
    lever_motor.write(lever_min);
    delay(1000);
    motors_attach_detach(1, 0);

    // MOTOR BODY
    motors_attach_detach(0, 1);
    body_motor.write(body_min);
    delay(1000);
    motors_attach_detach(1, 1);

    // ALERT
    play_tone(3, 3);

    #ifdef DEBUG_TEST
        test();
    #endif
}

void test() {
    play_tone(0, 3);
    delay(1000);
    play_tone(1, 3);
    delay(1000);
    play_tone(2, 3);
    delay(1000);
    play_tone(3, 3);
    delay(1000);
    set_leds(0, 100);
    delay(1000);
    set_leds(1, 100);
    delay(1000);
    set_leds(2, 100);
    delay(1000);
    set_leds(3, 100);
    delay(1000);
    set_leds(4, 100);
    delay(1000);
    set_leds(5, 100);
    delay(1000);
    set_leds(6, 100);
    delay(1000);
    set_leds(2, 100);
    delay(1000);
    move_body(1);
    delay(1000);
    move_lever();
    delay(1000);
    move_body(0);
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
            note = NOTE_FS3;
            break;
        case 1:
            note = NOTE_C5;
            break;
        case 2:
            note = NOTE_FS6;
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
    // 0 = OFF
    // ..
    // 100 = MAX

    if (color < 0 || color > 6 || bright < 0 || bright > 100) {
        return;
    }

    int led_r_bright = 255;
    int led_g_bright = 255;
    int led_b_bright = 255;
    int bright_aux = percent_to_bright(bright);

    switch (color) {
        case 0:
            // GREEN
            led_g_bright = bright_aux;
            break;
        case 1:
            // YELLOW
            led_r_bright = bright_aux;
            led_g_bright = bright_aux;
            break;
        case 2:
            // RED
            led_r_bright = bright_aux;
            break;
        case 3:
            // CYAN
            led_g_bright = bright_aux;
            led_b_bright = bright_aux;
            break;
        case 4:
            // BLUE
            led_b_bright = bright_aux;
            break;
        case 5:
            // MAGENTA
            led_r_bright = bright_aux;
            led_b_bright = bright_aux;
            break;
        case 6:
            // WHITE
            led_r_bright = bright_aux;
            led_g_bright = bright_aux;
            led_b_bright = bright_aux;
            break;
    }

    analogWrite(led_r_pin, led_r_bright);
    analogWrite(led_g_pin, led_g_bright);
    analogWrite(led_b_pin, led_b_bright);

    return;
}

//----------------------------------------------------------------------------//
// FUNCTIONS (CHECK)                                                          //
//----------------------------------------------------------------------------//

void check_timeout() {
    #ifdef DEBUG_TIMEOUT
        Serial.print("timeout = ");
        Serial.println(timeout);
    #endif

    if (! front && millis() > timeout) {
        play_tone(0, 2);
        move_body(0);
    }

    return;
}

void check_prox() {
    int num_check = 5;
    float prox_sensor = 0;
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
        set_timeout();
        if (front) {
            set_leds(0, 100);
            play_tone(3, 1);
            move_body(1);
        } else {
            if (lever_ok) {
                move_lever();
                set_leds(0, 100);
                lever_ok=false;
            } else {
                set_leds(1, 100);
                play_tone(3, 3);
                lever_ok=true;
            }
        }
    } else {
        if (! front) {
            set_leds(0, 100);
            lever_ok=false;
        }
    }
}

//----------------------------------------------------------------------------//
// FUNCTIONS (MOVE)                                                           //
//----------------------------------------------------------------------------//

void move(int step, int time_step, int time_end, int pos1, int new_pos1, int pos2, int new_pos2) {
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
                pos1 += step;
              } else {
                pos1 -= step;
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
                pos2 += step;
              } else {
                pos2 -= step;
              }
              body_motor.write(pos2);
              moved = true;
            }
        }

        if (moved) {
            delay(time_step);
            moved = false;
        }
    } while (continue1 || continue2);

    delay(time_end);

    motors_attach_detach(1, 0);
    motors_attach_detach(1, 1);
}

void move_body(int pos) {
    // POS:
    // 0 = FRONT
    // 1 = BACK

    if (pos == 0) {
        front=true;
        move(5, 25, 200, 999, 999, body_max, body_min);
    } else {
        front=false;
        motors_attach_detach(0, 1);
        body_motor.write(body_max);
        delay(1500);
        motors_attach_detach(1, 1);
    }
}

void move_lever() {
    motors_attach_detach(0, 0);
    lever_motor.write(lever_max);
    delay(300);
    lever_motor.write(lever_min);
    delay(1500);
    motors_attach_detach(1, 0);
    delay(1000);
}

//----------------------------------------------------------------------------//
// MIS                                                                        //
//----------------------------------------------------------------------------//

void set_timeout() {
    timeout = millis() + timeout_value;
}

//----------------------------------------------------------------------------//
// MAIN                                                                       //
//----------------------------------------------------------------------------//

void loop() {
    check_timeout();
    check_prox();
}

