#define RTL_CORTEX_M0

#include <rtl/platform.hpp>

#include <hal/lpc1114/headers/LPC11xx.h>
#include <hal/lpc1114/headers/hdr_syscon.h>

#include <hal/lpc1100/system.hpp>
#include <hal/lpc1100/digital_io.hpp>
#include <hal/lpc1100/uart.hpp>
#include <rtl/assert.hpp>
#include <hal/lpc1100/interrupt.hpp>
#include <hal/lpc1100/clock.hpp>

#include <sys/format.hpp>

static void flash_access_time(uint32_t frequency);

static void flash_access_time(uint32_t frequency)
{
    uint32_t access_time, flashcfg_register;

    if (frequency < 20000000ul)             // 1 system clock for core speed below 20MHz
        access_time = FLASHCFG_FLASHTIM_1CLK;
    else if (frequency < 40000000ul)        // 2 system clocks for core speed between 20MHz and 40MHz
        access_time = FLASHCFG_FLASHTIM_2CLK;
    else                                    // 3 system clocks for core speed over 40MHz
        access_time = FLASHCFG_FLASHTIM_3CLK;

    // do not modify reserved bits in FLASHCFG register
    flashcfg_register = FLASHCFG;           // read register
    flashcfg_register &= ~(FLASHCFG_FLASHTIM_mask << FLASHCFG_FLASHTIM_bit);    // mask the FLASHTIM field
    flashcfg_register |= access_time << FLASHCFG_FLASHTIM_bit;  // use new FLASHTIM value
    FLASHCFG = flashcfg_register;           // save the new value back to the register
}

namespace dev = hal::lpc1100;

using assert_pin = dev::digital_output<dev::pin::PIO1_5>;
using output_pin = dev::digital_output<dev::pin::PIO0_8>;
using input_pin = dev::digital_input<dev::pin::PIO0_7>;

void assert_signal() {
  auto pin = assert_pin(hal::logic_level::high);

  for (auto i = 0; i < 5; ++i) {
    for (auto j = 0; j < 0x1FFFF; ++j) {
      pin.drive_high();
    }

    for (auto j = 0; j < 0x1FFFF; ++j) {
      pin.drive_low();
    }
  }
}

auto read_until(char* buffer, std::size_t& length, rtl::u8 pattern) {
  return [buffer, &length, pattern](const rtl::u8& data) {
    buffer[length++] = data;
    return data == pattern ? rtl::waitable::status::complete : rtl::waitable::status::pending;
  };
}

auto read_any() {
  return [](const rtl::u8&) {
    return rtl::waitable::status::complete;
  };
}

[[noreturn]] void main(const dev::reset_context& context) {
  auto frequency = 48_MHz;
  flash_access_time(48000000);           // configure flash access time first

  // set the main clock
  dev::clock<dev::clock_source::pll_in>::set_source(dev::clock_source::irc);
  dev::clock<dev::clock_source::pll_out>::enable(frequency);
  dev::clock<dev::clock_source::main>::set_source(dev::clock_source::pll_out);

  LPC_SYSCON->SYSAHBCLKDIV = 1;           // set AHB clock divider to 1

  auto uart = dev::uart0(9600_Hz);

  if (context.event == dev::reset_event::assert) {
    assert_signal();
  }

  constexpr auto v1 = rtl::grams<rtl::q32>{5.0f};
  //constexpr auto v2 = rtl::seconds<rtl::q32>{10.0f};
  constexpr auto v2 = 5_milliseconds;//rtl::my_quantity<rtl::q32, rtl::dimensionless>{rtl::q32{5.0f}};
  constexpr auto v3 = v1 / v2;
  //constexpr auto v4 = v1 * v3;
  constexpr rtl::quantity<rtl::q32, rtl::kilogram::per<rtl::second>> v4 = v3;
  constexpr auto scalar = v4.template as<rtl::kilogram::per<rtl::second>>(); // in kilograms/second
  //constexpr auto scalar2 = rtl::my_quantity<rtl::q32, rtl::
  
  //constexpr auto x = rtl::q32<rtl::rational_mode::best>(1.666981f);
  constexpr auto x = rtl::r32(18481.38438f);
  constexpr auto y = rtl::r32(3.1415926535897932384626433832795);

  uart.read(read_any()).wait();

  /*
  constexpr auto foo = rtl::MHz<rtl::q32>{{100.0f}};
  constexpr auto bar = rtl::Hz<rtl::q32>{foo};
  */

  uart.write(sys::format(std::pair{"", scalar.numerator()}, std::pair{"10s", "/"}, std::pair{"", scalar.denominator()})).wait();
  
  //output.drive_low();

  rtl::assert<x + (y - x) - 2.5f <= x + x>("test");

  //output.drive_high();  

  #if 0
  const char map[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  char buf[8];

  auto x = uart.read({buf, 1});
  x.wait();

  int test;

  for (auto i = 0; i < 8; ++i) {
      buf[7 - i] = map[((0x10000400 - ((rtl::uptr)&test)) >> (4 * i)) & 0xF];
  }

  uart.write({buf, sizeof(buf)}).wait();
  #endif

  //auto gpio2 = dev::gpio2<dev::pin::PIO0_8>(/* termination, interrupt settings, etc.. */);
  //auto uart = dev::uart<dev::pin::PIO1_7, dev::pin::PIO1_6>(/* uart settings */);

  /*
  while (true) {
    char buffer[32];
    std::size_t pos = 0;
    std::size_t offset = 0;

    uart.read(read_until(buffer, pos, '\r')).wait();

    uart.write([&](rtl::u8& data) {
      if (offset == pos) {
          return true;
      }

      data = buffer[offset++];
      return false;
    }).wait();
  }
  */

  /*
  while (true) {
    uart.send("hello\r\n", 7);

    for (auto i = 0; i < 0xFFFF; ++i) { __NOP(); };
  }
  */

  auto output = output_pin(hal::logic_level::high);
  auto input = input_pin(input_pin::termination::pullup);

  while (true) {
    for (auto i = 0; i < 0x1FFFFF; ++i) {
      output.drive_low();
    }

    for (auto i = 0; i < 0x1FFFFF; ++i) {
      output.drive_high();
    }
  }
}
