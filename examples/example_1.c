// ──────────────────────────────────────────────────────────────
//  Example: addition_subscript_equivalence.c
//  Source: "Writing a C Compiler" writing-a-c-compiler-tests
//  URL:    https://github.com/nlsandler/writing-a-c-compiler-tests
//  License: MIT (original)
//  Author: Nora Sandler
// ──────────────────────────────────────────────────────────────

int main(void)
{
    unsigned long x[300][5];
    for (int i = 0; i < 300; i = i + 1) {
        for (int j = 0; j < 5; j = j + 1) {
            x[i][j] = i * 5 + j;
        }
    }

    if (*(*(x + 20) + 3) != x[20][3]) {
        return 1;
    }

    if (&(*(*(x + 290) + 3)) != &x[290][3]) {
        return 2;
    }

    for (int i = 0; i < 300; i = i + 1) {
        for (int j = 0; j < 5; j = j + 1) {
            if (*(*(x + i) + j) != x[i][j]) {
                return 3;
            }
        }
    }

    *(*(x + 275) + 4) = 22000ul;
    if (x[275][4] != 22000ul) {
        return 4;
    }
    return 0;
}