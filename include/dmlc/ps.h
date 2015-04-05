/*!
 * @file   ps.h
 *
 * @brief  The parameter server interface
 */
#ifndef DMLC_PS_H_
#define DMLC_PS_H_
#if DMLC_USE_PS
#include "./base.h"
namespace dmlc {
namespace ps {

/*! @brief The default type of a key */
typedef uint64_t K;

/*!
 * @brief Parameter cache on worker nodes
 */
class KVCache {
 public:
  KVCache();
  ~KVCache();

  /*! @brief Timestamp dependencies */
  typedef std::initializer_list<int> Deps;

  /*! @brief Callback function */
  typedef std::function<void()> Call;

  /*!
   * @brief Pushes a list of key-value pairs into the parameter server
   *
   * It's a non-blocking call, which returns immediately once the message is
   * queued in the system's sending buffer. The actual push is finished only
   * after Wait(returned_timestamp) returns or the provided callback is called.
   *
   * Sample usage: assume we have two key-value pairs {1, (1.1, 1.2)}, {3,
   * (3.1,3.2)}, where the value is a 2-length float vector. We then can push these
   * two pairs into the parameter server:
   \code
     KVCache<float> cache;
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
  template<typename V>
  int Push(const std::vector<K>& keys,
           const std::vector<V>& values,
           const Deps& deps = {},
           const Call& callback = Call()) {
    return Push(
        keys.data(), keys.size(), values.data(), values.size(), deps, callback);
  }

  /*!
   * @brief Pulls the values associated with the keys from the parameter server
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
     KVCache<float> cache;
     std::vector<K> keys = {1, 3};
     std::vector<float> vals(4);
     cache.Pull(keys, &vals);
   \endcode
   * @return the timestamp of this request
   */
  template<typename V>
  int Pull(const std::vector<K>& keys,
           std::vector<V>* values,
           const Deps& deps = {},
           const Call& callback = Call()) {
    return Pull(
        keys.data(), keys.size(), values.data(), values.size(), deps, callback);
  }


  /*!
   * @brief Waits until a request has been finished
   *
   * Sample usage:
   \code
     int ts = cache.Pull(keys, &vals);
     Wait(ts);
     // now vals is ready for use
   \endcode
   */
  void Wait(int timestamp);

  /*! @brief C-array style Push and Pull */

  template<typename V>
  int Push(const K* key_ptr, size_t key_size,
           const V* val_ptr, size_t val_size,
           const Deps& deps = {},
           const Call& callback = Call()) {
    return Push_(key_ptr, key_size, val_ptr, val_size, false, deps, callback);
  }

  template<typename V>
  int Pull(const K* key_ptr, size_t key_size,
           V* val_ptr, size_t val_size,
           const Deps& deps = {},
           const Call& callback = Call()) {
    return Pull_(key_ptr, key_size, val_ptr, val_size, false, deps, callback);
  }

  /*!
   * @brief Zero-copy Push and Pull.
   *
   * Similar to the C-array style Push and Pull, but the data in key_ptr (also
   * val_ptr in Push) will not be copied to reduce the communication delay.
   * Therefore, it is the user's responsibility to keep the content of key_ptr
   * (and val_ptr) unchanged until the request is finished, namely Wait(ts)
   * returns or the callback is called.
   */
  template<typename V>
  int ZPush(const K* key_ptr, size_t key_size,
            const V* val_ptr, size_t val_size,
            const Deps& deps = {},
            const Call& callback = Call()) {
    return Push_(key_ptr, key_size, val_ptr, val_size, true, deps, callback);
  }

  template<typename V>
  int ZPull(const K* key_ptr, size_t key_size,
            V* val_ptr, size_t val_size,
            const Deps& deps = {},
            const Call& callback = Call()) {
    return Pull_(key_ptr, key_size, val_ptr, val_size, true, deps, callback);
  }

  /*! @brief advanced APIs */

  /*!
   * @brief Increases the clock by delta
   */
  void IncrClock(int delta = 1);

  /*! @brief Send a push message */
  int Push(Message* msg);

  /*! @brief Send a pull message */
  int Pull(Message* msg);
 private:

  template<typename V>
  int Push_(const K* key_ptr, size_t key_size, const V* val_ptr, size_t val_size,
            bool zero_copy, const Deps& deps, const Call& callback) {
    // TODO
    Message msg; return Push(&msg);
  }

  template<typename V>
  int Pull_(const K* key_ptr, size_t key_size, V* val_ptr, size_t val_size,
            bool zero_copy, const Deps& deps, const Call& callback) {
    // TODO
    Message msg; return Push(&msg);
  }
};














/// advanced functions ///

/*! @brief Return true if this node is a worker node. */
bool IsWorkerNode();

/*! @brief Return true if this node is a server node. */
bool IsWorkerNode();

/*! @brief Return true if this node is a scheduler node. */
bool IsSchedulerNode();

/*! @brief The global unique string ID of this node */
std::string MyNodeID()


}  // namespace ps
}  // namespace dmlc

#endif  // DMLC_USE_PS
#endif  /* DMLC_PS_H_ */
