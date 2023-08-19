#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define N 25
#define M 80
#define DEFAULT_CONFIG "config1.txt"

int positive_mod(int divisor, int denominator);
int check_adjasents(char *matrix, int n, int m, int i, int j);

int draw_and_update(char *field, int n, int m, int count);

int input(char *field, int n, int m, int *count_ptr);

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
    if (config != NULL) {
        char field[N][M];  // 0 - dead, 1 - alive
        int n = N, m = M;

        int count = 0;
        if (input((char *)field, n, m, &count) == EXIT_SUCCESS) {
            while (count >= 0) {
                count = draw_and_update((char *)field, n, m, count);
            }
        }
        fclose(config);
    } else {
        perror("An error has occured");
        exit_status = EXIT_FAILURE;
    }

    return exit_status;
}

char *get(char *field, int i, int j) { return field + i * M + j; }

int positive_mod(int divisor, int denominator) { return (divisor % denominator + denominator) % denominator; }

int check_adjasents(char *matrix, int n, int m, int i, int j) {
    int counter = 0;
    for (int di = -1; di <= 1; di++) {
        for (int dj = -1; dj <= 1; dj++) {
            if (di == 0 && dj == 0) continue;
            if (*get(matrix, positive_mod(i + di, n), positive_mod(j + dj, m))) counter++;
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

int input(char *field, int n, int m, int *count) {
    int exit_status = EXIT_SUCCESS;
    for (int i = 0; i < n && !exit_status; i++) {
        for (int j = 0; j < m && !exit_status; j++) {
            if (scanf(" %c", get(field, i, j)) != 1 ||
                !(*get(field, i, j) == '0' || *get(field, i, j) == '1'))
                exit_status = EXIT_FAILURE;
            *get(field, i, j) -= '0';
            if (*get(field, i, j)) (*count)++;
        }
    }
    return exit_status;
}

int draw_and_update(char *field, int n, int m, int count) {
    char next_field[N][M] = {0};
    printf("\033c");
    print_horizontal_border(m);
    for (int i = 0; i < n; i++) {
        printf("|");
        for (int j = 0; j < m; j++) {
            printf("%c", *get(field, i, j) ? 'o' : ' ');
            int neighbours = check_adjasents((char *)field, n, m, i, j);
            if (neighbours == 3 || (*get(field, i, j) && neighbours == 2))
                next_field[i][j] = 1;
            else
                next_field[i][j] = 0;

            if (*get(field, i, j) && !next_field[i][j])
                count--;
            else if (!*get(field, i, j) && next_field[i][j])
                count++;
        }
        puts("|");
    }
    print_horizontal_border(m);
    memcpy(field, (char *)next_field, n * m * sizeof(char));
    usleep(500000);
    return count;
}
