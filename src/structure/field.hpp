/*
 field.hpp
 Katsuki Ohto
 */

#ifndef MAHJONG_STRUCTURE_FIELD_HPP_
#define MAHJONG_STRUCTURE_FIELD_HPP_

#include "base.hpp"
#include "piece.hpp"
#include "meld.hpp"
#include "hand.hpp"
#include "action.hpp"

namespace Mahjong{
    
    /**************************盤面情報**************************/
    
    struct Field{
        // 盤面データ
        // 主観的情報のみ用いる場合はそれ以外の箇所の値は未定義
        MatchType matchType;
        Player myPlayerNum; // 自分のプレーヤー番号
        
        // 局をまたいで変化する情報(基本毎回再設定のため情報がくる)
        int games; // 何局目か
        int rounds; // 東場、南場
        std::array<Score, N_PLAYERS> score; // 累計得点
        BitArray32<4, N_PLAYERS> position, invPosition; // 現在順位
        Wind fieldWind; // 場風
        int repetitionBonus, reachBonus; // 積み棒とリーチ棒
        Player owner; // 親
        
        // 1局の中の情報
        int turn; // 手数
        int lastResponseTurn; // 最後に誰かが鳴いた手数
        std::array<int, N_PLAYERS> reachTurn; // リーチした手数
        int reaches; // 全体でのリーチ数
        Player turnPlayer; // ツモ手番プレーヤー
        std::array<ExtPieceSet, N_PLAYERS> pickedSet; // 鳴いて得た牌集合
        std::array<ExtPieceSet, N_PLAYERS> discardedSet; // 捨て牌(順序無視)
        std::array<ExtPieceSequence<N_DISCARD_PIECES>, N_PLAYERS> discardedSeq; // 捨て牌(順序考慮)
        std::array<int, N_PLAYERS> discards; // 捨て牌数
        ExtPieceSet open; // 全員に対して開示された牌の集合
        PieceSet dora; // ドラ
        int doras; // ドラ枚数
        int kongs; // 局全体でのカンの回数
        
        // 主観的にわからない情報
        Pack128<int32_t, N_PLAYERS> pieces; // 手牌の数(構成がわからなくても枚数さえわかればここに入れる)
        std::array<Hand, N_PLAYERS> hand; // 各プレーヤーの持ち牌
        std::array<UncertainPieces, N_PLAYERS> uncertain; // 主観的にまだ見えていない牌(各プレーヤーごと)
        Wall wall; // 山
        int wallPieces; // 山牌の数
        
        Wind playerWind(Player pn)const noexcept{ // プレーヤーpの自風 親なら東(WIND_E)
            return static_cast<Wind>((unsigned int)(pn - owner) % N_PLAYERS) + WIND_E;
        }
        Player windPlayer(Wind w)const noexcept{ // playerWind() の逆変換
            return static_cast<Player>((unsigned int)(owner + w - WIND_E) % N_PLAYERS);
        }
        bool isInReach(Player pn)const{
            return reachTurn[pn] >= 0;
        }
        bool isAllReach()const noexcept{
            return reaches >= N_PLAYERS;
        }
        bool isConcealed(Player pn)const{ // 門前状態
            return hand[pn].isConcealed();
        }
        bool isImmediate(Player pn)const{ // 立直一発
            return lastResponseTurn >= reachTurn[pn];
        }
        bool isLastTurn()const noexcept{ // ラスト一周はリーチできないので注意
            return turn >= N_TURNS - N_PLAYERS;
        }
        bool isFirstTurn()const noexcept{ // 最初の一周 + 誰も鳴いていない
            return (turn < N_PLAYERS) && (lastResponseTurn < 0);
        }
        bool isReachable(Player pn)const{ // リーチ出来るか(現時点ではリーチしていないことを前提に)
            return isConcealed(pn) && !isLastTurn();
        }
        ExtPiece lastDiscarded(Player pn)const{
            return discardedSeq[pn][discards[pn] - 1];
        }
        
