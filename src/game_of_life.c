#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define N 25
#define M 80

int pos_mod(int divisor, int denominator) { return (divisor % denominator + denominator) % denominator; }

int check_adjasents(char *matrix, int n, int m, int i, int j) {
    int counter = 0;
    for (int di = -1; di <= 1; di++) {
        for (int dj = -1; dj <= 1; dj++) {
            if (di == 0 && dj == 0) continue;
            if (matrix[pos_mod(i + di, n) * m + pos_mod(j + dj, m)]) counter++;
        }
    }
    return counter;
}

int main(void) {
    char field[N][M];  // 0 - dead, 1 - alive
    char next_field[N][M];
    int n = N, m = M;

    if (freopen("config1.txt", "r", stdin) == NULL) return -1;

    int count = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            scanf(" %c", field[i] + j);
            field[i][j] -= '0';
            if (field[i][j]) count++;
        }
    }

    while (count) {
        printf("+");
        for (int i = 0; i < m; i++) printf("-");
        printf("+\n");
        for (int i = 0; i < n; i++) {
            printf("|");
            for (int j = 0; j < m; j++) {
                printf("%c", field[i][j] ? 'o' : ' ');
                int neighbours = check_adjasents((char *)field, n, m, i, j);
                if (field[i][j] && neighbours >= 2 && neighbours <= 3)
                    next_field[i][j] = 1;
                else if (!field[i][j] && neighbours == 3)
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
        printf("+");
        for (int i = 0; i < m; i++) printf("-");
        printf("+\n");
        memcpy(field, next_field, n * m * sizeof(field[0][0]));
        sleep(1);
    }

    return 0;
}
