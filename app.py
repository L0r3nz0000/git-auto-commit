from flask import Flask, request, Response

app = Flask(__name__)

@app.route('/dio')
def hello_world():
  return 'Dio porco!'

@app.route("/")
def home():
  resp = dict(request.headers)
  return resp