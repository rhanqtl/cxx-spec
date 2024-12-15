#include <unordered_map>

#include "cxx-spec.h"

std::unordered_map<int, int> return_unordered_map() {
  return {};
}

describe("return_unordered_map") {
  it("returns an unordered_map") {
    expect(return_unordered_map())
        .to_eq(std::unordered_map<int, int>{{1, 1232}, {2, 1243}});
  }
}

CXXSPEC_MAIN()
