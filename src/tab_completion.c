#define _POSIX_C_SOURCE 200809L

#include <string.h>
#include <readline/history.h>
#include <readline/readline.h>
#include "function_list.h"

// Called repeatedly by readline to find matches
char* function_name_generator(const char* text, int state) {
  static int list_index;
  static size_t len;

  if (state == 0) {
    list_index = 0;
    len = strlen(text);
  }

  while (function_names[list_index]) {
    const char* name = function_names[list_index++];
    if (strncmp(name, text, len) == 0) {
      return strdup(name);  // Caller will free it
    }
  }
  return NULL;  // No more matches
}

char** function_name_completion(const char* text, int start, int end) {
  (void)end;
  // Complete only if at the beginning of the line or after space
  if (start == 0 || rl_line_buffer[start - 1] == ' ') {
    return rl_completion_matches(text, function_name_generator);
  }
  return NULL;
}


