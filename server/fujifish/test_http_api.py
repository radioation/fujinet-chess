
import pytest
import json



@pytest.fixture(scope="module")
def client():
    import os
    os.environ[ "GAME_SERVER_TABLES" ] = '[ { "servername":"Iceland", "instance_url_suffix":"Iceland", "bot_level": -1, "register_lobby": false }, { "servername":"Philippines", "instance_url_suffix":"manila", "bot_level": -1, "register_lobby": false } ]'
    
    from fujifish.api.http_api import app, initialize_tables
    from fujifish.api.chess_game import ChessGame
    initialize_tables()  # create game specified by 'GAME_SERVER_TABLES'
    with app.test_client() as test_client:
        yield test_client


def test_joingame(client):


    resp = client.post("/joingame")
    assert resp.data.decode('utf-8') == 'invalid gid\n'
    assert resp.status_code == 400


    resp = client.post("/joingame?table=hastings", data="AAAAAAAA\nW\n")
    assert resp.data.decode('utf-8') == 'table not found\n'
    assert resp.status_code == 404

    resp = client.post("/joingame?table=iceland", data=f"AAAA\n")
    assert resp.data.decode('utf-8') == 'invalid\n'
    assert resp.status_code == 400

    resp = client.post("/joingame?table=iceland", data=f"radyo\nW\n")
    ids = resp.data.decode('utf-8')
    playid =  ids[:8]
    side = ids[9:]
    assert playid.strip().isalnum() == True
    assert side == 'W'
    assert resp.status_code == 200

    resp = client.post("/joingame?table=iceland", data=f"gorm\nW\n")
    ids = resp.data.decode('utf-8')
    playid =  ids[:8]
    side = ids[9:]
    assert playid.strip().isalnum() == True
    assert side == 'B'
    assert resp.status_code == 200



    
def test_move_request(client):
    
    resp = client.post("/joingame?table=manila", data=f"shadow\nW\n")
    ids = resp.data.decode('utf-8')
    print(ids)
    playid =  ids[:8]
    side = ids[9:]

    resp = client.post("/joingame?table=manila", data=f"cosmicowl\nW\n")
    print(ids)


    # check post data
    resp = client.post("/move", data="S\nW\n")
    assert resp.data.decode('utf-8') == "invalid table\n"
    assert resp.status_code == 400

    # bad player format
    resp = client.post("/move?table=manila", data="-----\nW\na\n")
    assert resp.data.decode('utf-8') == "invalid format - p\n"
    assert resp.status_code == 400

    # bad move format
    resp = client.post("/move?table=manila", data="abcd1234\na\n")
    assert resp.data.decode('utf-8') == "invalid format - m\n"
    assert resp.status_code == 400

    # bad move format
    resp = client.post("/move?table=manila", data="1234asdf\n123456\n")
    assert resp.data.decode('utf-8') == "invalid format - m\n"
    assert resp.status_code == 400

    # bad time format
    resp = client.post("/move?table=manila", data="1234asdf\ne2e4\naaa\n")
    assert resp.data.decode('utf-8') == "invalid format - t\n"
    assert resp.status_code == 400

    # format 
    resp = client.post("/move?table=estarcion", data="1234asdf\ne2e4\n350\n")
    assert resp.data.decode('utf-8') == "table not found\n"
    assert resp.status_code == 404

    print(f'Do move for {playid}')
    resp = client.post("/move?table=manila", data=f"{playid}\ne2e4\n")
    assert resp.status_code == 200


def test_state(client):
    
    # create a newgame
    resp = client.post("/newgame", data="D\nW\n")
    ids = resp.data.decode("utf-8")
    gameid =  ids[:8]
    playid = ids[9:-1]
    client2 = app.test_client()
    resp = client2.post("/joingame", data=f"{gameid}\n")
    assert resp.status_code == 200
    play2id = resp.data.decode("utf-8").strip()
    assert len(play2id) == 8

    # status 
    resp = client.get(f"/status?gid={gameid}")
    assert resp.data.decode('utf-8') == 'TURN w:LAST -----:MVNO 0\n'
    assert resp.status_code == 200

    resp = client.post("/move", data=f"{gameid}\n{playid}\ne2e4\n")
    assert resp.status_code == 200

    # status 
    resp = client.get(f"/status?gid={gameid}")
    assert resp.data.decode('utf-8') == 'TURN b:LAST e2e4:MVNO 1\n'
    assert resp.status_code == 200

    resp = client.post("/move", data=f"{gameid}\n{playid}\ne2e4\n")
    assert resp.data.decode('utf-8') == 'player 2 turn'
    assert resp.status_code == 400

    resp = client.post("/move", data=f"{gameid}\n{playid}\ne7e6\n")
    assert resp.data.decode('utf-8') == 'player 2 turn'
    assert resp.status_code == 400

    # status 
    resp = client.get(f"/status?gid={gameid}")
    assert resp.data.decode('utf-8') == 'TURN b:LAST e2e4:MVNO 1\n'
    assert resp.status_code == 200



    resp = client2.post("/move", data=f"{gameid}\n{playid}\ne7e6\n")
    assert resp.data.decode('utf-8') == 'player 2 turn'
    assert resp.status_code == 400
    # status 
    resp = client.get(f"/status?gid={gameid}")
    assert resp.data.decode('utf-8') == 'TURN b:LAST e2e4:MVNO 1\n'
    assert resp.status_code == 200

    resp = client2.post("/move", data=f"{gameid}\n{play2id}\ne7e6\n")
    assert resp.status_code == 200
    # status 
    resp = client.get(f"/status?gid={gameid}")
    assert resp.data.decode('utf-8') == 'TURN w:LAST e7e6:MVNO 2\n'
    assert resp.status_code == 200



def test_mate(client):

    # create a newgame
    resp = client.post("/newgame", data="D\nW\n")
    ids = resp.data.decode("utf-8")
    gameid =  ids[:8]
    playid = ids[9:-1]
    client2 = app.test_client()
    resp = client2.post("/joingame", data=f"{gameid}\n")
    assert resp.status_code == 200
    play2id = resp.data.decode("utf-8").strip()
    assert len(play2id) == 8

    resp = client.post("/move", data=f"{gameid}\n{playid}\nf2f3\n")
    assert resp.data.decode('utf-8') == 'legal move'
    assert resp.status_code == 200

    resp = client2.post("/move", data=f"{gameid}\n{play2id}\ne7e5\n")
    assert resp.data.decode('utf-8') == 'legal move'
    assert resp.status_code == 200

    resp = client.post("/move", data=f"{gameid}\n{playid}\ng2g4\n")
    assert resp.data.decode('utf-8') == 'legal move'
    assert resp.status_code == 200

    resp = client2.post("/move", data=f"{gameid}\n{play2id}\nd8h4\n")
    assert resp.data.decode('utf-8') == 'legal move Check Mate'
    assert resp.status_code == 200

    # status 
    resp = client.get(f"/status?gid={gameid}")
    assert resp.data.decode('utf-8') == 'OVER 0-1 1:TURN w:LAST d8h4:MVNO 4\n'
    assert resp.status_code == 200



