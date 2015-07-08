/*!
 *  Copyright (c) 2015 by Contributors
 * \file uri_spec.h
 * \brief common specification of sugars in URI
 *    string passed to dmlc Create functions
 *    such as local file cache
 * \author Tianqi Chen
 */
#ifndef DMLC_IO_URI_SPEC_H_
#define DMLC_IO_URI_SPEC_H_
#include <cstring>
#include <string>
#include <sstream>
#include "./filesys.h"
namespace dmlc {
namespace io {
/*!
 * \brief some super set of URI
 * that allows sugars to be passed around
 */
struct URISpec {
  /*! \brief the real URI */
  std::string uri;
  /*! \brief the path to cache file */
  std::string cache_file;
  explicit URISpec(const char *uri,
                   unsigned part_index,
                   unsigned num_parts) {
    const char *dlm = strchr(uri, '#');
    if (dlm != NULL) {
      CHECK(strchr(dlm + 1, '#') == NULL)
          << "only one `#` is allowed in file path for cachefile specification";
      this->uri = std::string(uri, dlm - uri);
      std::ostringstream os;
      os << dlm + 1;
      if (num_parts != 1) {
        os << ".split" << num_parts << ".part" << part_index;
      }
      cache_file = os.str();
    } else {
      this->uri = uri;
    }
  }
};
}  // namespace io
}  // namespace dmlc
#endif  // DMLC_IO_URI_SPEC_H_
