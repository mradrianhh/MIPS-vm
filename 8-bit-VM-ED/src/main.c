#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>

#define FLUSH while (getchar() != '\n')

static void init();
static void start();
static void shutdown();

static int rc;
static char input;

int main()
{
    // Create a clock that loops while system is powered on.
    // At a set frequency, trigger the systick event.
    // All other devices, has a function subscribed to this event that gets triggered at each tick.
    // This way, everything happens synchronously, and we're able to 
    // emulate the nature of a computer.
    printf("Press any key to quit...\n");
    scanf(" %c", &input);

    return 0;
}