#include <string.h>
#include <stdbool.h>
#include "function_list.h"

const char* const function_names[] = {
  "sin", "cos", "tan", "asin", "acos", "atan",
  "sinh", "cosh", "tanh", "asinh", "acosh", "atanh",
  "ln", "log", "exp", "sqrt", "pow", "re", "im", "abs", "arg", "conj",
  "npdf", "ncdf", "nquant","gamma", "ln_gamma","beta","ln_beta",
  "re2c", "split_c", "j2r",
  "chs", "inv", "fuck", "help", "gravity", "pi", "e", "drop", "clst",
  "swap", "dup", "scon", "s2l", "s2u", "slen", "srev",
  "minv", "det", "eig", "tran", "reshape", "get_aij", "set_aij",
  "kron", "diag", "chol", "svd", "dim", "eye", "ones", "zeroes", "rand", "randn",
  "cmean", "rmean", "csum", "rsum", "cvar", "rvar",
  "cmin", "cmax", "rmin", "rmax",
  "roots", "pval",
  "rcl", "sto","pr",
  "pm","setprec","sfs","undo",
  ".*", "./", ".^",
  "eq","leq","lt","gt","geq","neq","and","or","not",
  "ddays","today","dateplus","dow","edmy",
  NULL
};
