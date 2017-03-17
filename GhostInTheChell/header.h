#include <algorithm>
#include <cassert>
#include <chrono>
#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <math.h>
#include <memory>
#include <queue>
#include <sstream>
#include <stdio.h>
#include <string>
#include <time.h>
#include <vector>

#ifdef LOCAL
#else
//#pragma GCC optimize "-O3"
//#pragma GCC optimize "O3,omit-frame-pointer,inline"
//#define NDEBUG
#endif

using namespace std;

#define FOR(i, a, b) for (int i = (a); i < (b); ++i)
#define REP(i, n) FOR(i, 0, n)
#define POW(n) ((n) * (n))
#define ALL(a) (a).begin(), (a).end()
#define cerr                                                                   \
  if (true)                                                                    \
  cerr
#define iptcerr                                                                \
  if (true)                                                                    \
  cerr
#define dump(v) cerr << #v << ": " << v << endl

#ifdef _MSC_VER
#include <Windows.h>
double get_ms() { return (double)GetTickCount64() / 1000; }
#else
#include <sys/time.h>
double get_ms() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return (double)t.tv_sec * 1000 + (double)t.tv_usec / 1000;
}
#endif

typedef long long ll;
typedef unsigned long long ull;

typedef vector<int> vi;
typedef vector<double> vd;
typedef vector<string> vs;
typedef vector<unsigned long long> vull;

template <typename T> void print(T a, int n, const string &split = " ") {
  for (int i = 0; i < n; i++) {
    cout << a[i];
    if (i + 1 != n)
      cout << split;
  }
  cout << endl;
}
template <typename T>
void print2d(T a, int w, int h, int width = -1, int br = 0) {
  for (int i = 0; i < h; ++i) {
    for (int j = 0; j < w; ++j) {
      if (width != -1)
        cout.width(width);
      cout << a[i][j] << ' ';
    }
    cout << endl;
  }
  while (br--)
    cout << endl;
}
template <typename T> void input(T &a, int n) {
  for (int i = 0; i < n; ++i)
    cin >> a[i];
}
template <class T> string to_s(const T &a) {
  ostringstream os;
  os << a;
  return os.str();
}

unsigned long xor128(void) {
  static unsigned long x = 123456789, y = 362436069, z = 521288629,
                       w = 88675123;
  unsigned long t;
  t = (x ^ (x << 11));
  x = y;
  y = z;
  z = w;
  return (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)));
}

class Timer {
private:
  double start_time;
  double elapsed;

#ifdef USE_RDTSC
  double get_sec() { return get_absolute_sec(); }
#else
  double get_sec() { return get_ms() / 1000; }
#endif

public:
  Timer() {}

  void start() { start_time = get_sec(); }
  double get_elapsed() { return elapsed = get_sec() - start_time; }
  double get_elapsed_ms() { return get_elapsed() * 1000; }
};

class Clock {
  static double getTime() {
    double result;
    result = ((double)(clock()) / CLOCKS_PER_SEC);
    return result;
  }

public:
  double start;
  Clock() { start = getTime(); }
  double get() { return getTime() - start; }
  ll getms() { return static_cast<ll>(get() * 1000); }
  void reset() { start = getTime(); }
};
Clock *gclock;

enum class CMD_STATE { MOVE = 0, BOMB, INC, MAX, NONE };

enum class SIDE { MY = 0, ENEMY, NT, MAX };

enum class ENTITY_TYPE { FACTORY = 0, BOMB, TROOP, MAX };

Timer gTimer = Timer();
const int MAX_FACTORY_NUM = 15;
const int MAX_LINK_NUM = 105;
const int MAX_DISTANCE = 20;
const int INC_VALUE = 10;
const int NONE_CMD = 10; // NONE_CMD分の1でNONEになる
const int NONE_CMD_INDEX = 9999;
const int SIM_TURN = 20; //シミュレートするターン数z
const int TURN_CMDS = 8; // 1ターンに何コマンド発行するか
int FactoryCount, Turn;
bool initFlag = true;                           //最初のターンかどうか
int Distance[MAX_FACTORY_NUM][MAX_FACTORY_NUM]; //工場間の距離
int NearFactory[MAX_FACTORY_NUM][2]; //一番近い工場 index,(MY,ENEMY)
vs inputs;
bool P1;

struct Cmd {
  int id;
  SIDE side;
  CMD_STATE state;
  int from;
  int to;
  int cyborgNum;
  int hash;

  string toString() {
    string ret = "";

    if (state == CMD_STATE::MOVE) {
      ret = ";MOVE " + to_string(from) + " " + to_string(to) + " " +
            to_string(cyborgNum);
    } else if (state == CMD_STATE::BOMB) {
      ret = ";BOMB " + to_string(from) + " " + to_string(to);
    } else if (state == CMD_STATE::INC) {
      ret = ";INC " + to_string(from);
    } else if (state == CMD_STATE::NONE) {
      ret = "";
    } else {
      assert(false);
    }

    return ret;
  }
};

