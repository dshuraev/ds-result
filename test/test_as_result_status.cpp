// tests/test_as_result_status.cpp
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <type_traits>
#include <utility>
#include <cstdint>

#include <doctest/doctest.h>

#include "result.hpp"

// ---- tiny "vk" stubs for conversion tests ----
namespace vk {
  enum class Result : int { eSuccess = 0, eError = -1, eSuboptimal = 1 };

  template <class T>
  struct ResultValue {
    T value;
    Result result;
  };
}

// ---- user-provided overloads in namespace ds (simple & explicit) ----
namespace ds {
inline constexpr std::int32_t FACILITY_VK = 1001;

[[nodiscard]] inline constexpr ds::Status
as_status(vk::Result r) noexcept {
  return (r == vk::Result::eSuccess || r == vk::Result::eSuboptimal)
       ? ds::Ok()
       : ds::Err(static_cast<int>(r), FACILITY_VK);
}

template <class T>
[[nodiscard]] inline constexpr ds::Result<T>
as_result(const vk::ResultValue<T>& rv)
    noexcept(noexcept(ds::Result<T>(rv.value))) {
  return (rv.result == vk::Result::eSuccess || rv.result == vk::Result::eSuboptimal)
       ? ds::Ok(rv.value)
       : ds::Err<T>(rv.result, FACILITY_VK);
}

template <class T>
[[nodiscard]] inline constexpr ds::Result<T>
as_result(vk::ResultValue<T>&& rv)
    noexcept(noexcept(ds::Result<T>(std::move(rv.value)))) {
  return (rv.result == vk::Result::eSuccess || rv.result == vk::Result::eSuboptimal)
       ? ds::Ok(std::move(rv.value))
       : ds::Err<T>(static_cast<int>(rv.result), FACILITY_VK);
}
} // namespace ds

// ---- a tracker to observe copy vs move paths ----
struct Tracker {
  int id{0};
  static inline int copies = 0;
  static inline int moves  = 0;

  Tracker() = default;
  explicit Tracker(int i): id(i) {}
  Tracker(const Tracker& o) : id(o.id) { ++copies; }
  Tracker(Tracker&& o) noexcept : id(o.id) { ++moves; }
  Tracker& operator=(const Tracker& o) { id = o.id; ++copies; return *this; }
  Tracker& operator=(Tracker&& o) noexcept { id = o.id; ++moves; return *this; }
  friend bool operator==(const Tracker& a, const Tracker& b){ return a.id == b.id; }
};

TEST_CASE("as_result pass-through on lvalue Result<T> copies payload when needed") {
  Tracker::copies = Tracker::moves = 0;

  ds::Result<Tracker> r = ds::Ok(Tracker{7});
  auto out = ds::as_result(r); // lvalue => copy into return value
  CHECK(out.has_value());
  CHECK(out->id == 7);
  CHECK(Tracker::copies >= 1); // at least one copy somewhere in the path
}

TEST_CASE("as_result pass-through on rvalue Result<T> moves payload") {
  Tracker::copies = Tracker::moves = 0;

  ds::Result<Tracker> r = ds::Ok(Tracker{9});
  auto out = ds::as_result(std::move(r)); // rvalue => move
  CHECK(out.has_value());
  CHECK(out->id == 9);
  CHECK(Tracker::moves >= 1);
}

TEST_CASE("as_result pass-through on const lvalue Result<int> preserves value") {
  const ds::Result<int> r = ds::Ok(42);
  auto out = ds::as_result(r);
  CHECK(out.has_value());
  CHECK(out.value() == 42);
}

TEST_CASE("as_status pass-through preserves success/error and error info") {
  ds::Status ok = ds::Ok();
  auto ok2 = ds::as_status(ok);
  CHECK(ok2.has_value());

  ds::Status err = ds::Err(5, 123);
  auto err2 = ds::as_status(err);
  CHECK(!err2.has_value());
  CHECK(err2.error().error_code == 5);
  CHECK(err2.error().facility  == 123);
}

// ---- noexcept mirrors underlying construction ----
static_assert(noexcept(ds::as_result(std::declval<ds::Result<int>&&>())),
              "rvalue Result<int> should be noexcept to move");
static_assert(noexcept(ds::as_result(std::declval<const ds::Result<int>&>())),
              "const lvalue Result<int> should be noexcept to copy");
static_assert(noexcept(ds::as_status(std::declval<ds::Status&&>())),
              "Status move should be noexcept");
static_assert(noexcept(ds::as_status(std::declval<const ds::Status&>())),
              "Status copy should be noexcept");

TEST_CASE("vk::Result -> Status via user overloads") {
  ds::Status s1 = ds::as_status(vk::Result::eSuccess);
  ds::Status s2 = ds::as_status(vk::Result::eSuboptimal);
  ds::Status s3 = ds::as_status(vk::Result::eError);

  CHECK(s1.has_value());
  CHECK(s2.has_value());
  CHECK(!s3.has_value());
  if (!s3) {
    CHECK(s3.error().facility == ds::FACILITY_VK);
  }
}

TEST_CASE("vk::ResultValue<T> -> Result<T> via user overloads (copy & move)") {
  vk::ResultValue<int> rv1{5, vk::Result::eSuccess};
  auto r1 = ds::as_result(rv1);
  CHECK(r1.has_value());
  CHECK(r1.value() == 5);

  vk::ResultValue<std::string> rv2{std::string("ok"), vk::Result::eSuccess};
  auto r2 = ds::as_result(std::move(rv2));
  CHECK(r2.has_value());
  CHECK(r2.value() == "ok");

  vk::ResultValue<int> rv3{0, vk::Result::eError};
  auto r3 = ds::as_result(rv3);
  CHECK(!r3.has_value());
  CHECK(r3.error().facility == ds::FACILITY_VK);
}