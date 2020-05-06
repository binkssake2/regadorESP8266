import os
import requests
import json
from flask import Flask, render_template, request, redirect

#api_url = 'https://api.jsonbin.io/b'
#bin_ID = '5e7d46aa862c46101abec6c9'
#secret_key = '$2b$10$sqXNBeWaUKx1wfpvNgpv.u4xUF.nUWVH4GIgZRqnaI30vibIYPv4.'
icon_component = '<link rel="shortcut icon" hrel="https://image.flaticon.com/icons/svg/653/653877.svg">'

api_url = 'https://regador-7d687.firebaseio.com'
dono = 'jojoi'
def read_from_db():

    url = f'{api_url}/{dono}.json'
    #url = f'{api_url}/{bin_ID}/'
    #headers = {'secret-key': secret_key}
    #req = requests.get(url, headers=headers)

    req = requests.get(url)
    return req.json()

def overwrite_db(infos):
    #url = f'{api_url}/{bin_ID}/'
    #headers = {
    #    'Content-Type': 'application/json',
    #    'secret-key': secret_key
    #}
    url = f'{api_url}/{dono}.json'

    #requests.put(url, json=infos, headers=headers)
    requests.put(url, json=infos)

def createeventoshtml(eventos):
    html = "<h1>Todos os eventos</h1>"

    for idx, evento in enumerate(eventos):
        eventohtml = f'<h3>Evento {idx}'
        iniciohtml = f'<ul><li>Hora de início: {evento[0][0]}:{evento[0][1]}</li>'
        durhtml = f'<li>Duração: {evento[1][0]}:{evento[1][1]}</li>'
        removehtml = f'<li><a href="/deletar/{idx}">Deletar</a></li></ul>'
        closehtml = "</h3>"
        html += (eventohtml + iniciohtml + durhtml + removehtml + closehtml)

    htmlcriar = '<a href="/criar">Criar novo evento</a>'
    html += htmlcriar

    return html


app = Flask(__name__)


@app.route("/")
def index():
    return icon_component + "<h1>Olá, essa é a aplicação que controla a rega das plantas!</hi>" \
    "<p>Para fins de usuários temos apenas uma rota de interesse, que é a que" \
    "controla os eventos. Para ir até ela basta clicar no link abaixo:</p>" \
    "<a href='/eventos'>Ir para eventos</a>"


@app.route('/deletar/<int:eventID>')
def deletar(eventID):
    infos = read_from_db()
    infos.pop(eventID)
    overwrite_db(infos)

    return redirect("/eventos", code=302)

@app.route('/criar')
def criar():
    return """
    <form action="/handlecriar">
      <label for="hora">Hora de inicio:</label><br>
      <input type="text" id="hora" name="hora" value="23:30"><br>
      <label for="dur">Duração:</label><br>
      <input type="text" id="dur" name="dur" value="00:30"><br><br>
      <input type="submit" value="Criar">
    </form>
    """

@app.route('/handlecriar')
def handlecriar():
    hora = request.args.get('hora').split(':')
    dur = request.args.get('dur').split(':')

    evento = [
        [
            int(hora[0]),
            int(hora[1])
        ],
        [
            int(dur[0]),
            int(dur[1])
        ]
    ]

    db = read_from_db()
    db.append(evento)
    overwrite_db(db)

    return redirect("/eventos", code=302)

@app.route('/eventos')
def eventos():
    infos = read_from_db()
    return createeventoshtml(infos)

@app.route('/db')
def dbview():
    infos = read_from_db()
    return json.dumps(infos)


#if __name__ == "__main__":
#    port = int(os.environ.get("PORT", 5000))
#    app.run(host='0.0.0.0', port=port)


