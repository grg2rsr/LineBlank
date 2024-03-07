// FINAL
// working 2 channel variant with us precision // 

/*

with Carlos wraparound trick!

*/

// counting CPU cycles:

// this didn't work
// https://developer.arm.com/documentation/ka001406/latest

// this did
// https://www.exploreembedded.com/wiki/LPC1768:_SysTick_Timer

// also interesting
// https://faculty-web.msoe.edu/johnsontimoj/EE2920/files2920/arm_m4_peripherals.pdf


#include <Arduino.h>

const int PIN = 0; // the input
const int n_outputs = 2;
const int OUT[n_outputs] = {1,2};

// Systick regs
#define STCTRL      (*( ( volatile unsigned long *) 0xE000E010 ))
#define STRVR       (*( ( volatile unsigned long *) 0xE000E014 ))
#define STCVR       (*( ( volatile unsigned long *) 0xE000E018 ))

#define SBIT_ENABLE     0
#define SBIT_TICKINT    1
#define SBIT_CLKSOURCE  2

u_int32_t t[n_outputs]  = {0,0};
volatile u_int32_t t_on[2] = {0,0};
volatile u_int32_t T = 42; // half period at 12khz in us

const u_int32_t us_per_cycle = 600;
// const u_int32_t pre[n_outputs] = {5*us_per_cycle, 10*us_per_cycle};
// const u_int32_t post[n_outputs] = {5*us_per_cycle, 10*us_per_cycle};

const u_int32_t pre[n_outputs] = {2*us_per_cycle, 2*us_per_cycle};
const u_int32_t post[n_outputs] = {34*us_per_cycle, 34*us_per_cycle};

int LED = 13;
bool state = false;
bool wraparound = false;

// interrupt service routine
void isr(){
    // Carlos wraparound trick!
    t_on[1] = t_on[0];
    t_on[0] = STCVR;
    T =  (STRVR + t_on[1] - t_on[0])%STRVR;
}

void setup() {
    // configuration of CPU cycle counter
    STCTRL = (1<<SBIT_ENABLE) | (1<<SBIT_TICKINT) | (1<<SBIT_CLKSOURCE);
    STCVR = (1<<0); // writing anything resets?
    STRVR = 0x00FFFFFF;

    // set up input
    pinMode(PIN, INPUT);
    pinMode(LED, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(PIN), isr, RISING);

    // set up outputs
    for (int i = 0; i < n_outputs; i++){
        pinMode(OUT[i], OUTPUT);
    }

    // Serial.begin(115200);
}

bool is_high[n_outputs] = {false,false};

void loop() {
    for (int i = 0; i < n_outputs; i++){
        if (((t[i] + STRVR - STCVR)%STRVR > T + pre[i]) && ((t[i] + STRVR - STCVR)%STRVR < T + post[i])) {
        // if ((t[i] - STCVR > T - pre[i]) && (t[i] - STCVR < T + post[i])) {
            digitalWrite(OUT[i], LOW);
            if (is_high[i] == false){
                is_high[i] = true;
            }
        }
        
        else if (((t[i] + STRVR - STCVR)%STRVR > T/2 + pre[i]) && ((t[i] + STRVR - STCVR)%STRVR < T/2 + post[i])) {
        digitalWrite(OUT[i], LOW);
            if (is_high[i] == false){
                is_high[i] = true;
            }
        }

        else {
            digitalWrite(OUT[i], HIGH);
            if (is_high[i] == true){
                is_high[i] = false;
                t[i] = t_on[0];
            }
        }
    }

}