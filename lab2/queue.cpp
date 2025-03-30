#include <unistd.h>
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <cstdlib>
#include <ctime>

#define RESET "\033[0m"
#define PURPLE "\033[35m"
#define GREEN "\033[32m"

// Функция для проверки ошибок
void check(int result, const char *message)
{
    if (result == -1)
    {
        perror(message);
        exit(EXIT_FAILURE);
    }
}

void IKnowNumber(mqd_t mq_to_guess, mqd_t mq_from_guess, const int start, const int end)
{
    pid_t my_pid = getpid();
    printf(RESET PURPLE);
    printf("%u: I know the number. Hey, try to guess!\n", my_pid);
    printf("%u: It's between %d and %d\n", my_pid, start, end);
    fflush(stdout);

    int tries_count = 1;
    int random_value = rand() % end + start;

    while (true)
    {
        int guessed_value;
        ssize_t bytes_read = mq_receive(mq_from_guess, (char *)&guessed_value, sizeof(int), NULL);
        check(bytes_read, "mq_receive failed");

        printf(RESET PURPLE);
        if (guessed_value == random_value)
        {
            printf("%u: You are right!!! Good job!\n", my_pid);
            printf("%u: You are win. Your tries count %d:\n", my_pid, tries_count);
            fflush(stdout);

            int answer = 1;
            check(mq_send(mq_to_guess, (const char *)&answer, sizeof(int), 0), "mq_send failed");
            break;
        }
        else
        {
            printf("%u: Your answer is incorrect, sorry. Try again.\n", my_pid);
            fflush(stdout);

            tries_count++;
            int answer = 0;
            check(mq_send(mq_to_guess, (const char *)&answer, sizeof(int), 0), "mq_send failed");
        }
    }
}

void IGuessNumber(mqd_t mq_to_know, mqd_t mq_from_know, const int start, const int end)
{
    pid_t my_pid = getpid();
    printf(RESET GREEN);
    printf("%u: I'll try to guess number!\n", my_pid);
    fflush(stdout);

    while (true)
    {
        int guess = rand() % end + start;
        printf("%u: I think it is %d\n", my_pid, guess);
        fflush(stdout);

        check(mq_send(mq_to_know, (const char *)&guess, sizeof(int), 0), "mq_send failed");

        int is_correct;
        ssize_t bytes_read = mq_receive(mq_from_know, (char *)&is_correct, sizeof(int), NULL);
        check(bytes_read, "mq_receive failed");

        printf(RESET GREEN);
        if (is_correct)
        {
            printf("%u: I knew I win!\n", my_pid);
            fflush(stdout);
            return;
        }
        else
        {
            printf("%u: Oh... I'll try again!\n", my_pid);
            fflush(stdout);
        }
    }
}

int main()
{
    // Атрибуты очереди
    struct mq_attr attr;
    attr.mq_flags = 0;             // Флаги (0 = блокирующий режим)
    attr.mq_maxmsg = 10;           // Максимальное количество сообщений в очереди
    attr.mq_msgsize = sizeof(int); // Максимальный размер одного сообщения
    attr.mq_curmsgs = 0;           // Текущее количество сообщений (0 при создании)

    // Удаление старых очередей
    mq_unlink("/know_to_guess");
    mq_unlink("/guess_to_know");

    // Создание очередей сообщений
    mqd_t mq_know_to_guess = mq_open("/know_to_guess", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
    mqd_t mq_guess_to_know = mq_open("/guess_to_know", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);

    check(mq_know_to_guess, "mq_open failed for know_to_guess");
    check(mq_guess_to_know, "mq_open failed for guess_to_know");

    pid_t child_id = fork();
    srand(getpid() + time(NULL));

    if (child_id == 0)
    {
        while (true)
        {
            IGuessNumber(mq_know_to_guess, mq_guess_to_know, 0, 20);
            printf("----------------NEW GAME-----------------\n");
            fflush(stdout);

            IKnowNumber(mq_guess_to_know, mq_know_to_guess, 0, 20);
        }
    }
    else
    {
        while (true)
        {
            IKnowNumber(mq_guess_to_know, mq_know_to_guess, 0, 20);

            IGuessNumber(mq_know_to_guess, mq_guess_to_know, 0, 20);
            printf("----------------NEW GAME-----------------\n");
            fflush(stdout);
        }
    }

    // Закрытие очередей и их удаление
    mq_close(mq_know_to_guess);
    mq_close(mq_guess_to_know);
    mq_unlink("/know_to_guess");
    mq_unlink("/guess_to_know");

    return 0;
}