struct Factory {
  int id;
  SIDE side;
  int cyborgNum;
  int prod;
  int prodRemain; //爆弾を食らって再生産できるまでのターン数
  int batlleCyborg; //戦闘用の変数
};

struct Troop {
  SIDE side;
  int from;
  int to;
  int cyborgNum;
  int remain;

  bool operator<(const Troop &right) const { return remain < right.remain; }
};

struct Bomb {
  SIDE side;
  int from;
  int to;
  int remain;

  bool operator<(const Bomb &right) const { return remain < right.remain; }
};

struct Player {
  int RemainBombs;
};

struct EntityMoves {
  vector<Troop> troop;
  vector<Bomb> bomb;
  int troopIndex; //どこまで参照したか
  int bombIndex;
  int troopMax;
  int bombMax;

  void sortEnt() {
    sort(ALL(troop));
    sort(ALL(bomb));
  }

  void add(const Troop &tr) {
    troop.emplace_back(tr);
    troopMax++;
  }

  void add(const Bomb &bo) {
    bomb.emplace_back(bo);
    bombMax++;
  }

  void clear() {
    troop.clear();
    bomb.clear();
    resetIndex();
    troopMax = bombMax = 0;
  }

  void resetIndex() { troopIndex = bombIndex = 0; }
};

struct Node {
  Factory factory[MAX_FACTORY_NUM];
  Player players[2];

  int Turn;

  const Factory &getFactory(int i) {
    assert(i > -1 && i < FactoryCount);
    return factory[i];
  }
};

Node MainNode;
EntityMoves MainEnt;
vector<Cmd> Cmds;
vi SideCmds[2];

struct CmdSet {
  EntityMoves ent;
  vector<int> side_cmds[2];
  bool bomb_chk[MAX_FACTORY_NUM][MAX_FACTORY_NUM][2];
  int troop_chk[MAX_FACTORY_NUM][MAX_FACTORY_NUM][2];
  bool inc_chk[MAX_FACTORY_NUM][2];
  Node chkNode;
  int ZobristHashList[0xFFFF + 10];
  int ZobristHash[2];

  void add(const Troop &tr) { ent.add(tr); }

  void add(const Bomb &bo) { ent.add(bo); }

  void clear() {
    REP(i, MAX_FACTORY_NUM)
    REP(j, MAX_FACTORY_NUM) REP(k, 2) bomb_chk[i][j][k] = false;
    REP(i, MAX_FACTORY_NUM)
    REP(j, MAX_FACTORY_NUM) REP(k, 2) troop_chk[i][j][k] = 0;
    REP(i, MAX_FACTORY_NUM) REP(k, 2) inc_chk[i][k] = false;
    chkNode = MainNode;
    REP(i, 2) side_cmds[i].clear();
    fill(ZobristHashList, ZobristHashList + 0xFFFF + 10, 0);
  }

