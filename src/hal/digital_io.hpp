#pragma once

/// @file
///
/// @brief Hardware abstraction layer for digital IO.
///
/// This HAL provides interfaces allowing physical IO pins to be driven to defined "high" or "low" levels, or their
/// digital state to be read. All voltage thresholds are hardware-dependent and are not exposed by this HAL.

namespace hal {

enum class logic_level {
  low, high
};



template <class T>
class digital_input
{
public:
  logic_level state() {
    return static_cast<T*>(this)->state();
  }
};

template <class T>
class digital_output
{
public:
  void drive_low() {
    static_cast<T*>(this)->drive_low();
  }

  void drive_high() {
    static_cast<T*>(this)->drive_high();
  }

  void drive(logic_level level) {
    if (level == logic_level::low) {
      drive_low();
    } else {
      drive_high();
    }
  }
};

}
