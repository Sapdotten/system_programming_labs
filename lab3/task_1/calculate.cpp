#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>

int determine_matrix_size(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == -1) {
        perror("Ошибка получения информации о файле");
        exit(EXIT_FAILURE);
    }

    off_t filesize = st.st_size;
    if (filesize == 0) {
        fprintf(stderr, "Файл пустой\n");
        exit(EXIT_FAILURE);
    }

    long num_elements = filesize / sizeof(double);

    int size = 0;
    while (size * size < num_elements) {
        size++;
    }

    if (size * size != num_elements) {
        fprintf(stderr, "Файл не содержит квадратную матрицу\n");
        exit(EXIT_FAILURE);
    }

    return size;
}

void read_matrix(const char *filename, double *matrix, int size) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Не удалось открыть файл для чтения");
        exit(EXIT_FAILURE);
    }

    ssize_t to_read = size * size * sizeof(double);
    ssize_t read_bytes = read(fd, matrix, to_read);
    if (read_bytes != to_read) {
        perror("Ошибка чтения файла");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
}

void multiply_matrices(double *A, double *B, double *C, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double sum = 0.0;
            for (int k = 0; k < size; k++) {
                sum += A[i * size + k] * B[k * size + j];
            }
            C[i * size + j] = sum;
        }
    }
}

void print_matrix(double *matrix, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            printf("%8.2f ", matrix[i * size + j]);
        }
        printf("\n");
    }
}

int main() {
    const char *fileA = "A";
    const char *fileB = "B";

    int sizeA = determine_matrix_size(fileA);
    int sizeB = determine_matrix_size(fileB);

    if (sizeA != sizeB) {
        fprintf(stderr, "Размеры матриц не совпадают!\n");
        exit(EXIT_FAILURE);
    }

    int size = sizeA;

    double *A = (double *)malloc(size * size * sizeof(double));
    double *B = (double *)malloc(size * size * sizeof(double));
    double *C = (double *)malloc(size * size * sizeof(double));

    if (!A || !B || !C) {
        perror("Ошибка выделения памяти");
        exit(EXIT_FAILURE);
    }

    read_matrix(fileA, A, size);
    read_matrix(fileB, B, size);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    multiply_matrices(A, B, C, size);

    gettimeofday(&end, NULL);

    double elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0; 
    elapsed_time += (end.tv_usec - start.tv_usec) / 1000.0;

    if (size <= 10) {
        printf("Результат умножения матриц:\n");
        print_matrix(C, size);
    }

    printf("Время умножения матриц: %.3f мс\n", elapsed_time);

    free(A);
    free(B);
    free(C);

    return 0;
}
