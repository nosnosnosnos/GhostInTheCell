#ifdef LOCAL
#include "header.h"
#endif

int main() {
  gclock = new Clock();
  CmdSet cmdset = {};

  while (1145148101919L) {
    initTurn();
    cmdset.clear();

    createEnemyCmdSets2(cmdset); //����̃R�}���h�Z�b�g
    createMyCmdSets2(cmdset);    //�����̃R�}���h�Z�b�g
    createEnemyCmdSets2(cmdset);
    createMyCmdSets2(cmdset);
    createEnemyCmdSets2(cmdset);
    createMyCmdSets2(cmdset);

    cerr << gclock->getms() << endl;
    cout << "MSG " << gclock->get() << ";";

    cout << cmdset.cmdList() << endl;
  }

  return 0;
}
