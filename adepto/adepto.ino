// Copyright (c) 2012 Peter Kropf. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


// Determine how much money was read by a Pyramid Technologies APEX-5400 series 
// bill acceptor.

// The bill acceptor should be configured to communicate by sending 10 pulses
// for every $1. The pulses should be set to fast - 50ms on, 50ms off.


int quantum = 62;      // milliseconds per step, 1/8 second

int s1steps  =  1 * (1000 / quantum);       // $1 - 1 second
int s5steps  =  3 * (1000 / quantum);       // $5 - 3 seconds
int s10steps =  9 * (1000 / quantum);       // $10 - 9 seconds
int s20steps = 18 * (1000 / quantum);       // $20 - 18 seconds

int steps = 0;                         // number of steps to work through
int current_step = 0;                  // number of steps left
unsigned long sequence_last_tick = millis();    // when was the last time a step was executed

int sw1_pin = 12;                      // input switch 1 - trigger the next relay for each button press
int sw2_pin = 11;                      // input switch 2 - run a sequence

const int relay_count = 8;                   // number of relays connected
int relay[relay_count] = {3, 4, 5, 6, 7, 8, 9, 10}; // relay pins

int single_current = 0;                 // next relay to be tripped with switch 1
int single_active = false;              // is a single pulse running?
int single_pause = 250;                 // number of milliseconds to wait between switch trips
int single_release = true;              // was the switch been released after being pressed
int single_duration = 62;               // number of milliseconds the single solenoid is to remain open
unsigned long single_last_tick = millis();

int apex_pin = 2;                      // what pin is the apex bill reader connect to?
int apex_interrupt = 0;                // interrupt to use when sensing a pulse
unsigned long apex_change = millis();  // when was the last time a pulse was received?

int dollar = 0;                        // what is the dollar amount that was read?
int pulses = 0;                        // counting the pulses sent
int pulses_per_dollar = 1;             // how many pulses are sent per dollar
int done_pulsing = 20 * pulses_per_dollar * 100;
                                       // how many milliseconds after the last pulse to 
                                       // consider the bill reading done
int sequence_active = false;
int apex_checked = true;

const int max_steps = 128;
char cr1[max_steps];
char cr2[max_steps];
char cr3[max_steps];
char cr4[max_steps];
char cr5[max_steps];
char cr6[max_steps];
char cr7[max_steps];
char cr8[max_steps];


const int steps1 = 9;
//                          
//                             1234567890
PROGMEM prog_char p1r1[]    = "100000000";
PROGMEM prog_char p1r2[]    = "010000000";
PROGMEM prog_char p1r3[]    = "001000000";
PROGMEM prog_char p1r4[]    = "000100000";
PROGMEM prog_char p1r5[]    = "000010000";
PROGMEM prog_char p1r6[]    = "000001000";
PROGMEM prog_char p1r7[]    = "000000100";
PROGMEM prog_char p1r8[]    = "000000010";

PROGMEM char *p1[] = {
  p1r1,
  p1r2,
  p1r3,
  p1r4,
  p1r5,
  p1r6,
  p1r7,
  p1r8,
};


const int steps2 = 21;
//                                      1         2         3
//                             123456789012345678901234567890123
PROGMEM prog_char p2r1[]    = "000000010000100000000";
PROGMEM prog_char p2r2[]    = "000000100000010000000";
PROGMEM prog_char p2r3[]    = "000001000000001000000";
PROGMEM prog_char p2r4[]    = "000010000000000100000";
PROGMEM prog_char p2r5[]    = "000100000000000010000";
PROGMEM prog_char p2r6[]    = "001000000000000001000";
PROGMEM prog_char p2r7[]    = "010000000000000000100";
PROGMEM prog_char p2r8[]    = "100000000000000000010";

PROGMEM char *p2[] = {
  p2r1,
  p2r2,
  p2r3,
  p2r4,
  p2r5,
  p2r6,
  p2r7,
  p2r8,
};


const int steps3 = 37;
//                                      1         2         3
//                             12345678901234567890123456789012345678
//                              12345678901234 123456789 123456
PROGMEM prog_char p3r1[]    = "1000000000000001000000000100000011110";
PROGMEM prog_char p3r2[]    = "1000000000000001000000000100000011110";
PROGMEM prog_char p3r3[]    = "1000000000000001000000000100000011110";
PROGMEM prog_char p3r4[]    = "1000000000000001000000000100000011110";
PROGMEM prog_char p3r5[]    = "1000000000000001000000000100000011110";
PROGMEM prog_char p3r6[]    = "1000000000000001000000000100000011110";
PROGMEM prog_char p3r7[]    = "1000000000000001000000000100000011110";
PROGMEM prog_char p3r8[]    = "1000000000000001000000000100000011110";