        // 主観的情報
        Score myScore()const{
            assertMyPlayerNum(); return score[myPlayerNum];
        }
        Wind myWind()const{
            assertMyPlayerNum(); return playerWind(myPlayerNum);
        }
        bool isInMyReach()const{
            assertMyPlayerNum(); return isInReach(myPlayerNum);
        }
        Hand& myHand(){
            assertMyPlayerNum(); return hand[myPlayerNum];
        }
        const Hand& myHand()const{
            assertMyPlayerNum(); return hand[myPlayerNum];
        }
        UncertainPieces& myUncertain(){
            assertMyPlayerNum(); return uncertain[myPlayerNum];
        }
        const UncertainPieces& myUncertain()const{
            assertMyPlayerNum(); return uncertain[myPlayerNum];
        }
        
        // 試合進行関数
        void reach(Player pn){ // リーチ
            ASSERT(examPlayerNum(pn), cerr << pn << endl;);
            reachTurn[pn] = turn - 1; // 先にdiscardが呼ばれるので
            reachBonus += 1;
            score[pn] -= SCORE_REACH;
            reaches += 1;
        }
        void declareReach(Player pn){ // リーチ宣言(変な打牌をすると失敗になる)
            // TODO: 記録しておけば諸々の推定に少しは使えるかも...
            ASSERT(examPlayerNum(pn), cerr << pn << endl;);
        }
        // 終局処理
        void switchOwner()noexcept{
            owner = nextPlayer(owner);
            if(owner == 0){
                rounds += 1;
            }
        }
        void drawToLeaveOwner()noexcept{
            repetitionBonus += 1;
        }
        void drawToSwitchOwner()noexcept{
            switchOwner();
            repetitionBonus += 1;
        }
        void winToLeaveOwner()noexcept{
            reachBonus = 0; // リーチ棒清算
            repetitionBonus += 1;
        }
        void winToSwitchOwner()noexcept{
            switchOwner();
            reachBonus = 0; // リーチ棒清算
            repetitionBonus = 0; // 0本場に戻る
        }
        
        void feedDiffScore(const std::array<Score, N_PLAYERS>& ds)noexcept{ // 1局の得点報告
            for(Player pn = 0; pn < N_PLAYERS; ++pn){
                score[pn] += ds[pn];
            }
        }
        void procTurn(Player pn)noexcept{
            turn += 1;
            turnPlayer = pn;
        }
        void procTurn()noexcept{
            turn += 1;
            turnPlayer = nextPlayer(turnPlayer);
        }
        
        void discard(Player pn, ExtPiece ep, bool parrot){ // 牌を捨てる
            //DERR << "player " << pn << " (" << playerWind(turnPlayer) << ") discarded " << ep;
            //DERR << " (" << (parrot ? "drawn" : "hand") << ")" << endl;
            
            turnPlayer = pn;
            
            // 捨て牌を追加
            discardedSet[turnPlayer] += ep;
            discardedSeq[turnPlayer][discards[turnPlayer]++] = ep;
            
            if(!isInReach(turnPlayer)){
                if(!examPlayerNum(myPlayerNum)
                   || turnPlayer == myPlayerNum){
                    // 本人の手牌から引く
                    // この時点で引いた牌が加算されている必要がある
                    // リーチのときは加算していないので引かない
                    hand[turnPlayer].subAll(ep);
                }
            }
            pieces[turnPlayer] -= 1;
            open += ep; // 全体開示牌追加
            // 本人以外のuncertainから引く
            for(Player p = 0; p < turnPlayer; ++p)uncertain[p] -= ep;
            for(Player p = turnPlayer + 1; p < N_PLAYERS; ++p)uncertain[p] -= ep;
            
            // ツモ切りのとき山牌の情報が確定(ただし自分の手番ではすでに分かっている)
            if(parrot){
                wall[wallIndexTurn(turn)] = ep;
            }
            //DERR << "Field::discard()" << endl << toString();
            ASSERT(exam(),);
        }
        
