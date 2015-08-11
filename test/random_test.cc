#include <string>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <dmlc/random.h>
#include <dmlc/recordio.h>

int main(int argc, char *argv[]) {  
  if (argc < 2) {
    printf("Usage: seed\n");
    return 0;
  }
  unsigned seed = atoi(argv[1]);
  using namespace dmlc;
  // shuffle
  std::cout << "Shuffle from 1 to 10\n";
  std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  Shuffler shuf(seed);
  shuf.Shuffle(&v);
  std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, " "));
  std::cout << "\n";
  
  std::cout << "Uniform Int, Range: 0 - 100 \n";
  UniformIntSampler<int> intsampler(0, 100, seed);
  for (int n=0; n<10; ++n)
      std::cout << intsampler.Get() << ' ';
  std::cout << '\n';

  std::cout << "Uniform Real, Range: 0 - 100 \n";
  UniformRealSampler<double> doublesampler(0.0, 100.0, seed);
  for (int n=0; n<10; ++n)
      std::cout << doublesampler.Get() << ' ';
  std::cout << '\n';

  std::cout << "Gaussian Distribution, Mean: 0, Standard Deviation: 10 \n";
  GaussianSampler<double> gaussiansampler(0.0, 10.0, seed);
  for (int n=0; n<10; ++n)
      std::cout << gaussiansampler.Get() << ' ';
  std::cout << '\n';
}