PROGMEM char *p3[] = {
  p3r1,
  p3r2,
  p3r3,
  p3r4,
  p3r5,
  p3r6,
  p3r7,
  p3r8,
};


const int steps4 = 77;
//                                      1         2         3         4         5         6         7         8
//                             12345678901234567890123456789012345678901234567890123456789012345678901234567890
PROGMEM prog_char p4r1[]    = "11000000000000000000000000000011001111000000000000000000000001111111111111110";
PROGMEM prog_char p4r2[]    = "00000110000000000000000000011000001111000000000000000000000001111111111111110";
PROGMEM prog_char p4r3[]    = "00000000000110000000000011000000001111000000000000000000000001111111111111110";
PROGMEM prog_char p4r4[]    = "00000000000000000110011000000000001111000000000000000000000001111111111111110";
PROGMEM prog_char p4r5[]    = "00000000000000000110011000000000000000000000111100000000000001111111111111110";
PROGMEM prog_char p4r6[]    = "00000000000110000000000011000000000000000000111100000000000001111111111111110";
PROGMEM prog_char p4r7[]    = "00000110000000000000000000011000000000000000111100000000000001111111111111110";
PROGMEM prog_char p4r8[]    = "11000000000000000000000000000000000000000000111100000000000001111111111111110";

PROGMEM char *p4[] = {
  p4r1,
  p4r2,
  p4r3,
  p4r4,
  p4r5,
  p4r6,
  p4r7,
  p4r8,
};


void setup()
{
  Serial.begin(57600);                    // setup the serial port for communications with the host computer
  //Serial.println("setup");
  randomSeed(analogRead(0));

  attachInterrupt(apex_interrupt, count_pulses, CHANGE);

  pinMode(sw1_pin, INPUT);
  pinMode(sw2_pin, INPUT);

  for (int i = 0; i < relay_count; i++) {
    pinMode(relay[i], OUTPUT);
  }
}


void count_pulses()
{
  int val = digitalRead(apex_pin);
  apex_checked = false;

  if (val == HIGH) {
    apex_change = millis();
    pulses += 1;
  }
}


void count_the_money()
{
  unsigned long int now = millis();

  switch (random(1, 5)) {
  case 1:
    dollar = 1;
    break;
  case 2:
    dollar = 5;
    break;
  case 3:
    dollar = 10;
    break;
  case 4:
    dollar = 20;
    break;
  }
  sequence_active = true;

  if (((now - apex_change) > done_pulsing) && ! apex_checked) { // no pulses for more than 1/10 second

    if (pulses == 1 * pulses_per_dollar) {           // $1
      //Serial.println("dollar: 1");
      dollar = 1;
      sequence_active = true;

    } else {

      if (pulses == 5 * pulses_per_dollar) {         // $5
        //Serial.println("dollar: 5");
        dollar = 5;
        sequence_active = true;

      } else {

        if (pulses == 10 * pulses_per_dollar) {      // $10
          //Serial.println("dollar: 10");
          dollar = 10;
          sequence_active = true;

        } else {

          if (pulses == 20 * pulses_per_dollar) {    // $20
            //Serial.println("dollar: 20");
            dollar = 20;
            sequence_active = true;
          }
        }
      }
    }

    pulses = 0;
    apex_checked = true;
  }
}


void trip(char note, int solenoid)
{
  switch (note) {
  case '0':
    digitalWrite(solenoid, LOW);
    break;

  case '1':
    digitalWrite(solenoid, HIGH);
    break;
  }
}


void setup1()
{
  strcpy_P(cr1, (char *)pgm_read_word(&(p1[0])));
  strcpy_P(cr2, (char *)pgm_read_word(&(p1[1])));
  strcpy_P(cr3, (char *)pgm_read_word(&(p1[2])));
  strcpy_P(cr4, (char *)pgm_read_word(&(p1[3])));
  strcpy_P(cr5, (char *)pgm_read_word(&(p1[4])));
  strcpy_P(cr6, (char *)pgm_read_word(&(p1[5])));
  strcpy_P(cr7, (char *)pgm_read_word(&(p1[6])));
  strcpy_P(cr8, (char *)pgm_read_word(&(p1[7])));
  steps = steps1;
  current_step = 0;
}


