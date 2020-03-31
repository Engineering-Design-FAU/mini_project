#include <msp430.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define RED_LED         BIT3            // Port 2.3
#define GREEN_LED       BIT4            // Port 2.4
#define BUTTON          BIT3            // Port 1.3
#define LIGHT_SENSOR    BIT2            // Port 1.2

int value=0, i=0 ;
int highFlag = 0;
int lowFlag = 0;
int high = 0; // count for high hand
int low = 0; // count for low hand
int clockWisePass [5] = {1,0,1,0,1};
int counterWisePass [5] = {1,0,1,0,0};
int stopPass [5] = {0,0,0,0,0};
int inputPass [5];
int index = 0;
int light = 0, lightroom = 0, dimled=50;
int temp = 0, temproom = 0;
int touch =0, touchroom =0;
int clockFlag = 0;
int counterFlag = 0;
int stopFlag = 0;
int dummy = 0;
int ADCReading [3];

void fadeLED(int valuePWM); //this is from skeleton code, not using it
void ConfigureAdc(void);
void getanalogvalues();

int main(void){
    WDTCTL = WDTPW + WDTHOLD;                   // Stop WDT
    P1OUT = 0;
    P2OUT = 0;
    P1DIR = 0;
    P1REN = 0;
    P2REN = 0;
    P2DIR = 0;
    P2DIR |= GREEN_LED;     // GREEN LED (BIT 0)
    P2DIR |= RED_LED;       // RED LED (BIT 3)
    P1DIR &= ~BUTTON;       //BUTTON IS AN INPUT
    P1OUT |= BUTTON;        // PULL-UP RESISTOR
    P1REN |= BUTTON;        // RESISTOR ENABLED
    P1IFG &= ~BUTTON;       // CLEAR INTERRUPT FLAG
    P1IE |= BUTTON;         // ENABLE BUTTON INTERRUPT

    __enable_interrupt();
    ConfigureAdc();

    // reading the initial room values, lightroom, touchroom, temproom
    // __delay_cycles(250);
    __delay_cycles(25000);
    getanalogvalues();
    lightroom = light;
    //touchroom = touch;
    //temproom = temp;
    __delay_cycles(250);

    __delay_cycles(200);

    for (;;){
        if(index > 4){ //a full password is entered
            clockFlag = 0;
            counterFlag = 0;
            stopFlag = 0;
            for(i=0; i<=4 ; i++){
                if(clockWisePass[i] != inputPass[i]){
                    clockFlag = 1; //if code gets here, then entered password does not match clockwise direction password
                }
            }
            if(clockFlag){
                //passwords did not match for clockwise direction
                //dummy =1;
                //now check for counter clockwise password
                for(i=0; i<=4 ; i++){
                    if(counterWisePass[i] != inputPass[i]){
                        counterFlag = 1; //if code get here, then entered password does not match counter clockwise direction password
                    }
                }
                if(counterFlag){
                    //neither passwords matched
                    //dummy = 3;
                    //check that entered code is not stopPass
                    for(i=0; i<=4 ; i++){
                        if(stopPass[i] != inputPass[i]){
                            stopFlag = 1; //if code gets here, then entered password does not match clockwise direction password
                        }
                    }
                    if(stopFlag){
                        //No passwords matched
                        //flash both LEDS
                        P2OUT |= GREEN_LED; //Turn on Green
                        P2OUT |= RED_LED; //Turn on Red
                        __delay_cycles(250000);
                        P2OUT &= ~GREEN_LED; //Turn off Green
                        P2OUT &= ~RED_LED; //Turn off Red
                    } else{
                        //stopPass matched
                        dummy = 5; //so that motor stops
                        P2OUT &= ~GREEN_LED; //Turn off Green
                    }
                } else{
                    //passwords matched for counter clockwise direction
                    //Flash Green LED slow constantly
                    //Make motor spin in counter clockwise direction
                    dummy = 2;
                }
            } else{
                //passwords matched for clockwise direction
                //Flash Green LED fast constantly
                //Make motor spin in clockwise direction
                dummy = 4;
            }
            index = 0; //reset index to enter password again
        }
        //according to dummy value do something
        if(dummy == 2){
            //passwords matched for counter clockwise direction
            //Flash Green LED slow constantly
            P2OUT |= GREEN_LED; //Turn on Green
            __delay_cycles(250000);
            P2OUT &= ~GREEN_LED; //Turn off Green
            //Make motor spin in counter clockwise direction
        } else if(dummy == 4){
            //passwords matched for clockwise direction
            //Flash Green LED fast constantly
            P2OUT |= GREEN_LED; //Turn on Green
            __delay_cycles(50000);
            P2OUT &= ~GREEN_LED; //Turn off Green
            //Make motor spin in clockwise direction
        }
        //reading light repeatedly at the beginning of the main loop
        getanalogvalues();
        //Observe the dead zone for no action to avoid flickering
        //I chose the range 1.05 to 1.10 of the value; that is no action if  (1.05 lightroom < light < 1.1 lightroom)
        if(light < lightroom * 1.10 && light > lightroom * 1.05) {}
        else{
            if(light >= lightroom * 1.10) { // if light value is 10% greater than initial value take another measurement to make sure it is high or low
                getanalogvalues(); //get another measurement to be sure and avoid a High measurement before every Low measurement
                if(light >= lightroom * 1.30){
                    //P2OUT |= GREEN_LED; //Turn on Green
                    // P2OUT &= ~RED_LED; //Turn off Red
                    __delay_cycles(50000);
                    lowFlag = 1;
                    highFlag = 0;
                } else if(light >= lightroom * 1.10){
                    // P2OUT |= RED_LED; //Turn on Red
                    // P2OUT &= ~GREEN_LED; //Turn off Green
                    __delay_cycles(50000);
                    highFlag = 1;
                    lowFlag = 0;
                }
            }
            if(light <= lightroom * 1.10) {
                P2OUT &= ~RED_LED;
                P2OUT &= ~GREEN_LED;
                __delay_cycles(200);
                //check flags and log corresponding password digit. This is done here so that we don't get multiple unintended entries
                if(lowFlag){
                    //BLINK RED LED ONCE SLOW
                    P2OUT |= RED_LED; //Turn on Red
                    __delay_cycles(250000);
                    P2OUT &= ~RED_LED; //Turn off Red
                    low += 1;
                        inputPass[index] = 0;
                        index +=1; //update index
                } else if(highFlag){
                    //BLINK RED LED TWICE FAST
                    P2OUT |= RED_LED; //Turn on Red
                    __delay_cycles(50000);
                    P2OUT &= ~RED_LED; //Turn off Red
                    __delay_cycles(50000);
                    P2OUT |= RED_LED; //Turn on Red
                    __delay_cycles(50000);
                    P2OUT &= ~RED_LED; //Turn off Red
                    high += 1;
                    inputPass[index] = 1;
                    index +=1; //update index
                }
                //reset flags
                lowFlag = 0;
                highFlag = 0;
            }
        }
    }
}


