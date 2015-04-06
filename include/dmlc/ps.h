/*!
 * @file   ps.h
 *
 * \brief  The parameter server interface
 */
#ifndef DMLC_PS_H_
#define DMLC_PS_H_
#if DMLC_USE_PS
#include "./base.h"
#include "./slice.h"
namespace dmlc {
namespace ps {

/*! \brief The default type of a key */
typedef uint64_t K;

/*!
 * \brief key-value cache for worker nodes
 *
 * @tparam V the type of value
 */
template<typename V>
class KVCache {
 public:
  /**
   * @param id the unique identity which is used to find the KVStore at the
   * parameter server. Negative IDs is preserved by system.
   */
  explicit KVCache(int id = 0);
  ~KVCache();

  /*! \brief Timestamp dependencies */
  typedef std::initializer_list<int> Deps;

  /*! \brief Callback function */
  typedef std::function<void()> Call;

  /*!
   * \brief Pushes a list of key-value pairs into the parameter server
   *
   * It's a non-blocking call, which returns immediately once the message is
   * queued in the system's sending buffer. The actual push is finished only
   * after Wait(returned_timestamp) returns or the provided callback is called.
   *
   * Sample usage: assume we have two key-value pairs {1, (1.1, 1.2)}, {3,
   * (3.1,3.2)}, where the value is a 2-length float vector. We then can push these
   * two pairs into the parameter server:
   \code
     KVCache<float> cache(0);
     std::vector<K> keys = {1, 3};
     std::vector<float> vals = {1.1, 1.2, 3.1, 3.2};
     cache.Push(keys, vals);
   \endcode
   *
   * @param keys a list of keys
   * @param values a list of values, whose size should be an integer multiple
   * the key size
   * @param deps the timestamp of the depended requests. This request will be
   * processed by the parameter servers only after the depended requests have
   * been processed.
   * @param callback the function will be executed after received the
   * finish ack from the parameter server
   *
   * @return the timestamp of this request.
   */
  int Push(const std::vector<K>& keys, const std::vector<V>& values,
           const Deps& deps = {}, const Call& callback = Call()) {
    return Push(Slice<K>(keys), Slice<K>(values), deps, callback);
  }

  /*!
   * \brief Pulls the values associated with the keys from the parameter server
   *
   * It's a non-blocking call, which returns immediately once the message is
   * queued in the system's sending buffer. The actual push is finished only
   * after Wait(returned_timestamp) returns or the provided callback is called.
   *
   * @param keys a list of keys
   * @param values the buffer for the pulled values, which should be pre-allocated
   * @param deps the timestamp of the depended requests. This request will be
   * processed by the parameter servers only after the depended requests have
   * been processed.
   * @param callback the function will be executed after the pulled values are
   * ready
   *
   * Sample usage: again assume each key is associated with a 2-length float
   * vector value. We then can pull the newest value from the parameter server:
   \code
     KVCache<float> cache(0);
     std::vector<K> keys = {1, 3};
     std::vector<float> vals(4);
     cache.Pull(keys, &vals);
   \endcode
   * @return the timestamp of this request
   */
  int Pull(const std::vector<K>& keys, std::vector<V>* values,
           const Deps& deps = {}, const Call& callback = Call()) {
    return Pull(Slice<K>(keys), values.data(), values.size(),
                deps, callback);
  }

  /*!
   * \brief Waits until a request has been finished
   *
   * Sample usage:
   \code
     int ts = cache.Pull(keys, &vals);
     Wait(ts);
     // now vals is ready for use
   \endcode
   */
  void Wait(int timestamp);

  /*! \brief another style Push and Pull */

  int Push(const Slice<K>& keys, const Slice<V>& values,
           const Deps& deps = {}, const Call& callback = Call()) {
    return Push_(keys, values, false, deps, callback);
  }

  int Pull(const Slice<K>& keys, V* val_data, size_t val_size,
           const Deps& deps = {}, const Call& callback = Call()) {
    return Pull_(keys, val_data, val_size, false, deps, callback);
  }

  /*!
   * \brief Zero-copy Push and Pull.
   *
   * Similar to the C-array style Push and Pull, but the data in key_ptr (also
   * val_ptr in Push) will not be copied to reduce the communication delay.
   * Therefore, it is the user's responsibility to keep the content of key_ptr
   * (and val_ptr) unchanged until the request is finished, namely Wait(ts)
   * returns or the callback is called.
   */
  int Push(const Slice<K>& keys, const Slice<V>& values,
           const Deps& deps = {}, const Call& callback = Call()) {
    return Push_(keys, values, true, deps, callback);
  }

