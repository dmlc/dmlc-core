#ifndef DMLC_DATA_CRC32_H_
#define DMLC_DATA_CRC32_H_
#include <stddef.h>
#include <stdint.h>
namespace dmlc {
namespace data {

/**
 * \brief A faster version of crc32 using sse4.2
 */
inline uint32_t CRC32HW(char *str, uint32_t len) {
  uint32_t q = len / sizeof(uint32_t),
           r = len % sizeof(uint32_t),
          *p = (uint32_t*) str, crc;

  crc = 0;
  while (q--) {
    __asm__ __volatile__(
        ".byte 0xf2, 0xf, 0x38, 0xf1, 0xf1;"
        :"=S" (crc)
        :"0" (crc), "c" (*p)
                         );
    p++;
  }

  str = (char*) p;
  while (r--) {
    __asm__ __volatile__(
        ".byte 0xf2, 0xf, 0x38, 0xf0, 0xf1"
        :"=S" (crc)
        :"0" (crc), "c" (*str)
                         );
    str++;
  }

  return crc;
}

}  // namespace data
}  // namespace dmlc
#endif /* DMLC_DATA_CRC32_H_ */
