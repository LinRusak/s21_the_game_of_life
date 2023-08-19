#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define N 25
#define M 80
#define DEFAULT_CONFIG "config1.txt"

int mod(int divisor, int denominator);
int check_adjasents(char **matrix, int n, int m, int i, int j);

int draw_and_update(char **field, int n, int m, int count);
int input(char **field, int n, int m, int *count_ptr);

char **allocate_matrix(int n, int m);

int main(int argc, char **argv) {
    int exit_status = EXIT_SUCCESS;

    FILE *config;
    if (argc <= 1) {
        printf("Configuration file is not passed.\nTaking a default configuration...\n");
        config = freopen(DEFAULT_CONFIG, "r", stdin);
    } else {
        printf("Configuration file '%s' is passed.\n", argv[1]);
        config = freopen(argv[1], "r", stdin);
    }

    int n = N, m = M;
    char **field = allocate_matrix(n, m);  // 0 - dead, 1 - alive
    if (config != NULL && field != NULL) {
        int count = 0;
        if (input(field, n, m, &count) == EXIT_SUCCESS) {
            while (count >= 0 && exit_status == EXIT_SUCCESS) {
                count = draw_and_update(field, n, m, count);
                if (count == -1) exit_status = EXIT_FAILURE;
            }
        }
        free(field);
        fclose(config);
    } else {
        if (field) free(field);
        if (config) fclose(config);
        exit_status = EXIT_FAILURE;
    }

    if (exit_status != EXIT_SUCCESS) perror("An error has occured");

    return exit_status;
}

// Matrix allocation

char **allocate_matrix(int n, int m) {
    char **matrix = malloc(n * m * sizeof(char) + n * sizeof(char *));
    for (int i = 0; i < n; i++) matrix[i] = ((char *)(matrix + n)) + m * i;
    memset(matrix + n, 0, n * m * sizeof(char));
    return matrix;
}

// Secondary functions

int mod(int divisor, int denominator) { return (divisor % denominator + denominator) % denominator; }

int check_adjasents(char **matrix, int n, int m, int i, int j) {
    int counter = 0;
    for (int di = -1; di <= 1; di++) {
        for (int dj = -1; dj <= 1; dj++) {
            if (di == 0 && dj == 0) continue;
            if (matrix[mod(i + di, n)][mod(j + dj, m)]) counter++;
        }
    }
    return counter;
}

// Input output section

void print_horizontal_border(int w) {
    printf("+");
    for (int i = 0; i < w; i++) printf("-");
    printf("+\n");
}

int input(char **field, int n, int m, int *count) {
    int exit_status = EXIT_SUCCESS;
    for (int i = 0; i < n && !exit_status; i++) {
        for (int j = 0; j < m && !exit_status; j++) {
            if (scanf(" %c", field[i] + j) != 1 || !(field[i][j] == '0' || field[i][j] == '1'))
                exit_status = EXIT_FAILURE;
            field[i][j] -= '0';
            if (field[i][j]) (*count)++;
        }
    }
    return exit_status;
}

int draw_and_update(char **field, int n, int m, int count) {
    char **next_field = allocate_matrix(n, m);
    if (next_field != NULL) {
        printf("\033c");
        print_horizontal_border(m);
        for (int i = 0; i < n; i++) {
            printf("|");
            for (int j = 0; j < m; j++) {
                printf("%c", field[i][j] ? 'o' : ' ');
                int neighbours = check_adjasents(field, n, m, i, j);
                if (neighbours == 3 || (field[i][j] && neighbours == 2))
                    next_field[i][j] = 1;
                else
                    next_field[i][j] = 0;

                if (field[i][j] && !next_field[i][j])
                    count--;
                else if (!field[i][j] && next_field[i][j])
                    count++;
            }
            puts("|");
        }
        print_horizontal_border(m);
        memcpy(field + n, next_field + n, n * m * sizeof(char));
        usleep(500000);
        free(next_field);
    } else
        count = -1;
    return count;
}
