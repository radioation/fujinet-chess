# tcp_protocol.py
import socketserver, threading

from fujifish.api.chess_game import GameTable, ChessGame, create_table, get_game, get_state 
import re


class TcpChessHandler(socketserver.StreamRequestHandler):
    def handle(self):
        print("connected")
        self.wfile.write(b"HELO\n")
        for line in self.rfile:
            #print("pre-strip: ", end="" )
            print(line)
            line = line.decode("utf-8").strip()
            #print("POST-strip: " + line)
            if not line:
                continue
            response = self.dispatch(line)
            print(response)
            self.wfile.write((response + "\n").encode("utf-8"))

    def dispatch(self, line: str) -> str:
        try:
            if line.startswith("J:"):
                parts = line.split(":")
                if len(parts) < 4:
                    return "ERR invalid\n"
                gid = parts[1]
                if len(gid) == 0:
                    return "ERR invalid gid\n"
                game, unlock = get_game(gid)
                try:
                    if game is not None: 

                        ( player_id, side ) = game.join_game(parts[2], parts[3])
                        return f"ACK {player_id}:{side}\n"
                    else:
                        return "ERR gid not found\n"
                finally:
                    unlock()
            elif line.startswith("M:"):
                parts = line.split(":")
                if len(parts) < 4: return "ERR invalid format\n"

                gid, pid, uci_move = parts[1], parts[2], parts[3]
                if not gid.isalnum() :
                    return "ERR invalid format - g\n"
                if not pid.isalnum() or ( pid.isalnum() and  len(pid) != 8 ):
                    return "ERR invalid format - p\n"
                if not uci_move.isalnum() or len(uci_move) > 5 or len(uci_move) < 4:
                    return "ERR invalid format - m\n"

                game,unlock = get_game(gid)
                try:
                    if game is None:
                        return "ERR invalid game\n"
                    movetime_ts = 300
                    move_result = game.do_move( pid, uci_move, movetime_ts)
                    print( move_result )
                    if move_result['valid'] == True:
                        #if 'engine_move' in move_result:
                        #    res = "ACK " + move_result['engine_move']
                        #else:
                        res = "ACK " + move_result['message']
                    else:
                        res = "ERR " + move_result['message']
                    return res  
                finally:
                    unlock()
            elif line.startswith("B:"):
                gid = line.split(":",1)[1]
                game = get_state(gid)
                sboard = str(game.board)
                print( sboard )
                
                strboard = "ACK " + re.sub(r"[\n\t\s]*", "", sboard)
                print( strboard )

                return strboard
            elif line.startswith("T:"):
                gid = line.split(":")[1]
                game = get_state(gid)
                ret = "ACK " +  game.settings_str()
                return ret

            elif line.startswith("S:"):
                gid = line.split(":")[1]
                game = get_state(gid)
                ret = "ACK " +  game.state_line()
                return ret
            elif line.startswith("L:"):
                game_ids = get_two_player_games()
                print( game_ids )
                ret = "ACK "
                for gid in game_ids:
                    ret += gid +":"
                return ret[:-1]
            else:
                return "ERR unknown"
        except Exception as e:
            return f"ERR {e}"


