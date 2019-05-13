#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define MAXTHREADS 4

int threads = 0;

void task1()
{
    sleep(10);
    printf("task 1\n");
    threads--;
}

void task2()
{
    sleep(20);
    printf("task 2\n");
    threads--;
}

void task3()
{
    sleep(30);
    printf("task 3\n");
    threads--;
}

int main(int argc, const char *argv[])
{
    pthread_t thread1 = NULL, thread2 = NULL, thread3 = NULL;
    // the manager/boss main thread
    // the manager will create a pool of threads ahead of time to serve
    // the requests .. in this example they are created when a request comes

    int n = 1;
    while (n)
    {
        if (threads <= MAXTHREADS)
        {
            threads++;
            // get a request
            scanf("%d", &n);
            switch (n)
            {
            case 1:
                pthread_create(&thread1, NULL, (void *)task1, NULL);
                break;
            case 2:
                pthread_create(&thread2, NULL, (void *)task2, NULL);
                break;
            default:
                pthread_create(&thread3, NULL, (void *)task3, NULL);
                break;
            }
        }
        else
        {
        }
    }
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    printf("done!\n");
    return 0;
}