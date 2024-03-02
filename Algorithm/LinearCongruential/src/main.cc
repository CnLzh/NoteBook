#include <iostream>

class Random {
 public:
  explicit Random(uint32_t s) : seed_(s & 0x7fffffffu) {
	// seed_ = (0,2^31-1),  0x7fffffffu = 2^31-1
	// s & 0x7fffffffu => seed_ = [0,2^31-1]
	// seed_ must not be zero or 2^31-1
	if (seed_ == 0 || seed_ == 2147483647L)
	  seed_ = 1;
  }

  // Random uniform distribution trend.
  uint32_t Next() {
	static const uint32_t M = 2147483647L;  // 2^31-1
	static const uint32_t A = 16807;

	// seed_ = (seed_ * A) (mod M)  A = 16807, M = 2^31-1
	uint64_t product = seed_ * A;

	// In fact, if: M = 2^n-1; then: (x >> n) % M == x % M;
	// (x >> n) % M = ((x % M) * ((M+1) % M)) % M = x % M
    // 
    // product = seed_ * A, seed_ < M, A < M, so product < M^2
    // (product >> 31) < M, (product >> 31) % M == (product >> 31)
    //
    // so, product % M = (product >> 31) + (product & M)
    // 
    // but if: a < M, b < M, a + b maybe > M, so if: seed_ > M, seed_ -=M.
    // 
	// On certain machines, 32-bit computations may be faster than 64-bit computations.
	seed_ = static_cast<uint32_t>((product >> 31) + (product & M));
	// The result may overflow by 1bit, so we may need to subtract M.
	if (seed_ > M) {
	  seed_ -= M;
	}
	return seed_;
  }

  // Random [0,n-1]
  uint32_t Uniform(int n) { return Next() % n; }

  // Random true for 1/n, otherwise false.
  bool OneIn(int n) { return (Next() % n) == 0; }

  // Random exponential bias towards smaller numbers.
  uint32_t Skewed(int n) { return Uniform(1 << Uniform(n + 1)); }

 private:
  uint32_t seed_;
};

int main() {
  Random random(12345);
  std::cout << "Next(): " << random.Next() << std::endl;
  std::cout << "Uniform(10): " << random.Uniform(10) << std::endl;
  std::cout << "OneIn(5): " << (random.OneIn(5) ? "true" : "false") << std::endl;
  std::cout << "Skewed(10): " << random.Skewed(10) << std::endl << std::endl;
  return 0;
}