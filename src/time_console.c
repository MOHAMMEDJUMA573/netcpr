/* time_console */

#include <stdio.h>
#include <time.h>

int main()
{
    time_t timer;
    time(&timer);
    printf("--- seconds: %ld ---\n", timer);
    printf("The local time is: %s \n", ctime(&timer));

    return 0;
}