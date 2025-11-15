#include <stdio.h>
#include "pico/stdlib.h"
#include "servo.h"

#define SERVO_GPIO 6

int main(void) {
    stdio_init_all();

    uint8_t choice = 0;

    servo_init(SERVO_GPIO);

    while (true) {
        choice = getchar();

        if (choice == 0x31) {
            printf("1\r\n");
        } else if (choice == 0x32) {
            printf("2\r\n");
        } else {
            printf("??\r\n");
        }
        
        switch (choice) {
            case '1':
                printf("one\r\n");
                servo_set_angle(0.0f);
                sleep_ms(500);
                servo_set_angle(180.0f);
                sleep_ms(500);
                break;
            case '2':
                printf("two\r\n");
                servo_set_angle(180.0f);
                sleep_ms(500);
                servo_set_angle(0.0f);
                sleep_ms(500);
                break;   
            default:
                printf("??\r\n");  
        }
    }
}
