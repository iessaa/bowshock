#pragma once

/// @file
///
/// @brief Waitable functionality.
///
/// The waitable object is a core concept representing an asynchronous operation.
///
/// There are different kinds of waitables, for instance *interrupt-driven* waitables are progressed in the context of
/// an interrupt handler, while *higher-order* waitables are driven by functions that depend on other waitables.
/// Ultimately all waitables **must** depend on some asynchronous event external to the processor.

#include <rtl/base.hpp>
#include <rtl/intrinsics.hpp>

namespace rtl
{

struct waitable {
public:
  enum class status {
    pending,
    complete,
    failed
  };

  /// @brief Waits for all of the waitables passed to complete or fail.
  template <typename... waitables> static auto wait_all(waitables&&... list) {
    while (!wait_all_inner(std::forward<waitables>(list)...)) {
      rtl::intrinsics::wait_for_interrupt();
    }
  }

  /// @brief Waits for any of the waitables passed to complete or fail.
  template <typename... waitables> static auto wait_any(waitables&&... list) {
    while (!wait_any_inner(std::forward<waitables>(list)...)) {
      rtl::intrinsics::wait_for_interrupt();
    }
  }

private:
  template <typename T2, template <typename> class waitable>
  static auto wait_all_inner(const waitable<T2>& head) {
    return !head.is_pending();
  }

  template <typename T2, template <typename> class waitable, typename... waitables>
  static auto wait_all_inner(const waitable<T2>& head, waitables&&... tail) {
    return !head.is_pending() && wait_all_inner(std::forward<waitables>(tail)...);
  }

  template <typename T2, template <typename> class waitable>
  static auto wait_any_inner(const waitable<T2>& head) {
    return !head.is_pending();
  }

  template <typename T2, template <typename> class waitable, typename... waitables>
  static auto wait_any_inner(const waitable<T2>& head, waitables&&... tail) {
    return !head.is_pending() || wait_any_inner(std::forward<waitables>(tail)...);
  }
};

}
