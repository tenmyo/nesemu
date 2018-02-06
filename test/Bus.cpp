// Gtest
#include <gtest/gtest.h>

// Target module header
#include "nes_emu/Bus.h"

// Local/Private headers
#include "nes_emu/Device/Sram.h"

// External headers

// System headers
#include <cstring> // memset, memcmp

using namespace nes_emu;
namespace {
uint_fast16_t gaddr;
BusAccessKind gop;
void errorCallback(uint_fast16_t addr, BusAccessKind op) {
  gaddr = addr;
  gop = op;
}
} // namespace

TEST(Bus16Test, MapToExists) {
  // Setup
  Bus16 bus(errorCallback);
  Sram<1024> sram;
  ASSERT_FALSE(sram.map(&bus, 0));
  // Do
  auto ret = sram.map(&bus, 0);
  // Verify
  EXPECT_TRUE(ret);
  EXPECT_EQ(ret.value(), std::errc::file_exists);
}
TEST(Bus16Test, MapToMiddle) {
  // Setup
  Bus16 bus(errorCallback);
  Sram<512> sram;
  auto p = sram.data();
  memset(p, 0xce, sram.size());
  ASSERT_FALSE(sram.map(&bus, 256));
  uint8_t buf[1 * 1024] = {0};
  uint_fast16_t addr = 256;
  uint_fast16_t bytes = 511;
  gaddr = 0;
  gop = BusAccessKind::kNone;
  // Do
  bus.read(addr, bytes, buf);
  // Verify
  EXPECT_EQ(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kNone);
  EXPECT_EQ(std::memcmp(buf, p, bytes), 0);
  EXPECT_NE(std::memcmp(buf + bytes, p + bytes, 1), 0);
}

TEST(Bus16Test, ReadForNotRegisterd) {
  // Setup
  Bus16 bus(errorCallback);
  uint8_t buf[2 * 1024];
  uint_fast16_t addr = 0x100;
  gaddr = 0;
  gop = BusAccessKind::kNone;
  // Do
  bus.read(addr, 1, buf);
  // Verify
  EXPECT_EQ(gaddr, addr);
  EXPECT_EQ(gop, BusAccessKind::kRead);
}
TEST(Bus16Test, ReadForRegisterdOne) {
  // Setup
  Bus16 bus(errorCallback);
  Sram<1024> sram;
  auto p = sram.data();
  memset(p, 0xce, sram.size());
  ASSERT_FALSE(sram.map(&bus, 0x8000));
  uint8_t buf[2 * 1024] = {0};
  uint_fast16_t addr = 0x8000;
  uint_fast16_t bytes = 0x100;
  gaddr = 0;
  gop = BusAccessKind::kNone;
  // Do
  bus.read(addr, bytes, buf);
  // Verify
  EXPECT_EQ(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kNone);
  EXPECT_EQ(std::memcmp(buf, p, bytes), 0);
  EXPECT_NE(std::memcmp(buf + bytes, p + bytes, 1), 0);
}
TEST(Bus16Test, ReadOver) {
  // Setup
  Bus16 bus(errorCallback);
  Sram<0x200> sram;
  auto p = sram.data();
  memset(p, 0xce, sram.size());
  ASSERT_FALSE(sram.map(&bus, 0x100));
  uint8_t buf[1 * 0x400] = {0};
  uint_fast16_t addr = 0x100;
  uint_fast16_t bytes = 0x300;
  gaddr = 0;
  gop = BusAccessKind::kNone;
  // Do
  bus.read(addr, bytes, buf);
  // Verify
  EXPECT_NE(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kRead);
}
TEST(Bus16Test, ReadAcrossTwoMap) {
  // Setup
  Bus16 bus(errorCallback);
  Sram<0x200> sram;
  Sram<0x200> sram2;
  auto p1 = sram.data();
  memset(p1, 0xce, sram.size());
  auto p2 = sram2.data();
  memset(p2, 0xa5, sram2.size());
  ASSERT_FALSE(sram.map(&bus, 0x200));
  ASSERT_FALSE(sram2.map(&bus, 0x400));
  uint8_t buf[0x500] = {0};
  uint_fast16_t addr = 0x200;
  uint_fast16_t bytes = 0x400;
  gaddr = 0;
  gop = BusAccessKind::kNone;
  // Do
  bus.read(addr, bytes, buf);
  // Verify
  EXPECT_EQ(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kNone);
  EXPECT_EQ(std::memcmp(buf, p1, 0x200), 0);
  EXPECT_EQ(std::memcmp(buf + 0x200, p2, 0x200), 0);
  EXPECT_EQ(buf[0x200 + 0x200], 0);
}

