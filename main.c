//
// Created by Paul Weber on 04.09.22.
//

#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"
#include "hardware/gpio.h"

int main(void);
//void display(int digit, int number, int *digit_bits, int *number_bits, int digit_mask, int number_mask, int FIRST_GPIO);
//void display_time(datetime_t *time, int FIRST_GPIO);

int main(void){


    int FIRST_GPIO = 0;

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

    int number_bits[10] = {
            0x3f,
            0x06,
            0x5b,
            0x4f,
            0x66,
            0x6d,
            0x7d,
            0x07,
            0x7f,
            0x67
    };

    int digit_bits[4] = {
            0xE,  // 0
            0xD,  // 1
            0xB,  // 2
            0x7,  // 3
    };

    int digit_mask = 0xF;

    int number_mask = 0xFF;

    //Convert relative Bitmasks into absolute---------------------------------------

    for(int i=0; i<10; i++){
        number_bits[i] = number_bits[i] << FIRST_GPIO;
    }

    for(int i=0; i<4; i++){
        digit_bits[i] = digit_bits[i] << FIRST_GPIO+8;
    }


    digit_mask = digit_mask << FIRST_GPIO+8;

    number_mask = number_mask << FIRST_GPIO;


    //Set GPIO Mode of pins to OUTPUT -----------------------------------------

    // We could use gpio_set_dir_out_masked() here
    for (int gpio = 0; gpio < 8; gpio++) {
        gpio_init(gpio);
        gpio_set_dir(gpio, GPIO_OUT);
    }

    for (int gpio = 0; gpio < 4; gpio++) {
        gpio_init(gpio+8);
        gpio_set_dir(gpio+8, GPIO_OUT);
    }

    //Configuring RTC ----------------------------------------------------------------

    // Start the RTC
    rtc_init();
    rtc_set_datetime(&time);

    int digit[4]; //create an array for all the numbers


    while(1){

        //sleep_us(64);

        rtc_get_datetime(&time);

        digit[0] = (time.min/10)%10;
        digit[1] = (time.min/1)%10;

        digit[2] = (time.sec/10)%10;
        digit[3] = (time.sec/1)%10;

        //Go through the 4 Digits and Display them

        for(int i=0; i<4; i++){

            gpio_put_masked(number_mask, number_bits[digit[i]]); //Put the bit mask on GPIO
            gpio_put_masked(digit_mask, digit_bits[i]); //Put the bit mask on GPIO

            sleep_ms(1);
        }
    }

    return 0;
}


/*
void display(int digit, int number, int *digit_bits, int *number_bits, int digit_mask, int number_mask, int FIRST_GPIO){


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



// This array converts a number 0-9 to a bit pattern to send to the GPIOs

}
*/