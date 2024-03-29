#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define N 25
#define M 80
#define DEFAULT_CONFIG "../configs/config1.txt"

#define TICK 1000

void init_window();

char **allocate_matrix(int n, int m);

int int_min(int a, int b);
int int_max(int a, int b);
int mod(int divisor, int denominator);
int check_adjasents(char **matrix, int n, int m, int i, int j);

void print_horizontal_border(int w);
int input(FILE *src, char **field, int n, int m, int *count_ptr);
void draw(char **field, int n, int m, int *count, char **next_field);
void handle_key_press(int *delay, int *count, char *pause);
int update(char **field, int n, int m, int *count, int *delay, int *wait, char *pause);

int main(void) {
    int exit_status = EXIT_SUCCESS;

    int n = N, m = M;
    char **field = allocate_matrix(n, m);  // 0 - dead, 1 - alive
    if (field != NULL) {
        int count = 0;
        FILE *check = NULL;
        int input_status = EXIT_FAILURE;
        if ((input_status = input(stdin, field, n, m, &count)) == EXIT_SUCCESS &&
            (check = freopen("/dev/tty", "r", stdin))) {
            init_window();
            int delay = 100000, wait = 0;
            char pause = 0;
            while (count > 0 && exit_status == EXIT_SUCCESS) {
                if (update(field, n, m, &count, &delay, &wait, &pause) == EXIT_FAILURE) {
                    perror("Error while screen updating has occured");
                    exit_status = EXIT_FAILURE;
                }
            }
            endwin();
        } else {
            if (input_status == EXIT_FAILURE)
                fprintf(stderr, "Incorrect configuration has entered.\n");
            else if (check == NULL)
                perror("An error while opening stdin occured");
            exit_status = EXIT_FAILURE;
        }

        free(field);
    } else {
        perror("An error while field allocation has occured");
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

int int_min(int a, int b) { return (a < b) ? a : b; }

int int_max(int a, int b) { return (a > b) ? a : b; }

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
    printw("p - pause the game\n");
    printw("q - exit");
}

void handle_key_press(int *delay, int *count, char *pause) {
    static const double step = 1.5;
    static const int max_delay = 500000, min_delay = TICK;

    char c = getch();
    if (c != ERR) {
        switch (c) {
            case '-':
                *delay = int_min((int)(*delay * step), max_delay);
                break;
            case '+':
            case '=':
                *delay = int_max((int)(*delay / step), min_delay);
                break;
            case 'q':
                *count = 0;
                break;
            case 'p':
                *pause = !(*pause);
                break;
        }
    }
}

int update(char **field, int n, int m, int *count, int *delay, int *wait, char *pause) {
    int exit_status = EXIT_SUCCESS;

    if (*wait <= 0 && !(*pause)) {
        char **next_field = allocate_matrix(n, m);
        if (next_field != NULL) {
            draw(field, n, m, count, next_field);
            memcpy(field + n, next_field + n, n * m * sizeof(char));
            free(next_field);
        } else
            exit_status = EXIT_FAILURE;
        *wait = *delay / TICK;
    }

    handle_key_press(delay, count, pause);

    usleep(TICK);
    (*wait)--;

    return exit_status;
}
