#include <msp430.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define RED_LED         BIT3            // Port 2.3
#define GREEN_LED       BIT4            // Port 2.4
#define BUTTON          BIT3            // Port 1.3
#define LIGHT_SENSOR    BIT2            // Port 1.2
#define LOW_PULSE       1.50            // Constant that light is multiplied by to detect low pulse
#define HIGH_PULSE      1.30            // Constant that light is multiplied by to detect high pulse
#define MOTOR_OUT1      BIT4            // Port 1.4
#define MOTOR_OUT2      BIT5            // Port 1.5
#define PWM             BIT6            // Port 2.6


int value=0, i=0 ;
int highFlag = 0, lowFlag = 0;
int high = 0; // count for high hand
int low = 0; // count for low hand
int clockWisePass [5] = {1,0,1,0,1};
int counterWisePass [5] = {1,0,1,0,0};
int stopPass [5] = {0,0,0,0,0};
int inputPass [5];
int index = 0;
int light = 0, lightroom = 0, dimled=50;
int clockFlag = 0, counterFlag = 0, stopFlag = 0;       // clockFlag (clockwise) & counterFlag (counterclockwise)
int resetPassFlag = 0;
int motorFlag = 0;                  // 1 - BRAKE, 2 - FW, 3 - BRAKE, 4 - RV
int ADCReading [3];

void fadeLED(int valuePWM);                 //this is from skeleton code, not using it
void ConfigureAdc(void);
void getAnalogValues(void);                 // Get analog values to convert to digital and set them to light variable
void processAnalogValues(void);             // Process light variable to set high/low flag
void greenLED(int green);                    // Function for setting green on or off
void redLED(int red);                       // Function for setting red on or off
void setMotor(int motorFlag);               // Set Motor Outputs

int main(void){
    WDTCTL = WDTPW + WDTHOLD;                   // Stop WDT
    P1OUT = 0;
    P2OUT = 0;
    P1DIR = 0;
    P1REN = 0;
    P2REN = 0;
    P2DIR = 0;
    P2DIR |= (GREEN_LED | RED_LED | PWM);                       // SET LEDs as OUTPUT
    P2SEL |= PWM;                                   // Select PWM as OUTPUT
    P1DIR &= ~BUTTON;                                    //BUTTON IS AN INPUT
    P1DIR |= (MOTOR_OUT1 | MOTOR_OUT2);             // SET PWM AND MOTOR OUTPUTS
    P1OUT |= BUTTON;                                    // PULL-UP RESISTOR
    P1REN |= BUTTON;                                    // RESISTOR ENABLED
    P1IFG &= ~BUTTON;                                   // CLEAR INTERRUPT FLAG
    P1IE |= BUTTON;                                     // ENABLE BUTTON INTERRUPT

    TA0CCR0 = 1000;         //Set the period in the Timer A0 Capture/Compare 0 register to 1000 us.
    TA0CCTL1 = OUTMOD_7;
    TA0CCR1 = 500;               //The period in microseconds that the power is ON. It's half the time, which translates to a 50% duty cycle.
    TA0CTL = TASSEL_2 + MC_1;    //TASSEL_2 selects SMCLK as the clock source, and MC_1 tells it to count up to the value in TA0CCR0.


    __enable_interrupt();
    ConfigureAdc();
    __bis_SR_register(LPM0_bits);       //Switch to low power mode 0.

    // reading the initial room value lightroom
    // __delay_cycles(250);
    __delay_cycles(25000);
    getAnalogValues();
    lightroom = light;

    __delay_cycles(250);

    __delay_cycles(200);

    for (;;){
        /*
         * When a high is detected it will reset the clockwise password
         * When a low is detected it will reset the counterclockwise password
         * The RED LED Blinks Accordingly
         */
        /*
        if(resetPassFlag){
            index = 0;
            getAnalogValues();
            processAnalogValues();
            while(index < 5){
                getAnalogValues();
                processAnalogValues();
            }
            resetPassFlag = 0;
        }
        */
        while(motorFlag){
          switch(motorFlag){
          case 1:
              greenLED(0);
              motorFlag = 0;
              break;
          case 2:
              greenLED(1);
              __delay_cycles(250000);
              greenLED(0);
              break;
          case 3:
              greenLED(0);
              motorFlag = 0;
              break;
          case 4:
              greenLED(1);
              __delay_cycles(50000);
              greenLED(0);
              break;
          }
          if(index > 4){
            break;
          }
          getAnalogValues();
          processAnalogValues();
        }
        if(index > 4){ //a full password is entered
            clockFlag = 1;
            counterFlag = 1;
            stopFlag = 1;
            // Check if it is clockwise password
            for(i=0; i<=4 ; i++){
                if(clockWisePass[i] != inputPass[i]){
                    clockFlag = 0;
                    break;
                }
            }
            // Check if it is counter-clockwise password
            for(i=0; i<=4 ; i++){
                if(counterWisePass[i] != inputPass[i]){
                    counterFlag = 0;
                    break;
                }
            }
            // Check if it is stop password
            for(i=0; i<=4 ; i++){
                if(stopPass[i] != inputPass[i]){
                    stopFlag = 0;
                    break;
                }
            }
            if(clockFlag){
                motorFlag = 2;
            }
            if(counterFlag){
                motorFlag = 4;
            }
            if(stopFlag){
                motorFlag = 1;
            }
            index = 0; //reset index to enter password again
        }
        //reading light repeatedly at the beginning of the main loop
        getAnalogValues();
        //Observe the dead zone for no action to avoid flickering
        processAnalogValues();
    }
}

