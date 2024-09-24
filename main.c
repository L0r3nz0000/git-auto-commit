#include <stdio.h>
#include <stdlib.h>
#include "requests.h" // post

int main() {
  char* REPLICATE_API_TOKEN = getenv("REPLICATE_API_TOKEN");
  if (REPLICATE_API_TOKEN == NULL) {
    printf("REPLICATE_API_TOKEN is not set, aborting.\n");
    return 1;
  }

  char* system_prompt="You will examine and explain the given code changes and write a commit message in Conventional Commits format.\
The first line of the commit message should be a 20 word Title summary include a type, optional scope, subject in text, seperated by a newline and the following body.\
The types should be one of:\
\t- fix: for a bug fix\
\t- feat: for a new feature\
\t- perf: for a performance improvement\
\t- revert: to revert a previous commit\
The body will explain the code change. Body will be formatted in well structured beautifully rendered and use relevant emojis\
if no code changes are detected, you will reply with no code change detected message.";

  char* host = "api.replicate.com";
  char* endpoint = "/v1/models/meta/meta-llama-3-70b-instruct/predictions";
  char* headers[] = {};
  char* data[] = {};

  char response[4096];
  post(host, endpoint, headers, 0, data, 0, response);

  return 0;
}