void setup5()
{
  strcpy_P(cr1, (char *)pgm_read_word(&(p2[0])));
  strcpy_P(cr2, (char *)pgm_read_word(&(p2[1])));
  strcpy_P(cr3, (char *)pgm_read_word(&(p2[2])));
  strcpy_P(cr4, (char *)pgm_read_word(&(p2[3])));
  strcpy_P(cr5, (char *)pgm_read_word(&(p2[4])));
  strcpy_P(cr6, (char *)pgm_read_word(&(p2[5])));
  strcpy_P(cr7, (char *)pgm_read_word(&(p2[6])));
  strcpy_P(cr8, (char *)pgm_read_word(&(p2[7])));
  steps = steps2;
  current_step = 0;
}


void setup10()
{
  strcpy_P(cr1, (char *)pgm_read_word(&(p3[0])));
  strcpy_P(cr2, (char *)pgm_read_word(&(p3[1])));
  strcpy_P(cr3, (char *)pgm_read_word(&(p3[2])));
  strcpy_P(cr4, (char *)pgm_read_word(&(p3[3])));
  strcpy_P(cr5, (char *)pgm_read_word(&(p3[4])));
  strcpy_P(cr6, (char *)pgm_read_word(&(p3[5])));
  strcpy_P(cr7, (char *)pgm_read_word(&(p3[6])));
  strcpy_P(cr8, (char *)pgm_read_word(&(p3[7])));
  steps = steps3;
  current_step = 0;
}


void setup20()
{
  strcpy_P(cr1, (char *)pgm_read_word(&(p4[0])));
  strcpy_P(cr2, (char *)pgm_read_word(&(p4[1])));
  strcpy_P(cr3, (char *)pgm_read_word(&(p4[2])));
  strcpy_P(cr4, (char *)pgm_read_word(&(p4[3])));
  strcpy_P(cr5, (char *)pgm_read_word(&(p4[4])));
  strcpy_P(cr6, (char *)pgm_read_word(&(p4[5])));
  strcpy_P(cr7, (char *)pgm_read_word(&(p4[6])));
  strcpy_P(cr8, (char *)pgm_read_word(&(p4[7])));
  steps = steps4;
  current_step = 0;
}


void check_sequence_switch()
{
  if (sequence_active) {
    //Serial.println("sequence active");
    if (digitalRead(sw2_pin) == LOW) {
      //Serial.print("sequence switch: ");
      //Serial.print(dollar);
      //Serial.println("");

      switch (dollar) {
      case 1:
        setup1();
        break;

      case 5:
        setup5();
        break;

      case 10:
        setup10();
        break;

      case 20:
        setup20();
        break;
      }

      dollar = 0;
    }
  }
}


void run_sequence()
{
  unsigned long int now = millis();

  if (sequence_active == true) {
    if (current_step < steps) {
      if ((now - sequence_last_tick) > quantum) {

        trip(cr1[current_step],    relay[0]);
        trip(cr2[current_step],    relay[1]);
        trip(cr3[current_step],    relay[2]);
        trip(cr4[current_step],    relay[3]);
        trip(cr5[current_step],    relay[4]);
        trip(cr6[current_step],    relay[5]);
        trip(cr7[current_step],    relay[6]);
        trip(cr8[current_step],    relay[7]);

        //for (int i = 0; i < relay_count; i++) {
        //  trip(relay_steps[i][current_step], relay[i]);
        //}

        current_step += 1;
        sequence_last_tick = now;   // remember when the last quantum happened
      }
    } else {

      sequence_active = false;
    }
  }
}


void check_single_switch()
{
  if (single_active == false && single_release == true) {
    if (digitalRead(sw1_pin) == LOW) {
      //Serial.print("single switch tripped: ");
      //Serial.print(single_current);
      //Serial.println("");
      single_active = true;
      digitalWrite(relay[single_current], HIGH);
      single_last_tick = millis();
      single_release = false;
    }
  } else {
    if (digitalRead(sw1_pin) == HIGH) {
      single_release = true;
    }
  }
}


void single_step()
{
  unsigned long int now = millis();

  if (single_active == true) {
    if ((now - single_last_tick) > quantum) {
      //Serial.println("single switch off");
      digitalWrite(relay[single_current], LOW);
      single_current += 1;
      if (single_current > relay_count) {
        single_current = 0;
      }
      single_active = false;
    }
  }
}


void loop()
{
  //Serial.print("loop: ");
  //Serial.print(pulses);
  //Serial.println("");
  check_single_switch();
  single_step();

  count_the_money();
  check_sequence_switch();
  run_sequence();
}

