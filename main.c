//
// Created by Paul Weber on 04.09.22.
//

#include "pico/stdlib.h"
#include "hardware/rtc.h"
#include "pico/util/datetime.h"
#include "hardware/gpio.h"
#include "stdio.h"
#include "pico/multicore.h"


typedef struct{
    int FIRST_GPIO;
    int number[10];
    int digit[4];
    int digit_mask;
    int number_mask;
    int time_digit[4]; //create an array for all the numbers
} bits;

//Structure to store the Time and Date
datetime_t time = {
        .year  = 2020,
        .month = 06,
        .day   = 05,
        .dotw  = 5, // 0 is Sunday, so 5 is Friday
        .hour  = 15,
        .min   = 37,
        .sec   = 00
};

int main(void);

void setup(bits *bits);

void set_time();

void button_hour();

void button_minute();

void display_time();

int main(void){
    multicore_launch_core1(display_time);

    while(1){
        int button_press = 0;

        while(gpio_get(0)!=0){
            sleep_ms(1);
            button_press++;

            if(button_press > 1000){
             set_time();
            }
        }
    }
}


void setup(bits *bits){

    bits->FIRST_GPIO = 1; //First Pin to use for the 7 Segment Display

    //Create Bitmask Arrays-------------------------

    /*
    Structure of Display

    --A--
    F   B
    --G--
    E   C
    --D--   H

    Pin 0 is First_GPIO

    Pin 0 = A
    Pin 1 = B
    Pin 2 = C
    Pin 3 = D
    Pin 4 = E
    Pin 5 = F
    Pin 6 = G
    Pin 7 = H

    Pin 8  = Digit 0
    Pin 9  = Digit 1
    Pin 10 = Digit 2
    Pin 11 = Digit 3
    */

    //these Arrays are used to convert a number into
    bits->number[0] = 0x3f; //0
    bits->number[1] = 0x06; //1
    bits->number[2] = 0x5b; //2
    bits->number[3] = 0x4f; //3
    bits->number[4] = 0x66; //4
    bits->number[5] = 0x6d; //5
    bits->number[6] = 0x7d; //6
    bits->number[7] = 0x07; //7
    bits->number[8] = 0x7f; //8
    bits->number[9] = 0x67; //9

    //Bitmasks for the Digits, when a Pin is set to low, the corresponding Digit will glow
    bits->digit[0] = 0xE; // 0
    bits->digit[1] = 0xD; // 1
    bits->digit[2] = 0xB; // 2
    bits->digit[3] = 0x7; // 3

    //Selection mask for Pins
    bits->digit_mask = 0xF;
    bits->number_mask = 0xFF;

    //Convert relative Bitmasks into absolute---------------------------------------

    for(int i=0; i<10; i++){
        bits->number[i] = bits->number[i] << bits->FIRST_GPIO;
    }

    bits->number_mask = bits->number_mask << bits->FIRST_GPIO;

    //The Digits use +8 Because the Number Pins come before them
    for(int i=0; i<4; i++){
        bits->digit[i] = bits->digit[i] << bits->FIRST_GPIO+8;
    }

    bits->digit_mask = bits->digit_mask << bits->FIRST_GPIO+8;

    //Set GPIO Mode of pins to OUTPUT -----------------------------------------

    // We could use gpio_set_dir_out_masked() here
    for (int gpio = 0+bits->FIRST_GPIO; gpio < 8+bits->FIRST_GPIO; gpio++) {
        gpio_init(gpio);
        gpio_set_dir(gpio, GPIO_OUT);
    }

    for (int gpio = 0+bits->FIRST_GPIO; gpio < 4+bits->FIRST_GPIO; gpio++) {
        gpio_init(gpio+8);
        gpio_set_dir(gpio+8, GPIO_OUT);
    }

    //Configuring RTC ----------------------------------------------------------------

    // Start the RTC and set to time and date in the time structure
    rtc_init();
    rtc_set_datetime(&time);
}

void set_time(){
    bits bits;
    setup(&bits);

    sleep_ms(10);

    //Interrupt
    gpio_set_irq_enabled_with_callback(0, GPIO_IRQ_EDGE_FALL, true, &button_hour); // Enable IRQ on GPIO 0 on a rising edge, then open the function test





    int block = 0;
    int press = 0;

    int button_press = 0;

    gpio_deinit(11);
    gpio_deinit(12);

    while(block<2){



        rtc_get_datetime(&time);

        //Convert Hours and Minutes into single digits
        bits.time_digit[0] = (time.hour/10)%10;
        bits.time_digit[1] = (time.hour/1)%10;

        bits.time_digit[2] = (time.min/10)%10;
        bits.time_digit[3] = (time.min/1)%10;




        if(block==0){




            while(gpio_get(0)!=0 && block==0){
                sleep_ms(1);
                button_press++;

                if(button_press > 1000){
                    block = 1;
                    gpio_set_irq_enabled_with_callback(0, GPIO_IRQ_EDGE_FALL, true, &button_minute); // Enable IRQ on GPIO 0 on a rising edge, then open the function test

                    gpio_init(11);
                    gpio_init(12);

                    gpio_deinit(9);
                    gpio_deinit(10);

                    gpio_set_dir(11, GPIO_OUT);
                    gpio_set_dir(12, GPIO_OUT);

                }

            }
            button_press = 0;
        }

        else if(block == 1){




            while(gpio_get(0)!=0 && block==1){
                sleep_ms(1);
                button_press++;

                if(button_press > 1000){
                    block = 2;
                    gpio_set_irq_enabled_with_callback(0, GPIO_IRQ_EDGE_FALL, false, &button_minute); // Enable IRQ on GPIO 0 on a rising edge, then open the function test

                }


            }
            button_press = 0;

        }
    }

    gpio_init(9);
    gpio_init(10);

    gpio_set_dir(9, GPIO_OUT);
    gpio_set_dir(10, GPIO_OUT);

    sleep_ms(1000);

    multicore_reset_core1();
    multicore_launch_core1(display_time);
}


void display_time(){
    bits bits;
    setup(&bits);

    while(1){
        rtc_get_datetime(&time); // get time

        //Convert Hours and Minutes into single digits
        bits.time_digit[0] = (time.hour/10)%10;
        bits.time_digit[1] = (time.hour/1)%10;

        bits.time_digit[2] = (time.min/10)%10;
        bits.time_digit[3] = (time.min/1)%10;



        //Go through the 4 Digits and Display them
        for(int i=0; i<4; i++){
            gpio_put_masked(bits.number_mask, bits.number[bits.time_digit[i]]); //Put the number bit mask on GPIO
            gpio_put_masked(bits.digit_mask, bits.digit[i]); //Put the digit bit mask on GPIO

            busy_wait_ms(1);
        }

    }
}

void button_hour(){
    if(time.hour<23){
        time.hour++;
    }
    else if(time.hour == 23){
        time.hour=0;
    }

    rtc_set_datetime(&time);
    busy_wait_ms(100);
}

void button_minute(){
    if(time.min<59){
        time.min++;
    }
    else if(time.min == 59){
        time.min=0;
    }

    rtc_set_datetime(&time);
    busy_wait_ms(100);
}