#ifndef CXX_SPEC_H
#define CXX_SPEC_H

#include <fmt/base.h>

#include <type_traits>
#include <unordered_map>
#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <fmt/format.h>

#include <functional>
#include <iostream>
#include <utility>
#include <vector>

template <typename T>
struct fmt::formatter<std::vector<T>> : fmt::formatter<T> {
  template <typename FormatContext>
  auto format(const std::vector<T> &vec, FormatContext &ctx) const {
    fmt::format_to(ctx.out(), "[");
    for (size_t i = 0; i < vec.size(); ++i) {
      if (i != 0)
        fmt::format_to(ctx.out(), ", ");
      fmt::formatter<T>::format(vec[i], ctx);
    }
    return fmt::format_to(ctx.out(), "]");
  }
};

template <typename K, typename V, typename... Args>
struct fmt::formatter<std::unordered_map<K, V, Args...>> {
  fmt::formatter<K> key_fmt;
  fmt::formatter<V> value_fmt;

  // Add required parse method
  constexpr auto parse(format_parse_context &ctx) {
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && *it != '}') {
      throw format_error("invalid format");
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const std::unordered_map<K, V, Args...> &map,
              FormatContext &ctx) const {
    fmt::format_to(ctx.out(), "{");
    for (auto it = map.begin(); it != map.end(); ++it) {
      if (it != map.begin())
        fmt::format_to(ctx.out(), ", ");

      fmt::format_to(ctx.out(), "[");
      key_fmt.format(it->first, ctx);
      fmt::format_to(ctx.out(), "] =");

      value_fmt.format(it->second, ctx);
    }
    return fmt::format_to(ctx.out(), "}");
  }
};

namespace cxxspec::detail {
template <typename T>
constexpr std::string_view get_type_name() {
  constexpr auto func_name = std::string_view{__PRETTY_FUNCTION__};
#if defined(__clang__)
  constexpr auto kOffset = func_name.find("[T = ") + 5;
  constexpr auto len = func_name.length() - 1 - kOffset;
#elif defined(__GNUC__)
  constexpr auto kOffset = func_name.find("[with T = ") + 10;
  constexpr auto kFirstSemiPos = func_name.find_first_of(';');
  constexpr auto len = kFirstSemiPos - kOffset;
#else
#error "compiler not supported"
#endif
  return func_name.substr(kOffset, len);
}
}  // namespace cxxspec::detail

namespace cxxspec::detail {
class TestSuiteBase;

inline std::vector<std::reference_wrapper<TestSuiteBase>> all_test_suites;

inline void register_test_suite(TestSuiteBase &suite) {
  all_test_suites.push_back(suite);
}
}  // namespace cxxspec::detail

namespace cxxspec::detail {
class TestSuiteBase {
 public:
  struct MismatchItem {
    const bool exception;
    const std::string expected;
    const std::string got;

    MismatchItem(bool is_exn, const std::string &e, const std::string &g)
        : exception{is_exn}, expected{e}, got{g} {}
  };

 public:
  TestSuiteBase(std::string_view name) : _name{name} {
    ::cxxspec::detail::register_test_suite(*this);
  }
  TestSuiteBase(const TestSuiteBase &) = delete;
  TestSuiteBase(TestSuiteBase &&) = delete;
  TestSuiteBase &operator=(const TestSuiteBase &) = delete;
  TestSuiteBase &operator=(TestSuiteBase &&) = delete;
  virtual ~TestSuiteBase() = default;

 public:
  std::string_view name() {
    return _name;
  }

  bool current_scenario(std::string_view scenario) {
    _mismatches_by_scenario.emplace_back();
    _mismatches_by_scenario.back().first = scenario;
    return true;
  }

  template <typename T>
  void mismatch(T &&expect, T &&actual) {
    current_message_store().emplace_back(
        false, fmt::format("{}", std::forward<T>(expect)),
        fmt::format("{}", std::forward<T>(actual)));
  }

  template <typename ExpectedT, typename GotT = void>
  void mismatch_exception() {
    if constexpr (!std::is_same_v<GotT, void>)
      current_message_store().emplace_back(
          true, fmt::format("{}", get_type_name<ExpectedT>()),
          fmt::format("{}", get_type_name<GotT>()));
    else
      current_message_store().emplace_back(
          true, fmt::format("{}", get_type_name<ExpectedT>()), "<nothing>");
  }

  virtual void operator()() = 0;

 private:
  std::vector<MismatchItem> &current_message_store() {
    return _mismatches_by_scenario.back().second;
  }