  bool addSideCmdIndex(int i, bool isNoHashCheck = false) {
    const Cmd &cmd = Cmds[i];

    if (cmd.state != CMD_STATE::NONE) {
      ZobristHash[(int)cmd.side] ^= cmd.hash;
      if (!isNoHashCheck &&
          ZobristHashList[ZobristHash[(int)cmd.side] >> 16] ==
              (ZobristHash[(int)cmd.side] & 0xFFFF)) {
        // hash衝突
        ZobristHash[(int)cmd.side] ^= cmd.hash;
        return false;
      }
    }

    if (cmd.state == CMD_STATE::MOVE) {
      if (bomb_chk[cmd.from][cmd.to][static_cast<int>(cmd.side)]) {
        ZobristHash[(int)cmd.side] ^= cmd.hash;
        ;
        return false;
      } //既に爆弾軌道上だった
      if (chkNode.getFactory(cmd.from).cyborgNum < cmd.cyborgNum) {
        ZobristHash[(int)cmd.side] ^= cmd.hash;
        ;
        return false;
      } //サイボーグの量がMaxを超えている

      chkNode.factory[cmd.from].cyborgNum -= cmd.cyborgNum;
      troop_chk[cmd.from][cmd.to][static_cast<int>(cmd.side)] += cmd.cyborgNum;
    } else if (cmd.state == CMD_STATE::BOMB) {
      if (troop_chk[cmd.from][cmd.to][static_cast<int>(cmd.side)]) {
        ZobristHash[(int)cmd.side] ^= cmd.hash;
        ;
        return false;
      } //既にTroop軌道上だった
      if (bomb_chk[cmd.from][cmd.to][static_cast<int>(cmd.side)]) {
        ZobristHash[(int)cmd.side] ^= cmd.hash;
        ;
        return false;
      } //既に爆弾軌道上だった
      if (chkNode.players[static_cast<int>(cmd.side)].RemainBombs <= 0) {
        ZobristHash[(int)cmd.side] ^= cmd.hash;
        ;
        return false;
      }

      chkNode.players[static_cast<int>(cmd.side)].RemainBombs--;
      bomb_chk[cmd.from][cmd.to][static_cast<int>(cmd.side)] = true;
    } else if (cmd.state == CMD_STATE::INC) {
      assert(cmd.from >= 0 && cmd.from < FactoryCount);
      assert(chkNode.factory[cmd.from].side == cmd.side);
      if (inc_chk[cmd.from][static_cast<int>(cmd.side)]) {
        ZobristHash[(int)cmd.side] ^= cmd.hash;
        ;
        return false;
      } //既にINC済み

      Factory &f = chkNode.factory[cmd.from];
      if (f.prod == 3) {
        ZobristHash[(int)cmd.side] ^= cmd.hash;
        ;
        return false;
      } //すでに最大まで成長している
      if (f.cyborgNum < INC_VALUE) {
        ZobristHash[(int)cmd.side] ^= cmd.hash;
        ;
        return false;
      } //成長できない

      f.cyborgNum -= 10;
      f.prod++;

      inc_chk[cmd.from][static_cast<int>(cmd.side)] = true;
    }

    ZobristHashList[ZobristHash[(int)cmd.side] >> 16] =
        (ZobristHash[(int)cmd.side] & 0xFFFF);
    side_cmds[static_cast<int>(cmd.side)].emplace_back(i);
    return true;
  }

  void pop_cmd(int i) {
    const Cmd &cmd = Cmds[i];

    assert(side_cmds[static_cast<int>(cmd.side)].back() == i);

    if (cmd.state == CMD_STATE::MOVE) {
      chkNode.factory[cmd.from].cyborgNum += cmd.cyborgNum;
      troop_chk[cmd.from][cmd.to][static_cast<int>(cmd.side)] -= cmd.cyborgNum;
    } else if (cmd.state == CMD_STATE::BOMB) {
      chkNode.players[static_cast<int>(cmd.side)].RemainBombs++;
      bomb_chk[cmd.from][cmd.to][static_cast<int>(cmd.side)] = false;
    } else if (cmd.state == CMD_STATE::INC) {
      if (!inc_chk[cmd.from][static_cast<int>(cmd.side)]) {
        int aa = 0;
      }
      assert(inc_chk[cmd.from][static_cast<int>(cmd.side)]);
      Factory &f = chkNode.factory[cmd.from];

      f.cyborgNum += 10;
      f.prod--;
      inc_chk[cmd.from][static_cast<int>(cmd.side)] = false;

      assert(f.prod >= 0);
    }

    if (cmd.state != CMD_STATE::NONE)
      ZobristHash[(int)cmd.side] ^= cmd.hash;
    side_cmds[static_cast<int>(cmd.side)].pop_back();
  }

  //セットされているすべてのコマンドを解除する
  void pop_all(SIDE side) {
    int size = side_cmds[static_cast<int>(side)].size();

    for (int i = size - 1; i >= 0; --i) {
      pop_cmd(side_cmds[static_cast<int>(side)][i]);
    }
  }

  void pop_one(SIDE side) { pop_cmd(side_cmds[static_cast<int>(side)].back()); }

  //基本swapのみ
  bool setRandom(SIDE side) {
    //追加候補のコマンド
    int CmdIndex = Cmds.back().id - ((side == SIDE::MY) ? 1 : 0);
    if (Cmds[CmdIndex].side != side) {
      int aa = 0;
    }
    assert(Cmds[CmdIndex].side == side);

    // 10%の確率でNONE
    if (xor128() % 10 == 0 || SideCmds[(int)side].empty()) {
      // cmd.state = CMD_STATE::NONE;
    } else {
      CmdIndex = SideCmds[(int)side][xor128() % SideCmds[(int)side].size()];
    }

    //必ず追加する
    if (side_cmds[static_cast<int>(side)].size() < TURN_CMDS) {
      return addSideCmdIndex(CmdIndex);
    }
    // swap
    else {
      int swapId = xor128() % side_cmds[static_cast<int>(side)].size();
      //最後の要素と交換
      swap(side_cmds[static_cast<int>(side)][swapId],
           side_cmds[static_cast<int>(side)]
                    [side_cmds[static_cast<int>(side)].size() - 1]);

      //最後の要素保存
      int CmdLast = side_cmds[static_cast<int>(side)].back();

      //最後の要素削除
      pop_one(side);

      //要素追加
      if (!addSideCmdIndex(CmdIndex)) {
        //失敗 最後の要素を入れ直す
        bool f = addSideCmdIndex(CmdLast, true);
        assert(f);
        return false;
      } else {
        return true;
      }
    }
  }

