#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/wait.h"
#include "string.h"

#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "sys/mman.h"
#include "semaphore.h"

const int MAX_SIZE = 100;

int main(int argc, char* argv[]){
    char mmap_file_name[MAX_SIZE];
    strcpy(mmap_file_name, argv[1]);

    char semaphor_name[MAX_SIZE];
    char semaphor_pr_name[MAX_SIZE];
    strcpy(semaphor_name, argv[2]);
    strcpy(semaphor_pr_name, argv[3]);

    int mmap_f_d = shm_open(mmap_file_name, O_RDWR | O_CREAT, 0777);
    ftruncate(mmap_f_d, MAX_SIZE);//?????
    char* mmap_f_pointer = mmap(NULL, MAX_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_f_d, 0);

    sem_t* semaphor = sem_open(semaphor_name, 0);
    sem_t* semaphor_pr = sem_open(semaphor_pr_name, 0);

    write(1, "``Начало работы``\n", sizeof(char) * strlen("``Начало работы``\n"));
    char vowels[] = {'a', 'e', 'i', 'o', 'u', 'y'};
    char string[MAX_SIZE];
    while(1){

        sem_wait(semaphor);

        strcpy(string, mmap_f_pointer);
        string[ strlen(mmap_f_pointer) ] = 0;

        sem_post(semaphor_pr);

        if(strlen(string) == 0){ break; }

        for (int index = 0; index < strlen(string); ++index) {
            if (memchr(vowels, string[index], 6) == NULL) {
                write(1, &string[index], 1);
            }
        }
    }

    munmap(mmap_f_pointer, 0);
    sem_close(semaphor);
    sem_close(semaphor_pr);

    return 0;
}