//===-- nes_emu/Bus.cpp - Bus class implements  -----------------*- C++ -*-===//
//
// This file is distributed under the Boost Software License. See LICENSE.TXT
// for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implements of the Bus class, which is emulate the
/// system bus.
///
//===----------------------------------------------------------------------===//

//==============================================================================
//= Dependencies
//==============================================================================
// Main module header
#include "nes_emu/Bus.h"

// Local/Private headers
#include "nes_emu/Device.h"

// External headers

// System headers
#include <algorithm>
#include <cstring>
#include <iostream>

namespace nes_emu {

std::optional<std::errc> Bus16::mapMemory(Device *dev, uint_fast16_t address,
                                          size_t bytes, void *mem) {
  auto page_begin = address >> this->kPageSizeBits;
  auto page_end = (address + bytes - 1) >> this->kPageSizeBits;
  auto mem_addr = static_cast<uint8_t *>(mem);
  // chack already exists
  for (auto page = page_begin; page <= page_end; ++page) {
    if (this->map_table_[page]) {
      return std::errc::file_exists;
    }
  }
  // map
  auto offset = address - (page_begin << this->kPageSizeBits);
  for (auto page = page_begin; page <= page_end; ++page) {
    this->map_table_[page] = std::make_unique<Map>(
        dev, mem_addr, (page << this->kPageSizeBits) + offset,
        std::min(bytes, this->kPageSize));
    bytes -= this->kPageSize;
    mem_addr += this->kPageSize;
    offset = 0;
  }
  return std::nullopt;
}

void Bus16::read(uint_fast16_t address, size_t bytes, uint8_t *buffer) {
  decltype(bytes) readed_bytes = 0;
  while (readed_bytes < bytes) {
    auto reading_address = address + readed_bytes;
    auto page = reading_address >> this->kPageSizeBits;
    const auto &map = this->map_table_[page];
    if (map) {
      auto offset = reading_address - map->Address;
      auto reading_bytes = std::min(bytes - offset, map->Bytes);
      if ((reading_address < map->Address) ||
          (map->Address + map->Bytes < reading_address + reading_bytes)) {
        if (this->notify_error_ != nullptr) {
          this->notify_error_(address + reading_bytes - 1,
                              BusAccessKind::kRead);
        }
        return;
      }
      memcpy(buffer + readed_bytes, this->map_table_[page]->Memory + offset,
             reading_bytes);
      readed_bytes += reading_bytes;
    } else {
      if (this->notify_error_ != nullptr) {
        this->notify_error_(address, BusAccessKind::kRead);
      }
      return;
    }
  }
}

void Bus16::dumpMap() const {
  std::cout << "dump map(" << this->kPageSize << ")\n";
  std::cout << "--------\n";
  for (const auto &map : this->map_table_) {
    if (map) {
      std::cout << std::hex << map->Address << "\t" << std::hex << map->Bytes
                << std::endl;
    }
  }
}

} // namespace nes_emu
