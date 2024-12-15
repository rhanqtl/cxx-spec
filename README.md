![CI/CD](https://github.com/rhanqtl/cxx-spec/actions/workflows/ci/badge.svg)

# CxxSpec: A RSpec-like Test Framework For BDD

## Overview

CxxSpec is a header-only test framework for BDD, with this library, you can
write tests like this:

```cpp
#include "cxx-spec.h"

int safe_add(int a, int b);

describe("function_to_test") {
  it("returns 5 for 2 + 3") {
    expect(safe_add(2, 3)).to_eq(5);
  }

  it("throws an exception when overflow") {
    expect(safe_add(std::numeric_limits<int>::max(), 1))
        .to_throw<std::domain_error>();
  }
}

int safe_add(int a, int b) {
  if (a == 0)
    return b;
  if (b == 0)
    return a;

  auto is_neg = [](int x) { return x < 0; };
  const auto is_neg_a = is_neg(a), is_neg_b = is_neg(b);
  if (is_neg_a != is_neg_b)
    return a + b;

  if (is_neg_a) {
    if (a < std::numeric_limits<int>::min() - b)
      throw std::domain_error();
  } else {
    if (a > std::numeric_limits<int>::max() - b)
      throw std::domain_error();
  }
  return a + b;
}
```

Refer to directory `examples` for more examples.

## External Dependencies

External dependencies include:

* `fmtlib` or `{fmt}`

## Use As CMake Build Dependency

TODO
