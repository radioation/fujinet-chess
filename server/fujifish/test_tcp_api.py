import pytest
import socket
import threading
import time
import socketserver

from fujifish.api.tcp_api import TcpChessHandler

from fujifish.api.chess_game import ChessGame, initialize_tables
import os


@pytest.fixture
def tcp_server():
    os.environ[ "GAME_SERVER_TABLES" ] = '[ { "servername":"Iceland", "instance_url_suffix":"Iceland", "bot_level": -1, "register_lobby": false }, { "servername":"Philippines", "instance_url_suffix":"manila", "bot_level": -1, "register_lobby": false } , { "servername":"Hastings", "instance_url_suffix":"hastings", "bot_level": -1, "register_lobby": false } , { "servername":"bangkok", "instance_url_suffix":"bangkok", "bot_level": -1, "register_lobby": false } ]'
    
    initialize_tables()

    server = socketserver.ThreadingTCPServer(("127.0.0.1", 0), TcpChessHandler)
    host, port = server.server_address

    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    yield host, port
    server.shutdown()
    server.server_close()
    thread.join()


def send_cmd(port, line):
    with socket.create_connection(("127.0.0.1", port), timeout=20) as sock:
        # First line is greeting
        greeting = sock.recv(1024).decode().strip()
        sock.sendall((line + "\r").encode())
        data = sock.recv(1024).decode().strip()
        return greeting, data


def test_joingame(tcp_server):
    host, port = tcp_server
    greet, resp = send_cmd(port, "J:\n")
    assert greet.startswith("HELO")
    ##ids = resp.split()[1]
    ##gameid = ids[:8]
    ##playid = ids[9:-1]
    #assert resp == 'ERR invalid'

    greet, resp = send_cmd(port, "J:AAAAAAAA\n")
    assert resp == 'ERR invalid'

    greet, resp = send_cmd(port, "J::radyo:W\n")
    assert resp == 'ERR invalid gid'

    greet, resp = send_cmd(port, "J:estarcion:radyo:W\n")
    assert resp == 'ERR gid not found' 


    greet, resp = send_cmd(port, "M:------:W-\n")
    assert resp == 'ERR invalid format'

    greet, resp = send_cmd(port, "M:------:W:a\n")
    assert resp == 'ERR invalid format - g'

    greet, resp = send_cmd(port, "M:asdf1234:W:a\n")
    assert resp == 'ERR invalid format - p'

    greet, resp = send_cmd(port, "M:asdf1234:1234asdf:a\n")
    assert resp == 'ERR invalid format - m'

    greet, resp = send_cmd(port, "M:asdf1234:1234asdf:123456\n")
    assert resp == 'ERR invalid format - m'

    greet, resp = send_cmd(port, "M:asdf1234:1234asdf:e2e4\n")
    assert resp == 'ERR invalid game'

    greet, resp = send_cmd(port, "J:hastings:radyo:W\n")
    assert resp.startswith("ACK ")
    ids = resp.split()[1]
    pid = ids[:8]
    side = ids[9:10]
    assert pid.isalnum() == True
    assert side == 'W'

    greet, resp = send_cmd(port, "J:hastings:gorm:W\n")
    ids = resp.split()[1]
    pid2 = ids[:8]
    side2 = ids[9:10]
    assert pid2.isalnum() == True
    assert side2 == 'B'

    greet, resp = send_cmd(port, f"M:hastings:{pid}:e2e4\n")
    assert resp.startswith("ACK ")
    pid = resp.split()[1]
    assert pid.isalnum() == True


def test_status(tcp_server):
    gameid = 'manila'

    host, port = tcp_server
    greet, resp = send_cmd(port, f"J:{gameid}:radyo:W\n")
    ids = resp.split()[1]
    pid = ids[:8]
    side = ids[9:10]
    assert pid.isalnum() == True
    assert side == 'W'

    greet, resp = send_cmd(port, f"J:{gameid}:gorm:W\n")
    ids = resp.split()[1]
    pid2 = ids[:8]
    side2 = ids[9:10]
    assert pid2.isalnum() == True
    assert side2 == 'B'



    # status
    greet, resp = send_cmd(port, f"S:{gameid}\n")
    assert resp == 'ACK TURN w:LAST -----:MVNO 0'

    greet, resp = send_cmd(port, f'M:{gameid}:{pid}:e2e4\n')
    assert resp.startswith("ACK ")
    

    # status
    greet, resp = send_cmd(port, f"S:{gameid}\n")
    assert resp == 'ACK TURN b:LAST e2e4:MVNO 1'

    greet, resp = send_cmd(port, f'M:{gameid}:{pid}:e2e4\n')
    assert resp == "ERR player 2 turn"

    greet, resp = send_cmd(port, f'M:{gameid}:{pid}:e7e6\n')
    assert resp == "ERR player 2 turn"

    # status
    greet, resp = send_cmd(port, f"S:{gameid}\n")
    assert resp == 'ACK TURN b:LAST e2e4:MVNO 1'



    greet, resp = send_cmd(port, f'M:{gameid}:{pid}:e7e6\n')
    assert resp == "ERR player 2 turn"
    # status
    greet, resp = send_cmd(port, f"S:{gameid}\n")
    assert resp == 'ACK TURN b:LAST e2e4:MVNO 1'


    greet, resp = send_cmd(port, f'M:{gameid}:{pid2}:e7e6\n')
    assert resp.startswith("ACK ")
    # status
    greet, resp = send_cmd(port, f"S:{gameid}\n")
    assert resp == 'ACK TURN w:LAST e7e6:MVNO 2'


def test_mate(tcp_server):
    gameid='iceland'
    host, port = tcp_server
    greet, resp = send_cmd(port, f"J:{gameid}:radyo:W\n")
    assert resp.startswith("ACK ")
    ids = resp.split()[1]
    pid = ids[:8]
    side = ids[9:10]
    assert pid.isalnum() == True
    assert side == 'W'

    greet, resp = send_cmd(port, f"J:{gameid}:gorm:W\n")
    assert resp.startswith("ACK ")
    ids = resp.split()[1]
    pid2 = ids[:8]
    side2 = ids[9:10]
    assert pid2.isalnum() == True
    assert side2 == 'B'



    greet, resp = send_cmd(port, f'M:{gameid}:{pid}:f2f3\n')
    greet, resp = send_cmd(port, f'M:{gameid}:{pid2}:e7e5\n')
    greet, resp = send_cmd(port, f'M:{gameid}:{pid}:g2g4\n')
    greet, resp = send_cmd(port, f'M:{gameid}:{pid2}:d8h4\n')
    assert resp == 'ACK legal move Check Mate'
    greet, resp = send_cmd(port, f"S:{gameid}\n")
    assert resp == 'ACK OVER 0-1 1:TURN w:LAST d8h4:MVNO 4'



