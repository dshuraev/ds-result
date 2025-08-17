#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest/doctest.h"
#include "result.hpp"

enum class Facilities : std::int32_t { general = 0, vulkan = 1 };
enum class CustomEc : std::int32_t { success = 0, failure = 1 };

TEST_CASE("Status Ok/Err basic behavior") {
  auto ok = ds::Ok();
  CHECK(ok.has_value());

  auto err = ds::Err(CustomEc::failure, Facilities::vulkan);
  CHECK_FALSE(err.has_value());
  CHECK(err.error().facility == static_cast<std::int32_t>(Facilities::vulkan));
  CHECK(err.error().error_code == static_cast<std::int32_t>(CustomEc::failure));

  auto err2 = ds::Err(CustomEc::failure);
  CHECK_FALSE(err.has_value());
  CHECK(err.error().error_code == static_cast<std::int32_t>(CustomEc::failure));
}

TEST_CASE("Result<T> Ok/Err basic behavior") {
  auto ok_val = ds::Ok<int>(42);
  CHECK(ok_val.has_value());
  CHECK(*ok_val == 42);

  auto err_val = ds::Err<int>(CustomEc::failure, Facilities::general);
  CHECK_FALSE(err_val.has_value());
  CHECK(err_val.error().facility == static_cast<std::int32_t>(Facilities::general));
  CHECK(err_val.error().error_code == static_cast<std::int32_t>(CustomEc::failure));
}

TEST_CASE("Move semantics - non-trivial type") {
  struct MoveOnly {
    std::string data;
    MoveOnly(std::string d) : data(std::move(d)) {}
    MoveOnly(const MoveOnly&)                = delete;
    MoveOnly& operator=(const MoveOnly&)     = delete;
    MoveOnly(MoveOnly&&) noexcept            = default;
    MoveOnly& operator=(MoveOnly&&) noexcept = default;
  };

  SUBCASE("Ok value move") {
    auto res1 = ds::Ok<MoveOnly>("hello");
    auto res2 = std::move(res1);

    CHECK(res2.has_value());
    CHECK(res2->data == "hello");
  }

  SUBCASE("Err value move") {
    auto res1 = ds::Err<MoveOnly>(CustomEc::failure, Facilities::vulkan);
    auto res2 = std::move(res1);

    CHECK_FALSE(res2.has_value());
    CHECK(res2.error().facility == static_cast<std::int32_t>(Facilities::vulkan));
  }
}
