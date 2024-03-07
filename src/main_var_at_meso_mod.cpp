// works

/*

this file:
'phase' plus offset, with freq doubling
preceding pulses can be accomplished by
inverting the channel and setting the offsets



with Carlos wraparound trick

*/

// counting CPU cycles:
// https://www.exploreembedded.com/wiki/LPC1768:_SysTick_Timer


#include <Arduino.h>

const int PIN = 0; // the input pin
const int n_outputs = 2;
const int OUT[n_outputs] = {1,2}; // the output pins

// Systick regs
#define STCTRL      (*( ( volatile u_int32_t *) 0xE000E010 ))
#define STRVR       (*( ( volatile u_int32_t *) 0xE000E014 ))
#define STCVR       (*( ( volatile u_int32_t *) 0xE000E018 ))

#define SBIT_ENABLE     0
#define SBIT_TICKINT    1
#define SBIT_CLKSOURCE  2

const u_int32_t us_per_cycle = 600; // CPU constant

u_int32_t t[n_outputs]  = {0,0};

volatile u_int32_t T = 83*us_per_cycle; // first estimate, period at 12khz in us

const u_int32_t phase[n_outputs] = {15*us_per_cycle, 0*us_per_cycle}; // not really phase but offset
const u_int32_t dur[n_outputs] = {10*us_per_cycle, 10*us_per_cycle}; // duration
const bool is_inverted[n_outputs] = {true, false}; // invert output

// interrupt service routine
volatile u_int32_t t_on[2] = {0,0};
void isr(){
    // Carlos wraparound trick!
    t_on[1] = t_on[0]; // shift
    t_on[0] = STCVR; // t_on[0] is always last trigger
    T =  (STRVR + t_on[1] - t_on[0])%STRVR;
}

void setup() {
    // configuration of CPU cycle counter
    STCTRL = (1<<SBIT_ENABLE) | (1<<SBIT_TICKINT) | (1<<SBIT_CLKSOURCE);
    STCVR = (1<<0); // writing anything resets?
    STRVR = 0x00FFFFFF;

    // set up input
    pinMode(PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN), isr, RISING);

    // set up outputs
    for (int i = 0; i < n_outputs; i++){
        pinMode(OUT[i], OUTPUT);
    }

    // for debug only
    // pinMode(13, OUTPUT); 
    // Serial.begin(115200);
}

void loop() {
    for (int i = 0; i < n_outputs; i++){

        // the time, relative to last trigger, carlos wraparound safe
        u_int32_t t = (t_on[0] - STRVR - STCVR) % STRVR;

        // write output high(low)
        if (( t > phase[i]) && (t < phase[i] + dur[i]) ) {
            if (is_inverted[i] == true){
                digitalWrite(OUT[i], LOW);
            }
            else{
                digitalWrite(OUT[i], HIGH);
            }
        }

        // f-doubling
        else if (( t > T/2 + phase[i]) && (t < T/2 + phase[i] + dur[i]) ) {
            // write output
            if (is_inverted[i] == true){
                digitalWrite(OUT[i], LOW);
            }
            else{
                digitalWrite(OUT[i], HIGH);
            }
        }

        // write output low(high)
        else {
            if (is_inverted[i] == true){
                digitalWrite(OUT[i], HIGH);
            }
            else{
                digitalWrite(OUT[i], LOW);
            }
        }
    }

}
