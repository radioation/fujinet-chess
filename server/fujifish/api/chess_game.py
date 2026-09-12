import os
import threading
import copy

from lobby.lobby_client import GameClient, LobbyClient, GamePlayer, get_lobby
from dataclasses import dataclass, asdict, field
from typing import List, Optional, Dict, Any
import json

import uuid
import chess
import chess.engine

import threading

MAX_PLAYERS = 2
MIN_PLAYERS = 1



class TableMutex:
    def __init__(self) -> None:
        self._locks: Dict[str, threading.Lock] = {}
        self._guard = threading.Lock()

    def Lock(self, key: str) -> Callable[[], None]:
        # Return an unlock() closure (to mirror Go style)

        with self._guard:
            lock = self._locks.get(key)
            if lock is None:
                lock = threading.Lock()
                self._locks[key] = lock
        lock.acquire()

        unlocked = False

        def unlock() -> None:
            nonlocal unlocked # not part of inner function
            if not unlocked:
                lock.release()
                unlocked = True

        # return unlock() for unlocking.
        return unlock


STATE_MAP: Dict[ str, ChessGame ] = {}
TABLES : List[GameTable]  = []
table_mutex = TableMutex()

def initialize_tables():

    tables_json = os.getenv("GAME_SERVER_TABLES", "" )
    print(tables_json)
    raw_list = json.loads( tables_json )
    for table in raw_list:
        servername = table.get("servername")
        instance_url_suffix = table.get("instance_url_suffix").lower()
        bot_level = int(table.get("bot_level"))
        register_lobby = table.get("register_lobby")
        table_obj, chess_game = create_table( servername, instance_url_suffix, bot_level, register_lobby )
        TABLES.append(table_obj)
        STATE_MAP[ instance_url_suffix ] =chess_game
        chess_game.update_lobby()


def get_state( table:str ) -> Tuple[ Optional[ChessGame] ]:
    tbl = table.lower()
    #plyr = ""
    #if len(player) > 0:
    #    plyr = player.lower()

    unlock_fcn = table_mutex.Lock( tbl )
    state = None
    tmp_state = STATE_MAP.get( tbl )
    if tmp_state is not None:
        state = copy.deepcopy( tmp_state )
        #state.set_client_player_by_name( plyr )
    unlock_fcn()
    return state

def get_game( table:str ) -> Tuple[ Optional[ChessGame], Callable[ [], None]]:
    tbl = table.lower()
    #plyr = ""
    #if len(player) > 0:
    #    plyr = player.lower()

    unlock_fcn = table_mutex.Lock( tbl )
    state = STATE_MAP.get( tbl )
    #if tmp_state is not None:
    #    state = copy.deepcopy( tmp_state )
    #    #state.set_client_player_by_name( plyr )

    return state, unlock_fcn


def save_state( state: ChessGame ):
    STATE_MAP[ state.table] = state


def cleanup():
    for table, state in STATE_MAP.items():
        try:
            unlock_fcn = table_mutex.Lock( table )
            print("Try delete: " + state.servername )
            state.delete_from_lobby()
            unlock_fcn()
        except Exception as e:
            print(f"[cleanup] failed to delete lobby for table={getattr(gs, 'table', '?')}: {e}")






@dataclass
class GameTable:
    table: str    # description, 
    name: str     # short name (3 chars)
    current_players: int = 0
    max_players: int = MAX_PLAYERS


@dataclass 
class Player:
    name: str
    player_id: str  
    move: str
    side: str
    is_bot: bool
    def __init__( self, name: str, side: str,  is_bot: bool ):
        self.name = name 
        self.player_id = str(uuid.uuid4())[:8].upper() # always set but ma
        self.move = ''
        self.side = side
        self.is_bot = is_bot


