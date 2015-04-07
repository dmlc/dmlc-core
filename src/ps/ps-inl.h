/**
 * @file   ps-inl.h
 * @brief  Implementation of ps.h
 */
#ifndef DMLC_PS_INL_H_
#define DMLC_PS_INL_H_
#include "dmlc/ps.h"
namespace dmlc {
namespace ps {

/// worker nodes

template<typename V>
KVCache<V>::KVCache(int id) {

}

template<typename V>
KVCache<V>::~KVCache() {

}

template<typename V>
void KVCache<V>::Wait(int timestamp) {

}


template<typename V>
int KVCache<V>::Push(CBlob<K> keys, CBlob<V> values, const SyncOpts& opts) {
  return 0;
}

template<typename V>
int KVCache<V>::Pull(CBlob<K> keys, Blob<V> values, const SyncOpts& opts) {

  return 0;
}


template<typename V>
int KVCache<V>::Push(const SBlob<K>& keys, const SBlob<V>& values,
                     const SyncOpts& opts) {

  return 0;
}

template<typename V>
int KVCache<V>::Pull(const SBlob<K>& keys, SBlob<V>* values,
                     const SyncOpts& opts) {

  return 0;
}


template<typename V>
void KVCache<V>::IncrClock(int delta) {

}

/// server nodes

}  // namespace ps
}  // namespace dmlc
#endif /* DMLC_PS_INL_H_ */
