//===-- nes_emu/Device.h - Device class declaration -------------*- C++ -*-===//
//
// This file is distributed under the Boost Software License. See LICENSE.TXT
// for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the declaration of the device class, which is emulate the
/// any device.
///
//===----------------------------------------------------------------------===//

#ifndef NES_EMU_DEVICE_H
#define NES_EMU_DEVICE_H

//==============================================================================
//= Dependencies
//==============================================================================
// Local/Private Headers

// External headers

// System headers
#include <cstdint>
#include <optional>
#include <system_error>

namespace nes_emu {
class Bus16;
class Device {
public:
  Device() = default;
  virtual ~Device() noexcept = default;
  // disallow copy
  Device(const Device &) = delete;
  Device &operator=(const Device &) = delete;
  // allow move
  Device(Device &&) noexcept = default;
  Device &operator=(Device &&) noexcept = default;
  virtual std::optional<std::errc> map(Bus16 *bus, uint_fast16_t address) = 0;
};
} // namespace nes_emu

#endif // NES_EMU_DEVICE_H
