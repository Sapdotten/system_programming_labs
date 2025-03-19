#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <iostream>
#include <random>

#define RESET "\033[0m"
#define PURPLE "\033[35m"
#define GREEN "\033[32m"
#define ARE_YOU_WINNING "⢸⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⡷⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n⢸⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡇⠢⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n⢸⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡇⠀⠀⠈⠑⢦⡀⠀⠀⠀⠀⠀\n⢸⠀⠀⠀⠀⢀⠖⠒⠒⠒⢤⠀⠀⠀⠀⠀⡇⠀⠀⠀⠀⠀⠙⢦⡀⠀⠀⠀⠀\n⢸⠀⠀⣀⢤⣼⣀⡠⠤⠤⠼⠤⡄⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠙⢄⠀⠀⠀⠀\n⢸⠀⠀⠑⡤⠤⡒⠒⠒⡊⠙⡏⠀⢀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠑⠢⡄⠀\n⢸⠀⠀⠀⠇⠀⣀⣀⣀⣀⢀⠧⠟⠁⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡇⠀\n⢸⠀⠀⠀⠸⣀⠀⠀⠈⢉⠟⠓⠀⠀⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸\n⢸⠀⠀⠀⠀⠈⢱⡖⠋⠁⠀⠀⠀⠀⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸\n⢸⠀⠀⠀⠀⣠⢺⠧⢄⣀⠀⠀⣀⣀⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸\n⢸⠀⠀⠀⣠⠃⢸⠀⠀⠈⠉⡽⠿⠯⡆⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸\n⢸⠀⠀⣰⠁⠀⢸⠀⠀⠀⠀⠉⠉⠉⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸\n⢸⠀⠀⠣⠀⠀⢸⢄⠀⠀⠀⠀⠀⠀⠀⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸\n⢸⠀⠀⠀⠀⠀⢸⠀⢇⠀⠀⠀⠀⠀⠀⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸\n⢸⠀⠀⠀⠀⠀⡌⠀⠈⡆⠀⠀⠀⠀⠀⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸\n⢸⠀⠀⠀⠀⢠⠃⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸\n⢸⠀⠀⠀⠀⢸⠀⠀⠀⠁⠀⠀⠀⠀⠀⠀⠀⠷"
using namespace std;

volatile sig_atomic_t last_sig;
volatile sig_atomic_t sig_val;

void get_signal_with_value(int signo, siginfo_t *info, void *context)
{
    if (signo == SIGTERM)
    {
        sig_val = info->si_value.sival_int;
    }
}

void get_signal_without_value(int signo, siginfo_t *info, void *context)
{
    last_sig = signo;
}

void IKnowNumber(const pid_t opponent_pid, const int start, const int end,
                 sigset_t *set)
{
    pid_t my_pid = getpid();
    siginfo_t info;
    struct timespec timeout = {5, 0};
    printf(RESET PURPLE);
    printf("%u: I know the number. Hey, %u, try to guess!\n", my_pid,
           opponent_pid);
    printf("%u: It's between %d and %d\n", my_pid, start, end);
    fflush(stdout);
    int tries_count = 1;
    int random_value = rand() % end + start;
    sigqueue(opponent_pid, SIGUSR1, sigval{0});
    while (true)
    {
        int sig = sigtimedwait(set, &info, &timeout);

        if (sig_val == random_value)
        {
            printf(RESET PURPLE);
            printf("%u: You are right!!! Good job!\n", my_pid);
            printf("%u: You are win. Your tries count %d:\n", my_pid, tries_count);
            fflush(stdout);
            sigqueue(opponent_pid, SIGUSR1, sigval{0});
            break;
        }
        else
        {
            printf(RESET PURPLE);
            printf("%u: Your answer is incorrect, sorry. Try again.\n", my_pid);
            fflush(stdout);
            tries_count++;
            sigqueue(opponent_pid, SIGUSR2, sigval{0});
        }
    }
}

void IGuessNumber(const pid_t opponent_pid, const int start, const int end,
                  sigset_t *set)
{
    siginfo_t info;
    struct timespec timeout = {5, 0};
    pid_t my_pid = getpid();
    int sig = sigtimedwait(set, &info, &timeout);
    printf(RESET GREEN);
    printf("%u: I'll try to guess number!\n", my_pid);
    while (true)
    {
        int guess = rand() % end + start;
        printf("%u: I think it is %d\n", my_pid, guess);
        fflush(stdout);
        union sigval sv;
        sv.sival_int = guess;
        sigqueue(opponent_pid, SIGTERM, sv);
        last_sig = 0;

        int sig = sigtimedwait(set, &info, &timeout);
        printf(RESET GREEN);
        if (last_sig == SIGUSR1)
        {
            printf("%u: I knew i win!\n", my_pid);
            fflush(stdout);
            return;
        }
        else if (last_sig == SIGUSR2)
        {
            printf("%u: Oh... I'll try again!\n", my_pid);
            fflush(stdout);
        }
    }
}

void set_passive_settings(sigset_t *set)
{
    sigemptyset(set);
    struct sigaction sa;
    sa.sa_sigaction = get_signal_without_value;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGUSR2);
    sigaddset(&sa.sa_mask, SIGUSR1);

    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
}

void set_active_settings(sigset_t *set)
{
    struct sigaction sa;
    sa.sa_sigaction = get_signal_with_value;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGTERM);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigfillset(set);
    sigdelset(set, SIGTERM);
}

int main()
{
    pid_t child_id = fork();
    if (child_id == -1)
    {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    siginfo_t info;
    struct timespec timeout = {5, 0};
    srand(getpid() + time(NULL));
    sigset_t empty_set;
    sigfillset(&empty_set);
    sigdelset(&empty_set, SIGCONT);
    if (child_id == 0)
    {
        sigset_t set;

        while (true)
        {

            set_passive_settings(&set);
            IGuessNumber(getppid(), 0, 20, &set);
            set_active_settings(&set);
            printf(RESET);
            printf("-----------NEWGAME-----------\n");
            fflush(stdout);
            IKnowNumber(getppid(), 0, 20, &set);
            sigqueue(getppid(), SIGCONT, sigval{0});
            int sig = sigtimedwait(&empty_set, &info, &timeout);
        }
        exit(0);
    }
    else
    {
        sigset_t set;
        while (true)
        {
            set_active_settings(&set);
            IKnowNumber(child_id, 0, 20, &set);
            set_passive_settings(&set);
            IGuessNumber(child_id, 0, 20, &set);
            printf(RESET);
            printf("-----------NEWGAME-----------\n");
            printf("ARE YOU WINNING, SON?\n");
            printf(ARE_YOU_WINNING);
            printf("\n");
            fflush(stdout);
            int sig = sigtimedwait(&empty_set, &info, &timeout);
            sigqueue(getppid(), SIGCONT, sigval{0});
        }
    }

    return 0;
}