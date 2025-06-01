#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "function_list.h"

void whose_place(void) {
  printf("Your place or mine?\n");
  return;
}

#include <stdio.h>

void splash_screen(void) {
  time_t now = time(NULL);
  char* started = ctime(&now);
  
  printf("\n");
  printf("╔══════════════════════════════════════════════╗\n");
  printf("║                                              ║\n");
  printf("║     Mico's Matrix & Scalar RPN Calculator    ║\n");
  printf("║          Version beta 0.1  (2025)            ║\n");
  printf("║                                              ║\n");
  printf("║  > Enter RPN expressions                     ║\n");
  printf("║  > Type 'help' for commands                  ║\n");
  printf("║  > Press 'q' or ctrl+d to quit               ║\n");
  printf("║                                              ║\n");
  printf("╚══════════════════════════════════════════════╝\n");
  printf("         Started on: %s", started);  // already has newline
  printf("\n");
}


#define BOLD      "\033[1m"
#define ITALIC    "\033[3m"
#define UNDERLINE "\033[4m"
#define RESET     "\033[0m"

#define title(s) printf(BOLD s RESET "\n")
#define subtitle(s) printf(UNDERLINE s RESET "\n")

void help_menu(void) {
  //  printf("\n\nMico's toy Matrix and Scalar RPN Calculator\n");
  printf("\n");
  title("RPN Calculator for real and complex scalars and matrices");
  subtitle("Entering data");
  printf("    All inputs are case sensitive. Enter strings as \"string\".\n");
  printf("    Enter complex numbers as in: (1,3) or (-1.2e-4, 0.7e2).\n");
  printf("    Enter inline matrices as in J language [#rows #cols $ values].\n");
  printf("    Matrix entries can be real or complex.\n");
  printf("    Read matrix from file as [#rows, #cols, \"filename\"].\n");
  subtitle("Stack manipulations");
  printf("    drop, dup, swap, clst, nip, tuck, roll, over\n");
  subtitle("Matrix functions");
  printf("    Get individual matrix elements with get_aij; set them with set_aij.\n");  
  printf("    Special matrices: eye, ones, rand, randn, rrange.\n");  
  printf("    Manipulation: reshape, diag, to_diag, split_mat, join_h, join_v \n");
  printf("    Cummulative sums and products: cumsum_r, cumsum_c, cumprod_r, cumprod_c \n");  
  printf("    Basic matrix statistics: csum, rsum, cmean, rmean, cvar, rvar\n");  
  printf("    Matrix min and max: cmin, rmin, cmax, rmax\n");  
  printf("    Linear algebra: tran, ', det, minv, pinv, chol, eig, svd\n");  
  subtitle("Math functions");
  printf("    +, -, *, /, ^, pct, pctchg \n");
  printf("    Basic probability: npdf, ncdf, nquant {quantiles}\n");
  printf("    Other math functions: ln, exp, log, chs, inv, ^\n");
  printf("    Trigonometry: sin, cos, tan, asin, acos, atan\n");
  printf("    Hyperbolic: sinh, cosh, tanh, asinh, acosh, atanh\n");
  printf("    Polynomials: evaluation and zeros\n");
  printf("    Special functions: gamma, ln_gamma, beta, ln_beta\n");
  subtitle("Comparison and logic functions");
  printf("    eq, leq, lt, gt, geq, neq, and,  or, not\n");
  subtitle("Complex functions");
  printf("    re, im, abs, arg, re2c, split_c, j2r {join 2 reals into complex}\n");
  subtitle("Constants");
  printf("    pi, e, gravity, inf, nan\n");
  subtitle("Register functions");
  printf("    sto, rcl, pr {print registers}, save, load, ffr \n");
  subtitle("String functions");
  printf("    scon, s2u, s2l, slen, srev, int2str\n");
  subtitle("Financial and date functions");
  printf("    npv, irr, ddays, dateplus, today \n");
  subtitle("Output format options");
  printf("    setprec {set print precision}, sfs {fix<->sci}\n");
  printf("\n");
 }

void list_all_functions(void) {
    printf("Built-in functions:\n\n");
    int count = 0;
    for (int i = 0; function_names[i] != NULL; ++i) {
        printf("%-16s", function_names[i]);  // left-align in 16-char width
        count++;
        if (count % 4 == 0)
            printf("\n");
    }
    if (count % 4 != 0)
        printf("\n");  // final newline if last line wasn't complete
}

// Compare function for qsort
int compare_strings(const void* a, const void* b) {
    const char* sa = *(const char**)a;
    const char* sb = *(const char**)b;
    return strcmp(sa, sb);
}

void list_all_functions_sorted(void) {
    // Step 1: Count functions
    int count = 0;
    while (function_names[count] != NULL) count++;

    // Step 2: Copy to a temporary array
    const char** sorted = malloc(count * sizeof(char*));
    if (!sorted) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    for (int i = 0; i < count; ++i)
        sorted[i] = function_names[i];

    // Step 3: Sort
    qsort(sorted, count, sizeof(char*), compare_strings);

    // Step 4: Print 6 per row
    printf("Built-in functions:\n\n");
    for (int i = 0; i < count; ++i) {
        printf("%-16s", sorted[i]);
        if ((i + 1) % 6 == 0)
            printf("\n");
    }
    if (count % 6 != 0)  printf("\n");
    printf("\n");

    // Cleanup
    free(sorted);
}
