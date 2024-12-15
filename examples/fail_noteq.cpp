#include <stdexcept>
#include <tuple>

#include "cxx-spec.h"

std::tuple<int, int> quo_rem(int a, int b) {
  if (b == 0) throw std::invalid_argument("b must be non-zero");
  return std::make_tuple(a / b, a % b);
}

describe("quo_rem") {
  it("returns the quotient and remainder of two integers") {
    auto [q, r] = quo_rem(10, 3);
    expect(q).to_eq(2);
    expect(r).to_eq(2);
  }
}

CXXSPEC_MAIN()