        // チー・ポンは打牌までが同時行動であるが、打牌は後ほど discard() 関数で扱う
        void chow(Player pn, Meld m, ExtPiece picked, const ExtPieceSet& opened){ // チー
            hand[pn].addOpenedSeq(m, picked, opened); // 役一覧に追加
            if(!examPlayerNum(myPlayerNum) || pn == myPlayerNum){ // 手牌から除く
                hand[pn].subSeqExceptAll(m.piece(), toPiece(picked), m.red());
            }
            pieces[pn] -= N_CHOW_PIECES - 1; // 手牌の枚数を引く
            open += opened; // 全体開示牌追加
            pickedSet[pn] += picked; // もらった牌追加
            // 本人以外のuncertainから引く
            for(Player p = 0; p < pn; ++p)uncertain[p] -= opened;
            for(Player p = pn + 1; p < N_PLAYERS; ++p)uncertain[p] -= opened;
            turnPlayer = nextPlayer(pn); // 次のプレーヤー設定
            lastResponseTurn = turn; // 新しい鳴き
        }
        void pong(Player pn, Meld m, ExtPiece picked, const ExtPieceSet& opened){ // ポン
            hand[pn].addOpenedGroup(m, picked, opened); // 役一覧に追加
            if(!examPlayerNum(myPlayerNum) || pn == myPlayerNum){ // 手牌から除く
                hand[pn].subGroupAll(m.piece(), N_PONG_PIECES - 1, m.red());
            }
            pieces[pn] -= N_PONG_PIECES - 1; // 手牌の枚数を引く
            open += opened; // 全体開示牌追加
            pickedSet[pn] += picked; // もらった牌追加
            // 本人以外のuncertainから引く
            for(Player p = 0; p < pn; ++p)uncertain[p] -= opened;
            for(Player p = pn + 1; p < N_PLAYERS; ++p)uncertain[p] -= opened;
            turnPlayer = nextPlayer(pn); // 次のプレーヤー設定
            lastResponseTurn = turn; // 新しい鳴き
        }
        
        // カンは一度牌を引くのでカン確定時点までの情報のみ
        void drawKong(Player pn, Meld m, const ExtPieceSet& opened){ // 暗槓 opened に全ての牌が入る
            hand[pn].addOpenedGroupNoPick(m, opened); // 役一覧に追加
            if(!examPlayerNum(myPlayerNum) || pn == myPlayerNum){ // 手牌から除く
                hand[pn].subGroupAll(m.piece(), N_KONG_PIECES, m.red());
            }
            pieces[pn] -= N_KONG_PIECES; // 手牌の枚数を引く
            open += opened; // 全体開示牌追加
            // 本人以外のuncertainから引く
            for(Player p = 0; p < pn; ++p)uncertain[p] -= opened;
            for(Player p = pn + 1; p < N_PLAYERS; ++p)uncertain[p] -= opened;
            // 手番プレーヤーそのまま(ただし次は嶺上牌を引く)
            lastResponseTurn = turn; // 鳴きではないが地和や一発が崩れる
        }
        void addKong(Player pn, ExtPiece added){ // 小明槓(成立時点)
            hand[pn].expandOpenedGroup(added); // 牌情報
            if(!examPlayerNum(myPlayerNum) || pn == myPlayerNum){ // 手牌から除く
                hand[pn].subAll(added);
            }
            pieces[pn] -= 1; // 手牌の枚数を引く
            open += added; // 全体開示牌追加
            // 本人以外のuncertainから引く
            for(Player p = 0; p < pn; ++p)uncertain[p] -= added;
            for(Player p = pn + 1; p < N_PLAYERS; ++p)uncertain[p] -= added;
            lastResponseTurn = turn; // 地和や一発が崩れる
        }
        void responseKong(Player pn, Meld m, ExtPiece picked, const ExtPieceSet& opened){ // 大明槓
            hand[pn].addOpenedGroup(m, picked, opened); // 役一覧に追加
            if(!examPlayerNum(myPlayerNum) || pn == myPlayerNum){ // 手牌から除く
                hand[pn].subGroupAll(m.piece(), N_KONG_PIECES - 1, m.red());
            }
            pieces[pn] -= N_KONG_PIECES - 1; // 手牌の枚数を引く
            open += opened; // 全体開示牌追加
            pickedSet[pn] += picked; // もらった牌追加
            // 本人以外のuncertainから引く
            for(Player p = 0; p < pn; ++p)uncertain[p] -= opened;
            for(Player p = pn + 1; p < N_PLAYERS; ++p)uncertain[p] -= opened;
            // 手番プレーヤーそのまま(ただし次は嶺上牌を引く)
            lastResponseTurn = turn; // 地和や一発が崩れる
        }
        void pushDora(ExtPiece adm){ // ドラ表示牌を受け取る
            wall[wallIndexDoraMarker(doras)] = adm; // 山のうちドラ表示牌の場所の牌が確定
            dora += toNextPiece(toPiece(adm)); // ドラは次の牌
            doras += 1;
            open += adm;
            for(Player p = 0; p < N_PLAYERS; ++p)uncertain[p] -= adm;
        }
        
