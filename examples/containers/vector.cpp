#include <vector>

#include "cxx-spec.h"

std::vector<int> return_vector() {
  return {};
}

describe("return_vector") {
  it("returns a vector") {
    expect(return_vector()).to_eq(std::vector<int>{1, 2, 3});
  }
}

CXXSPEC_MAIN()
