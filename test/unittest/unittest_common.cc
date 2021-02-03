#include <gtest/gtest.h>
#include <dmlc/common.h>

TEST(Common, omp_rethrow_std_exception) {
  try {
    OMP_INIT();
    #pragma omp parallel num_threads(2)
    {
      OMP_BEGIN();
      size_t size = -1;
      double* d = new double[size];
      OMP_END();
    }
    OMP_THROW();
  } catch (std::exception& e) {
    return;
  }
  FAIL() << "uncaught std::exception";
}

TEST(Common, omp_rethrow_dmlc_exception) {
  try {
    OMP_INIT();
    #pragma omp parallel num_threads(2)
    {
      OMP_BEGIN();
      throw dmlc::Error("");
      OMP_END();
    }
    OMP_THROW();
  } catch (dmlc::Error& e) {
    return;
  }
  FAIL() << "uncaught dmlc::Error";
}