        void initMatch(MatchType amt, Player pn,
                       const std::array<Score, N_PLAYERS>& ascore){ // 1対戦開始
            matchType = amt;
            myPlayerNum = pn;
            
            games = 0;
            rounds = 0;
            repetitionBonus = reachBonus = 0;
            fieldWind = WIND_E; // 基本は東から
            owner = 0; // 基本は0から
            score = ascore; // 初期状態が同点でなくても良いように
            setPosition();
            DERR << "Field::initMatch()" << endl;
        }
        void initGame(Wind afw, int agn, int arep,
                      int abets, Player ao, ExtPiece adm,
                      const std::array<ExtPieceSet, N_PLAYERS>& ahand,
                      const std::array<int, N_PLAYERS>& apieces){ // 1局開始
            // 初期化
            for(int i = 0; i < N_PLAYERS; ++i){
                pickedSet[i].clear();
                discardedSet[i].clear();
                discardedSeq[i].clear();
                hand[i].clear();
                uncertain[i].fill();
            }
            discards.fill(0);
            open.clear();
            dora.clear();
            doras = 0;
            kongs = 0;
            wall.clear();
            turn = 0;
            lastResponseTurn = -1;
            reachTurn.fill(-1);
            reaches = 0;
            turnPlayer = owner;
            
            // 取得した情報をセット
            
            fieldWind = afw;
            games = agn;
            repetitionBonus = arep;
            reachBonus = abets;
            owner = ao;
            pushDora(adm); // ドラ情報
            
            // 牌に関する情報をセット
            pieces = apieces;
            int sumPieces = 0;
            for(Player pn = 0; pn < N_PLAYERS; ++pn){
                hand[pn].setAll(ahand[pn]);
                // 他人の牌がわかった場合もuncertainには加えておく
                uncertain[pn] = ~ahand[pn];
                uncertain[pn] -= adm; // ドラ表示牌は開示
                sumPieces += pieces[pn];
                
                // 判明した駒を配布順を仮定して並べておく
                iterateExtPieceWithCount(hand[pn].piece, [this, pn](int index, ExtPiece ep)->void{
                    this->wall[index * N_PLAYERS + pn] = ep;
                });
            }
            
            // 同時に明らかになる情報をセット
            wallPieces = N_ALL_PIECES - N_LEFT_PIECES - sumPieces;
            
            DERR << "Field::initGame()" << endl;
            DERR << toDebugString() << endl;
            ASSERT(exam(),);
            
        }
        void setDealt(Player pn, const ExtPieceSet& ps){ // 配牌時の情報設定
            hand[pn].setAll(ps);
            uncertain[pn] = ~ps;
            
            DERR << "Field::setDealt()" << endl;
        }
        void setTurn(Player pn, ExtPiece ep){ // ツモ手番での情報設定
            turnPlayer = pn;
            wall[wallIndexTurn(turn)] = ep;
            // この時点で牌を足しておく
            // リーチのときはツモ切りしかできないので足さない
            if(examExtPiece(ep)){ // どの牌かわかる場合
                if(!isInReach(turnPlayer)){
                    hand[turnPlayer].addAll(ep);
                }
                uncertain[turnPlayer] -= ep; // この時点引いた本人のみ情報を得る
            }
            pieces[turnPlayer] += 1; // 一時的に枚数増加
            
            //DERR << "Field::setTurn()" << endl << toDebugString() << endl;
            ASSERT(exam(),);
        }
        /*void setTurnAfterKong(Player pn, ExtPiece ep){ // カン後の同じプレーヤーのツモ手番での情報設定
            turnPlayer = pn;
            wall[wallIndexTurnAfterKong(kongs)] = ep;
            // この時点で牌を足しておく
            // リーチのときはツモ切りしかできないので足さない
            if(examExtPiece(ep)){ // どの牌かわかる場合
                if(!isInReach(turnPlayer)){
                    hand[turnPlayer].addAll(ep);
                }
                uncertain[turnPlayer] -= ep; // この時点引いた本人のみ情報を得る
            }
            pieces[turnPlayer] += 1; // 一時的に枚数増加
            
            DERR << "Field::setTurnAfterKong()" << endl;
            DERR << toDebugString() << endl;
            ASSERT(exam(),);
        }*/
        void setPosition()noexcept{
            // 得点更新が終わっていることを仮定
            position.clear();
            for(Player p0 = 0; p0 < N_PLAYERS; ++p0){
                for(Player p1 = 0; p1 < p0; ++p1){
                    if(score[p0] <= score[p1]){ // 同点なら第1局の風が優先
                        position.add(p0, 1);
                    }
                }
                for(Player p1 = p0 + 1; p1 < N_PLAYERS; ++p1){
                    if(score[p0] < score[p1]){
                        position.add(p0, 1);
                    }
                }
            }
            invPosition = invert(position);
        }
        void closeGame(){ // 1局終了
            setPosition();
            DERR << "Field::closeGame()" << endl;
        }
        void closeMatch(){ // 1対戦終了
            DERR << "Field::closeMatch()" << endl;
        }
        
