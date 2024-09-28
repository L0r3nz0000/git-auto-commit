import replicate
import argparse

system_prompt = """You will examine and explain the given code changes and write a commit message in Conventional Commits format.
The first line of the commit message should be a 20 word Title summary include a type, optional scope, subject in text, seperated by a newline and the following body.
The types should be one of:
\t- fix: for a bug fix
\t- feat: for a new feature
\t- perf: for a performance improvement
\t- revert: to revert a previous commit
The body will explain the code change. Body will be formatted in well structured beautifully rendered and use relevant emojis
if no code changes are detected, you will reply with no code change detected message."""

parser = argparse.ArgumentParser(
                    prog='git auto commit',
                    description='Generates a commit message using LLM models')

parser.add_argument('--model-name', type=str, help='LLM model to use (only replicate models are supported)')
args = parser.parse_args()

if __name__ == "__main__":
  if args.model_name:
    model = args.model_name
  else:
    model = "meta/meta-llama-3.1-405b-instruct"
  
  input = {
      'stream': True,
      'input': {
          'top_p': 0.9,
          'prompt': '',
          'min_tokens': 0,
          'temperature': 0.6,
          'prompt_template': '<|begin_of_text|><|start_header_id|>system<|end_header_id|>\n\n' + system_prompt + '<|eot_id|><|start_header_id|>user<|end_header_id|>\n\n{prompt}<|eot_id|><|start_header_id|>assistant<|end_header_id|>\n\n',
          'presence_penalty': 1.15
      },
      'system_prompt': system_prompt
  }
  
  for event in replicate.stream(model, input=input):
    print(str(event), end="", flush=True)