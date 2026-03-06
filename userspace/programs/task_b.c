#include <printf.h>
#include <stdint.h>

int main(void)
{
    printf("[Task B] started (return 52 after 6 'PONG')\n");

    for (long long i = 0; ; i++)
    {
        if (i % 500000000 == 0 && i != 0)
        {
            printf("[Task B] PONG %lld!\n", i);

            if (i / 500000000 == 6)
                return 52;
        }
    }
}