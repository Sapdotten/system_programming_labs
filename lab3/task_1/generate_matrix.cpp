#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h> 
#include <unistd.h> 
#include <sys/stat.h> 

void generate_matrix(double *matrix, int size) {
    for (int i = 0; i < size * size; i++) {
        matrix[i] = (double)rand() / RAND_MAX * 100.0;
    }
}

void save_matrix_to_file(double *matrix, int size, const char *filename) {
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Не удалось открыть файл для записи");
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_to_write = size * size * sizeof(double);
    ssize_t written = write(fd, matrix, bytes_to_write);
    if (written != bytes_to_write) {
        perror("Ошибка записи в файл");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
}

int determine_matrix_size(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == -1) {
        perror("Не удалось получить информацию о файле");
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
        fprintf(stderr, "Размер файла не соответствует квадратной матрице\n");
        exit(EXIT_FAILURE);
    }

    return size;
}

int main() {
    int size;
    printf("Введите размер матрицы: ");
    scanf("%d", &size);

    double *matrix = (double *)malloc(size * size * sizeof(double));
    if (matrix == NULL) {
        perror("Ошибка выделения памяти для матрицы");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    generate_matrix(matrix, size);
    char *filename;
    printf("Введите название файла: ");
    scanf("%s", filename);
    save_matrix_to_file(matrix, size, filename);
    printf("Матрица сохранена в файл '%s'\n", filename);

    free(matrix);

    size = determine_matrix_size(filename);
    printf("Определённый размер матрицы: %d x %d\n", size, size);
    return 0;
}