@dataclass
class ChessGame:
    active_player: int

    #players: List[Player] overkill, only two players ever
    player_1: Player
    player_2: Player
    client_player: int
    table: str
    servername: str
    register_lobby: bool
    max_players: int
    lobby: lobbyClient
    bot_level: int
    engine_config: str
    is_single_player: bool
    hash: str = ""

    # if bot_level is None, no bot will be added.
    def __init__( self, instance_url_suffix: str, servername: str, bot_level: int, register_lobby: bool ):
        self.active_player = -1
        self.board = chess.Board()      
        self.engine_moves = []          # list of chess engine moves
        self.player_1 = None
        self.player_2 = None
        self.instance_url_suffix = instance_url_suffix
        self.servername = servername
        self.register_lobby = register_lobby
        self.max_players = 2
        self.curr_player = 0    # 0 : no player yet
        self.moves = []
        self.lobby = None
        if register_lobby is True:
            self.lobby = get_lobby()
        if bot_level < 1:
            print("NO SYNTH")
            self.bot_level = 0 
            self.engine_config = ""
            self.is_single_player = False
        else:
            # Map skill level 1-10 to Stockfish config.
            self.bot_level = max(1, min(10, bot_level))
            if self.bot_level == 10:
                # Full strength
                self.engine_config = {"UCI_LimitStrength": False}
            else:
                elo_min, elo_max = 1320, 3190
                step = (elo_max - elo_min) / 9  # 9 intervals (levels 1–9)
                target_elo = int(elo_min + (self.bot_level - 1) * step)
                self.engine_config ={"UCI_LimitStrength": True, "UCI_Elo": target_elo}
            print(f'>> Got bot level: {bot_level} use {self.engine_config}')
            self.is_single_player = True
            self.add_player( "BOT" + str( self.bot_level ), "", True )

    def add_player( self, player: str, side: str, is_bot: bool ) -> ( str, str ) :
        if self.player_1 is not None and self.player_2 is not None: 
            print( f'>> at player max')
            return ("", "")

        print( f'>> Adding player {player} ')
        if self.player_1 is None:
            self.player_1 = Player( name = player, side = side, is_bot = is_bot );
            return( self.player_1.player_id, self.player_1.side )
        else: 
            use_side = 'B'
            if self.player_1.is_bot:
                p2_side = side
                # human chooses side, not bot
                if side == 'W':
                    self.player_1.side = 'B'
                    self.current_player = 2
                else:
                    self.player_1.side = 'W'
                    # TODO: have bot do opening move here?
                    self.current_player = 2
            else:  # not a bot, first player  had choice
                if self.player_1.side == 'B':
                    use_side = 'W'
                    self.current_player = 2
                else:
                    self.current_player = 1
                
            self.player_2 = Player( name = player, side = use_side, is_bot = is_bot );
            return( self.player_2.player_id, self.player_2.side )

        return ("", "")


    def set_client_player_by_name( self, player:str ) -> None:
        # no name, just a viewer
        if len( player ) == 0 :
            self.client_player = -1
            return
        # has a name, so probably has an index in players list
        for index, item in enumerate( self.players ):
            if item.name == player:
                break
        else:
            index = -1
        self.client_player = index

        if self.client_player < 0 and len( self.players ) < self.max_players :
            self.add_player( player, '', False )
            self.client_player = len( self.players) - 1
            self.update_lobby()    

