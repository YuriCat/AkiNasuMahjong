/*
 floodgate.cpp
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>

#include "../lib/picojson.h"

#include "easywsclient.hpp"

#include "eggplant.hpp"

#include "protocol/mjai.hpp"

namespace Mahjong{
    namespace Floodgate{
        
        template<class engine_t>
        std::string communicate(engine_t& engine, picojson::value& v, uint64_t tempTime){
            
            picojson::object& o = v.get<picojson::object>();
            picojson::object send;
            
            const std::string& typeString = o["type"].get<std::string>();
            
            if(typeString == "hello"){ // ログイン成功時
                
                send["type"] = (picojson::value)std::string("join");
                send["name"] = (picojson::value)engine.name();
                send["room"] = (picojson::value)std::string("default");
                
            }else if(typeString == "start_game"){ // 1対戦開始時
                
                Player myPlayerNum = static_cast<Player>((int)o["id"].get<double>());
                MatchType matchType = mjai::toMatchType(o["gametype"].get<std::string>());
                
                std::array<Score, N_PLAYERS> initialScore;
                initialScore.fill(SCORE_INITIAL);
                engine.field().initMatch(matchType, myPlayerNum, initialScore);
                engine.initMatch();
                
                DERR << engine.field().toSubjectiveString();
                
                send["type"] = (picojson::value)std::string("none");
                
            }else if(typeString == "start_kyoku"){ // 1局開始時
                // 情報取得
                Wind fieldWind = toWind(o["bakaze"].get<std::string>());
                int gameNum = o["kyoku"].get<double>() - 1; // 1から始まるので調整
                int repetition = o["honba"].get<double>();
                int bets = o["kyotaku"].get<double>();
                Player owner = static_cast<Player>(int(o["oya"].get<double>()));
                ExtPiece doraMarker = mjai::toExtPiece(o["dora_marker"].get<std::string>());
                
                picojson::array& hands = o["tehais"].get<picojson::array>();
                std::array<ExtPieceSet, N_PLAYERS> hand;
                std::array<int, N_PLAYERS> pieces;
                for(Player p = 0; p < N_PLAYERS; ++p){
                    picojson::array& ho = hands[p].get<picojson::array>();
                    hand[p].clear();
                    pieces[p] = 0;
                    for(const picojson::value& po : ho){
                        const std::string pstr = po.get<std::string>();
                        ExtPiece ep = mjai::toExtPiece(pstr);
                        //cerr << ep << " " << int(ep) << endl;
                        // 相手の牌がわかる場合，不明な場合どちらにも対処
                        if(examExtPiece(ep)){
                            hand[p] += ep;
                        }
                        pieces[p] += 1;
                    }
                }
                
                cerr << toString(hand) << endl;
                
                engine.field().initGame(fieldWind, gameNum, repetition, bets, owner, doraMarker, hand, pieces);
                engine.initGame();
                
                DERR << engine.field().toSubjectiveString();
                
                send["type"] = (picojson::value)std::string("none");
                
            }else if(typeString == "tsumo"){
                
                Player turnPlayer = static_cast<Player>(int(o["actor"].get<double>()));
                ExtPiece drawn = mjai::toExtPiece(o["pai"].get<std::string>()); // ツモ牌
                
                //cerr << "open " << engine.field().open << " " << engine.field().open.sum() << endl;
                
                if(engine.field().open.sum() != engine.field().doras){ // 初手でなければ、手番をここで進める
                    engine.field().procTurn(turnPlayer);
                }
                engine.field().setTurn(turnPlayer, drawn);
                
                if(engine.playerNum() == turnPlayer){
                    // ツモ手番の行動決定
                    bool decided = false;
                    picojson::value& v = o["possible_actions"];
                    if(!v.is<picojson::null>()){
                        picojson::array& possibleActions = v.get<picojson::array>();
                        if(possibleActions.size() > 0){
                            for(int i = 0; i < possibleActions.size(); ++i){
                                picojson::object obj = possibleActions[i].get<picojson::object>();
                                std::string type = obj["type"].get<std::string>();
                                if(type == "hora"){ // 上がり
                                    send = obj;
                                    decided = true;
                                }
                            }
                        }
                    }
                    if(!decided){
                        TurnAction act = engine.turn(drawn);
                        
                        if(act.drawKong()){ // 暗槓
                            send["type"] = (picojson::value)std::string("ankan");
                            send["actor"] = (picojson::value)double(int(engine.playerNum()));
                            ExtPieceSet ps = act.toKongConsumed();
                            picojson::array consumedObj;
                            iterateExtPiece(ps, [&consumedObj](ExtPiece ep)->void{
                                consumedObj.push_back((picojson::value)toString(ep));
                            });
                            send["consumed"] = (picojson::value)consumedObj;
                        }else if(act.addKong()){ // 小明槓
                            send["type"] = (picojson::value)std::string("kakan");
                            send["actor"] = (picojson::value)double(int(engine.playerNum()));
                            ExtPiece ep = act.extPiece();
                            send["pai"] = (picojson::value)toString(ep);
                            // 既にポンした牌の情報も再度送信するので、手牌情報から検索する
                            //ExtPieceSet ps = engine.field().myHand().toGroupConsumed(toPiece(ep));
                            ExtPieceSet ps = engine.field().myHand().openedPiece.pick(toPiece(ep)); // 現行ルールならこれでいいけど適当すぎる
                            picojson::array consumedObj;
                            iterateExtPiece(ps, [&consumedObj](ExtPiece ep)->void{
                                consumedObj.push_back((picojson::value)toString(ep));
                            });
                            send["consumed"] = (picojson::value)consumedObj;
                        }else if(act.reach()){ // 打牌 + リーチ
                            // リーチのときはここではリーチとだけいう
                            send["type"] = (picojson::value)std::string("reach");
                            send["actor"] = (picojson::value)double(int(engine.playerNum()));
                        }else{ // 通常の打牌
                            send["type"] = (picojson::value)std::string("dahai");
                            send["actor"] = (picojson::value)double(int(engine.playerNum()));
                            send["pai"] = (picojson::value)toString(act.discarded());
                            send["tsumogiri"] = (picojson::value)bool(act.parrot());
                        }
                    }
                }else{
                    send["type"] = (picojson::value)std::string("none");
                }
                engine.clearLastMeld();
                
            }else if(typeString == "dahai"){ // 打牌
                
                Player turnPlayer = static_cast<Player>(int(o["actor"].get<double>()));
                ExtPiece discarded = mjai::toExtPiece(o["pai"].get<std::string>()); // 打牌
                bool parrot = o["tsumogiri"].get<bool>();
                
                // 前の行動によって処理が変化する?
                Meld lastMeld = engine.lastMeld();
                
                // 通常の打牌
                engine.field().discard(turnPlayer, discarded, parrot);
                
                DERR << engine.field().toSubjectiveString();
                
                // ここで反応行動を検討する
                if(engine.playerNum() != turnPlayer  // 自分以外の打牌のみ
                   && !engine.field().isInReach(engine.playerNum())){ // 自分がリーチ中でない
                    bool decided = false;
                    
                    picojson::value& v = o["possible_actions"];
                    if(!v.is<picojson::null>()){
                        picojson::array& possibleActions = v.get<picojson::array>();
                        if(possibleActions.size() > 0){
                            // 行動を検討(デバッグ時はpossible_actionsが無くても自分で行動生成する)
                            for(int i = 0; i < possibleActions.size(); ++i){
                                picojson::object obj = possibleActions[i].get<picojson::object>();
                                std::string type = obj["type"].get<std::string>();
                                if(type == "hora"){ // 上がり
                                    send = obj;
                                    decided = true;
                                }
                            }
                        }
                    }
                    
                    if(!decided){
                        ResponseAction act = engine.response(discarded);
                        if(act.any()){
                            if(act.responseKong()){ // 大明槓
                                send["type"] = (picojson::value)std::string("daiminkan");
                            }else if(act.pong()){
                                send["type"] = (picojson::value)std::string("pon");
                            }else if(act.chow()){
                                send["type"] = (picojson::value)std::string("chi");
                            }else{ // ??
                                ASSERT(0, cerr << act << " " << act.data() << endl;);
                            }
                            send["actor"] = (picojson::value)double(int(engine.playerNum()));
                            send["target"] = (picojson::value)double(int(turnPlayer));
                            send["pai"] = (picojson::value)toString(discarded);
                            
                            // 含める自分の牌の配列を設定
                            ExtPieceSet eps = act.toConsumed();
                            eps -= discarded;
                            
                            picojson::array consumedObj;
                            iterateExtPiece(eps, [&consumedObj](ExtPiece ep)->void{
                                consumedObj.push_back((picojson::value)toString(ep));
                            });
                            
                            send["consumed"] = (picojson::value)consumedObj;
                            
                        }else{
                            send["type"] = (picojson::value)std::string("none");
                        }
                    }
                }else{
                    send["type"] = (picojson::value)std::string("none");
                }
            }else if(typeString == "pon"){ // ポン
                Player pn = static_cast<Player>(int(o["actor"].get<double>()));
                Player supplyer = static_cast<Player>(int(o["target"].get<double>()));
                ExtPieceSet opened;
                opened.clear();
                ExtPiece picked = mjai::toExtPiece(o["pai"].get<std::string>());
                picojson::value& v = o["consumed"];
                if(!v.is<picojson::null>()){
                    picojson::array& consumedObj = v.get<picojson::array>();
                    for(int i = 0; i < consumedObj.size(); ++i){
                        ExtPiece ep = mjai::toExtPiece(consumedObj[i].get<std::string>());
                        opened += ep;
                    }
                }
                Meld pong = toGroupMeld(opened + picked);
                
                // 自分の場合には続けて打牌
                engine.field().pong(pn, pong, picked, opened);
                
                if(pn == engine.playerNum()){
                    ResponseAction act = engine.lastResponseAction();
                    
                    send["type"] = (picojson::value)std::string("dahai");
                    send["actor"] = (picojson::value)double(int(engine.playerNum()));
                    send["pai"] = (picojson::value)toString(act.discarded());
                    send["tsumogiri"] = (picojson::value)false;
                }else{
                    send["type"] = (picojson::value)std::string("none");
                }
            }else if(typeString == "chi"){ // チー
                Player pn = static_cast<Player>(int(o["actor"].get<double>()));
                Player supplyer = static_cast<Player>(int(o["target"].get<double>()));
                ExtPieceSet opened;
                opened.clear();
                ExtPiece picked = mjai::toExtPiece(o["pai"].get<std::string>());
                picojson::value& v = o["consumed"];
                if(!v.is<picojson::null>()){
                    picojson::array& consumedObj = v.get<picojson::array>();
                    for(int i = 0; i < consumedObj.size(); ++i){
                        ExtPiece ep = mjai::toExtPiece(consumedObj[i].get<std::string>());
                        opened += ep;
                    }
                }
                Meld chow = toSeqMeld(opened + picked);
                
                // 自分の場合には続けて打牌
                engine.field().chow(pn, chow, picked, opened);
                
                if(pn == engine.playerNum()){
                    ResponseAction act = engine.lastResponseAction();
                    
                    send["type"] = (picojson::value)std::string("dahai");
                    send["actor"] = (picojson::value)double(int(engine.playerNum()));
                    send["pai"] = (picojson::value)toString(act.discarded());
                    send["tsumogiri"] = (picojson::value)false;
                }else{
                    send["type"] = (picojson::value)std::string("none");
                }
            }else if(typeString == "kakan"){ // 小明槓
                // ロンあがり出来るはず
                bool decided = false;
                picojson::value& v = o["possible_actions"];
                if(!v.is<picojson::null>()){
                    picojson::array& possibleActions = v.get<picojson::array>();
                    if(possibleActions.size() > 0){
                        // 行動を検討(デバッグ時はpossible_actionsが無くても自分で行動生成する)
                        for(int i = 0; i < possibleActions.size(); ++i){
                            picojson::object obj = possibleActions[i].get<picojson::object>();
                            std::string type = obj["type"].get<std::string>();
                            if(type == "hora"){ // 上がり
                                send = obj;
                                decided = true;
                            }
                        }
                    }
                }
                if(!decided){
                    Player pn = static_cast<Player>(int(o["actor"].get<double>()));
                    ExtPiece added = mjai::toExtPiece(o["pai"].get<std::string>());
                    
                    engine.field().addKong(pn, added);
                    send["type"] = (picojson::value)std::string("none");
                }
            }else if(typeString == "daiminkan"){ // 大明槓
                Player pn = static_cast<Player>(int(o["actor"].get<double>()));
                Player supplyer = static_cast<Player>(int(o["target"].get<double>()));
                ExtPieceSet opened;
                opened.clear();
                ExtPiece picked = mjai::toExtPiece(o["pai"].get<std::string>());
                picojson::value& v = o["consumed"];
                if(!v.is<picojson::null>()){
                    picojson::array& consumedObj = v.get<picojson::array>();
                    for(int i = 0; i < consumedObj.size(); ++i){
                        ExtPiece ep = mjai::toExtPiece(consumedObj[i].get<std::string>());
                        opened += ep;
                    }
                }
                Meld pong = toGroupMeld(opened + picked);
                engine.field().responseKong(pn, pong, picked, opened);
                send["type"] = (picojson::value)std::string("none");
            }else if(typeString == "ankan"){ // 暗槓
                Player pn = static_cast<Player>(int(o["actor"].get<double>()));
                ExtPieceSet opened;
                opened.clear();
                picojson::value& v = o["consumed"];
                if(!v.is<picojson::null>()){
                    picojson::array& consumedObj = v.get<picojson::array>();
                    for(int i = 0; i < consumedObj.size(); ++i){
                        ExtPiece ep = mjai::toExtPiece(consumedObj[i].get<std::string>());
                        opened += ep;
                    }
                }
                Meld kong = toGroupMeld(opened);
                engine.field().drawKong(pn, kong, opened);
                send["type"] = (picojson::value)std::string("none");
            }else if(typeString == "dora"){ // カンによってドラ追加
                ExtPiece doraMarker = mjai::toExtPiece(o["dora_marker"].get<std::string>());
                engine.field().pushDora(doraMarker);
                send["type"] = (picojson::value)std::string("none");
            }else if(typeString == "reach"){
                
                Player turnPlayer = static_cast<Player>(int(o["actor"].get<double>()));
                engine.field().declareReach(turnPlayer);
                if(turnPlayer == engine.playerNum()){
                    // 自分のリーチのときはここで打牌の情報を送る
                    TurnAction act = engine.lastTurnAction();
                    
                    send["type"] = (picojson::value)std::string("dahai");
                    send["actor"] = (picojson::value)double(int(engine.playerNum()));
                    send["pai"] = (picojson::value)toString(act.discarded());
                    send["tsumogiri"] = (picojson::value)bool(act.parrot());
                }else{
                    send["type"] = (picojson::value)std::string("none");
                }
            }else if(typeString == "reach_accepted"){
                Player turnPlayer = static_cast<Player>(int(o["actor"].get<double>()));
                engine.field().reach(turnPlayer);
                // TODO: betの情報を更新
                send["type"] = (picojson::value)std::string("none");
            }else if(typeString == "hora"){
                // あがりプレーヤーの情報
                
                // 得点差分情報
                std::array<Score, N_PLAYERS> diffScore;
                diffScore.fill(Score(0));
                picojson::value& v = o["deltas"];
                if(!v.is<picojson::null>()){
                    picojson::array& diffScoreObj = v.get<picojson::array>();
                    for(Player pn = 0; pn < N_PLAYERS; ++pn){
                        Score ds = static_cast<Score>(int(diffScoreObj[pn].get<double>()));
                        diffScore[pn] = ds;
                    }
                }
                engine.field().feedDiffScore(diffScore);
                send["type"] = (picojson::value)std::string("none");
            }else if(typeString == "ryukyoku"){
                // 各プレーヤーの牌の情報
                
                // 得点差分情報
                std::array<Score, N_PLAYERS> diffScore;
                diffScore.fill(Score(0));
                picojson::value& v = o["deltas"];
                if(!v.is<picojson::null>()){
                    picojson::array& diffScoreObj = v.get<picojson::array>();
                    for(Player pn = 0; pn < N_PLAYERS; ++pn){
                        Score ds = static_cast<Score>(int(diffScoreObj[pn].get<double>()));
                        diffScore[pn] = ds;
                    }
                }
                engine.field().feedDiffScore(diffScore);
                send["type"] = (picojson::value)std::string("none");
            }else if(typeString == "end_kyoku"){
                engine.field().closeGame();
                engine.closeGame();
                send["type"] = (picojson::value)std::string("none");
            }else if(typeString == "end_game"){
                engine.field().closeMatch();
                engine.closeMatch();
            }else if(typeString == "error"){}
            
            picojson::value val(send);
            return val.serialize();
        }
        
        template<class engine_t>
        std::string communicate(engine_t& engine, const std::string& message, uint64_t tempTime){
            const char *cstr = message.c_str();
            picojson::value v;
            picojson::parse(v, cstr, cstr + message.size());
            return communicate(engine, v, tempTime);
        }
        
        int sendMessage(int sock, const std::string& msg){
            int len = msg.size() + 8;
            char *sbuffer = new char[len];
            memset(sbuffer, 0, sizeof(char) * len);
            snprintf(sbuffer, len, msg.c_str());
            int err = send(sock, sbuffer, strlen(sbuffer), 0);
            if(err < 0){
                CERR << "failed to send message(" << err << ")." << endl;
            }
            delete[] sbuffer;
            return err;
        }
        
        std::string recvMessage;
        void handleMessage(const std::string &message){
            recvMessage = message;
        }
        
        template<class engine_t>
        int gameLoop(const std::string& host, int port, engine_t& engine, int limitGames){
            
            ClockMicS cl; // 時間計測用
            
#ifdef FLOODGATE
            using easywsclient::WebSocket;
            while(1){
                WebSocket::pointer ws = WebSocket::from_url("ws://www.logos.t.u-tokyo.ac.jp/mjai/");
                while(ws->getReadyState() != WebSocket::CLOSED){
                    cl.start(); // 時間計測開始
                    while(1){
                        recvMessage = "";
                        ws->poll();
                        ws->dispatch(handleMessage);
                        if(recvMessage != ""){
                            uint64_t tempTime = cl.restart();
                            cerr << "Serer >> " << recvMessage << endl;
                            if(recvMessage.find("error") != std::string::npos){ // エラー
                                cerr << "error " << recvMessage << endl;
                                // 他のプレーヤーのエラーの場合もあるので落とさない
                                //goto END_GAME;
                                return 1;
                            }else if(recvMessage.find("end_game") == std::string::npos){ // 通常
                                std::string sentMessage = communicate(engine, recvMessage, tempTime);
                                cerr << "Client << " << sentMessage << endl;
                                ws->send(sentMessage);
                                recvMessage = "";
                            }else{ // 試合終了
                                //goto END_GAME;
                                return 0;
                            }
                            break;
                        }
                    }
                }
            END_GAME:
                ws->close();
            }
#endif
            
            CERR << "host: " << host << " port: " << port << endl;
            
            while(1){
#ifdef _WIN32
                // Windows 独自の設定
                WSADATA data;
                if(SOCKET_ERROR == WSAStartup(MAKEWORD(2, 0), &data)){
                    CERR << "failed to initialize WSA-data." << endl;
                    exit(1);
                }
#endif // _WIN32
                
                // open socket
                const int sock = socket(AF_INET, SOCK_STREAM, 0);
                if(sock < 0){
                    CERR << "failed to open socket." << endl;
                    exit(1);
                }
                //sockaddr_in 構造体のセット
                struct sockaddr_in addr;
                addr.sin_port = htons(port);
                addr.sin_family = AF_INET;
                addr.sin_addr.s_addr = inet_addr(host.c_str());
                // 接続
                if(connect(sock, (struct sockaddr*)&addr, sizeof(addr))){ // ソケット接続失敗
                    CERR << "failed to connect to server." << endl;
                    exit(1);
                }
                
                // 通信
                int L = 8192;
                char *rbuffer = new char[L];
                memset(rbuffer, 0, L);
                std::string unfinished = "";
                
                // 時間計測開始
                cl.start();
                
                while(1){
                    if(strlen(rbuffer) <= 0){
                        // recvの返り値はssize_t
                        int bytes = recv(sock, rbuffer, L, 0);
                        if(bytes == 0){
                            CERR << "disconnected..." << endl;
                            exit(1);
                        }else{
                            while(bytes == L){
                                L = 2 * L;
                                char *nrbuffer = new char[L];
                                memset(nrbuffer, 0, L);
                                memcpy(nrbuffer, rbuffer, bytes);
                                delete[] rbuffer;
                                rbuffer = nrbuffer;
                                bytes += recv(sock, rbuffer + bytes, L - bytes, 0);
                            }
                        }
                    }
                    
                    std::string str = std::string(rbuffer);
                    int len = strlen(rbuffer);
                    memmove(rbuffer, rbuffer + len + 1, L - len - 1);
                    memset(rbuffer + L - len - 1, 0, len + 1);
                    
                    std::vector<std::string> commands = split(str, '\n');
                    
                    for(std::string command : commands){
                        if(command.size() <= 0){ continue; }
                        
                        uint64_t tempTime = cl.restart();
                        
                        command = unfinished + command; // 前にjsonが完成していないテキストと繋げる
                        cerr << "Server >> " << command << endl;
                        
                        if(command.find("end_game") != std::string::npos){
                            goto END_GAME_SOCK;
                        }
                        
                        const char* json = command.c_str();
                        picojson::value v;
                        std::string jerr;
                        picojson::parse(v, json, json + strlen(json), &jerr);
                        
                        if(jerr.empty()){
                            std::string sentMessage = communicate(engine, v, tempTime);
                            if(sentMessage.size() > 2){
                                cerr << "Client << " << sentMessage << endl;
                                if(sendMessage(sock, sentMessage + "\n") < 0){
                                    return -1;
                                }
                            }
                        }else{
                            // json形式が完成していない
                            unfinished = command;
                        }
                    }
                }
            END_GAME_SOCK:
                if(rbuffer != nullptr){
                    delete[] rbuffer;
                }
                
                // close socket
#ifdef _WIN32
                closesocket(sock);
                WSACleanup();
#else
                close(sock);
#endif // _WIN32
            }
            return 0;
        }
    }
}

int main(int argc, char *argv[]){
    
    std::string host = "127.0.0.1";
    unsigned int port = 11600;
    std::string param_dir = "./data/";
    int limitGames = 65536;
    
    Mahjong::Eggplant::EggplantAI eggplantAI;
    
    for(int c = 1; c < argc; ++c){
        if(!strcmp(argv[c], "-h")){ // host
            host = std::string(argv[c + 1]);
        }else if(!strcmp(argv[c], "-i")){ // input directory
            param_dir = std::string(argv[c + 1]);
        }else if(!strcmp(argv[c], "-p")){ // port
            port = atoi(argv[c + 1]);
        }else if(!strcmp(argv[c], "-s")){ // random seed
            eggplantAI.setRandomSeed(atoi(argv[c + 1]));
        }else if(!strcmp(argv[c], "-n")){ // player name
            eggplantAI.setName(std::string(argv[c + 1]));
        }else if(!strcmp(argv[c], "-g")){ // num of games
            limitGames = atoi(argv[c + 1]);
        }
    }
    
    Mahjong::Floodgate::gameLoop(host, port, eggplantAI, limitGames);
    
    return 0;
}