  // MYのコマンドを列挙する
  string cmdList() {
    string ret = "WAIT";

    for (int i : side_cmds[static_cast<int>(SIDE::MY)]) {
      if (Cmds[i].state == CMD_STATE::BOMB)
        MainNode.players[static_cast<int>(SIDE::MY)].RemainBombs--;

      assert(Cmds[i].side == SIDE::MY);
      ret += Cmds[i].toString();
    }

    return ret;
  }
};

//初期化
void init() {
  Cmds.reserve(10000);
  SideCmds[0].reserve(10000);
  SideCmds[1].reserve(10000);
  inputs.reserve(500);

  REP(i, 2)
  MainNode.players[i].RemainBombs = 2; //残りボムカウント
}

void calcNearFactory() {
  // init
  REP(f1, MAX_FACTORY_NUM) REP(i, 2) { NearFactory[f1][i] = -1; }

  // MY
  REP(f1, FactoryCount) {
    int mi = 10000;
    int index = -1;
    REP(f2, FactoryCount) if (MainNode.getFactory(f2).side == SIDE::MY) {
      if (mi > Distance[f1][f2]) {
        mi = Distance[f1][f2];
        index = f2;
      }
    }
    NearFactory[f1][(int)SIDE::MY] = index;
  }

  // ENEMY
  REP(f1, FactoryCount) {
    int mi = 10000;
    int index = -1;
    REP(f2, FactoryCount) if (MainNode.getFactory(f2).side == SIDE::ENEMY) {
      if (mi > Distance[f1][f2]) {
        mi = Distance[f1][f2];
        index = f2;
      }
    }
    NearFactory[f1][(int)SIDE::ENEMY] = index;
  }
}

//コマンド生成等
void createCmd() {
  Cmds.clear();
  SideCmds[0].clear();
  SideCmds[1].clear();

  map<int, bool> hash;

  // Bomb
  //最大で同時に2つまでしかできないので考える必要がある
  REP(side, 2)
  if (MainNode.players[side].RemainBombs > 0)
    REP(target, FactoryCount)
    if ((int)(MainNode.getFactory(target).side) == (side ^ 1)) {
      int NearFactoryIndex = NearFactory[target][side];
      if (NearFactoryIndex == -1)
        continue;
      if ((SIDE)side == SIDE::ENEMY)
        continue; //相手の爆弾は考えない

      Cmd cmd;
      cmd.state = CMD_STATE::BOMB;
      cmd.from = NearFactoryIndex;
      cmd.to = target;
      cmd.cyborgNum = 0;
      cmd.side = (SIDE)side;

      cmd.id = Cmds.size();
      Cmds.emplace_back(cmd);

      int h = xor128() % 0xFFFF;
      while (hash.count(h)) {
        h = xor128() % 0xFFFF;
      }
      cmd.hash = h;
      hash[cmd.hash] = true;

      SideCmds[(int)cmd.side].push_back(cmd.id);
    }

  // moveコマンド
  REP(f1, FactoryCount) {
    REP(f2, FactoryCount) {
      if (f1 == f2)
        continue;

      //送り元の数に比例
      const Factory &f1o = MainNode.getFactory(f1);
      if (f1o.side == SIDE::NT)
        continue; //中立なので移動できない
      if (Distance[f1][f2] > 6)
        continue; // test 6以上全部カット

      //初期値 n-3 以降 /2で減衰 3以上
      for (int num = f1o.cyborgNum - 3; num >= 0;
           num -= (num > 50) ? num / 2 : 1) {
        // if ((f1o.cyborgNum - 3) / 4 > num)continue;
        Cmd cmd;
        cmd.state = CMD_STATE::MOVE;
        cmd.from = f1;
        cmd.to = f2;
        cmd.cyborgNum = f1o.cyborgNum - num;
        cmd.side = f1o.side;
        cmd.id = Cmds.size();
        Cmds.emplace_back(cmd);

        int h = xor128() % 0xFFFF;
        while (hash.count(h)) {
          h = xor128() % 0xFFFF;
        }
        cmd.hash = h;
        hash[cmd.hash] = true;

        SideCmds[(int)cmd.side].push_back(cmd.id);
      }
    }
  }

  // Increase
  REP(target, FactoryCount) if (MainNode.getFactory(target).side != SIDE::NT) {
    if (MainNode.getFactory(target).cyborgNum < INC_VALUE)
      continue;
    if (MainNode.getFactory(target).prod == 3)
      continue;

    Cmd cmd;
    cmd.state = CMD_STATE::INC;
    cmd.from = target;
    cmd.to = target;
    cmd.cyborgNum = 0;
    cmd.side = MainNode.getFactory(target).side;

    cmd.id = Cmds.size();
    Cmds.emplace_back(cmd);

    int h = xor128() % 0xFFFF;
    while (hash.count(h)) {
      h = xor128() % 0xFFFF;
    }
    cmd.hash = h;
    hash[cmd.hash] = true;

    SideCmds[(int)cmd.side].push_back(cmd.id);
  }

  // NONE
  {
    REP(i, 2) {
      Cmd cmd;
      cmd.state = CMD_STATE::NONE;
      cmd.id = Cmds.size();
      cmd.side = (SIDE)i;
      Cmds.emplace_back(cmd);
    }
  }
}

