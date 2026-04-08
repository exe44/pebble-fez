typedef struct PolyPath
{
  const uint8_t *point_idxs;
  uint8_t point_count;
} PolyPath;

typedef struct DigitPolyData
{
  const PolyPath *contours;
  uint8_t contour_count;
  const PolyPath *solid_polys;
  uint8_t solid_poly_count;
} DigitPolyData;

static const GPoint digit_poly_points[] = {
  { 0,  0 }, { 10,  0 }, { 20,  0 }, { 30,  0 },
  { 0, 10 }, { 10, 10 }, { 20, 10 }, { 30, 10 },
  { 0, 20 }, { 10, 20 }, { 20, 20 }, { 30, 20 },
  { 0, 30 }, { 10, 30 }, { 20, 30 }, { 30, 30 },
  { 0, 40 }, { 10, 40 }, { 20, 40 }, { 30, 40 }
};

// 16, 17, 18, 19
// 12, 13, 14, 15
//  8,  9, 10, 11
//  4,  5,  6,  7
//  0,  1,  2,  3


// 16-------19
// |  13-14  |
// |  |   |  |
// |  5---6  |
// 0---------3
static const uint8_t contour_0_outer[] = { 0, 3, 19, 16 };
static const uint8_t contour_0_inner[] = { 5, 6, 14, 13 };
static const PolyPath digit_contours_0[] = {
  { contour_0_outer, ARRAY_LENGTH(contour_0_outer) },
  { contour_0_inner, ARRAY_LENGTH(contour_0_inner) },
};
static const uint8_t poly_0_a[] = { 0, 1, 17, 16 };
static const uint8_t poly_0_b[] = { 0, 3, 19, 16, 12, 14, 6, 4 };
static const PolyPath digit_solids_0[] = {
  { poly_0_a, ARRAY_LENGTH(poly_0_a) },
  { poly_0_b, ARRAY_LENGTH(poly_0_b) },
};

//    17-18
//    |   |
//    |   |
//    |   |
//    1---2
static const uint8_t contour_1[] = { 1, 2, 18, 17 };
static const PolyPath digit_contours_1[] = {
  { contour_1, ARRAY_LENGTH(contour_1) },
};
static const uint8_t poly_1_a[] = { 1, 2, 18, 17 };
static const PolyPath digit_solids_1[] = {
  { poly_1_a, ARRAY_LENGTH(poly_1_a) },
};

// 16-------19
// 12----14  |
// 8-----10-11
// |      6--7
// 0---------3
static const uint8_t contour_2[] = { 0, 3, 7, 6, 10, 11, 19, 16, 12, 14, 10, 8 };
static const PolyPath digit_contours_2[] = {
  { contour_2, ARRAY_LENGTH(contour_2) },
};
static const uint8_t poly_2_a[] = { 0, 3, 7, 6, 10, 11, 19, 16, 12, 14, 10, 8 };
static const PolyPath digit_solids_2[] = {
  { poly_2_a, ARRAY_LENGTH(poly_2_a) },
};

// 16-------19
// 12-13     |
//    9--10  |
// 4------6  |
// 0---------3
static const uint8_t contour_3[] = { 0, 3, 19, 16, 12, 13, 9, 10, 6, 4 };
static const PolyPath digit_contours_3[] = {
  { contour_3, ARRAY_LENGTH(contour_3) },
};
static const uint8_t poly_3_a[] = { 0, 3, 19, 16, 12, 13, 9, 10, 6, 4 };
static const PolyPath digit_solids_3[] = {
  { poly_3_a, ARRAY_LENGTH(poly_3_a) },
};

// 16-17 18-19
// |  13-14  |
// 8-----10  |
//        |  |
//        2--3
static const uint8_t contour_4[] = { 2, 3, 19, 18, 14, 13, 17, 16, 8, 10 };
static const PolyPath digit_contours_4[] = {
  { contour_4, ARRAY_LENGTH(contour_4) },
};
static const uint8_t poly_4_a[] = { 2, 3, 19, 18, 14, 13, 17, 16, 8, 10 };
static const PolyPath digit_solids_4[] = {
  { poly_4_a, ARRAY_LENGTH(poly_4_a) },
};

