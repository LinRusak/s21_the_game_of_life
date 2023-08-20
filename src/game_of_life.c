#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define N 25
#define M 80
#define DEFAULT_CONFIG "../configs/config1.txt"

int mod(int divisor, int denominator);
int check_adjasents(char **matrix, int n, int m, int i, int j);

int update(char **field, int n, int m, int *count, int *delay);
int input(FILE *src, char **field, int n, int m, int *count_ptr);

char **allocate_matrix(int n, int m);

void init_window();

int main(int argc, char **argv) {
    int exit_status = EXIT_SUCCESS;

    FILE *config;
    if (argc <= 1) {
        printf("Configuration file is not passed.\nTaking a default configuration...\n");
        config = fopen(DEFAULT_CONFIG, "r");
    } else {
        printf("Configuration file '%s' is passed.\n", argv[1]);
        config = fopen(argv[1], "r");
    }

    int n = N, m = M;
    char **field = allocate_matrix(n, m);  // 0 - dead, 1 - alive
    if (config != NULL && field != NULL) {
        int count = 0;
        if (input(config, field, n, m, &count) == EXIT_SUCCESS) {
            fclose(config);
            init_window();
            int delay = 100000;
            while (count > 0 && exit_status == EXIT_SUCCESS) {
                if (update(field, n, m, &count, &delay) == EXIT_FAILURE) {
                    perror("Error while screen updating has occured");
                    exit_status = EXIT_FAILURE;
                }
            }
            endwin();
        } else {
            fprintf(stderr, "Incorrect configuration file has passed.\n");
            fclose(config);
        }

        free(field);
    } else {
        if (field) free(field);
        if (config) {
            perror("An error while field allocation has occured");
            fclose(config);
        } else
            perror("An error while opening file has occured");
        exit_status = EXIT_FAILURE;
    }

    return exit_status;
}

void init_window() {
    WINDOW *stdscr;
    stdscr = initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);
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
    printw("+");
    for (int i = 0; i < w; i++) printw("-");
    printw("+\n");
}

int input(FILE *src, char **field, int n, int m, int *count) {
    int exit_status = EXIT_SUCCESS;
    for (int i = 0; i < n && !exit_status; i++) {
        for (int j = 0; j < m && !exit_status; j++) {
            do {
                if (fscanf(src, "%c", field[i] + j) != 1 || !(strchr("0 |\n", field[i][j])))
                    exit_status = EXIT_FAILURE;
            } while (strchr("|\n", field[i][j]) && exit_status == 0);
            field[i][j] = (field[i][j] == ' ') ? 0 : 1;
            if (field[i][j]) (*count)++;
        }
    }
    return exit_status;
}

void draw(char **field, int n, int m, int *count, char **next_field) {
    clear();
    print_horizontal_border(m);
    for (int i = 0; i < n; i++) {
        printw("|");
        for (int j = 0; j < m; j++) {
            printw("%c", field[i][j] ? 'o' : ' ');
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
        printw("|\n");
    }
    print_horizontal_border(m);
    printw("+/- increase and decrease game speed\n");
    printw("q - exit");
}

int update(char **field, int n, int m, int *count, int *delay) {
    static const double step = 1.5;
    int exit_status = EXIT_SUCCESS;

    char **next_field = allocate_matrix(n, m);
    if (next_field != NULL) {
        draw(field, n, m, count, next_field);
        memcpy(field + n, next_field + n, n * m * sizeof(char));

        char c = getch();
        if (c != ERR) {
            switch (c) {
                case '-':
                    *delay = (int)(*delay * step);
                    break;
                case '+':
                case '=':
                    *delay = (int)(*delay / step);
                    break;
                case 'q':
                    *count = 0;
                    break;
            }
        }

        usleep(*delay);
        free(next_field);
    } else
        exit_status = EXIT_FAILURE;
    return exit_status;
}