void input() {
  Turn++;
  assert(Turn == MainNode.Turn + 1);
  MainNode.Turn = Turn;

  if (initFlag) {
    cin >> FactoryCount;
    cin.ignore();
    inputs.push_back(to_string(FactoryCount));

    int link;
    cin >> link;
    cin.ignore();
    inputs.push_back(to_string(link));

    REP(i, link) {
      int f1, f2, d;
      cin >> f1 >> f2 >> d;
      cin.ignore();
      Distance[f1][f2] = Distance[f2][f1] = d;
      inputs.push_back(to_string(f1) + " " + to_string(f2) + " " +
                       to_string(d));
    }

    initFlag = false;
  }

  MainEnt.clear();

  int ent;
  cin >> ent;
  cin.ignore();
  gclock->reset();
  for (string str : inputs) {
    iptcerr << str << endl;
  }
  iptcerr << ent << endl;

  REP(i, ent) {
    int entityId;
    string entityType;
    int arg1;
    int arg2;
    int arg3;
    int arg4;
    int arg5;
    cin >> entityId >> entityType >> arg1 >> arg2 >> arg3 >> arg4 >> arg5;
    cin.ignore();
    iptcerr << entityId << " " << entityType << " " << arg1 << " " << arg2
            << " " << arg3 << " " << arg4 << " " << arg5 << " " << endl;

    SIDE side = SIDE::NT;
    if (arg1 == 1) {
      side = SIDE::MY;
    } else if (arg1 == -1) {
      side = SIDE::ENEMY;
    }

    if (entityType == "FACTORY") {
      // entityId--; //0-indexd
      assert(entityId >= 0 && entityId < MAX_FACTORY_NUM);

      Factory &f = MainNode.factory[entityId];
      f.side = side;
      f.cyborgNum = arg2;
      f.prod = arg3;
      assert(arg3 >= 0 && arg3 <= 3);
      f.prodRemain = arg4;
    } else if (entityType == "TROOP") {
      Troop tr;
      tr.side = side;
      tr.from = arg2;
      tr.to = arg3;
      tr.cyborgNum = arg4;
      tr.remain = arg5;
      assert(tr.from >= 0);
      assert(tr.to >= 0);
      MainEnt.add(tr);
    } else {
      assert(entityType == "BOMB");
      if (side == SIDE::ENEMY)
        continue; //相手の爆弾は考えない
      Bomb b;
      b.side = side;
      b.from = arg2;
      b.to = arg3;
      b.remain = arg4;
      MainEnt.add(b);
    }
  }

  if (Turn == 1) {
    P1 = MainNode.factory[1].side == SIDE::MY;
  }

  MainEnt.sortEnt();
}

void initTurn() {
  if (initFlag) {
    init();
  }
  input();
  calcNearFactory();
  createCmd();
}

void initFactory(Node &node) {
  REP(i, FactoryCount) { node.factory[i].batlleCyborg = 0; }
}

void moveTroop(Node &node, EntityMoves &ent, CmdSet &cmdset) {
  // ent
  int dt = node.Turn - Turn;
  assert(dt > 0);
  assert(ent.troopMax == ent.troop.size());
  assert(ent.bombMax == ent.bomb.size());

  for (; ent.troopIndex < ent.troopMax; ++ent.troopIndex) {
    const Troop &tr = ent.troop[ent.troopIndex];
    assert(tr.remain >= dt);
    assert(tr.to >= 0 && tr.to < FactoryCount);
    if (tr.remain > dt)
      break;

    Factory &f = node.factory[tr.to];

    if (f.side != SIDE::NT) {
      f.batlleCyborg += (f.side == tr.side) ? tr.cyborgNum : -tr.cyborgNum;
    } else {
      f.batlleCyborg += (tr.side == SIDE::MY) ? tr.cyborgNum : -tr.cyborgNum;
    }
  }

  // cmdset
  for (; cmdset.ent.troopIndex < cmdset.ent.troopMax; ++cmdset.ent.troopIndex) {
    const Troop &tr = cmdset.ent.troop[cmdset.ent.troopIndex];
    assert(tr.remain >= dt);
    assert(tr.to >= 0 && tr.to < FactoryCount);
    if (tr.remain > dt)
      break;

    Factory &f = node.factory[tr.to];

    if (f.side != SIDE::NT) {
      f.batlleCyborg += (f.side == tr.side) ? tr.cyborgNum : -tr.cyborgNum;
    } else {
      f.batlleCyborg += (tr.side == SIDE::MY) ? tr.cyborgNum : -tr.cyborgNum;
    }
  }
}

