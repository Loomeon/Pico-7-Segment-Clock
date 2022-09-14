//
// Created by Paul Weber on 04.09.22.
//

#include "pico/stdlib.h"
#include "hardware/rtc.h"
#include "pico/util/datetime.h"
#include "hardware/gpio.h"
#include "stdio.h"

int main(void);

void test();


int main(void){

    int FIRST_GPIO = 1; //First Pin to use for the 7 Segment Display

    //Interrupt

    gpio_set_irq_enabled_with_callback(0, GPIO_IRQ_LEVEL_HIGH, true, &test);

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

    int number_bits[10] = {
            0x3f, //0
            0x06, //1
            0x5b, //2
            0x4f, //3
            0x66, //4
            0x6d, //5
            0x7d, //6
            0x07, //7
            0x7f, //8
            0x67  //9
    };

    //Bitmasks for the Digits, when a Pin is set to low, the corresponding Digit will glow

    int digit_bits[4] = {
            0xE,  // 0
            0xD,  // 1
            0xB,  // 2
            0x7,  // 3
    };

    int digit[4]; //create an array for all the numbers

    //Selection mask for Pins
    int digit_mask = 0xF;
    int number_mask = 0xFF;

    //Convert relative Bitmasks into absolute---------------------------------------

    for(int i=0; i<10; i++){
        number_bits[i] = number_bits[i] << FIRST_GPIO;
    }

    number_mask = number_mask << FIRST_GPIO;

    //The Digits use +8 Because the Number Pins come before them
    for(int i=0; i<4; i++){
        digit_bits[i] = digit_bits[i] << FIRST_GPIO+8;
    }

    digit_mask = digit_mask << FIRST_GPIO+8;


    //Set GPIO Mode of pins to OUTPUT -----------------------------------------

    // We could use gpio_set_dir_out_masked() here
    for (int gpio = 0+FIRST_GPIO; gpio < 8+FIRST_GPIO; gpio++) {
        gpio_init(gpio);
        gpio_set_dir(gpio, GPIO_OUT);
    }

    for (int gpio = 0+FIRST_GPIO; gpio < 4+FIRST_GPIO; gpio++) {
        gpio_init(gpio+8);
        gpio_set_dir(gpio+8, GPIO_OUT);
    }

    //Configuring RTC ----------------------------------------------------------------

    // Start the RTC and set to time and date in the time structure
    rtc_init();
    rtc_set_datetime(&time);


    while(1){
        rtc_get_datetime(&time); // get time

        printf("test");

        //Convert Hours and Minutes into single digits
        digit[0] = (time.min/10)%10;
        digit[1] = (time.min/1)%10;

        digit[2] = (time.sec/10)%10;
        digit[3] = (time.sec/1)%10;

        //Go through the 4 Digits and Display them
        for(int i=0; i<4; i++){
            gpio_put_masked(number_mask, number_bits[digit[i]]); //Put the number bit mask on GPIO
            gpio_put_masked(digit_mask, digit_bits[i]); //Put the digit bit mask on GPIO

            sleep_ms(1);
        }
    }
}

void test(){
    busy_wait_ms(1000);
}