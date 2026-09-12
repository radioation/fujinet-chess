import os
import signal
import threading
import copy


from lobby.lobby_client import GameClient, LobbyClient, GamePlayer, get_lobby

import json



from flask import Flask, request, Response

from fujifish.api.chess_game import GameTable, ChessGame, create_table, get_game, get_state

#######################################################
#
# Flask HTTP interface
#
app = Flask(__name__)




# Register the handlers
def shutdown_handler( signal_int, frame ):
    print(f"Caught signal: {signal_int}")
    cleanup()
    sys.exit(0)

signal.signal(signal.SIGINT, shutdown_handler)
signal.signal(signal.SIGTERM, shutdown_handler)







@app.post("/joingame")
def http_joingame():
    body = request.get_data(as_text=True) or ""
    tbl = request.args.get("table")
    if tbl is None:
        return Response("invalid gid\n", mimetype="text/plain", status=400)
    print ("BODY: " + body)
    print ("table: " + tbl)
    lines = [ln.strip() for ln in body.splitlines() if ln.strip() != ""]
    if len(lines) > 1 :
        #game = get_game(gid)
        game, unlock = get_game(tbl)
        try:
            if game is not None:
                # try to join
                playerid,side = game.join_game(lines[0], lines[1] )
                return Response( playerid + ":" + side , mimetype="text/plain")
        finally:
            unlock()
        return Response("table not found\n", mimetype="text/plain", status=404)

            
    else:
        return Response("invalid\n", mimetype="text/plain", status=400)


@app.post("/move")
def http_move():
    """
    Body format (plain text, LF line endings):
        <uci_move>\n
        [<movetime_ms>]\n        # optional, default 300
    Returns:
        - "invalid move\n"           if POST body is invalid.
        - "illegal move\n"           if move is not legal.
        - "<bestmove>\n"             if valid, give best move from stockfish (single player will move)
    """
    body = request.get_data(as_text=True) or ""
    tbl = request.args.get("table")
    if tbl is None:
        return Response("invalid table\n", mimetype="text/plain", status=400)

    game, unlock = get_game(tbl)
    if game is None:
        return Response("table not found\n", mimetype="text/plain", status=404)

    try:
        print ("BODY: " + body)
        print ("table: " + tbl)
        lines = [ln.strip() for ln in body.splitlines() if ln.strip() != ""]
 
        # need both player and the UCI move )
        if len(lines) < 2:
            return Response("invalid format\n", mimetype="text/plain", status=400)
       
        pid,  uci_move = lines[0], lines[1].lower()
 
        if not pid.isalnum() or ( pid.isalnum() and  len(pid) != 8 ):
            return Response("invalid format - p\n", mimetype="text/plain", status=400)
        if not uci_move.isalnum() or len(uci_move) > 5 or len(uci_move) < 4:
            return Response("invalid format - m\n", mimetype="text/plain", status=400)
 
        movetime_ms = 300
        if len(lines) >= 3 :
            if lines[2].isdigit():
                movetime_ms = int(lines[3])
            else:
                return Response("invalid format - t\n", mimetype="text/plain", status=400)
 
 
        move_result = game.do_move( pid, uci_move, movetime_ms)
        if move_result['valid'] == True:
            #if 'engine_move' in move_result:
            #    return Response( move_result['engine_move'], mimetype="text/plain", status = 200)
            #else:
            return Response( move_result['message'], mimetype="text/plain", status = 200)
        else:
            return Response(move_result['message'], mimetype="text/plain", status=400)
    finally:
        unlock()


@app.get("/board")
def http_board():
    table = request.args.get('table')
    if not table: return Response("ERR missing table\n", mimetype="text/plain", status = 400 )
    game = get_state(table)
    if not game: return Response("ERR no game\n", mimetype="text/plain", status = 404 )

    return Response(str(game.board) + "\n", mimetype="text/plain")


@app.get("/status")
def http_status():
    table = request.args.get('table')
    print(" GOT TABLE " + table )
    if not table: return Response("ERR missing table\n", mimetype="text/plain", status = 400 )
    game = get_state(table)
    print(" GOT GAME " )
    if not game: return Response("ERR no game\n", mimetype="text/plain", status = 404 )

    print( game.state_line() )

    return Response(str(game.state_line()) + "\n", mimetype="text/plain")