// 16-------19
// |     14-15
// 8-----10-11
// 4------6  |
// 0---------3
static const uint8_t contour_5[] = { 0, 3, 11, 10, 14, 15, 19, 16, 8, 10, 6, 4 };
static const PolyPath digit_contours_5[] = {
  { contour_5, ARRAY_LENGTH(contour_5) },
};
static const uint8_t poly_5_a[] = { 0, 3, 11, 10, 14, 15, 19, 16, 8, 10, 6, 4 };
static const PolyPath digit_solids_5[] = {
  { poly_5_a, ARRAY_LENGTH(poly_5_a) },
};

// 16-17
// |  |
// |  9-----11
// |         |
// 0---------3
static const uint8_t contour_6[] = { 0, 3, 11, 9, 17, 16 };
static const PolyPath digit_contours_6[] = {
  { contour_6, ARRAY_LENGTH(contour_6) },
};
static const uint8_t poly_6_a[] = { 0, 3, 11, 9, 17, 16 };
static const PolyPath digit_solids_6[] = {
  { poly_6_a, ARRAY_LENGTH(poly_6_a) },
};

// 16-------19
// 12----14  |
//    9--10-11
//    |   |
//    1---2
static const uint8_t contour_7[] = { 1, 2, 10, 11, 19, 16, 12, 14, 10, 9 };
static const PolyPath digit_contours_7[] = {
  { contour_7, ARRAY_LENGTH(contour_7) },
};
static const uint8_t poly_7_a[] = { 1, 2, 10, 11, 19, 16, 12, 14, 10, 9 };
static const PolyPath digit_solids_7[] = {
  { poly_7_a, ARRAY_LENGTH(poly_7_a) },
};

// 16-------19
// |         |
// |  9--10  |
// |  5---6  |
// 0---------3
static const uint8_t contour_8_outer[] = { 0, 3, 19, 16 };
static const uint8_t contour_8_inner[] = { 5, 6, 10, 9 };
static const PolyPath digit_contours_8[] = {
  { contour_8_outer, ARRAY_LENGTH(contour_8_outer) },
  { contour_8_inner, ARRAY_LENGTH(contour_8_inner) },
};
static const uint8_t poly_8_a[] = { 0, 3, 15, 14, 6, 5, 13, 12 };
static const uint8_t poly_8_b[] = { 8, 9, 13, 14, 10, 11, 19, 16 };
static const PolyPath digit_solids_8[] = {
  { poly_8_a, ARRAY_LENGTH(poly_8_a) },
  { poly_8_b, ARRAY_LENGTH(poly_8_b) },
};

// 16-------19
// |         |
// 8-----10  |
//        |  |
//        2--3
static const uint8_t contour_9[] = { 2, 3, 19, 16, 8, 10 };
static const PolyPath digit_contours_9[] = {
  { contour_9, ARRAY_LENGTH(contour_9) },
};
static const uint8_t poly_9_a[] = { 2, 3, 19, 16, 8, 10 };
static const PolyPath digit_solids_9[] = {
  { poly_9_a, ARRAY_LENGTH(poly_9_a) },
};

static const DigitPolyData digit_poly_data[] = {
  { digit_contours_0, ARRAY_LENGTH(digit_contours_0), digit_solids_0, ARRAY_LENGTH(digit_solids_0) },
  { digit_contours_1, ARRAY_LENGTH(digit_contours_1), digit_solids_1, ARRAY_LENGTH(digit_solids_1) },
  { digit_contours_2, ARRAY_LENGTH(digit_contours_2), digit_solids_2, ARRAY_LENGTH(digit_solids_2) },
  { digit_contours_3, ARRAY_LENGTH(digit_contours_3), digit_solids_3, ARRAY_LENGTH(digit_solids_3) },
  { digit_contours_4, ARRAY_LENGTH(digit_contours_4), digit_solids_4, ARRAY_LENGTH(digit_solids_4) },
  { digit_contours_5, ARRAY_LENGTH(digit_contours_5), digit_solids_5, ARRAY_LENGTH(digit_solids_5) },
  { digit_contours_6, ARRAY_LENGTH(digit_contours_6), digit_solids_6, ARRAY_LENGTH(digit_solids_6) },
  { digit_contours_7, ARRAY_LENGTH(digit_contours_7), digit_solids_7, ARRAY_LENGTH(digit_solids_7) },
  { digit_contours_8, ARRAY_LENGTH(digit_contours_8), digit_solids_8, ARRAY_LENGTH(digit_solids_8) },
  { digit_contours_9, ARRAY_LENGTH(digit_contours_9), digit_solids_9, ARRAY_LENGTH(digit_solids_9) },
};
