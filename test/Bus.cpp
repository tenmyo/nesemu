#include "nes_emu/Bus.h"
#include "nes_emu/Device/Sram.h"

#include <gtest/gtest.h>

#include <array>
#include <cstring>

using namespace nes_emu;
namespace {
uint_fast16_t gaddr;
BusAccessKind gop;
void errorCallback(uint_fast16_t addr, BusAccessKind op) {
  gaddr = addr;
  gop = op;
}
} // namespace

TEST(Bus16Test, ReadForNotRegisterd) {
  Bus16 bus(errorCallback);
  uint8_t buf[2 * 1024];
  uint_fast16_t addr = 0x100;
  gaddr = 0;
  gop = BusAccessKind::kNone;
  bus.read(addr, 1, buf);
  EXPECT_EQ(gaddr, addr);
  EXPECT_EQ(gop, BusAccessKind::kRead);
}
TEST(Bus16Test, ReadForRegisterdOne) {
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
  bus.read(addr, bytes, buf);
  EXPECT_EQ(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kNone);
  EXPECT_EQ(std::memcmp(buf, p, bytes), 0);
  EXPECT_NE(std::memcmp(buf + bytes, p + bytes, 1), 0);
}
TEST(Bus16Test, MapToExists) {
  Bus16 bus(errorCallback);
  Sram<1024> sram;
  EXPECT_FALSE(sram.map(&bus, 0));
  auto ret = sram.map(&bus, 0);
  EXPECT_TRUE(ret);
  EXPECT_EQ(ret.value(), std::errc::file_exists);
}
TEST(Bus16Test, MapToMiddle) {
  Bus16 bus(errorCallback);
  Sram<512> sram;
  auto p = sram.data();
  memset(p, 0xce, sram.size());
  EXPECT_FALSE(sram.map(&bus, 256));
  uint8_t buf[1 * 1024] = {0};
  uint_fast16_t addr = 256;
  uint_fast16_t bytes = 511;
  gaddr = 0;
  gop = BusAccessKind::kNone;
  bus.read(addr, bytes, buf);
  EXPECT_EQ(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kNone);
  EXPECT_EQ(std::memcmp(buf, p, bytes), 0);
  EXPECT_NE(std::memcmp(buf + bytes, p + bytes, 1), 0);
}
TEST(Bus16Test, ReadOver) {
  Bus16 bus(errorCallback);
  Sram<0x200> sram;
  auto p = sram.data();
  memset(p, 0xce, sram.size());
  EXPECT_FALSE(sram.map(&bus, 0x100));
  uint8_t buf[1 * 0x400] = {0};
  uint_fast16_t addr = 0x100;
  uint_fast16_t bytes = 0x300;
  gaddr = 0;
  gop = BusAccessKind::kNone;
  bus.read(addr, bytes, buf);
  EXPECT_NE(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kRead);
}
TEST(Bus16Test, ReadAcrossTwoMap) {
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
  bus.read(addr, bytes, buf);
  EXPECT_EQ(gaddr, 0);
  EXPECT_EQ(gop, BusAccessKind::kNone);
  EXPECT_EQ(std::memcmp(buf, p1, 0x200), 0);
  EXPECT_EQ(std::memcmp(buf + 0x200, p2, 0x200), 0);
  EXPECT_EQ(buf[0x200 + 0x200], 0);
}