        std::string toCommonString()const{ // 客観的な局面情報
            std::ostringstream oss;
            oss << "game " << games << " (round " << fieldWind << "-" << owner << ") turn " << turn;
            oss << " player " << turnPlayer << " (" << playerWind(turnPlayer) << ")" << endl;
            oss << "score [";
            for(Player pn = 0; pn < N_PLAYERS; ++pn){
                oss << score[pn];
                if(pn != N_PLAYERS - 1){
                    oss << ", ";
                }
            }
            oss << "] (reachB:" << reachBonus << " repB:" << repetitionBonus << ") ";
            oss << " dora " << dora << endl;
            return oss.str();
        }
        std::string toString()const{ // 神視点での局面情報(プレーヤー視点では見える情報だけ)
            std::ostringstream oss;
            oss << toCommonString();
            for(Player pn = 0; pn < N_PLAYERS; ++pn){
                oss << pn;
                if(isInReach(pn)){
                    oss << "R";
                }else{
                    oss << "-";
                }
                oss << "(" << playerWind(pn) << ")" << " : (" << pieces[pn] << ")" << hand[pn].toString();
                oss << endl;
            }
            return oss.str();
        }
        std::string toSubjectiveString()const{ // 主観的な実況用
            std::ostringstream oss;
            oss << toCommonString();
            oss << "I'm " << myPlayerNum << " (" << myWind() << ")" << endl;
            oss << to2TableStrings(myHand().naive(), myUncertain().naive());
            return oss.str();
        }
        std::string toDebugString()const{ // 詳しく
            std::ostringstream oss;
            oss << toCommonString();
            for(Player p = 0; p < N_PLAYERS; ++p){
                oss << p << " (" << playerWind(p) << ")" << " : " << endl;
                oss << to2TableStrings(hand[p].naive(), uncertain[p].naive());
            }
            return oss.str();
        }
        
