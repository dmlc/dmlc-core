#include <dmlc/ps.h>

typedef float V;

int CreateServerNode(int argc, char *argv[]) {
  dmlc::ps::KVStore<V> server; server.Run();
  return 0;
}

int WorkerNodeMain(int argc, char *argv[]) {
  using namespace dmlc;
  using namespace dmlc::ps;

  KVCache<V> worker;

  /// push to server
  std::vector<K> key_1 = {1, 3, 5};
  std::vector<V> val_1 = {1, 1, 1};
  int ts = worker.Push(key_1, val_1);

  /// pull from server, but let the pull executed after push is done at the
  /// server
  SyncOpts pull_opts; pull_opts.deps = {ts};
  std::vector<V> recv_val_1(val_1.size());
  ts = worker.Pull(key_1, &recv_val_1, pull_opts);
  worker.Wait(ts);

  LOG(ERROR) << CBlob<V>(recv_val_1).ShortDebugString();

  /// zero-copy push & pull
  SBlob<K> key_2 = {2, 4, 5};
  SBlob<V> val_2 = {1, 1, 1};
  SBlob<V> recv_val_2(val_2.size());

  ts = worker.Push(key_2, val_2);

  pull_opts.deps = {ts};
  worker.Wait(worker.Pull(key_2, &recv_val_2, pull_opts));

  LOG(ERROR) << recv_val_2.ShortDebugString();

  return 0;
}

// TODO put it somewhere...
int main(int argc, char *argv[]) {
  CreateServerNode(argc, argv);
  WorkerNodeMain(argc, argv);
  return 0;
}
