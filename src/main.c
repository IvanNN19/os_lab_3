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

#include <time.h>

const int STRING_SIZE = 100;

int create_process();
int generator();

int main(){
    int i = 1; 
    srand(time(NULL));

    sem_unlink("/semafor_pr");
    sem_unlink("/semafor_ch1");
    sem_unlink("/semafor_ch2");

    write(1, "Создайте 2 файла\n", sizeof(char) * strlen("Создайте 2 файла\n"));
    char file_name1[STRING_SIZE], file_name2[STRING_SIZE];
    read(STDIN_FILENO, file_name1, STRING_SIZE);
    read(STDIN_FILENO, file_name2, STRING_SIZE);

    int filenam1 = open(file_name1, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    int filenam2 = open(file_name2, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
///////////////////////////////////////////////////////////////////////////////
    const char* mmaped_file_name = "mmaped_file";
    int mmaped_fd;
    char* mmaped_fp;

    mmaped_fd = shm_open(mmaped_file_name, O_RDWR | O_CREAT, 0777);
    ftruncate(mmaped_fd, STRING_SIZE); //размер разделяемой памяти
    mmaped_fp = mmap(NULL, STRING_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mmaped_fd, 0);//возвращает указатель на начало отображенной области памяти

    sem_t* semaphores[3];
    semaphores[0] = sem_open("/semafor_pr", O_CREAT, 0777, 1);
    semaphores[1] = sem_open("/semafor_ch1", O_CREAT, 0777, 0);
    semaphores[2] = sem_open("/semafor_ch2", O_CREAT, 0777, 0);

    // int tmp;
    // sem_getvalue(semaphores[0], &tmp);
    // printf("tmp: %d", tmp);

    pid_t pid_1 = create_process();
    if(pid_1 == 0){//ch1            
        dup2(filenam1, 1);
        close(filenam1);
        execl("child","", mmaped_file_name, "/semafor_ch1", "/semafor_pr", NULL);
        perror("exec");
        exit(-1);
    } else {
        pid_t pid_2 = create_process();
        if(pid_2 == 0){//ch2
            dup2(filenam2, 1);
            close(filenam2);
            execl("child", "", mmaped_file_name, "/semafor_ch2", "/semafor_pr", NULL);
            perror("exec");
            exit(-1);
            } else {//prnt
                char new_string[STRING_SIZE];
                write(1, "Введите строки\n", sizeof(char) * strlen("Введите строки\n"));
                while(fgets(new_string, STRING_SIZE, stdin)){
                    //if(i == 4){ break;}
                    int r = generator(); 
                    if(r < 81){    
                        sem_wait(semaphores[0]);
                        strcpy(mmaped_fp, new_string);
                        mmaped_fp[strlen(new_string)] = 0;
                        sem_post(semaphores[1]);
                    }else{
                        sem_wait(semaphores[0]);//frezz ch2
                        strcpy(mmaped_fp, new_string);
                        mmaped_fp[strlen(new_string)] = 0;
                        sem_post(semaphores[2]);
                    }
                    //printf("%d, %d\n", i, r);
                    i++;
                }
                printf("10\n");
                sem_wait(semaphores[0]);
                mmaped_fp[0] = 0;
                sem_post(semaphores[1]); sem_post(semaphores[2]);
                printf("10\n");
                
                
                wait(NULL);
                printf("END\n");
                close(filenam1);
                close(filenam2);

                
                
                if(munmap(mmaped_fp, STRING_SIZE) == -1){//закрываем разделяемую память
                    perror("munmap");
                }
                if(close(mmaped_fd) == -1){//закрываем объеект раз. памяти
                    perror("close");
                }
                if(shm_unlink(mmaped_file_name) == -1){ //удаляем объект РП из Файловой системы.
                    perror("shm_unlink");
                }
                
            }   
        }
    return 0;
}


int create_process() {
    pid_t pid = fork();
    if (-1 == pid)
    {
        perror("Error while fork");
    }
    return pid;
}

int generator(){
    int rng = rand() % 100;
    return rng;
}
