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
#include "nes_emu/Bus.h"

// External headers

// System headers
#include <cstdint>
#include <optional>
#include <system_error>

namespace nes_emu {
class Device {
public:
  Device() = default;
  virtual ~Device() noexcept = default;
  // disallow copy & move
  Device(const Device &) = delete;
  Device &operator=(const Device &) = delete;
  Device(Device &&) noexcept = delete;
  Device &operator=(Device &&) noexcept = delete;
  virtual std::optional<std::errc> map(Bus16 *bus,
                                       Bus16::AddressType address) = 0;
};
} // namespace nes_emu

#endif // NES_EMU_DEVICE_H