void moveBomb(Node &node, EntityMoves &ent, CmdSet &cmdset) {
  // const int size = node.bomb.size();
  // for (int i = size - 1; i >= 0; --i) {
  //	Bomb& tr = node.bomb[i];
  //	assert(tr.remain > 0);
  //	tr.remain--;
  //}

  ////ent
  // int dt = node.Turn - Turn;
  // assert(dt > 0);

  // for (; ent.bombIndex < ent.bomb.size(); ++ent.bombIndex){
  //	const Bomb& tr = ent.bomb[ent.bombIndex];
  //	assert(tr.remain >= dt);
  //	assert(tr.to >= 0 && tr.to < FactoryCount);
  //	if (tr.remain > dt)break;

  //	Factory& f = node.factory[tr.to];

  //	if (f.side != SIDE::NT) {
  //		f.batlleCyborg += (f.side == tr.side) ? tr.cyborgNum :
  //-tr.cyborgNum;
  //	}
  //	else {
  //		f.batlleCyborg += (tr.side == SIDE::MY) ? tr.cyborgNum :
  //-tr.cyborgNum;
  //	}
  //}

  ////cmdset
  // for (cmdset.ent.troopIndex; cmdset.ent.troopIndex <
  // cmdset.ent.troop.size(); ++cmdset.ent.troopIndex){
  //	const Troop& tr = cmdset.ent.troop[cmdset.ent.troopIndex];
  //	assert(tr.remain >= dt);
  //	assert(tr.to >= 0 && tr.to < FactoryCount);
  //	if (tr.remain > dt)break;

  //	Factory& f = node.factory[tr.to];

  //	if (f.side != SIDE::NT) {
  //		f.batlleCyborg += (f.side == tr.side) ? tr.cyborgNum :
  //-tr.cyborgNum;
  //	}
  //	else {
  //		f.batlleCyborg += (tr.side == SIDE::MY) ? tr.cyborgNum :
  //-tr.cyborgNum;
  //	}
  //}
}

void executeCmds(Node &node, CmdSet &cmdset) {
  // 2ターン目以降は実行しない
  int dt = node.Turn - Turn;
  assert(dt > 0);
  if (dt >= 2)
    return;

  bool bomb_chk[MAX_FACTORY_NUM][MAX_FACTORY_NUM][2] =
      {}; //デバック用のチェック
  bool troop_chk[MAX_FACTORY_NUM][MAX_FACTORY_NUM][2] = {};

  REP(ss, 2)
  for (int i : cmdset.side_cmds[ss]) {
    const Cmd &cmd = Cmds[i];

    if (cmd.state == CMD_STATE::MOVE) {
      assert(cmd.from >= 0 && cmd.from < FactoryCount);
      assert(node.factory[cmd.from].cyborgNum >= cmd.cyborgNum &&
             cmd.cyborgNum >= 0);
      assert(node.factory[cmd.from].side == cmd.side);

      Troop tr;
      tr.side = cmd.side;
      tr.from = cmd.from;
      tr.to = cmd.to;
      tr.cyborgNum = cmd.cyborgNum;
      tr.remain = Distance[tr.from][tr.to] + 1;
      node.factory[cmd.from].cyborgNum -= tr.cyborgNum;
      assert(tr.from >= 0);
      assert(tr.to >= 0);
      cmdset.add(tr);

      assert(!bomb_chk[tr.from][tr.to][(int)tr.side]);
      troop_chk[tr.from][tr.to][(int)tr.side] = true;
    } else if (cmd.state == CMD_STATE::BOMB) {
      assert(cmd.from >= 0 && cmd.from < FactoryCount);
      assert(node.factory[cmd.from].side == cmd.side);

      Bomb b;
      b.side = cmd.side;
      b.from = cmd.from;
      b.to = cmd.to;
      b.remain = Distance[b.from][b.to] + 1;
      cmdset.add(b);

      node.players[static_cast<int>(b.side)].RemainBombs--;

      assert(node.players[static_cast<int>(b.side)].RemainBombs >= 0);
      assert(!troop_chk[b.from][b.to][(int)b.side]);
      bomb_chk[b.from][b.to][(int)b.side] = true;
    } else if (cmd.state == CMD_STATE::INC) {
      assert(cmd.from >= 0 && cmd.from < FactoryCount);
      assert(node.factory[cmd.from].side == cmd.side);

      Factory &f = node.factory[cmd.from];
      assert(f.prod >= 0 && f.prod < 3);

      f.cyborgNum -= 10;
      f.prod++;
    } else if (cmd.state == CMD_STATE::NONE) {
    } else {
      assert(false);
    }
  }

  cmdset.ent.sortEnt();
  assert(cmdset.ent.troopMax == cmdset.ent.troop.size());
  assert(cmdset.ent.bombMax == cmdset.ent.bomb.size());
}

