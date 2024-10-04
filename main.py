import replicate
import argparse
import subprocess

system_prompt = "Generate a concise git commit message that summarizes the key changes. Stay high-level and combine smaller changes to overarching topics. Skip describing any reformatting changes, write the message between <message></message> tags and if there are no changes, don't make up a message, just write 'no changes'"

parser = argparse.ArgumentParser(
                    prog='git auto commit',
                    description='Generates a commit message using LLM models')

parser.add_argument('--model-name', type=str, help='LLM model to use (only replicate models are supported)')
args = parser.parse_args()

if __name__ == "__main__":
  if args.model_name:
    model = args.model_name
  else:
    model = "meta/meta-llama-3-70b-instruct"
    
  
  changes = subprocess.run(['git', 'diff', '--staged'], stdout=subprocess.PIPE).stdout.decode('utf-8')
  
  prompt = {
    "prompt": changes,
    "max_tokens": 200,
    "system_prompt": system_prompt
  }
  
  message = ""
  print("Thinking... ⌛")
  
  for event in replicate.stream(model, input=prompt):
    message += str(event)
  message = message.strip()
  
  if message.lower() == "no changes":
    print("No changes detected. ✅")
    exit(0)
  
  if "<message>" in message:
    message = message[message.find("<message>") + len("<message>"):]
  else:
    print("No changes detected. ✅")
    exit(1)
  if "</message>" in message:
    message = message[:message.find("</message>")]
  else:
    print("Errore nella generazione del messaggio. ❌")
    exit(1)
  
  print(message)
  print("Enter: accept message and commit, Ctrl-C: close without committing.")

  try:
    input()
  except KeyboardInterrupt:
    print("Operation aborted. ❌")
    exit(0)
    
  subprocess.run(['git', 'commit', '-m', message])