#include <semaphore.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


void withdraw(int *BankAccount, int *localBalance){
  int need = rand() % 51; // random number between 0 and 50
  printf("Poor Student needs $%d\n", need);
  if(need <= *localBalance){
    *localBalance -= need;
    *BankAccount = *localBalance;
    printf("Poor Student: Withdraws $%d / Balance = $%d\n", need, *localBalance);
  } else {
    printf("Poor Student: Not Enough Cash ($%d)\n", *localBalance);
  }
}
void deposit(int *BankAccount, int *localBalance){
  if(*localBalance < 100){
    //deposit because number was even and account has less than 100
    int amount = rand() % 101;
    if(amount % 2 == 0){
      *localBalance += amount;
      *BankAccount = *localBalance;
      printf("Dear old Dad: Deposits $%d / Balance = $%d\n", amount, *localBalance);
    } else {
      printf("Dear old Dad: Doesn't have any money to give\n");
    }
  } else {
      printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", *localBalance);
  }
}


int main(int argc, char **argv)
{
  srand(time(NULL));
  int fd, i,nloop=10,zero=0,*BankAccount, localBalance;
  sem_t *mutex;

  //open a file and map it into memory this is to hold the shared counter
  fd = open("shared_memory.txt",O_RDWR|O_CREAT,S_IRWXU);
  write(fd,&zero,sizeof(int));
  BankAccount = mmap(NULL,sizeof(int),PROT_READ |PROT_WRITE,MAP_SHARED,fd,0);
  close(fd);

  /* create, initialize semaphore */
 if ((mutex = sem_open("examplesemaphore", O_CREAT, 0644, 1)) == SEM_FAILED) {
    perror("semaphore initilization");
    exit(1);
  }
 
  if (fork() == 0) { /* child process*/
    for (i = 0; i < nloop; i++) {
      sem_wait(mutex);
      int sleep_time = rand() % 6;
      sleep(sleep_time);
      printf("Poor Student: Attempting to Check Balance\n");
      localBalance = *BankAccount;

      int randNum = rand() % 11;
      if(randNum %2 == 0){
        withdraw(BankAccount, &localBalance);
      } else {
        printf("Poor Student: Last Checking Balance = $%d\n", localBalance);
      }
      sem_post(mutex);
    }
    exit(0);
  }
  /* back to parent process */
  for (i = 0; i < nloop; i++) {
    sem_wait(mutex);
    printf("Dear Old Dad: Attempting to Check Balance\n");
    int randNum = rand() % 11;
          if(randNum %2 == 0){
            deposit(BankAccount, &localBalance);
          } else {
            printf("Dear Old Dad: Last Checking Balance = $%d\n", localBalance);
          } 
    printf("parent leaving critical section\n");
    sem_post(mutex);
    sleep(1);
  }
  exit(0);
}