void produceCyborg(Node &node) {
  REP(i, FactoryCount) {
    Factory &f = node.factory[i];
    if (f.side == SIDE::NT)
      continue;
    if (f.prodRemain > 0)
      continue;

    f.cyborgNum += f.prod;
    assert(f.cyborgNum >= 0);
    assert(f.prod >= 0 && f.prod <= 3);
  }
}

void solveBattles(Node &node) {
  REP(i, FactoryCount) {
    Factory &f = node.factory[i];
    if (f.batlleCyborg == 0)
      continue;
    assert(f.cyborgNum >= 0);

    if (f.side == SIDE::NT) {

      SIDE side = (f.batlleCyborg > 0) ? SIDE::MY : SIDE::ENEMY;
      f.cyborgNum -= abs(f.batlleCyborg);

      //倒せなかった
      if (f.cyborgNum >= 0)
        continue;

      f.side = side;
      f.cyborgNum *= -1;
    } else {
      f.cyborgNum += f.batlleCyborg;

      if (f.cyborgNum >= 0)
        continue;
      f.cyborgNum *= -1;
      f.side = (SIDE)((int)(f.side) ^ 1);
    }

    assert(f.cyborgNum >= 0);
  }
}

void bombExplode(Node &node, EntityMoves &ent, CmdSet &cmdset) {
  // ent
  int dt = node.Turn - Turn;
  assert(dt > 0);
  assert(ent.troopMax == ent.troop.size());
  assert(ent.bombMax == ent.bomb.size());

  for (; ent.bombIndex < ent.bombMax; ++ent.bombIndex) {
    const Bomb &tr = ent.bomb[ent.bombIndex];
    assert(tr.remain >= dt);
    assert(tr.to >= 0 && tr.to < FactoryCount);
    if (tr.remain > dt)
      break;

    Factory &f = node.factory[tr.to];

    if (f.cyborgNum / 2 > 10) {
      f.cyborgNum /= 2;
    } else {
      f.cyborgNum -= 10;
      f.cyborgNum = max(f.cyborgNum, 0);
    }

    f.prodRemain = 5;
  }

  // cmdset
  for (; cmdset.ent.bombIndex < cmdset.ent.bombMax; ++cmdset.ent.bombIndex) {
    const Bomb &tr = cmdset.ent.bomb[cmdset.ent.bombIndex];
    assert(tr.remain >= dt);
    assert(tr.to >= 0 && tr.to < FactoryCount);
    if (tr.remain > dt)
      break;

    Factory &f = node.factory[tr.to];

    if (f.cyborgNum / 2 > 10) {
      f.cyborgNum /= 2;
    } else {
      f.cyborgNum -= 10;
      f.cyborgNum = max(f.cyborgNum, 0);
    }

    f.prodRemain = 5;
  }
}

void updateFactory(Node &node) {
  REP(i, FactoryCount) {
    assert(node.factory[i].prodRemain >= 0);
    if (node.factory[i].prodRemain > 0)
      node.factory[i].prodRemain--;
  }
}

void simlate(Node &node, int T, EntityMoves &ent, CmdSet &cmdset) {
  ent.resetIndex();
  cmdset.ent.clear();

  REP(t, T) {
    node.Turn++;

    initFactory(node);
    moveTroop(node, ent, cmdset);
    // moveBomb(node, ent, cmdset);

    executeCmds(node, cmdset);

    produceCyborg(node);

    solveBattles(node);

    bombExplode(node, ent, cmdset);

    updateFactory(node);
  }
}