  int Pull(const Slice<K>& keys, V* val_data, size_t val_size,
           const Deps& deps = {}, const Call& callback = Call()) {
    return Pull_(keys, val_data, val_size, true, deps, callback);
  }

  /*! \brief advanced APIs */

  /*!
   * \brief Increases the clock by delta
   */
  void IncrClock(int delta = 1);

  /*! \brief Send a push message */
  int Push(Message* msg);

  /*! \brief Send a pull message */
  int Pull(Message* msg);

 private:

  int Push_(const Slice<K>& keys, const Slice<V>& values,
            bool zero_copy, const Deps& deps, const Call& callback) {
    // TODO
    Message msg; return Push(&msg);
  }

  int Pull(const Slice<K>& keys, V* val_data, size_t val_size,
           bool zero_copy, const Deps& deps, const Call& callback) {
    // TODO
    Message msg; return Push(&msg);
  }
};


typedef -1 DYNAMIC_LEN;

/*!
 * \brief key-value store for server nodes
 *
 * @tparam V the value type
 * @tparam val_len the length of a value = val_len * sizeof(V), which is also
 * could be a dynamic length DYNAMIC_LEN, such as neural network
 */
template <typename V, int val_len = 1, >
class KVStore {
 public:

  /**
   * \brief Process key-value pairs in online or batch style
   *
   * - ONLINE: individual key-value pairs received from workers are feed into
   *   user-defined writer/reader one by one.
   *
   * - BATCH: all key-value pairs received from a worker in a Push/Pull request
   *   are feed into writer/reader togeter
   *
   * Implementation & Performance
   *
   * - ONLINE: use unordered_map or other equivalence data structure to store KV
   *   pairs. It is suitable when new keys appears during running, such as
   *   SGD/online learning algorithms. However, both read and write could be 5x
   *   slower comparing to BATCH
   *
   * - BATCH: use array to store KV pairs. Suitable for the keys set is fixed at
   *   the beginning, such as batch algorithm. Both read and write are fast, but
   *
   */
  enum Type { ONLINE, BATCH };

  /**
   * @param type which affects how key-value pairs are feed into updater and
   *  initializer, see comments below
   * @param id the unique identity. Negative IDs is preserved by system.
   */
  KVStore(int id = 0, Type type = ONLINE);
  ~KVStore();

  /**
   * \brief User-defined function
   *
   * @param key pointer to the received keys
   * @param key_size number of received keys, which is 1 if the type is
   * ONLINE, or the number of keys received from a worker in a push or pull
   * request
   * @param src_val pointer to the source value buffer
   * @param dst_val pointer to the destination value buffer
   */
  typedef std::function<void(const K* key, size_t key_size,
                             const V* src_val, V* dst_val)> UDF;

  /**
   * \brief Handle PULL requests from worker nodes
   *
   * - src_val: value stored at the parameter server, whose size is key_size *
   *   val_len * sizeof(V)
   *
   * - dst_val: value received from a worker node, whose size is key_size *
   *   recv_val_len * sizeof(V)
   *
   * @param udf
   * @param recv_val_len the value length of received KV pairs
   */
  void SetReader(const UDF& udf, int recv_val_len);

  /**
   * \brief Handle PUSH requests from worker nodes
   *
   * - src_val: value received from a worker node, whose size is key_size *
   *   recv_val_len * sizeof(V)
   *
   * - dst_val: value stored at the parameter server, whose size is key_size *
   *   val_len * sizeof(V)
   *
   * @param udf
   * @param recv_val_len the value length of received KV pairs
   */
  void SetWriter(const UDF& udf, int recv_val_len);

  /**
   * \brief Will be called when the first time a Push or a Pull request is
   * received on keys
   *
   * - src_val: NULL
   *
   * - dst_val: value stored at the parameter server, whose size is key_size *
   *   val_len * sizeof(V)
   *
   * @param udf
   * @param recv_val_len the value length of received KV pairs
   */
  void SetIntializer(const UDF& udf, int recv_val_len);

  /**
   * \brief Must be called after all set functions are done
   */
  void Init();
};


/// functions to query my node information ///

/*! \brief Return true if this node is a worker node. */
bool IsWorkerNode();

/*! \brief Return true if this node is a server node. */
bool IsWorkerNode();

/*! \brief Return true if this node is a scheduler node. */
bool IsSchedulerNode();

/*! \brief The global unique string ID of this node */
std::string MyNodeID()


}  // namespace ps
}  // namespace dmlc

#endif  // DMLC_USE_PS
#endif  /* DMLC_PS_H_ */
