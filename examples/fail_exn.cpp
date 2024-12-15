#include <stdexcept>
#include <tuple>

#include "cxx-spec.h"

std::tuple<int, int> quo_rem(int a, int b) {
  if (b == 0)
    throw std::invalid_argument("b must be non-zero");
  return std::make_tuple(a / b, a % b);
}

describe("quo_rem") {
  it("throws an exception when the divisor is zero") {
    expect(quo_rem(10, 0)).to_throw<std::domain_error>();
  }
}

void do_nothing() {}

describe("do_nothing") {
  it("throws an exception") {
    expect(do_nothing()).to_throw<std::domain_error>();
  }
}

CXXSPEC_MAIN()