void ConfigureAdc(void){
   ADC10CTL1 = INCH_2 | CONSEQ_1;           // A2 + A1 + A0, single sequence
   ADC10CTL0 = ADC10SHT_2 | MSC | ADC10ON;
   while (ADC10CTL1 & BUSY);
   ADC10DTC1 = 0x03;                    // 3 conversions THIS IS FROM SKELETON CODE, MAY NEED TO CHANGE
   ADC10AE0 |= LIGHT_SENSOR;            // ADC10 option select THIS IS FROM SKELETON CODE, MAY NEED TO CHANGE
}

void fadeLED(int valuePWM){
    P1SEL |= (BIT6);                            // P1.0 and P1.6 TA1/2 options
    CCR0 = 100 - 0;                             // PWM Period
    CCTL1 = OUTMOD_3;                           // CCR1 reset/set
    CCR1 = valuePWM;                            // CCR1 PWM duty cycle
    TACTL = TASSEL_2 + MC_1; // SMCLK, up mode
}

void getanalogvalues(){
    i = 0;
    temp = 0;
    light = 0;
    touch =0;              // set all analog values to zero
    for(i=1; i<=50 ; i++){                       // read all three analog values 50 times each and average
        ADC10CTL0 &= ~ENC;
        while (ADC10CTL1 & BUSY);                       //Wait while ADC is busy
        ADC10SA = (unsigned)&ADCReading[0];         //RAM Address of ADC Data, must be reset every conversion
        ADC10CTL0 |= (ENC | ADC10SC);                   //Start ADC Conversion
        while (ADC10CTL1 & BUSY);                       //Wait while ADC is busy
        light += ADCReading[0];                         // sum  all 50 reading for the three variables
        touch += ADCReading[1];
        temp += ADCReading[2];
    }
    light = light/50; touch = touch/50; temp = temp/50;    // Average the 50 reading for the three variables
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void){
    __bic_SR_register_on_exit(CPUOFF);
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void){
    P2OUT |= GREEN_LED;             //DISABLE GREEN LED
    P1IFG &= ~BUTTON;               // CLEAR INTERRUPT FLAG FOR PORT 1.3
}
