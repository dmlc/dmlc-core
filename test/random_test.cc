#include <string>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <dmlc/random.h>
#include <dmlc/recordio.h>
#include <time.h>

using namespace std;

int main(int argc, char *argv[]) {  
  if (argc < 2) {
    printf("Usage: seed\n");
    return 0;
  }
  unsigned seed = atoi(argv[1]);
  using namespace dmlc;
  RandomSampler rnd(seed);
  // Shuffle Test
  std::cout << "Shuffle from 1 to 10\n";
  std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  rnd.Shuffle(&v);
  std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, " "));
  std::cout << "\n";
  // Uniform Int Test
  std::cout << "Uniform Int, Range: 0 - 100 \n";
  for (int n=0; n<10; ++n)
      std::cout << rnd.GetUniformInt(0, 100) << ' ';
  std::cout << '\n';
  // Uniform Real Test
  std::cout << "Uniform Real, Range: 0 - 1 \n";
  for (int n=0; n<10; ++n)
      std::cout << rnd.GetUniformReal(0.0, 1.0) << ' ';
  std::cout << '\n';
  // Gaussian Test
  std::cout << "Gaussian Distribution, Mean: 0, Standard Deviation: 10 \n";
  for (int n=0; n<10; ++n)
      std::cout << rnd.GetGaussian(0.0, 10.0) << ' ';
  std::cout << '\n';

  // Speed Test : Generate 20,000,000 gaussian
  std::cout << "Speed Test \n";
  time_t start, end;
  double res = 0;
  time(&start);
  for (int i = 0; i < 20000000; i++)
    res += rnd.GetGaussian(0.0, 1.0);
  time(&end);
  std::cout << "Get Result: " << res << " in " << difftime(end, start) << "s\n";
}
