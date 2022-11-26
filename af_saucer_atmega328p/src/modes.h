#include <avr/io.h>
#include <stdbool.h>

void updateLEDState(uint16_t newState);
void updateFlasherState(bool newState);
void updateLEDs();
void updateFlasher();