        bool examPlayersInfo(Player pn)const{
            if(!hand[pn].exam()){
                cerr << "Field::examPlayersInfo() : illegal hand of " << pn << endl;
                return false;
            }
            if(!uncertain[pn].exam()){
                cerr << "Field::examPlayersInfo() : illegal uncertain of " << pn << endl;
                return false;
            }
            if(!discardedSet[pn].exam()){
                cerr << "Field::examPlayersInfo() : illegal discarded set of " << pn << endl;
                return false;
            }
            if(!pickedSet[pn].exam()){
                cerr << "Field::examPlayersInfo() : illegal picked set of " << pn << endl;
                return false;
            }
            return true;
        }
        bool examDora()const{ // ドラ関連
            if(!dora.exam()){
                cerr << "Field::examDora() : illegal piece-set |dora|" << endl;
                return false;
            }
            if(dora.sum() != doras){
                cerr << "Field::examDora() : inconsistent num of doras" << dora << " <-> " << doras << endl;
            }
            // ドラ表示牌の位置を設定できているか確認
            // 嶺上牌の特別処理をしないのならばどうでもいい
            /*ExtPieceSet tmp = dora;
            for(int d = 0; d < doras; ++d){
                if(wall[])
            }*/
            return true;
        }
        bool examUncertainPieces()const{
            //オープンになっている牌 + 捨て牌 + 自分の手牌 + 山牌の確定部分 + 未確定牌で丁度全ての牌になるかチェック
            return true;
        }
        bool examSettledPieces()const{
            // 設定済みの牌チェック turn でのツモより前を仮定する
            // TODO: 各プレーヤーの手牌の部分が設定されていることを前提にするか?
            // オープンになっている牌 + 捨て牌 + 手牌 + 山牌 で丁度全ての牌になるかチェック
            ExtPieceSet eps;
            eps.clear();
            for(Player pn = 0; pn < N_PLAYERS; ++pn){
                eps += hand[pn].piece; // 手牌
                eps += hand[pn].openedPiece; // 鳴いた牌
                eps -= pickedSet[pn]; // もらった牌 他家の捨て牌と被るため引く
                eps += discardedSet[pn]; // 捨てた牌
            }
            for(int i = wallIndexTurn(turn); i < wall.size(); ++i){
                eps += wall[i];
            }
            if(eps != EXT_PIECE_SET_ALL){
                cerr << "Field::examSettledPieces() : hand(concealed, opened) - picked + discarded + wall != ALL" << endl;
                cerr << "all : " << eps.array() << eps.red() << endl;
                for(Player pn = 0; pn < N_PLAYERS; ++pn){
                    cerr << "picked : " << pickedSet[pn] << " discarded : " << discardedSet[pn] << endl;
                }
                cerr << "wall : " << wall << endl;
                cerr << toString();
                return false;
            }
            return true;
        }
        bool examScore()const{
            Score sum = static_cast<Score>(0);
            for(Player pn = 0; pn < N_PLAYERS; ++pn){
                sum += score[pn];
            }
            sum += reachBonus * SCORE_REACH;
            if(sum != SCORE_INITIAL * N_PLAYERS){
                cerr << "Field::examScore() : inconsistent score" << endl;
                cerr << toString();
                return false;
            }
            return true;
        }
        bool exam()const{
            if(reachBonus < 0){ return false; }
            if(repetitionBonus < 0){ return false; }
            if(!examScore()){ return false; }
            if(!examDora()){ return false; }
            // 他人関連情報
            for(Player p = 0; p < N_PLAYERS; ++p){
                if(p != myPlayerNum){
                    if(!examPlayersInfo(p)){
                        cerr << "Field::exam() : illegal player's info of " << p << endl;
                        return false;
                    }
                }
            }
            // 手牌の枚数チェック(主観的なときは自分のみ)
            if(examPlayerNum(myPlayerNum)){
                const int okDiff = isInMyReach() ? 1 : 0;
                int diff = pieces[myPlayerNum] - myHand().allPieces;
                if(!(0 <= diff && diff <= okDiff)){
                    cerr << "Field::exam() : " << "inconsistent my num of pieces ";
                    cerr << myHand().allPieces << " (hand) ";
                    cerr << " <-> " << pieces[myPlayerNum] << " (num of pieces)" << endl;
                    return false;
                }
            }
            return true;
        }
        bool examSubjectiveInfo()const{
            // 主観的情報として利用する場合のバリデーション
            if(examPlayerNum(myPlayerNum)){
                if(!examPlayersInfo(myPlayerNum)){
                    cerr << "Field::exam() : illegal player's info of " << myPlayerNum << endl;
                    return false;
                }
            }
            if(!exam()){ return false; }
            return true;
        }
        bool examInSimulation()const{
            // シミュレーション中 不完全情報を設定しているのでその部分もチェック
            if(!examSettledPieces()){ return false; }
            if(!exam()){ return false; }
            return true;
        }
        