#    def __init__(self, mode = 'S', player_1_side = None, level = 3, game_id = None ):
#        if game_id is None:
#            self.id = str(uuid.uuid4())[:8].upper()
#        else:
#            self.id = game_id
#        self.board = chess.Board()      
#        self.engine_moves = []          # list of chess engine moves
#        self.mode = mode                # single player 'S' or double player 'D'
#        self.player_1_side = player_1_side  # NA if not selected yet.
#        self.player_1_id = str(uuid.uuid4())[:8].upper() # always set, 
#        self.player_2_id = str(uuid.uuid4())[:8].upper() # always set but ma
#        self.curr_player = 0    # 0 : no player
#        if self.mode == 'D' :
#            if player_1_side == 'W':
#                self.curr_player = 1
#            elif player_1_side == 'B':
#                self.curr_player = 2
#
#        # Map skill level 1-10 to Stockfish config.
#        self.skill_level = max(1, min(10, level))
#        if level == 10:
#            # Full strength
#            self.engine_config = {"UCI_LimitStrength": False}
#        else:
#            elo_min, elo_max = 1320, 3190
#            step = (elo_max - elo_min) / 9  # 9 intervals (levels 1–9)
#            target_elo = int(elo_min + (level - 1) * step)
#            self.engine_config ={"UCI_LimitStrength": True, "UCI_Elo": target_elo}
#


    def client_leave(self) -> None:
        # not an actual game, so no logic to really check.
        if self.client_player < 0:
            return

        del( self.players[ self.client_player ] )

    def update_lobby(self) -> None:
        if not self.register_lobby :
            return
        human_player_slots, human_player_count = self.get_human_player_count_info()
        #self.lobby.send_state_to_lobby( human_player_slots, human_player_count, True, self.servername, ?table=" + self.table )
        self.lobby.send_state_to_lobby( human_player_slots, human_player_count, True, self.servername, "?table=" + self.instance_url_suffix )


    def delete_from_lobby(self) -> None:
        if not self.register_lobby :
            # not in lobby, so nothing to do
            return
        self.lobby.delete_from_lobby( self.servername, "?table=" + self.instance_url_suffix )


    def get_human_player_count_info(self) -> (int, int):
        print(">> GET PLAYER COUNT")
        human_available_slots = int( os.getenv( "GAME_SERVER_MAX_PLAYERS", "2" ) )
        print(f'>>   human_available_slots {human_available_slots}')
        human_player_count = 0
        if self.is_single_player:
            human_available_slots -= 1
        if self.player_1 is not None and self.player_1.is_bot == False :
            human_player_count +=1
        if self.player_2 is not None and self.player_2.is_bot == False :
            human_player_count +=1


        return human_available_slots, human_player_count




    def join_game( self, player:str, player_side:str = None ) -> (str,str):
        # two player game:
        if self.bot_level == 0: # no bot set.
            (player_id, side ) = self.add_player( player, player_side, False )
            return ( player_id, side )
            #if self.player_1_side == None:
            #    # not yet set, so joining player is #1
            
            #    if player_side == None:  # Didn't pick, assign white.
            #        self.player_1_side = 'W'
            #        self.curr_player = 1
            #    else:
            #        # they get to pick their side
            #        self.player_1_side = player_side
            #        if player_side == 'W':
            #            self_curr_player = 1
            #        else:
            #            self_curr_player = 2
            #    return self.player_1_id
            #else:
            #    # a player is already present, player 2 will take what they get and like it.
            #    if self.player_1_side == 'W':
            #        self.player_2_side == 'B'
            #    else:
            #        self.player_2_side == 'W'
            #    return self.player_2_id
        #else:  # S
            #if player_side == None:
            #    self.player_1_side = 'W'
            #    self.curr_player = 1
            #else:
            #    self.player_1_side = 'B'
            #    self.curr_player = 2
            #return self.player_1_id
        return ("","")

    def do_move( self, pid, uci, movetime_ms ):
        ## single player mode, player 2 should NEVER be able to move
        #if self.mode == 'S' and pid == self.player_2_id:
        #    return ( { "valid": False, "message":"player 1 turn" } )

        ## single player mode before p2 joins, don't allow
        #if self.mode == 'D' and self.player_2_id == 'NA':
        #    return ( { "valid": False, "message":"game not started" } )
        #
        if self.current_player == 1 and pid != self.player_1.player_id:
            return ( { "valid": False, "message":"player 1 turn" } )
        if self.current_player == 2 and pid != self.player_2.player_id:
            return ( { "valid": False, "message":"player 2 turn" } )
        
        try:
             mv = chess.Move.from_uci(uci)
        except ValueError:
            return ( { "valid": False, "message":"illegal move" } )

        if mv not in self.board.legal_moves:
            return ( { "valid": False, "message":"illegal move" } )

        # if we're here, save the move
        self.board.push(mv)
    
        if self.board.is_checkmate():
            return ( { "valid": True, "message":"legal move Check Mate" } )
        if self.board.is_stalemate():
            return ( { "valid": True, "message":"legal move Stale Mate" } )
        if self.board.is_insufficient_material():
            return ( { "valid": True, "message":"legal move Draw" } )

            
        # get reply from stockfish.
        with chess.engine.SimpleEngine.popen_uci(os.environ['ENGINE_PATH']) as eng:
            if self.is_single_player == True:
                # potentially make dumb moves for single player.
                print( self.engine_config )
                eng.configure( self.engine_config )

            res = eng.play(self.board, chess.engine.Limit(time=movetime_ms/1000.0))
            if res.move is None:
                # No legal engine reply (mate/stalemate)
                if self.board.is_checkmate():
                    return ( { "valid": True, "message":"legal move Check Mate" } )
                if self.board.is_stalemate():
                    return ( { "valid": True, "message":"legal move Stale Mate" } )
                if self.board.is_insufficient_material():
                    return ( { "valid": True, "message":"legal move Draw" } )
                return ( { "valid": False, "message":"None" } )

            # compute best move
            best = res.move.uci()
            print("BEST: " +best)

            if self.is_single_player == False:
                self.current_player = 2 if self.current_player == 1 else 1
                self.engine_moves.append(res.move)
            else:
                self.board.push(res.move)
            
            return ( { "valid": True, "message":"legal move", "engine_move":best } )

    def settings_str(self):
        return f"mode {self.mode}:p1side {self.player_1_side}:level {self.bot_level}:current_player {self.current_player}\n"

    def state_line(self):
        #if self.mode == 'D' and self.player_2_id == "NA":
        #    return f"TURN -:LAST -----:MVNO 0"
        if self.board.outcome() == None:
            return f"TURN {'w' if self.board.turn else 'b'}:LAST {self.board.move_stack[-1] if self.board.move_stack else '-----'}:MVNO {len(self.board.move_stack)}"
        return f"OVER {self.board.outcome().result()} {self.board.outcome().termination.value}:TURN {'w' if self.board.turn else 'b'}:LAST {self.board.move_stack[-1] if self.board.move_stack else '-----'}:MVNO {len(self.board.move_stack)}"
         




GAMES = {}
GAMES_LOCK = threading.Lock()



def create_table( table: str, server_name: str, bot_level: int, register_lobby: bool ) -> (GameTable, GameState) :

    chess_game = ChessGame( table, server_name, bot_level, register_lobby )

    table_obj = GameTable( name = server_name, table = table )

    return table_obj, chess_game



#def new_game(mode,side,level) -> ChessGame:
#    g = ChessGame(mode,side,level)
#    GAMES[g.id] = g
#    return g
#
#def get_game(gid: str) -> ChessGame | None:
#    return GAMES.get(gid)
#
#
#def get_two_player_games():
#    two_player_games = [ key for key, value in GAMES.items() if value.mode == "D"]
#    return two_player_games




