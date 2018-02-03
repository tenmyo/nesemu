//===-- nes_emu/Bus.h - Bus class declaration -------------------*- C++ -*-===//
//
// This file is distributed under the Boost Software License. See LICENSE.TXT
// for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the declaration of the Bus class, which is emulate the
/// system bus.
///
//===----------------------------------------------------------------------===//

#ifndef NES_EMU_BUS_H
#define NES_EMU_BUS_H

//==============================================================================
//= Dependencies
//==============================================================================
// Local/Private Headers
#include "nes_emu/Device.h"

// External headers

// System headers
#include <cstddef>      // size_t
#include <cstdint>      // uint_fast16_t
#include <memory>       // unique_ptr
#include <optional>     // optional
#include <system_error> // errc

namespace nes_emu {
struct Map {
  Map(Device *owner, uint8_t *memory, uint_fast16_t address, size_t bytes)
      : Owner(owner), Memory(memory), Address(address), Bytes(bytes) {}
  Device *Owner;
  uint8_t *Memory;
  uint_fast16_t Address;
  size_t Bytes;
};

enum class BusAccessKind { kNone, kRead, kWrite };

class Bus16 {
public:
  using ErrorCallback = void (*)(uint_fast16_t addr, BusAccessKind op);
  explicit Bus16(ErrorCallback cb) noexcept
      : notify_error_(cb), map_table_{nullptr} {}
  ~Bus16() noexcept = default;
  // disallow copy
  Bus16(const Bus16 &) = delete;
  Bus16 &operator=(const Bus16 &) = delete;
  // allow move
  Bus16(Bus16 &&) noexcept = default;
  Bus16 &operator=(Bus16 &&) noexcept = default;

  std::optional<std::errc> mapMemory(Device *dev, uint_fast16_t address,
                                     size_t bytes, void *mem);
  void read(uint_fast16_t address, uint_fast16_t bytes, uint8_t *buffer);
  uint8_t read8(uint_fast16_t address);
  uint16_t read16(uint_fast16_t address);
  uint32_t read32(uint_fast16_t address);
  uint64_t read64(uint_fast16_t address);
  void write(const void *buffer, size_t bytes, uint_fast16_t destination);
  void write8(uint_fast16_t address, const uint8_t &value);
  void write16(uint_fast16_t address, const uint16_t &value);
  void write32(uint_fast16_t address, const uint32_t &value);
  void write64(uint_fast16_t address, const uint64_t &value);
  void dumpMap() const;

private:
  static constexpr auto kAddressBits = 16;
  static constexpr auto kPageSizeBits = 10;
  static constexpr uint_fast16_t kPageSize = 1 << kPageSizeBits;
  static constexpr std::uintptr_t kPageMask = kPageSize - 1;
  ErrorCallback notify_error_;
  std::unique_ptr<Map> map_table_[1ULL << (kAddressBits - kPageSizeBits)];
};

} // namespace nes_emu

#endif // NES_EMU_BUS_H
