/*
 * This file is part of Mico's toy RPN Calculator
 *
 * Mico's toy RPN Calculator is free software: 
 * you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mico's toy RPN Calculator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mico's toy RPN Calculator. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <stdbool.h>
#include <gsl/gsl_complex_math.h>
#include "stack.h"
#include "unary_fun.h"
#include "binary_fun.h"
#include "stat_fun.h"
#include "spec_fun.h"
#include "math_helpers.h"

double safe_frac(double a)
{
  double intpart;
  return modf(a, &intpart);
}

double safe_int(double a)
{
  double intpart;
  modf(a, &intpart);
  return intpart;
}

#include <complex.h>
#include <math.h>

double complex safe_frac_complex(double complex z) {
  double r_int, i_int;
  double real_frac = modf(creal(z), &r_int);
  double imag_frac = modf(cimag(z), &i_int);
  return real_frac + imag_frac * I;
}

double complex safe_int_complex(double complex z) {
  double r_int, i_int;
  modf(creal(z), &r_int);
  modf(cimag(z), &i_int);
  return r_int + i_int * I;
}

double negate_real(double x) {
  return -x;
}

double log10_real(double x) {
  return log(x) / log(10.0);
}

gsl_complex log10_complex(gsl_complex z) {
    gsl_complex ln_z = gsl_complex_log(z);
    return gsl_complex_div_real(ln_z, log(10.0));
}

gsl_complex negate_complex(gsl_complex x) {
    gsl_complex result;
    GSL_SET_COMPLEX(&result, -GSL_REAL(x), -GSL_IMAG(x));
    return result;
}

double one_over_real(double x) {
  if (x != 0.0) {
    return 1.0/x;
  } else {
    printf("Division by zero not allowed!\n");
    return 0.0;
  }
}

gsl_complex one_over_complex(gsl_complex x) {
  //  complex double a=to_double_complex(x);
 if (GSL_REAL(x) != 0.0 || GSL_IMAG(x) != 0.0) {
   return gsl_complex_inverse(x);
  } else {
    printf("Division by zero not allowed!\n");
    return gsl_complex_rect(0.0, 0.0);
  }
}

double complex log10_complex_not_gsl(double complex z) {
  return clog(z)/log(10.0);
}

gsl_complex to_gsl_complex(double complex z) {
    return gsl_complex_rect(creal(z), cimag(z));
}

double complex to_double_complex(gsl_complex z) {
    return GSL_REAL(z) + GSL_IMAG(z) * I;
}

bool is_zero_complex(double complex z) {
  return (creal(z) == 0.0) && (cimag(z) == 0.0);
}

// ****************************************************************
// ************************* Wrappers *****************************
// ****************************************************************

#define DEFINE_UNARY_WRAPPER(name, real_fn, complex_fn)             \
void name##_wrapper(Stack* stack) {                                 \
    ValueType top_type = stack_top_type(stack);                     \
    switch (top_type) {                                             \
    case TYPE_REAL:                                                 \
        apply_real_unary(stack, real_fn);                           \
        return;                                                     \
    case TYPE_COMPLEX:                                              \
        apply_complex_unary(stack, complex_fn);                        \
        return;                                                     \
    case TYPE_MATRIX_REAL:                                          \
        apply_real_matrix_unary_inplace(stack, real_fn);            \
        return;                                                     \
    case TYPE_MATRIX_COMPLEX:                                       \
        apply_complex_matrix_unary_inplace(stack, complex_fn);      \
        return;                                                     \
    default:                                                        \
        fprintf(stderr, "Unsupported type in %s\n", #name);         \
	return;							    \
    }								    \
}

// These functions do not mutate types
DEFINE_UNARY_WRAPPER(sin, sin, csin)
DEFINE_UNARY_WRAPPER(cos, cos, ccos)
DEFINE_UNARY_WRAPPER(tan, tan, ctan)
DEFINE_UNARY_WRAPPER(asin, asin, casin)
DEFINE_UNARY_WRAPPER(acos, acos, cacos)
DEFINE_UNARY_WRAPPER(atan, atan, catan)
DEFINE_UNARY_WRAPPER(sinh, sinh, csinh)
DEFINE_UNARY_WRAPPER(cosh, cosh, ccosh)
DEFINE_UNARY_WRAPPER(tanh, tanh, ctanh)
DEFINE_UNARY_WRAPPER(asinh, asinh, casinh)
DEFINE_UNARY_WRAPPER(acosh, acosh, cacosh)
DEFINE_UNARY_WRAPPER(atanh, atanh, catanh)
DEFINE_UNARY_WRAPPER(exp, exp, cexp)
DEFINE_UNARY_WRAPPER(chs, negate_real, negate_complex)
DEFINE_UNARY_WRAPPER(inv, one_over_real, one_over_complex)
DEFINE_UNARY_WRAPPER(frac, safe_frac, safe_frac_complex)
DEFINE_UNARY_WRAPPER(intg, safe_int, safe_int_complex)

// **** Start: Define helpers to evaluate logical not **** 
static inline double real_not(double x) {
    return !x;
}

static inline complex double complex_not(double complex z) {
  return (creal(z) == 0.0) + I*(cimag(z) == 0.0);
}

DEFINE_UNARY_WRAPPER(logical_not, real_not, complex_not)
// **** End: Define helpers to evaluate logical not **** 


// These functions do mutate types
void im_wrapper(Stack *stack) {
  ValueType a = stack_top_type(stack);
  if (a == TYPE_REAL) {
    StackElement b = pop(stack);
    push_real(stack, cimag(b.real));
  }  else if (a == TYPE_COMPLEX) {
    StackElement b = pop(stack);	
    push_real(stack, cimag(b.complex_val));}
  else if (a == TYPE_MATRIX_REAL)
    {
      // To Do: push a matrix of zeroes of the right size
    }
  else if (a == TYPE_MATRIX_COMPLEX)
    complex_matrix_imag_part(stack);
  else
    printf("im: unsupported type\n");
  return;
}

void re_wrapper(Stack *stack) {
  ValueType a = stack_top_type(stack);
  if (a == TYPE_REAL) {
    return; // nothing needs to be done
  }  else if (a == TYPE_COMPLEX) {
    StackElement b = pop(stack);	
    push_real(stack, creal(b.complex_val));}
  else if (a == TYPE_MATRIX_REAL)   {
      return; // nothing needs to be done
  } else if (a == TYPE_MATRIX_COMPLEX)
    complex_matrix_real_part(stack);
  else
    printf("re: unsupported type\n");
  return;
}

void abs_wrapper(Stack *stack) {
  ValueType a = stack_top_type(stack);
  if (a == TYPE_REAL) {
    StackElement b = pop(stack);	
    push_real(stack,fabs(b.real));
    return;
  }  else if (a == TYPE_COMPLEX) {
    StackElement b = pop(stack);	
    push_real(stack, cabs(b.complex_val));}
  else if (a == TYPE_MATRIX_REAL)   {
    apply_real_matrix_unary_inplace(stack, fabs);
    return; 
  } else if (a == TYPE_MATRIX_COMPLEX)
    // to do complex abs element by element
    return;
  else
    printf("abs: unsupported type\n");
  return;
}

void arg_wrapper(Stack *stack) {
  ValueType a = stack_top_type(stack);
  if (a == TYPE_REAL) {
    StackElement b = pop(stack);	
    push_real(stack,carg(b.real));
    return;
  }  else if (a == TYPE_COMPLEX) {
    StackElement b = pop(stack);	
    push_real(stack, carg(b.complex_val));}
  else if (a == TYPE_MATRIX_REAL)   {
    // To do: push a matrix of zeroes
    return; 
  } else if (a == TYPE_MATRIX_COMPLEX)
    // to do complex arg element by element
    return;
  else
    printf("arg: unsupported type\n");
  return;
}

void ln_wrapper(Stack *stack) {
  StackElement a = pop(stack);
  if (a.type == TYPE_REAL) {
    if (a.real >= 0.0) {
      push_real(stack, log(a.real));
      return;
    } else {
      push_complex(stack, clog(a.real+0*I));
      return;
    }} else if (a.type == TYPE_COMPLEX) {
    push_complex(stack, clog(a.real));
    return;
  } else {
    printf("ln: unsupported type\n");
    return;
  }
}

void log_wrapper(Stack *stack) {
  StackElement a = pop(stack);
  if (a.type == TYPE_REAL) {
    if (a.real >= 0.0) {
      push_real(stack, log10(a.real));
      return;
    } else {
      push_complex(stack, clog(a.real)/log(10.0));
      return;
    }} else if (a.type == TYPE_COMPLEX) {
    push_complex(stack, clog(a.real)/log(10.0));
    return;
  } else {
    printf("log: unsupported type\n");
    return;
  }
}

void sqrt_wrapper(Stack *stack) {
  StackElement a = pop(stack);
  if (a.type == TYPE_REAL) {
    if (a.real >= 0.0) {
      push_real(stack, sqrt(a.real));
      return;
  } else {
    push_complex(stack, csqrt(a.complex_val));
    return;
    }} else if (a.type == TYPE_COMPLEX) {
    push_complex(stack, csqrt(a.complex_val));
    return;
  }  else {
    printf("sqrt: unsupported type\n");
    return;
  }
}

void npdf_wrapper(Stack *stack) {
  ValueType a = stack_top_type(stack);
  if (a == TYPE_REAL) {
    StackElement b = pop(stack);
    push_real(stack, standard_normal_pdf(b.real));
    return;
  }  else if (a == TYPE_COMPLEX) {
    printf("npdf: unsupported type\n");
  } else if (a == TYPE_MATRIX_REAL) {
    apply_real_matrix_unary_inplace(stack, standard_normal_pdf);
    return;
  } else if (a == TYPE_MATRIX_COMPLEX)
    printf("npdf: unsupported type\n");
  return;
}

void ncdf_wrapper(Stack *stack) {
  ValueType a = stack_top_type(stack);
  if (a == TYPE_REAL) {
    StackElement b = pop(stack);
    push_real(stack, standard_normal_cdf(b.real));
    return;
  }  else if (a == TYPE_COMPLEX) {
    printf("ncdf: unsupported type\n");
  } else if (a == TYPE_MATRIX_REAL) {
    apply_real_matrix_unary_inplace(stack, standard_normal_cdf);
    return;
  } else if (a == TYPE_MATRIX_COMPLEX)
    printf("ncdf: unsupported type\n");
  return;
}

void nquant_wrapper(Stack *stack) {
  ValueType a = stack_top_type(stack);
  if (a == TYPE_REAL) {
    StackElement b = pop(stack);
    push_real(stack, standard_normal_quantile(b.real));
    return;
  }  else if (a == TYPE_COMPLEX) {
    printf("nquant: unsupported type\n");
  } else if (a == TYPE_MATRIX_REAL) {
    apply_real_matrix_unary_inplace(stack, standard_normal_quantile);
    return;
  } else if (a == TYPE_MATRIX_COMPLEX)
    printf("nquant: unsupported type\n");
  return;
}

void gamma_wrapper(Stack *stack) {
  ValueType a = stack_top_type(stack);
  if (a == TYPE_REAL) {
    StackElement b = pop(stack);
    push_real(stack, gamma_function(b.real));
    return;
  }  else if (a == TYPE_COMPLEX) {
    printf("nquant: unsupported type\n");
  } else if (a == TYPE_MATRIX_REAL) {
    apply_real_matrix_unary_inplace(stack, gamma_function);
    return;
  } else if (a == TYPE_MATRIX_COMPLEX)
    printf("nquant: unsupported type\n");
  return;
}

void ln_gamma_wrapper(Stack *stack) {
  ValueType a = stack_top_type(stack);
  if (a == TYPE_REAL) {
    StackElement b = pop(stack);
    push_real(stack, ln_gamma_function(b.real));
    return;
  }  else if (a == TYPE_COMPLEX) {
    printf("nquant: unsupported type\n");
  } else if (a == TYPE_MATRIX_REAL) {
    apply_real_matrix_unary_inplace(stack, ln_gamma_function);
    return;
  } else if (a == TYPE_MATRIX_COMPLEX)
    printf("nquant: unsupported type\n");
  return;
}

void beta_wrapper(Stack *stack) {
  if (stack_size(stack) < 2) {
    printf("Not enough elements on the stack\n");
    return;
  }
  ValueType a = stack_top_type(stack);
  ValueType b = stack_next2_top_type(stack);
  if ((a == TYPE_REAL) && (b == TYPE_REAL)) {
    StackElement x = pop(stack);
    StackElement y = pop(stack);
    push_real(stack, beta_function(x.real,y.real));
    return;
  }  else {
    printf("beta: unsupported type\n");
    return;
  }
}

void ln_beta_wrapper(Stack *stack) {
  if (stack_size(stack) < 2) {
    printf("Not enough elements on the stack\n");
    return;
  }
  ValueType a = stack_top_type(stack);
  ValueType b = stack_next2_top_type(stack);
  if ((a == TYPE_REAL) && (b == TYPE_REAL)) {
    StackElement x = pop(stack);
    StackElement y = pop(stack);
    push_real(stack, ln_beta_function(x.real,y.real));
    return;
  }  else {
    printf("ln_beta: unsupported type\n");
    return;
  }
}
