//
// Created by Paul Weber on 04.09.22.
//

#include "hardware/rtc.h"
#include "pico/util/datetime.h"
#include "hardware/gpio.h"
#include "pico/multicore.h"


typedef struct{
    int FIRST_GPIO; //stores the FIRST GPIO of the 7 Segment Display
    int number[10]; //Array to store number bitmasks
    int digit[4]; //Array to store digit bitmasks
    int number_mask; //Variable to store Pins that will be changed
    int digit_mask; //Variable to store Pins that will be changed
    int time_digit[4]; //create an array for all the numbers
} bits;

//Structure to store the Time and Date
datetime_t time = {
        .year  = 2022,
        .month = 9,
        .day   = 17,
        .dotw  = 6,
        .hour  = 12,
        .min   = 00,
        .sec   = 00
};

static absolute_time_t last_interrupt;

int main(void);

void setup(bits *bits);

void set_time();

void button_hour();

void button_minute();

void display_time();

int main(void){
    int button_press = 0; //Variable to store the Time the Button was pressed
    multicore_launch_core1(display_time); //launch the function to display time on core 1
    last_interrupt = get_absolute_time(); //get current time

    while(1){
        while(gpio_get(0)!=0){

            //count up once 1 each 1 ms
            button_press++;
            sleep_ms(1);

            //when counted to 1000 ms
            if(button_press > 1000){
                set_time(); //when button is pressed for 1000 ms go into set time mode
            }
        }
        button_press = 0; //reset counter of the time the button was pressed
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
        bits->digit[i] = bits->digit[i] << (bits->FIRST_GPIO+8);
    }

    bits->digit_mask = bits->digit_mask << (bits->FIRST_GPIO+8);

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

    //Interrupt
    // Enable IRQ on FIRST_GPIO -1 (GPIO 0) on falling edge, when activated open the button_hour function
    gpio_set_irq_enabled_with_callback(bits.FIRST_GPIO-1, GPIO_IRQ_EDGE_FALL, true, &button_hour);

    int block = 0; //Block 0 is Hours or Digit 0 and 1, Block 1 is Minutes or Digit 2 and 3
    int button_press = 0; //Variable to store the Time the Button was pressed

    //Disable Minute Display, so only Hours will be shown
    gpio_deinit(bits.FIRST_GPIO+10);
    gpio_deinit(bits.FIRST_GPIO+11);

    while(block<2){
        if(block==0){
            //Check if the Button is pressed, if it is pressed under 1000 ms IRQ will be executed
            while(gpio_get(0)!=0){

                //count up once 1 each 1 ms
                button_press++;
                sleep_ms(1);

                //when counted to 1000 ms
                if(button_press > 1000){
                    block++; //go to minute block

                    // Enable IRQ on FIRST_GPIO -1 (GPIO 0) on falling edge, when activated open the button_minute function
                    gpio_set_irq_enabled_with_callback(0, GPIO_IRQ_EDGE_FALL, true, &button_minute);


                    //Set the GPIO of the Minute Display back to normal
                    gpio_init(bits.FIRST_GPIO+10);
                    gpio_init(bits.FIRST_GPIO+11);
                    gpio_set_dir(bits.FIRST_GPIO+10, GPIO_OUT);
                    gpio_set_dir(bits.FIRST_GPIO+11, GPIO_OUT);

                    //Disable Hour Display
                    gpio_deinit(bits.FIRST_GPIO+8);
                    gpio_deinit(bits.FIRST_GPIO+9);

                    //wait to prevent button press in the next block
                    sleep_ms(1000);
                }

            }
            button_press = 0; //reset counter of the time the button was pressed
        }

        else if(block == 1){

            while(gpio_get(0)!=0 && block == 1){

                //count up once 1 each 1 ms
                button_press++;
                sleep_ms(1);

                //when counted to 1000 ms
                if(button_press > 1000){
                    block++; //go to minute block, this will just leave this loop. because this is the last block
                }
            }
            button_press = 0; //reset counter of the time the button was pressed
        }
    }

    //Disable IRQ
    gpio_set_irq_enabled_with_callback(0, GPIO_IRQ_EDGE_FALL, false, &button_minute);

    //Set the GPIO of the Hour Display back to normal
    gpio_init(bits.FIRST_GPIO+8);
    gpio_init(bits.FIRST_GPIO+9);
    gpio_set_dir(bits.FIRST_GPIO+8, GPIO_OUT);
    gpio_set_dir(bits.FIRST_GPIO+9, GPIO_OUT);

    time.sec=0; //Set the Seconds to 0
    rtc_set_datetime(&time); //set RTC to the changed time

    sleep_ms(5000); //wait to prevent reentering the set time mode for 5 seconds
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

            sleep_ms(1);
        }
    }
}

void button_hour(){

    //When entering IRQ compare time since last interrupt to current time, when difference is smaller than 150 ms do nothing
    if(absolute_time_diff_us(last_interrupt, get_absolute_time()) > 150000) {
        //count up 1 on button press, when hour is 23 and button is pressed, reset to 0
        if (time.hour < 23) {
            time.hour++;
        } else if (time.hour == 23) {
            time.hour = 0;
        }

        last_interrupt = get_absolute_time(); //set variable to current time
        rtc_set_datetime(&time); //set RTC to the changed time
    }
}

void button_minute(){

    //When entering IRQ compare time since last interrupt to current time, when difference is smaller than 150 ms do nothing
    if(absolute_time_diff_us(last_interrupt, get_absolute_time()) > 150000){
        //count up 1 on button press, when minute is 59 and button is pressed, reset to 0
        if(time.min < 59){
            time.min++;
        }
        else if(time.min == 59){
            time.min = 0;
        }

        time.sec = 0; //set seconds to zero to prevent minutes to change while changing

        last_interrupt = get_absolute_time(); //set variable to current time
        rtc_set_datetime(&time); //set RTC to the changed time
    }
}