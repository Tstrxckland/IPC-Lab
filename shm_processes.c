#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>

int main()
{
    int ShmID;
    int *ShmPTR;

    int *BankAccount;
    int *Turn;

    pid_t pid;

    /* Create shared memory for two integers */
    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);

    if (ShmID < 0)
    {
        perror("shmget");
        exit(1);
    }

    /* Attach shared memory */
    ShmPTR = (int *) shmat(ShmID, NULL, 0);

    if (ShmPTR == (void *) -1)
    {
        perror("shmat");
        exit(1);
    }

    /* Set up the two shared variables */
    BankAccount = &ShmPTR[0];
    Turn = &ShmPTR[1];

    *BankAccount = 0;
    *Turn = 0;

    /* Create the child process */
    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        exit(1);
    }

    /* =====================================================
       CHILD PROCESS - Poor Student
       ===================================================== */
    if (pid == 0)
    {
        int i;
        int account;
        int balance;

        /* Give child its own random seed */
        srand(time(NULL) ^ getpid());

        for (i = 0; i < 25; i++)
        {
            /* Sleep between 0 and 5 seconds */
            sleep(rand() % 6);

            /* Wait until it is the child's turn */
            while (*Turn != 1)
            {
                /* no-op */
            }

            /* Copy shared BankAccount into local account */
            account = *BankAccount;

            /* Student needs between $0 and $50 */
            balance = rand() % 51;

            printf("Poor Student needs $%d\n", balance);

            if (balance <= account)
            {
                account = account - balance;

                printf(
                    "Poor Student: Withdraws $%d / Balance = $%d\n",
                    balance,
                    account
                );
            }
            else
            {
                printf(
                    "Poor Student: Not Enough Cash ($%d)\n",
                    account
                );
            }

            /* Copy local account back into shared memory */
            *BankAccount = account;

            /* Give turn back to parent */
            *Turn = 0;
        }

        /* Detach shared memory */
        shmdt(ShmPTR);

        exit(0);
    }

    /* =====================================================
       PARENT PROCESS - Dear Old Dad
       ===================================================== */
    else
    {
        int i;
        int account;
        int balance;

        /* Give parent its own random seed */
        srand(time(NULL) ^ getpid());

        for (i = 0; i < 25; i++)
        {
            /* Sleep between 0 and 5 seconds */
            sleep(rand() % 6);

            /* Wait until it is Dad's turn */
            while (*Turn != 0)
            {
                /* no-op */
            }

            /* Copy shared BankAccount into local account */
            account = *BankAccount;

            if (account <= 100)
            {
                /* Random amount between $0 and $100 */
                balance = rand() % 101;

                if (balance % 2 == 0)
                {
                    account = account + balance;

                    printf(
                        "Dear old Dad: Deposits $%d / Balance = $%d\n",
                        balance,
                        account
                    );
                }
                else
                {
                    printf(
                        "Dear old Dad: Doesn't have any money to give\n"
                    );
                }
            }
            else
            {
                printf(
                    "Dear old Dad: Thinks Student has enough Cash ($%d)\n",
                    account
                );
            }

            /* Copy local account back into shared memory */
            *BankAccount = account;

            /* Give turn to child */
            *Turn = 1;
        }

        /* Wait for child */
        wait(NULL);

        /* Detach shared memory */
        shmdt(ShmPTR);

        /* Remove shared memory */
        shmctl(ShmID, IPC_RMID, NULL);
    }

    return 0;
}