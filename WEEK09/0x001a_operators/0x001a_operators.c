#include <stdio.h>
#include "pico/stdlib.h"
#include "dht11.h"

int main(void) {
    stdio_init_all();

    dht11_init(4);

    int x = 5;
    int y = 10;
    int arithmetic_operator = (x * y);
    int increment_operator = x++;
    bool relational_operator = (x > y);
    bool logical_operator = (x > y) && (y > x);
    int bitwise_operator = (x<<1);  // x is now 6 because of x++ or 0b00000110 and (x<<1) is 0b00001100 or 12 
    int assignment_operator = (x += 5);

    while (true) {
        printf("arithmetic_operator: %d\r\n", arithmetic_operator);
        printf("increment_operator: %d\r\n", increment_operator);
        printf("relational_operator: %d\r\n", relational_operator);
        printf("logical_operator: %d\r\n", logical_operator);
        printf("bitwise_operator: %d\r\n", bitwise_operator);
        printf("assignment_operator: %d\r\n", assignment_operator);
        
        float hum, temp;
        if (dht11_read(&hum, &temp)) {
            printf("Humidity: %.1f%%, Temperature: %.1f°C\r\n", hum, temp);
        } else {
            printf("DHT11 read failed\r\n");
        }
        
        sleep_ms(2000);
    }
}