ll eval(const Node &node) {
  //全部のユニット数, prod数

  ll ret = 0;

  REP(i, FactoryCount) {
    if (node.factory[i].side == SIDE::NT)
      continue;
    ret += (node.factory[i].side == SIDE::MY) ? node.factory[i].cyborgNum
                                              : -node.factory[i].cyborgNum;
    ret += (node.factory[i].side == SIDE::MY) ? node.factory[i].prod * 50
                                              : -node.factory[i].prod * 50;

    int eindex = (P1) ? 2 : 1;
    int dist = 25 - Distance[eindex][i];
    dist /= 10;
    ret += (node.factory[i].side == SIDE::MY)
               ? node.factory[i].cyborgNum * dist
               : -node.factory[i].cyborgNum * dist;
  }

  return ret;
}

void createEnemyCmdSets(CmdSet &cmdset) {

  CmdSet maxCmds = cmdset;
  ll maxEval = 0;
  ll preEval = 0;
  {
    Node node = MainNode;
    simlate(node, SIM_TURN, MainEnt, cmdset);
    maxEval = preEval = eval(node) * -1; //敵側なので-1
  }

  REP(i, SideCmds[(int)SIDE::ENEMY].size()) {
    const int CmdIndex = SideCmds[(int)SIDE::ENEMY][i];
    const Cmd &cmd = Cmds[CmdIndex];
    assert(cmd.side == SIDE::ENEMY);
    if (!cmdset.addSideCmdIndex(CmdIndex))
      continue; //挿入できなかった

    Node node = MainNode;
    simlate(node, SIM_TURN, MainEnt, cmdset);
    ll e = eval(node) * -1; //敵側なので-1

    //最高値の保存
    if (maxEval < e) {
      maxEval = e;
      maxCmds = cmdset;
    }

    if (preEval < e) {
      int aaa = 0;
    }
    //スコアが落ちたので外す
    else {
      cmdset.pop_cmd(CmdIndex);
    }

    preEval = e;
  }

  cmdset = maxCmds;
}

void createMyCmdSets(CmdSet &cmdset) {

  CmdSet maxCmds = cmdset;
  ll maxEval = 0;
  ll preEval = 0;
  {
    Node node = MainNode;
    simlate(node, SIM_TURN, MainEnt, cmdset);
    maxEval = preEval = eval(node);
  }

  REP(i, SideCmds[(int)SIDE::MY].size()) {
    const int CmdIndex = SideCmds[(int)SIDE::MY][i];
    const Cmd &cmd = Cmds[CmdIndex];
    assert(cmd.side == SIDE::MY);
    if (!cmdset.addSideCmdIndex(CmdIndex))
      continue; //挿入できなかった

    Node node = MainNode;
    simlate(node, SIM_TURN, MainEnt, cmdset);
    ll e = eval(node);

    // BOMBにバイアスをかける
    if (MainNode.players[(int)SIDE::MY].RemainBombs == 1 &&
        cmd.state == CMD_STATE::BOMB) {
      e -= 20;
    }

    if (maxEval < e) {
      maxEval = e;
      maxCmds = cmdset;
    }

    if (preEval < e) {
      int aaa = 0;
    }
    //スコアが落ちたので外す
    else {
      cmdset.pop_cmd(CmdIndex);
    }

    preEval = e;
  }

  cmdset = maxCmds;
}

// SA
void createEnemyCmdSets2(CmdSet &cmdset) {

  CmdSet maxCmds = cmdset;
  ll maxEval = 0;
  ll preEval = 0;
  {
    Node node = MainNode;
    simlate(node, SIM_TURN, MainEnt, cmdset);
    maxEval = preEval = eval(node) * -1; //敵側なので-1
  }

  ll time = gclock->getms();
  ll end = time + 7;

  while (time < end) {
    if (!cmdset.setRandom(SIDE::ENEMY))
      continue;

    Node node = MainNode;
    simlate(node, SIM_TURN, MainEnt, cmdset);
    ll e = eval(node) * -1; //敵側なので-1

    //最高値の保存
    if (maxEval < e) {
      maxEval = e;
      maxCmds = cmdset;
    }

    time = gclock->getms();
  }

  cmdset = maxCmds;
}

void createMyCmdSets2(CmdSet &cmdset) {

  CmdSet maxCmds = cmdset;
  ll maxEval = 0;
  ll preEval = 0;
  {
    Node node = MainNode;
    simlate(node, SIM_TURN, MainEnt, cmdset);
    maxEval = preEval = eval(node); //敵側なので-1
  }

  ll time = gclock->getms();
  ll end = time + 7;

  while (time < end) {
    if (!cmdset.setRandom(SIDE::MY))
      continue;

    Node node = MainNode;
    simlate(node, SIM_TURN, MainEnt, cmdset);
    ll e = eval(node); //敵側なので-1

    //最高値の保存
    if (maxEval < e) {
      maxEval = e;
      maxCmds = cmdset;
    }

    time = gclock->getms();
  }

  cmdset = maxCmds;
}
