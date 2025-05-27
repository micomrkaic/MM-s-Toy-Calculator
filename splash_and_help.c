#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "function_list.h"

void whose_place(void) {
  printf("Your place or mine?\n");
  return;
}

void splash_screen(void) {
  time_t now = time(NULL);
  char* started = ctime(&now);
  
  printf("\n");
  printf("╔══════════════════════════════════════════════╗\n");
  printf("║                                              ║\n");
  printf("║     Mico's Matrix & Scalar RPN Calculator    ║\n");
  printf("║          Version alpha 0.1  (2025)           ║\n");
  printf("║                                              ║\n");
  printf("║  > Enter RPN expressions                     ║\n");
  printf("║  > Type 'help' for commands                  ║\n");
  printf("║  > Press 'q' or ctrl+d to quit               ║\n");
  printf("║                                              ║\n");
  printf("╚══════════════════════════════════════════════╝\n");
  printf("         Started on: %s", started);  // already has newline
  printf("\n");
}

void help_menu(void) {
  //  printf("\nMico's toy Matrix and Scalar RPN Calculator\n");
  printf("RPN Calculator for real and complex scalars and matrices\n");
  printf("Rules to enter data types\n");
  printf("    All inputs are case sensitive. Enter strings as \"string\".\n");
  printf("    Enter complex numbers as in: (1,3) or (-1.2e-4, 0.7e2).\n");
  printf("    Enter inline matrices as in J language [#rows #cols $ values].\n");
  printf("    Matrix entries can be real or complex.\n");
  printf("    Read matrix from file as [#rows, #cols, \"filename\"].\n");
  printf("Stack functions\n");
  printf("    drop, dup, swap, clst, nip, tuck, roll, over\n");
  printf("Matrix functions\n");
  printf("    Get individual matrix elements with get_aij; set them with set_aij.\n");  
  printf("    Special matrices: eye, ones, rand, randn, rrange.\n");  
  printf("    Manipulate matrices: reshape, diag, to_diag, split_mat\n");  
  printf("    Basic matrix statistics: csum, rsum, cmean, rmean, cvar, rvar\n");  
  printf("    Matrix min and max: cmin, rmin, cmax, rmax\n");  
  printf("Math functions\n");
  printf("    +, -, *, /, ^, [%%, %%chg] \n");
  printf("    Basic probability: npdf, ncdf, nquant {quantiles}\n");
  printf("    Other math functions: ln, exp, log, chs, inv, ^\n");
  printf("    Trig functions: sin, cos, tan, asin, acos, atan\n");
  printf("    Hyperbolic functions: sinh, cosh, tanh, asinh, acosh, atanh\n");
  printf("    Polynomials: evaluation and zeros\n");
  printf("    Special functions: gamma, ln_gamma, beta, ln_beta\n");
  printf("Comparison and logic functions\n");
  printf("    eq, leq, lt, gt, geq, neq, and,  or, not\n");
  printf("Complex functions\n");
  printf("    re, im, abs, arg, re2c, split_c, j2r {join 2 reals into complex}\n");
  printf("Constant\n");
  printf("    pi, e, gravity\n");
  printf("Registers functions\n");
  printf("    sto, rcl, pr {print registers}, save, load \n");
  printf("String functions\n");
  printf("    concat, s2u [to upper], s2l {to lower}, slen, srev {reverse}\n");
  printf("Financial and date functions\n");
  printf("    [pv, npv, irr], ddays, dateplus, today \n");
  printf("Outptut format\n");
  printf("    setprec {set print precision}, sfs {fix<->sci}\n");
  printf("\n **** Still to be done in [] brackets ****\n");
  printf("Plotting, 1-D integration\n");
  printf("\n");
 }

void list_all_functions(void) {
    printf("Available functions:\n\n");
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
    printf("Available functions (sorted):\n\n");
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
