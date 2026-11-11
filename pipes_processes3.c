#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int pipe1[2];   // cat -> grep
    int pipe2[2];   // grep -> sort

    pid_t pid1, pid2;

    /* Make sure one command-line argument was given */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <grep_argument>\n", argv[0]);
        return 1;
    }

    /* Create first pipe */
    if (pipe(pipe1) == -1)
    {
        perror("pipe1");
        return 1;
    }

    /* Create second pipe */
    if (pipe(pipe2) == -1)
    {
        perror("pipe2");
        return 1;
    }

    /* Create P2 */
    pid1 = fork();

    if (pid1 < 0)
    {
        perror("fork");
        return 1;
    }

    /* ---------------- P1: Parent ---------------- */
    if (pid1 > 0)
    {
        /*
         * Parent executes:
         * cat scores
         *
         * Output goes into pipe1.
         */

        close(pipe1[0]);

        dup2(pipe1[1], STDOUT_FILENO);

        close(pipe1[1]);

        /* Parent does not use pipe2 */
        close(pipe2[0]);
        close(pipe2[1]);

        execlp("cat", "cat", "scores", NULL);

        perror("execlp cat");
        exit(1);
    }

    /* ---------------- P2: Child ---------------- */
    else
    {
        /* Create P3, the child's child */
        pid2 = fork();

        if (pid2 < 0)
        {
            perror("fork");
            exit(1);
        }

        /* ---------------- P2: grep ---------------- */
        if (pid2 > 0)
        {
            /*
             * Input comes from cat through pipe1.
             * Output goes into pipe2.
             */

            close(pipe1[1]);

            dup2(pipe1[0], STDIN_FILENO);

            close(pipe1[0]);

            close(pipe2[0]);

            dup2(pipe2[1], STDOUT_FILENO);

            close(pipe2[1]);

            execlp("grep", "grep", argv[1], NULL);

            perror("execlp grep");
            exit(1);
        }

        /* ---------------- P3: sort ---------------- */
        else
        {
            /*
             * Input comes from grep through pipe2.
             * sort prints normally to stdout.
             */

            close(pipe1[0]);
            close(pipe1[1]);

            close(pipe2[1]);

            dup2(pipe2[0], STDIN_FILENO);

            close(pipe2[0]);

            execlp("sort", "sort", NULL);

            perror("execlp sort");
            exit(1);
        }
    }

    return 0;
}