        void assertMyPlayerNum()const{
            ASSERT(examPlayerNum(myPlayerNum), cerr << myPlayerNum << endl;);
        }
    };
    
    struct FieldTemporalInfo{
        // 場の状態を一時的に保存するためのクラス
        bool afterKong;
        TurnAction turnAction;
        std::array<AddedAction, N_PLAYERS> addedAction;
        std::array<ResponseAction, N_PLAYERS> responseAction;
    };
    
    struct DecidedActionInfo{
        
    };
    
    /*FieldStatus doTurnAction(Field *const pfield, const TurnAction& ta, FieldTemporalInfo *const pfti){
        if(ta.discard()){ // 打牌
            pfield->discard(pfield->turnPlayer, ta.discarded(), ta.parrot());
            pfti->discarded = ta.discarded();
            return STATUS_DISCARDED;
        }else if(ta.drawKong()){ // 暗槓
            pfield->drawKong(turnPlayer, decidedTurnAction.toKongMeld(),
                             decidedTurnAction.toKongConsumed());
            pfti->afterKong = true;
            return STATUS_TURN_START;
        }else{ // 小明槓
            // 槍槓があるのでここでは進めない
            pfti->added = ta.extPiece();
            return STATUS_ADDED;
        }
    }
    FieldStatus doResponseAction(Field *const pfield, const ResponseAction& ra){
        if(action.discard()){ // 打牌
            pfield->discard(pfield->turnPlayer, ta.discarded(), ta.parrot());
        }else if(action.drawKong()){ // 暗槓
            pfield->drawKong(turnPlayer, decidedTurnAction.toKongMeld(),
                             decidedTurnAction.toKongConsumed());
        }else{ // 小明槓
            pfield->addKong(turnPlayer, added);
        }
    }
    
    template<class action_t>
    FieldStatus doAction(Field *const pfield, const action_t& action);
    
    template<>
    FieldStatus doAction(Field *const pfield, const TurnAction& action){
        doTurnAction(pfield, action);
    }
    template<>
    FieldStatus doAction(Field *const pfield, const ResponseAction& action){
        doResponseAction(pfield, action);
    }*/
}

#endif // MAHJONG_STRUCTURE_FIELD_HPP_