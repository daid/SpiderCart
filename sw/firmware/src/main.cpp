#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"

#include "pins.h"

int main() {
    stdio_init_all();
    while(1) {
        printf("Test...\n");
        sleep_ms(500);
    }
    return 0;
}