TEST(Bus16Test, WriteForNotRegisterd) {
  // Setup
  Bus16 bus(errorCallback);
  const uint8_t buf[2] = {0};
  uint_fast16_t addr = 0x100;
  gaddr = 0;
  gop = BusAccessKind::kNone;
  // Do
  bus.write(buf, sizeof(buf), addr);
  // Verify: error
  EXPECT_EQ(gaddr, addr);
  EXPECT_EQ(gop, BusAccessKind::kWrite);
}
TEST(Bus16Test, WriteForRegisterdOne) {
  // Setup
  Bus16 bus(errorCallback);
  Sram<0x200> sram;
  auto p = sram.data();
  memset(p, 0xce, sram.size());
  ASSERT_FALSE(sram.map(&bus, 0x8000));
  const uint8_t buf[0x200] = {0};
  uint_fast16_t addr = 0x8000;
  size_t bytes = 0x100;
  gaddr = 0;
  gop = BusAccessKind::kNone;
  // Do
  bus.write(buf, bytes, addr);
  // Verify: no error
  EXPECT_EQ(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kNone);
  // Verify: written just the given bytes
  EXPECT_EQ(std::memcmp(buf, p, bytes), 0);
  EXPECT_EQ(p[bytes], 0xce);
}
TEST(Bus16Test, WriteOver) {
  // Setup
  Bus16 bus(errorCallback);
  Sram<0x200> sram;
  auto p = sram.data();
  memset(p, 0xce, sram.size());
  ASSERT_FALSE(sram.map(&bus, 0x100));
  uint8_t buf[0x400] = {0};
  uint_fast16_t addr = 0x100;
  size_t bytes = 0x300;
  gaddr = 0;
  gop = BusAccessKind::kNone;
  // Do
  bus.write(buf, bytes, addr);
  // Verify: error
  EXPECT_NE(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kWrite);
}

TEST(Bus16Test, WriteReadForRegisterdOne) {
  // Setup
  Bus16 bus(errorCallback);
  Sram<0x400> sram;
  auto p = sram.data();
  memset(p, 0xce, sram.size());
  ASSERT_FALSE(sram.map(&bus, 0x8000));
  const uint8_t buf[0x200] = {0};
  uint8_t buf2[0x200];
  memset(buf2, 0x12, sizeof(buf));
  uint_fast16_t addr = 0x8000;
  size_t bytes = 0x100;
  gaddr = 0;
  gop = BusAccessKind::kNone;
  // Do
  bus.write(buf, bytes, addr);
  // Verify: no error
  EXPECT_EQ(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kNone);
  // Verify: written just the given bytes
  EXPECT_EQ(std::memcmp(buf, p, bytes), 0);
  EXPECT_EQ(p[bytes], 0xce);
  // Do
  bus.read(addr, bytes, buf2);
  // Verify: no error
  EXPECT_EQ(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kNone);
  // Verify: read just the given bytes
  EXPECT_EQ(std::memcmp(buf, buf2, bytes), 0);
  EXPECT_EQ(buf2[bytes], 0x12);
}
TEST(Bus16Test, WriteReadAcrossTwoMap) {
  // Setup
  Bus16 bus(errorCallback);
  Sram<0x200> sram;
  Sram<0x300> sram2;
  auto p1 = sram.data();
  memset(p1, 0xce, sram.size());
  auto p2 = sram2.data();
  memset(p2, 0xa5, sram2.size());
  ASSERT_FALSE(sram.map(&bus, 0x200));
  ASSERT_FALSE(sram2.map(&bus, 0x400));
  const uint8_t buf[0x500] = {0};
  uint8_t buf2[0x500];
  memset(buf2, 0x12, sizeof(buf));
  uint_fast16_t addr = 0x200;
  size_t bytes = 0x400;
  gaddr = 0;
  gop = BusAccessKind::kNone;
  // Do
  bus.write(buf, bytes, addr);
  // Verify: no error
  EXPECT_EQ(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kNone);
  // Verify: written just the given bytes
  EXPECT_EQ(std::memcmp(buf, p1, 0x200), 0);
  EXPECT_EQ(std::memcmp(buf + 0x200, p2, 0x200), 0);
  EXPECT_EQ(p2[0x200], 0xa5);
  // Do
  bus.read(addr, bytes, buf2);
  // Verify: no error
  EXPECT_EQ(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kNone);
  // Verify: read just the given bytes
  EXPECT_EQ(std::memcmp(buf, buf2, bytes), 0);
  EXPECT_EQ(buf2[bytes], 0x12);
}

