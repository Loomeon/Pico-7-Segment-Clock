//
// Created by Paul Weber on 04.09.22.
//

#include <stdio.h>
#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"
#include "hardware/gpio.h"

int main(void);
void init_RTC(datetime_t time);
void display(int digit, int number, int FIRST_GPIO);
void display_time(datetime_t time);


int Button = 0; //Button to set the Time (which is also used as a interrupt)


int main(void){

    // Start on Friday 5th of June 2020 15:45:00
    datetime_t t = {
            .year  = 2020,
            .month = 06,
            .day   = 05,
            .dotw  = 5, // 0 is Sunday, so 5 is Friday
            .hour  = 15,
            .min   = 45,
            .sec   = 00
    };

    init_RTC(t);

    printf("%d", t.hour);







    return 0;
}

//Configuring RTC ----------------------------------------------------------------
void init_RTC(datetime_t time){

    // Start the RTC
    rtc_init();
    rtc_set_datetime(&time);
}

void display(int digit, int number, int FIRST_GPIO){
/*

    Structure

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

    Pins fit Perfectly into a single Byte, i created a Bitmask for all the Numbers

    */

// This array converts a number 0-9 to a bit pattern to send to the GPIOs
    int number_bits[10] = {
            0x3f,  // 0
            0x06,  // 1
            0x5b,  // 2
            0x4f,  // 3
            0x66,  // 4
            0x6d,  // 5
            0x7d,  // 6
            0x07,  // 7
            0x7f,  // 8
            0x67   // 9
    };

    int digit_bits[10] = {
            0xE,  // 0
            0xD,  // 1
            0xB,  // 2
            0x7,  // 3
    };

    int number_mask = 0xFF << FIRST_GPIO; //get the right position for the pins

    gpio_put_masked(number_mask, number_bits[number]); //Put the bit mask on GPIO

    int digit_mask = 0xF << FIRST_GPIO+8;

    gpio_put_masked(digit_mask, digit_bits[digit]); //Put the bit mask on GPIO
}

void display_time(datetime_t time){

}