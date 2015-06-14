"""
Python interface for dmlc-core/BlockData_iter
Author: Wentao TIAN
"""
import cPickle as pickle
import ctypes
from ctypes import byref
from ctypes import POINTER
import os
import sys
import warnings
import numpy as np
from scipy import sparse
if os.name == 'nt':
    WRAPPER_PATH = os.path.dirname(__file__) + '\\..\\windows\\x64\\Release\\rabit_wrapper%s.dll'
else:
    WRAPPER_PATH = os.path.dirname(__file__) + '/libdmlc_wrapper%s.so'
dmlclib = None

# load in xgboost library
def loadlib__(lib = 'standard'):    
    global dmlclib
    if dmlclib != None:
        warnings.Warn('rabit.int call was ignored because it has already been initialized', level = 2)
        return
    if lib == 'standard':
        dmlclib = ctypes.cdll.LoadLibrary(WRAPPER_PATH % '')

    #dmlclib.RabitGetRank.restype = ctypes.c_int
    #dmlclib.RabitGetWorldSize.restype = ctypes.c_int
    #dmlclib.RabitVersionNumber.restype = ctypes.c_int
    dmlclib.DMLC_RowBlockIter_Next.restype = ctypes.c_int
    dmlclib.DMLC_RowBlockIter_NumCol.restype = ctypes.c_uint32
    #dmlclib.TEST_write_numpy.argtypes = [np.ctypeslib.ndpointer(dtype=np.float32), np.uint64]
    #dmlclib.DMLC_RowBlockIter_CreateFromUri.argtypes = [
    #    ctypes.c_char_p, ctypes.c_int, ctypes.c_int, ctypes.c_char_p]
def unloadlib__():
    global dmlclib
    del dmlclib
    dmlclib = None

# reduction operators
MAX = 0
MIN = 1
SUM = 2
BITOR = 3

def check_err__():    
    """
    reserved function used to check error    
    """
    return
def vec_from_cptr(ptr):
    return np.ctypeslib.as_array(ptr)

class Row(object):
    def __init__(me):
        pass
    def __init__(me, label, index, value):
        me.label = label
        me.index = index
        me.value = value

class RowBlock(object):
    def __init__(me):
        me.p_offset = None
        me.p_label = None
    def GetRow(id):
        pass

class RowBlockIter(object):
    def __init__(me):
        me.data_ptr = None
    def CreateFromUri(me, uri, part_index, num_parts, format):
        #print 'get = ', dmlclib.DMLC_test(ctypes.c_char_p(uri))

        me.data_ptr = dmlclib.DMLC_RowBlockIter_CreateFromUri(
            ctypes.c_char_p(uri), 
            ctypes.c_int(part_index), 
            ctypes.c_int(num_parts), 
            ctypes.c_char_p(format))

    def BeforeFirst(me):
        dmlclib.DMLC_RowBlockIter_Beforefirst(me.data_ptr);
        me.has_next = False
        me.has_value = False
        me.spmat = None
    def Next(me):
        me.has_next =  True if dmlclib.DMLC_RowBlockIter_Next(me.data_ptr) > 0 else False
        #print 'me.has_next = ', me.has_next
        me.has_value = False
        me.spmat = None

        me.label = None
        me.offset = None
        me.index = None
        me.value = None

        return me.has_next
    def Value(me):
        if me.has_next == 0:
            return 
        me._size = ctypes.c_uint64()
        me._label = POINTER(ctypes.c_float)()
        me._offset = POINTER(ctypes.c_uint64)()
        me._index = POINTER(ctypes.c_uint32)()
        me._value = POINTER(ctypes.c_float)()
        dmlclib.DMLC_RowBlockIter_Value(me.data_ptr, 
            byref(me._size),  byref(me._offset), byref(me._label), 
            byref(me._index), byref(me._value))
        me.length = me._size.value;
        me.label = np.ctypeslib.as_array(me._label, shape = (me.length ,))
        me.offset = np.ctypeslib.as_array(me._offset, shape = (me.length + 1,))
        me.n_elem = me.offset[-1]
        me.index = np.ctypeslib.as_array(me._index, shape = (me.n_elem ,))
        me.value = np.ctypeslib.as_array(me._value, shape = (me.n_elem ,))
        #print me.label
        #print me.offset
        #print me.index
        #print me.value
        me.has_spmat = False
        me.has_value = True
    def NumCol(me):
         return dmlclib.DMLC_RowBlockIter_NumCol(me.data_ptr)
    def setNumFeat(me, f):
        me.nbrfeat = f
    def ValueCSR(me):
        if not me.has_value:
            me.Value()
        if me.has_value and me.spmat == None:
            me.spmat = sparse.csr_matrix((me.value, me.index, me.offset))
        return me.spmat

    def getRow(me, idx): 
        offset_s = me.offset[idx]
        offset_e = me.offset[idx + 1]
        return sparse.csr_matrix(
            (me.value[offset_s:offset_e], me.index[offset_s:offset_e], 
                np.array([0, offset_e - offset_s])), shape=(1, me.nbrfeat))

    def CSRReduceSum(me, cluster, output):
        reduce_id = cluster.astype(np.uint32).ravel()
        reduce_id_ptr = cluster.ctypes.data_as(POINTER(ctypes.c_uint32))
        output_ptr = output.ctypes.data_as(POINTER(ctypes.c_float))

        dmlclib.DMLC_CSR_MapReduce_SUM(me._size, me._offset, me._index, me._value,
            reduce_id_ptr  , output_ptr, 
            output.shape[0], output.shape[1])

def sparse_matrix_test():
    data = np.array([1,2,3])
    col = np.array([0,1,2])
    rstart = np.array([0,1,1,3])
    spmat = sparse.csr_matrix((data, col, rstart))
    print spmat.toarray() 
    data[0] = 454545
    print spmat.toarray()
def nparray_test():
    a = np.array([1,2,3,4,5])
    b = a[1:2]
    b[0] = 1000
    print a
def load_data():
    data = RowBlockIter();
    data.CreateFromUri(sys.argv[1], 0, 1, 'libsvm')
    data.BeforeFirst()
    data.Next()

    data.ValueCSR()
    print data.spmat, '\n'
    data.Next()
    data.ValueCSR()
    print data.spmat, '\n'
    data.Next()
    data.ValueCSR()
    print data.spmat, '\n'
def main():
    #sparse_matrix_test()
    #nparray_test()
    load_data()

loadlib__()

if __name__ == "__main__":
    main()