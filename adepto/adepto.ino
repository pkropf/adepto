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
unsigned long sequence_last_step = millis();    // when was the last time a step was executed

int sw1_pin = 12;                      // input switch 1
int sw2_pin = 13;                      // input switch 2

const int relay_count = 8;                   // number of relays connected
int relay[relay_count] = {3, 4, 5, 6, 7, 8, 9, 10}; // relay pins

char relay_steps[relay_count][1024];

int single_current = 0;                 // next relay to be tripped with switch 1
int single_active = false;              // is a single pulse running?
int single_pause = 250;                 // number of milliseconds to wait between switch trips
int single_release = true;              // was the switch been released after being pressed
int single_duration = 62;               // number of milliseconds the single solenoid is to remain open
unsigned long single_last_step = millis();

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

  if (((now - apex_change) > done_pulsing) && ! apex_checked) { // no pulses for more than 1/10 second

    if (pulses == 1 * pulses_per_dollar) {           // $1
      dollar = 1;
      sequence_active = true;

    } else {

      if (pulses == 5 * pulses_per_dollar) {         // $5
        dollar = 5;
        sequence_active = true;

      } else {

        if (pulses == 10 * pulses_per_dollar) {      // $10
          dollar = 10;
          sequence_active = true;

        } else {

          if (pulses == 20 * pulses_per_dollar) {    // $20
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


void setup_steps()
{
  //random(0, max - 1);
}


void check_sequence_switch()
{
  if (sequence_active) {
    if (digitalRead(sw2_pin) == LOW) {

      setup_steps();

      if (dollar == 1) {

        steps = s1steps;
        current_step = 0;

      } else {
        if (dollar == 5) {
          
          steps = s5steps;
          current_step = 0;

        } else {
          if (dollar == 10) {
            
            steps = s10steps;
            current_step = 0;

          } else {
            if (dollar == 20) {
              
              steps = s20steps;
              current_step = 0;

            }
          }
        }
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
      if ((now - sequence_last_step) > quantum) {

        for (int i = 0; i < relay_count; i++) {
          digitalWrite(relay[i], relay_steps[i][current_step]);
        }

        current_step += 1;
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
      single_last_step = millis();
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
    if ((now - single_last_step) > quantum) {
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
  //Serial.println("loop");
  check_single_switch();
  single_step();

  check_sequence_switch();
  count_the_money();
  //run_sequence();
}

