#include <unistd.h>
#include <iostream>
#include <cstring>
#define RESET "\033[0m"
#define PURPLE "\033[35m"
#define GREEN "\033[32m"

void IKnowNumber(int reader, int writer, int sync_writer, const int start, const int end)
{

  pid_t my_pid = getpid();
  int game_start = 1;
  printf(RESET PURPLE);
  printf("%u: I know the number. Hey, try to guess!\n", my_pid);
  printf("%u: It's between %d and %d\n", my_pid, start, end);
  fflush(stdout);
  int tries_count = 1;
  int random_value = rand() % end + start;
  write(sync_writer, &game_start, sizeof(int));
  while (true)
  {
    int guessed_value;
    read(reader, &guessed_value, sizeof(int));
    printf(RESET PURPLE);
    if (guessed_value == random_value)
    {
      printf("%u: You are right!!! Good job!\n", my_pid);
      printf("%u: You are win. Your tries count %d:\n", my_pid, tries_count);
      fflush(stdout);
      int answer = 1;
      write(writer, &answer, sizeof(int));
      break;
    }
    else
    {
      printf("%u: Your answer is incorrect, sorry. Try again.\n", my_pid);
      fflush(stdout);
      tries_count++;
      int answer = 0;
      write(writer, &answer, sizeof(int));
    }
  }
}

void IGuessNumber(int reader, int writer, int sync_reader, const int start, const int end)
{
  pid_t my_pid = getpid();
  int game_start;
  read(sync_reader, &game_start, sizeof(int));
  printf(RESET GREEN);
  printf("%u: I'll try to guess number!\n", my_pid);
  while (true)
  {
    int guess = rand() % end + start;
    int is_correct;
    printf("%u: I think it is %d\n", my_pid, guess);
    fflush(stdout);
    write(writer, &guess, sizeof(int));
    read(reader, &is_correct, sizeof(int));
    printf(RESET GREEN);
    if (is_correct)
    {
      printf("%u: I knew i win!\n", my_pid);
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
  int know_to_guess[2], guess_to_know[2], sync_pipe[2];
  pipe(know_to_guess);
  pipe(guess_to_know);
  pipe(sync_pipe);
  char buff[100];
  pid_t child_id = fork();
  srand(getpid() + time(0));
  if (child_id == 0)
  {
    while (true)
    {
      IGuessNumber(know_to_guess[0], guess_to_know[1], sync_pipe[0], 0, 20);
      printf("----------------NEW GAME-----------------\n");
      fflush(stdout);
      IKnowNumber(guess_to_know[0], know_to_guess[1], sync_pipe[1], 0, 20);
    }
  }
  else
  {
    while (true)
    {
      IKnowNumber(guess_to_know[0], know_to_guess[1], sync_pipe[1], 0, 20);
      IGuessNumber(know_to_guess[0], guess_to_know[1], sync_pipe[0], 0, 20);
      printf("----------------NEW GAME-----------------\n");
      fflush(stdout);
    }
  }

  return 0;
}