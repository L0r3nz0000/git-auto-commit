#include <stdio.h>
#include <stdlib.h>
#include "requests.h" /* struct Headers, struct Data, struct Response, get, post, add_header */

int main() {
  char* REPLICATE_API_TOKEN = getenv("REPLICATE_API_TOKEN");
  if (REPLICATE_API_TOKEN == NULL) {
    printf("REPLICATE_API_TOKEN is not set, aborting.\n");
    return 1;
  }

  char* system_prompt = "You will examine and explain the given code changes and write a commit message in Conventional Commits format.\
The first line of the commit message should be a 20 word Title summary include a type, optional scope, subject in text, seperated by a newline and the following body.\
The types should be one of:\
\t- fix: for a bug fix\
\t- feat: for a new feature\
\t- perf: for a performance improvement\
\t- revert: to revert a previous commit\
The body will explain the code change. Body will be formatted in well structured beautifully rendered and use relevant emojis\
if no code changes are detected, you will reply with no code change detected message.";

  char *data_fmt = "{\
      'stream': True,\
      'input': {\
          'top_p': 0.9,\
          'prompt': '',\
          'min_tokens': 0,\
          'temperature': 0.6,\
          'prompt_template': '<|begin_of_text|><|start_header_id|>system<|end_header_id|>\n\n%s<|eot_id|><|start_header_id|>user<|end_header_id|>\n\n{prompt}<|eot_id|><|start_header_id|>assistant<|end_header_id|>\n\n',\
          'presence_penalty': 1.15\
      }\
  }";  // %s = system prompt

  char data[2048];
  sprintf(data, data_fmt, system_prompt);

  char* host = "api.replicate.com";
  char* endpoint = "/v1/models/meta/meta-llama-3-70b-instruct/predictions";
  Headers headers;

  size_t buffer_size = snprintf(NULL, 0, "Bearer %s", REPLICATE_API_TOKEN) + 1;  // Calcola la dimensione necessaria
  
  char *authorization = malloc(buffer_size);  // Alloca la memoria
  if (authorization == NULL) {
      // Gestisci errore di allocazione
      return 1;
  }

  sprintf(authorization, "Bearer %s", REPLICATE_API_TOKEN);

  add_header(&headers, (Header) { "Authorization", authorization });
  add_header(&headers, (Header) { "Content-Type", "application/json" });

  char *response = post(host, endpoint, headers, 443)->text;

  printf("%s", response);
  return 0;
}