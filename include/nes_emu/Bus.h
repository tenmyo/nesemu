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

// External headers

// System headers
#include <array>        // array
#include <cstddef>      // size_t
#include <cstdint>      // uint_fast16_t
#include <functional>   // function
#include <memory>       // unique_ptr
#include <optional>     // optional
#include <system_error> // errc
#include <utility>      // move

namespace nes_emu {
class Device;

enum class BusAccessKind { kNone, kRead, kWrite };

template <size_t address_bits> class Bus {

public:
  using AddressType = uint_fast16_t;
  explicit Bus(std::function<void(AddressType, BusAccessKind)> cb) noexcept
      : notify_error_(std::move(std::move(cb))) {}
  ~Bus() noexcept;
  // disallow copy
  Bus(const Bus &) = delete;
  Bus &operator=(const Bus &) = delete;
  // allow move
  Bus(Bus &&) noexcept = default;
  Bus &operator=(Bus &&) noexcept = default;

  std::optional<std::errc> mapMemory(Device *dev, AddressType address,
                                     size_t bytes, void *mem);
  void read(AddressType address, size_t bytes, void *buffer) const noexcept;
  uint8_t read8(AddressType address) const noexcept {
    uint8_t ret;
    this->read(address, sizeof(ret), &ret);
    return ret;
  }
  uint16_t read16(AddressType address) const noexcept {
    uint16_t ret;
    this->read(address, sizeof(ret), &ret);
    return ret;
  }
  uint32_t read32(AddressType address) const noexcept {
    uint32_t ret;
    this->read(address, sizeof(ret), &ret);
    return ret;
  }
  uint64_t read64(AddressType address) const noexcept {
    uint64_t ret;
    this->read(address, sizeof(ret), &ret);
    return ret;
  }
  void write(const void *buffer, size_t bytes,
             AddressType destination) noexcept;
  void write8(AddressType destination, const uint8_t &value) noexcept {
    this->write(&value, sizeof(value), destination);
  }
  void write16(AddressType destination, const uint16_t &value) noexcept {
    this->write(&value, sizeof(value), destination);
  }
  void write32(AddressType destination, const uint32_t &value) noexcept {
    this->write(&value, sizeof(value), destination);
  }
  void write64(AddressType destination, const uint64_t &value) noexcept {
    this->write(&value, sizeof(value), destination);
  }
  void dumpMap() const noexcept;

private:
  struct MemoryMap {
    explicit MemoryMap(Device *owner, void *memory, AddressType address,
                       size_t bytes)
        : Owner(owner), Memory(static_cast<uint8_t *>(memory)),
          Address(address), Bytes(bytes) {}
    Device *Owner;
    uint8_t *Memory;
    AddressType Address;
    size_t Bytes;
  };
  static constexpr auto kAddressBits = address_bits;
  static constexpr auto kPageSizeBits = 10;
  static constexpr auto kPageNum = 1ULL << (kAddressBits - kPageSizeBits);
  static constexpr AddressType kPageSize = 1 << kPageSizeBits;
  static constexpr std::uintptr_t kPageMask = kPageSize - 1;
  std::function<void(AddressType, BusAccessKind)> notify_error_;
  std::array<std::unique_ptr<MemoryMap>, kPageNum> map_table_;
};

using Bus16 = Bus<16>;
extern template class Bus<16>;

} // namespace nes_emu

#endif // NES_EMU_BUS_H
