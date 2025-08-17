// util_result.hpp
#pragma once

#include <cstdint>      // std::int32_t
#include <expected>     // std::expected, std::unexpected
#include <type_traits>  // is_enum_v, underlying_type_t
#include <utility>      // forward, move, to_underlying (C++23)
#include <variant>      // std::monostate
#include <version>      // __cpp_lib_to_underlying

namespace ds::detail {
/// Convert enum or integral to int32_t
template <typename X>
[[nodiscard]] constexpr inline std::int32_t to_i32(X v) noexcept {
  if constexpr (std::is_enum_v<X>) {
#if __cpp_lib_to_underlying >= 202102L
    return static_cast<std::int32_t>(std::to_underlying(v));
#else
    return static_cast<std::int32_t>(static_cast<std::underlying_type_t<X>>(v));
#endif
  } else {
    static_assert(std::is_integral_v<X>, "to_i32 expects enum or integral type");
    return static_cast<std::int32_t>(v);
  }
}
}  // namespace ds::detail

namespace ds {

struct ErrorInfo {
  std::int32_t facility {0};  // generic facility
  std::int32_t error_code {0};
};

template <typename T>
using Result = std::expected<T, ErrorInfo>;

using Status = Result<std::monostate>;

// ---------- Result<T> helpers ----------

// REVIEW: use `noexcept(noexcept(ds::Result<std::decay_t<T>(std::forward<T>(value))))` and construct Result of `T0` where `using T0 = std::decay_t<T>`
// REVIEW: add guards  requires (!ds::is_result_v<std::remove_cvref_t<U>> && !ds::is_status_v<std::remove_cvref_t<U>>) to prevent nesting

template <typename T>
[[nodiscard]] constexpr inline ds::Result<T> Ok(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>) {
  return ds::Result<T>(std::forward<T>(value));
}

template <typename T, typename... Args>
[[nodiscard]] constexpr inline ds::Result<T> Ok(Args&&... args) noexcept(
    std::is_nothrow_constructible_v<T, Args&&...>) {
  return ds::Result<T>(std::in_place, std::forward<Args>(args)...);
}

template <typename T, typename CodeT = std::int32_t, typename FacilityT = std::int32_t>
  requires((std::is_enum_v<FacilityT> || std::is_integral_v<FacilityT>) &&
           (std::is_enum_v<CodeT> || std::is_integral_v<CodeT>))
[[nodiscard]] constexpr inline ds::Result<T> Err(CodeT code, FacilityT facility = FacilityT {0}) noexcept {
  return std::unexpected(ds::ErrorInfo {ds::detail::to_i32(facility), ds::detail::to_i32(code)});
}

// ---------- Status (void-like) helpers ----------

[[nodiscard]] constexpr inline ds::Status Ok() noexcept { return std::monostate {}; }

template <typename CodeT = std::int32_t, typename FacilityT = std::int32_t>
  requires((std::is_enum_v<FacilityT> || std::is_integral_v<FacilityT>) &&
           (std::is_enum_v<CodeT> || std::is_integral_v<CodeT>))
[[nodiscard]] constexpr inline ds::Status Err(CodeT code, FacilityT facility = FacilityT {0}) noexcept {
  return ds::Err<std::monostate>(code, facility);
}
}  // namespace ds

namespace ds {
// ---- is_result / is_status ----
template <typename>
struct is_result : std::false_type {};
template <typename T>
struct is_result<ds::Result<T>> : std::true_type {};
template <typename X>
inline constexpr bool is_result_v = is_result<std::remove_cvref_t<X>>::value;

template <typename T>
inline constexpr bool is_status_v = std::is_same_v<std::remove_cvref_t<T>, ds::Status>;

template <typename R>
  requires ds::is_result_v<R>
[[nodiscard]] constexpr auto as_result(R&& r) noexcept(noexcept(std::remove_cvref_t<R>(std::forward<R>(r))))
    -> std::remove_cvref_t<R> {
  return std::forward<R>(r);
}

template <typename S>
  requires ds::is_status_v<S>
[[nodiscard]] constexpr auto as_status(S&& s) noexcept(noexcept(std::remove_cvref_t<S>(std::forward<S>(s))))
    -> std::remove_cvref_t<S> {
  return std::forward<S>(s);
}

}  // namespace ds
