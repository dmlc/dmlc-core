/**
 * @file   ps.h
 *
 * @brief  The parameter server interface
 */
#ifndef DMLC_PS_H_
#define DMLC_PS_H_
#include "./base.h"

namespace dmlc {
namespace ps {

/**
 * @brief Return true if this node is a worker node.
 */
bool IsWorkerNode();

/**
 * @brief Return true if this node is a server node.
 */
bool IsWorkerNode();

/**
 * @brief Return true if this node is a scheduler node.
 */
bool IsSchedulerNode();

/**
 * @brief The global unique string ID of this node
 */
std::string MyNodeID()


/**
 * @brief The default type of a key
 */
typedef uint64_t K;

/**
 * @brief Communication message
 */
struct Message;

/**
 * @brief Worker parameters
 */
class WorkerParam {
 public:
  WorkerParam();
  ~WorkerParam();

  /**
   * @brief Time dependencies
   */
  typedef std::initializer_list<int> TimeDeps;

  /**
   * @brief Pushes a list of key-value pairs into the servers
   *
   * @param keys a list of keys
   * @param values a list of values, whose size should be an integer multiple
   * the key size
   * @param time_deps time dependencies of this request
   *
   * @return the timestamp of this request
   */
  template<typename V>
  int Push(const std::vector<K>& keys, const std::vector<V>& values,
           const TimeDeps& time_deps = {});

  /**
   * @brief Callback function
   */
  typedef std::function<void()> Callback;

  /**
   * @brief Pulls the values accosiated with the keys from the servers
   *
   * @param keys a list of keys
   * @param values the pulled values, whose space should be pre-allocated
   * @param time_deps time dependencies of this request
   * @param callback the function will be called by another system thread when
   * the values have been pulled
   *
   * @return the timestamp of this request
   */
  template<typename V>
  int Pull(const std::vector<K>& keys, std::vector<V>* values,
           const TimeDeps& time_deps = {},
           const Callback& callback = *Callback());

  /**
   * @brief Waits until a request has been finished
   */
  void Wait(int timestamp);

  /**
   * @brief Increases the clock by delta
   */
  void IncrClock(int delta = 1);

  ///// zero-copy Push/Pull /////

  /**
   * @brief Push a list of key-value pairs into the servers
   *
   * @param key_ptr pointer of the key list
   * @param key_size the number of keys
   * @param value_ptr pointer of the value list
   * @param value_size the number of value, which should be an integer multiple
   * of key_size
   * @param zero_copy if true, it's the user's responsibility to keep the
   * content of key_ptr and value_ptr not changed until this request is
   * finished, such as Wait() returns
   *
   * @return the timestamp of this push request
   */
  template<typename V>
  int Push(const K* key_ptr, size_t key_size,
           const V* value_ptr, size_t *value_size,
           bool zero_copy = false, const TimeDeps& time_deps = {});


  template<typename V>
  int Pull(const K* key_ptr, size_t key_size,
           V* value_ptr, size_t *value_size,
           bool zero_copy = false, const TimeDeps& time_deps = {},
           const Callback& callback = *Callback());

  ///// fully advanced Push/Pull ////

  /**
   * @brief Send a request push message to msg->recver
   */
  int Push(Message* msg);

  /**
   * @brief Send a request pull message to msg->recver
   */
  int Pull(Message* msg);

};

}  // namespace ps
}  // namespace dmlc

#endif  /* DMLC_PS_H_ */
