//===-- nes_emu/Device/MemoryMappedDevice.h - MemoryMappedDevice *- C++ -*-===//
//
// This file is distributed under the Boost Software License. See LICENSE.TXT
// for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the declaration of the MemoryMappedDevice class.
///
//===----------------------------------------------------------------------===//

#ifndef NES_EMU_DEVICE_MEMORYMAPPEDDEVICE_H
#define NES_EMU_DEVICE_MEMORYMAPPEDDEVICE_H

//==============================================================================
//= Dependencies
//==============================================================================
// Local/Private Headers
#include "nes_emu/Device.h"

// External headers

// System headers

namespace nes_emu {
class MemoryMappedDevice : public Device {
protected: // Can only create subclasses.
  MemoryMappedDevice();

public:
  ~MemoryMappedDevice() noexcept override;
  // disallow copy & move
  MemoryMappedDevice(const MemoryMappedDevice &) = delete;
  MemoryMappedDevice &operator=(const MemoryMappedDevice &) = delete;
  MemoryMappedDevice(MemoryMappedDevice &&) noexcept = delete;
  MemoryMappedDevice &operator=(MemoryMappedDevice &&) noexcept = delete;
};
} // namespace nes_emu

#endif // NES_EMU_DEVICE_MEMORYMAPPEDDEVICE_H