void greenLED(int green){
    if(green){
        P2OUT |= GREEN_LED;
    } else {
        P2OUT &= ~GREEN_LED;
    }
}

void redLED(int red){
    if(red){
        P2OUT |= RED_LED;
    } else{
        P2OUT &= ~RED_LED;
    }
}

void setMotor(int value){
    //according to motorFlag value do something
    switch(value){
    case 1:
        P1OUT &= ~MOTOR_OUT1;
        P1OUT &= ~MOTOR_OUT2;
        break;
    case 2:
        P1OUT |= MOTOR_OUT1;
        P1OUT &= ~MOTOR_OUT2;
        break;
    case 3:
        P1OUT |= MOTOR_OUT1;
        P1OUT |= MOTOR_OUT2;
        break;
    case 4:
        P1OUT &= ~MOTOR_OUT1;
        P1OUT |= MOTOR_OUT2;
        break;
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

void getAnalogValues(void){
    i = 0;
    light = 0;           // set all analog values to zero
    for(i=1; i<=50 ; i++){                       // read all three analog values 50 times each and average
        ADC10CTL0 &= ~ENC;
        while (ADC10CTL1 & BUSY);                       //Wait while ADC is busy
        ADC10SA = (unsigned)&ADCReading[0];         //RAM Address of ADC Data, must be reset every conversion
        ADC10CTL0 |= (ENC | ADC10SC);                   //Start ADC Conversion
        while (ADC10CTL1 & BUSY);                       //Wait while ADC is busy
        light += ADCReading[0];                         // sum  all 50 reading for the three variables
    }
    light = light/50;    // Average the 50 reading for the three variables
}

void processAnalogValues(void){
    //I chose the range 1.05 to 1.10 of the value; that is no action if  (1.05 lightroom < light < 1.1 lightroom)
    if(light < lightroom * 1.10 && light > lightroom * 1.05) {}
    else{
        if(light >= lightroom * HIGH_PULSE) { // if light value is 10% greater than initial value take another measurement to make sure it is high or low
            getAnalogValues(); //get another measurement to be sure and avoid a High measurement before every Low measurement
            if(light >= lightroom * LOW_PULSE){
                __delay_cycles(50000);
                lowFlag = 1;
                highFlag = 0;
            } else if(light >= lightroom * HIGH_PULSE){
                __delay_cycles(50000);
                highFlag = 1;
                lowFlag = 0;
            }
        }
        if(light <= lightroom * HIGH_PULSE) {
            redLED(0);
            greenLED(0);
            __delay_cycles(200);
            //check flags and log corresponding password digit. This is done here so that we don't get multiple unintended entries
            if(lowFlag){
                //BLINK RED LED ONCE SLOW
                redLED(1);                              //Turn on Red
                __delay_cycles(250000);
                redLED(0);                              //Turn off Red
                low += 1;
                inputPass[index] = 0;
                index +=1; //update index
            } else if(highFlag){
                //BLINK RED LED TWICE FAST
                redLED(1);                              //Turn on Red
                __delay_cycles(50000);
                redLED(0);                               //Turn off Red
                __delay_cycles(50000);
                redLED(1);                               //Turn on Red
                __delay_cycles(50000);
                redLED(0);                              //Turn off Red
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

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void){
    __bic_SR_register_on_exit(CPUOFF);
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void){
    resetPassFlag = 1;
    P1IFG &= ~BUTTON;               // CLEAR INTERRUPT FLAG FOR PORT 1.3
}