 public:
  const std::string_view _name;
  std::vector<std::pair<std::string, std::vector<MismatchItem>>>
      _mismatches_by_scenario;
};

template <typename T>
class ExpectWrapper {
 public:
  ExpectWrapper(TestSuiteBase &test_case, std::function<T()> thunk, int line)
      : _test_case{test_case}, _thunk{std::move(thunk)}, _line{line} {}

  template <typename U,
            typename = std::enable_if_t<std::is_convertible_v<U, T>>>
  void to_eq(U &&expected) {
    auto got = _thunk();
    if (!(got == expected)) {
      _test_case.mismatch(expected, got);
    }
  }
  void to_be_true() {
    to_eq(true);
  }
  void to_be_false() {
    to_eq(false);
  }

  template <typename E>
  void to_throw() {
    int thrown = 0;
    try {
      _thunk();
    } catch (const E &) {
      thrown = 1;
    } catch (...) {
      thrown = -1;
    }
    if (thrown == 0)
      _test_case.mismatch_exception<E>();
    else if (thrown == -1)
      _test_case.mismatch_exception<E, std::exception>();
  }

 private:
  TestSuiteBase &_test_case;
  std::function<T()> _thunk;
  int _line;
};

template <>
class ExpectWrapper<void> {
 public:
  ExpectWrapper(TestSuiteBase &test_case, std::function<void()> thunk, int line)
      : _test_case{test_case}, _thunk{std::move(thunk)}, _line{line} {}

  template <typename E>
  void to_throw() {
    int thrown = 0;
    try {
      _thunk();
    } catch (const E &) {
      thrown = 1;
    } catch (...) {
      thrown = -1;
    }
    if (thrown == 0)
      _test_case.mismatch_exception<E>();
    else if (thrown == -1)
      _test_case.mismatch_exception<E, std::exception>();
  }

 private:
  TestSuiteBase &_test_case;
  std::function<void()> _thunk;
  int _line;
};
}  // namespace cxxspec::detail

namespace cxxspec::detail {
inline int main() {
  if (auto n = all_test_suites.size(); n == 0)
    std::cerr << "No test suites to run\n";
  else if (n == 1)
    std::cerr << "Total 1 test suite to run\n";
  else
    std::cerr << "Total " << n << " test suites to run\n";

  bool ok = true;

  bool first = true;
  for (auto suite_ref : all_test_suites) {
    if (first)
      first = false;
    else
      std::cerr << '\n';

    auto &suite = suite_ref.get();

    std::cerr << "Running " << suite.name() << '\n';
    try {
      suite();
    } catch (...) {
      std::cerr << "  Exception thrown\n";
      continue;
    }

    for (const auto &[scenario, mismatches] : suite._mismatches_by_scenario) {
      if (mismatches.empty()) {
        std::cerr << "  [PASS] Scenario \"" << scenario << "\"\n";
      } else {
        ok = false;
        std::cerr << "  [FAIL] Scenario \"" << scenario << "\"\n";
        for (const auto &[is_exn, expected, got] : mismatches) {
          if (is_exn) {
            std::cerr << "    expected to throw: " << expected << '\n';
            std::cerr << "                  got: " << got << '\n';
          } else {
            std::cerr << "    expected: " << expected << '\n';
            std::cerr << "         got: " << got << '\n';
          }
        }
      }
    }
  }

  return !ok;
}
}  // namespace cxxspec::detail

#define _CONCAT(x, y) _CONCAT_INTERNAL(x, y)
#define _CONCAT_INTERNAL(x, y) x##y

#define describe(name) _describe_impl(_CONCAT(_TestSuite_, __COUNTER__), name)
#define _describe_impl(suite_name, name)                       \
  struct suite_name final : ::cxxspec::detail::TestSuiteBase { \
    suite_name() : TestSuiteBase{name} {}                      \
    void operator()() override;                                \
  };                                                           \
  suite_name _CONCAT(suite_name, _inst);                       \
  void suite_name::operator()()

#define it(scenario) if (current_scenario(scenario))

#define expect(expr)                                          \
  ::cxxspec::detail::ExpectWrapper<decltype(expr)> {          \
    *this,                                                    \
        [&] {                                                 \
          if constexpr (std::is_same_v<decltype(expr), void>) \
            expr;                                             \
          else                                                \
            return expr;                                      \
        },                                                    \
        __LINE__                                              \
  }

#define CXXSPEC_MAIN()                \
  int main() {                        \
    return ::cxxspec::detail::main(); \
  }

#endif  // CXX_SPEC_H
