// implementations in ctypes
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include <cstring>
#include <string>
#include <dmlc/data.h>
#include <dmlc/io.h>
#include <dmlc/logging.h>
#include "dmlc_wrapper.h"
using namespace dmlc;




extern "C" {

  DMLC_DLL void DMLC_CSR_MapReduce_SUM(
         const size_t length,
         const size_t* offset,
         const index_t* index,
         const real_t* value,
         const index_t* reduce_id,
         real_t* output,
         const index_t n_row,
         const index_t n_col
         )
  {

    for (size_t row = 0; row < length; ++row) {
      size_t s = offset[row], e = offset[row + 1];
      index_t oidx = reduce_id[row];
      assert(oidx >= 0);
      assert(oidx < n_row);
      size_t out_offset = oidx * n_col;
      for (size_t j = s; j < e ; ++j) {
        output[out_offset + index[j]] += value[j];
      }
    }

  }



  DMLC_DLL void* DMLC_RowBlockIter_CreateFromUri(
         const char *uri,
         int part_index,
         int num_parts,
         const char *type)
  {
    return RowBlockIter<index_t>::Create(uri,
       part_index,
       num_parts,
       type);
  }
  DMLC_DLL void  DMLC_RowBlockIter_Init(RowBlockIter<index_t>* data) {

  }
  DMLC_DLL void  DMLC_RowBlockIter_Deallocate(RowBlockIter<index_t>* data) {
    delete data;
  }
  DMLC_DLL int  DMLC_RowBlockIter_Next(RowBlockIter<index_t>* data) {
    bool has_next = data->Next();
    //printf("has_next = %d", has_next);
    return has_next;
  }
  DMLC_DLL void DMLC_RowBlockIter_Beforefirst(RowBlockIter<index_t>* data) {
    data->BeforeFirst();
  }
  DMLC_DLL index_t DMLC_RowBlockIter_NumCol(RowBlockIter<index_t>* data) {
    return data->NumCol();
  }
  DMLC_DLL void  DMLC_RowBlockIter_Value(

    RowBlockIter<index_t>* data, 
    size_t *size, 
    const size_t **offset,
    const real_t **label,
    const index_t **index,
    const real_t **value) 
  {
    //printf(" sizeof size_t = %d", sizeof(size_t) );
    const RowBlock<index_t>& batch = data->Value();
    *size = batch.size;
    *offset = batch.offset;
    *label = batch.label;
    *index = batch.index;
    *value = batch.value;
  }
  DMLC_DLL void TEST_write_numpy(real_t* p, size_t len) {
    for (int i=0;i<len;i++)
      p[i] =i;
  }

};

//}  // C
