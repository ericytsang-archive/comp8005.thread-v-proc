#include <stdio.h>
#include <sys/time.h>

int main();
long current_timestamp();

int main()
{
    printf("Hello, World!\n");
    return 0;
}

long current_timestamp()
{
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    return te.tv_sec*1000L + te.tv_usec/1000; // caculate milliseconds
}