TEST(Bus16Test, WriteRead8) {
  // Setup
  Bus16 bus(errorCallback);
  Sram<0x100> sram;
  auto p = sram.data();
  memset(p, 0xce, sram.size());
  ASSERT_FALSE(sram.map(&bus, 0x8000));
  uint_fast16_t addr = 0x8000;
  gaddr = 0;
  gop = BusAccessKind::kNone;
  // Do
  bus.write8(addr, 0x01);
  // Verify: no error
  EXPECT_EQ(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kNone);
  // Do
  auto ret = bus.read8(addr);
  // Verify: no error
  EXPECT_EQ(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kNone);
  EXPECT_EQ(ret, 0x01);
}
TEST(Bus16Test, WriteRead16) {
  // Setup
  Bus16 bus(errorCallback);
  Sram<0x100> sram;
  auto p = sram.data();
  memset(p, 0xce, sram.size());
  ASSERT_FALSE(sram.map(&bus, 0x8000));
  uint_fast16_t addr = 0x8000;
  gaddr = 0;
  gop = BusAccessKind::kNone;
  // Do
  bus.write16(addr, 0x0123);
  // Verify: no error
  EXPECT_EQ(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kNone);
  // Do
  auto ret = bus.read16(addr);
  // Verify: no error
  EXPECT_EQ(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kNone);
  EXPECT_EQ(ret, 0x0123);
}
TEST(Bus16Test, WriteRead32) {
  // Setup
  Bus16 bus(errorCallback);
  Sram<0x100> sram;
  auto p = sram.data();
  memset(p, 0xce, sram.size());
  ASSERT_FALSE(sram.map(&bus, 0x8000));
  uint_fast16_t addr = 0x8000;
  gaddr = 0;
  gop = BusAccessKind::kNone;
  // Do
  bus.write32(addr, 0x01234567);
  // Verify: no error
  EXPECT_EQ(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kNone);
  // Do
  auto ret = bus.read32(addr);
  // Verify: no error
  EXPECT_EQ(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kNone);
  EXPECT_EQ(ret, 0x01234567);
}
TEST(Bus16Test, WriteRead64) {
  // Setup
  Bus16 bus(errorCallback);
  Sram<0x100> sram;
  auto p = sram.data();
  memset(p, 0xce, sram.size());
  ASSERT_FALSE(sram.map(&bus, 0x8000));
  uint_fast16_t addr = 0x8000;
  gaddr = 0;
  gop = BusAccessKind::kNone;
  // Do
  bus.write64(addr, 0x0123456789abcdef);
  // Verify: no error
  EXPECT_EQ(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kNone);
  // Do
  auto ret = bus.read64(addr);
  // Verify: no error
  EXPECT_EQ(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kNone);
  EXPECT_EQ(ret, 0x0123456789abcdef);
}
