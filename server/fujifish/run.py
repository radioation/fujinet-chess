import signal, sys, os

from dotenv import load_dotenv

import threading, socketserver

from fujifish.api.http_api import app #, initialize_tables
from fujifish.api.tcp_api import TcpChessHandler

# don't lobby yet.
# from lobby.lobby_client import init_lobby


tcp_server = None

def start_tcp():
    global tcp_server
    

def shutdown( signum, frame ):
    if tcp_server:
        tcp_server.shutdown()
        tcp_server.server_close()
    sys.exit(0)

if __name__ == "__main__":

    # get environment from .env files
    load_dotenv()    

    server_host = os.getenv('SERVER_HOST', "0.0.0.0")
    server_port = int(os.getenv('SERVER_PORT', 5364))


    #lobby_endpoint = os.getenv('LOBBY_ENDPOINT_UPSERT')
    #init_lobby( lobby_endpoint )
    #initialize_tables()

    app.run(host=server_host, port=server_port)


