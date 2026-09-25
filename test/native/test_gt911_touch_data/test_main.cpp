#include <unity.h>

#include "board/gt911_touch_data.h"

namespace {

void test_decodes_first_point_record_from_track_id() {
  const board::Gt911PointData point{
      7,
      0x23,
      0x01,
      0xD2,
      0x00,
      0x34,
      0x12,
      0,
  };

  TEST_ASSERT_EQUAL_HEX16(0x814F, board::kGt911FirstPointRegister);
  TEST_ASSERT_EQUAL_UINT16(291, board::gt911X(point));
  TEST_ASSERT_EQUAL_UINT16(210, board::gt911Y(point));
}

}  // namespace

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_decodes_first_point_record_from_track_id);
  return UNITY_END();
}
