/*	Author: Nathan Lee
 *  Partner(s) Name: none
 *	Lab Section: 022
 *	Assignment: Lab #10  Exercise #4
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo Link: https://drive.google.com/drive/folders/10-GIz9jdI6ETzrDHKwmD5hs8ZIrnGecr?usp=sharing
 *
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0; 
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn(){
    TCCR1B = 0x0B;
    OCR1A = 125;
    TIMSK1 = 0x02;
    TCNT1 = 0;
    _avr_timer_cntcurr = _avr_timer_M;
    SREG |= 0x80;
}
void TimerOff(){
    TCCR1B = 0x00;
}
void TimerISR(){
    TimerFlag = 1;
}
ISR(TIMER1_COMPA_vect){
    _avr_timer_cntcurr--;
    if(_avr_timer_cntcurr == 0){
        TimerISR();
        _avr_timer_cntcurr = _avr_timer_M;
    }
}
void TimerSet(unsigned long M){
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}

enum ThreeLEDsSM{TStart, T0, T1, T2} Tstate;
unsigned char threeLEDs = 0x00;

void ThreeLEDsSM_Tick(){
    switch(Tstate){
        case TStart:
            //threeLEDs = 0x00;
            Tstate = T0;
            break;
        case T0:
            Tstate = T1;
            break;
        case T1:
            Tstate = T2;
            break;
        case T2:
            Tstate = T0;
            break;
        default:
        break;
    }
    switch(Tstate){
        case T0:
            threeLEDs = 0x01;
            break;
        case T1:
        case T2:
            threeLEDs = threeLEDs << 1;
            break;
        default:
        break;
    }
}

enum BlinkingLEDSM{BStart, B0, B1} Bstate;
unsigned char blinkingLED = 0x00;

void BlinkingLEDSM_Tick(){
    switch(Bstate){
        case BStart:
            blinkingLED = 0x00;
            Bstate = B0;
            break;
        case B0:
            Bstate = B1;
            break;
        case B1:
            Bstate = B0;
            break;
        default:
        break;
    }
    switch(Bstate){
        case B0:
            blinkingLED = 0x00;
            break;
        case B1:
            blinkingLED = 0x08;
            break;
        default:
        break;
    }
}

enum SoundSM{SStart, Soff, Son} Sstate;
unsigned char tmpA = 0x00;
unsigned char sound = 0x00;

void SoundSM_Tick(){
    switch(Sstate){
        case SStart:
            sound = 0;
            Sstate = Soff;
            break;
        case Soff:
            sound = 0;
            if(tmpA == 0x04){
                sound = 0x10;
                Sstate = Son;
            }
            else{
                Sstate = Soff;
            }
            break;
        case Son:
            sound = 0x10;
            Sstate = Soff;
            break;
        default:
        break;
    }
    switch(Sstate){
        case SStart:
            sound = 0;
            break;
        case Soff:
            sound = 0;
            break;
        case Son:
            sound = 0x10;
            break;
        default:
        break;
    }
}

enum FreqSM{FStart, Fup_press, Fdown_press, Fwait} Fstate;
unsigned char frequency = 0x01;

void FreqSM_Tick(){
    switch(Fstate){
        case FStart:
            frequency = 0x01;
            Fstate = Fwait;
            break;
        case Fwait:
            if((tmpA&0x3) == 0){
                Fstate = Fwait;
            }
            else if((tmpA & 0x3) == 0x01){
                if(frequency < 25){frequency = frequency + 1;}
                Fstate = Fup_press;
            }
            else if((tmpA&0x3) == 0x02){
                if(frequency > 1){frequency = frequency - 1;}
                Fstate = Fdown_press;
            }
            else{
                Fstate = Fwait;
            }
            break;
        case Fup_press:
            if((tmpA&0x3) == 0){
                Fstate = Fwait;
            }
            else if((tmpA&0x3) == 0x01){
                Fstate = Fup_press;
            }
            else if((tmpA&0x3) == 0x02){
                if(frequency > 1){frequency = frequency - 1;}
                Fstate = Fdown_press;
            }
            else{
                Fstate = Fwait;
            }
            break;
        case Fdown_press:
            if((tmpA&0x3) == 0){
                Fstate = Fwait;
            }
            else if((tmpA&0x3) == 0x01){
                if(frequency < 25){frequency = frequency + 1;}
                Fstate = Fup_press;
            }
            else if((tmpA&0x3) == 0x02){
                Fstate = Fdown_press;
            }
            else{
                Fstate = Fwait;
            }
            break;
        default:
        break;
    }
}

enum CombineLEDsSM{CStart, Cout} Cstate;

void CombineLEDsSM_Tick(){
    switch(Cstate){
        case CStart:
            Cstate = Cout;
            break;
        case Cout:
            Cstate = Cout;
            break;
        default:
        break;
    }

    switch(Cstate){
        case Cout:
             PORTB = (blinkingLED | threeLEDs) | sound;
             //(blinkingLED | threeLEDs)
        default:
        break;
    }
}


int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB = 0xFF; PORTB = 0x00;
    DDRA = 0xFF; PORTA = 0xFF;

    /* Insert your solution below */
    unsigned long BL_elapsedTime = 300;
    unsigned long TL_elapsedTime = 1000;
    unsigned char S_elapsedTime = 1;
    unsigned char F_elapsedTime = 1;
    const unsigned long timerPeriod = 1;

    unsigned char update = 0;

    Tstate = TStart;
    Bstate = BStart;
    Cstate = CStart;
    Sstate = SStart;
    Fstate = FStart;

    TimerSet(timerPeriod);
    TimerOn();
    while (1) {
        tmpA = ~PINA;
        if(F_elapsedTime >= 100){
            FreqSM_Tick();
            F_elapsedTime = 0;
            update = 0x01;
        }
        if(S_elapsedTime >= frequency){
            SoundSM_Tick();
            S_elapsedTime = 0;
            update = 0x01;
        }
        if (TL_elapsedTime >= 300) {
            ThreeLEDsSM_Tick();
            TL_elapsedTime = 0;
            update = 0x01;
        }
        if (BL_elapsedTime >= 1000) {
            BlinkingLEDSM_Tick();
            BL_elapsedTime = 0;
            update = 0x01;
        }
        
        if (update != 0) {
                CombineLEDsSM_Tick();
        }

        update = 0x00;
        while(!TimerFlag){}
        TimerFlag = 0;
        BL_elapsedTime += timerPeriod;
        TL_elapsedTime += timerPeriod;
        S_elapsedTime += timerPeriod;
        F_elapsedTime += timerPeriod;
    }
    return 1;
}
