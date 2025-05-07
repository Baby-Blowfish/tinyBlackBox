// test/test_frame.c
// Check 프레임워크를 사용한 Frame 및 FramePool 모듈 단위 테스트
// 아래 테스트 파일은 다양한 시나리오를 검증하기 위해 상세 주석을 포함합니다.

#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <check.h>
#include "frame.h"         // Frame API 인터페이스
#include "frame_pool.h"    // FramePool API 인터페이스

// ================================
// Frame 모듈 테스트
// ================================

// test_frame_init_invalid:
// - frame_init 함수에 NULL 프레임 포인터를 전달할 때
//   1) 반환값이 -1 이어야 하고
//   2) errno가 EINVAL 로 설정되어야 함을 확인합니다.
START_TEST(test_frame_init_invalid) {
    errno = 0;                                        // errno 초기화
    int ret = frame_init(NULL, 1, 1, GRAY);           // NULL 전달
    ck_assert_int_eq(ret, -1);                        // -1 반환 검증
    ck_assert_int_eq(errno, EINVAL);                  // errno == EINVAL 검증
}
END_TEST

// test_frame_create_destroy:
// - frame_create로 동적 할당된 프레임이 NULL이 아니어야 하고,
// - Getter(frame_get_width/height/depth)를 통해 속성이 올바르게 설정되었는지 확인 후
// - frame_destroy로 해제 시 메모리 누수 없이 종료되어야 함.
START_TEST(test_frame_create_destroy) {
    Frame *f = frame_create(4, 3, RGB);               // 가로4, 세로3, RGB(3바이트)
    ck_assert_ptr_ne(f, NULL);                        // 생성된 포인터가 NULL이 아니어야 함

    // Getter를 통한 내부 상태 검증
    ck_assert_uint_eq(frame_get_width(f), 4);         // width == 4
    ck_assert_uint_eq(frame_get_height(f), 3);        // height == 3
    ck_assert_int_eq(frame_get_depth(f), RGB);        // depth == RGB

    frame_destroy(f);                                 // 내부 버퍼+구조체 해제
}
END_TEST


// ================================
// FramePool 모듈 테스트
// ================================

// test_pool_alloc_release:
// - pool 크기가 2 인 상태에서
//   1) fp_alloc() 로 블록 할당 후 refcount 확인
//   2) fp_release() 호출 시 free_list로 블록이 돌아오는지 확인
START_TEST(test_pool_alloc_release) {
    FramePool *p = frame_pool_create(2, 2, 2, GRAY); // pool_size=2, resolution=2x2, GRAY
    ck_assert_ptr_ne(p, NULL);                       // 풀 생성 확인

    FrameBlock *blk = fp_alloc(p, 1);                // init_count=1 로 블록 할당
    ck_assert_ptr_ne(blk, NULL);                     // 블록이 NULL 아니어야 함
    ck_assert_int_eq(fp_get_block_refcount(blk), 1); // refcount == 1 확인

    fp_release(p, blk);                              // 블록 반환
    // 반환 후 사용 가능한 블록 수는 원래 풀 크기(2)여야 함
    ck_assert_int_eq(fp_available_count(p), 2);

    frame_pool_destroy(p);                           // 풀 메모리 해제
}
END_TEST

// test_pool_exhaustion:
// - pool 크기 1 상태에서 두 번째 할당 시도 시
//   1) NULL 반환
//   2) errno == ENOMEM 설정 검증
START_TEST(test_pool_exhaustion) {
    FramePool *p = frame_pool_create(1, 1, 1, GRAY); // pool_size=1
    ck_assert_ptr_ne(p, NULL);

    errno = 0;
    FrameBlock *b1 = fp_alloc(p, 1);                 // 첫 블록 할당
    ck_assert_ptr_ne(b1, NULL);

    FrameBlock *b2 = fp_alloc(p, 1);                 // 두 번째 할당 시도
    ck_assert_ptr_eq(b2, NULL);                      // NULL 반환 확인
    ck_assert_int_eq(errno, ENOMEM);                 // ENOMEM 설정 확인

    frame_pool_destroy(p);
}
END_TEST

// test_pool_retain_multiple:
// - init_count=2로 블록 할당 후
//   1) fp_retain() 호출 시 refcount 증가
//   2) fp_release() 반복 호출 시 refcount 감소 및
//      마지막 release 시 free_list로 반환되는 시점까지 검증
START_TEST(test_pool_retain_multiple) {
    FramePool *p = frame_pool_create(1, 1, 1, GRAY);
    ck_assert_ptr_ne(p, NULL);

    FrameBlock *blk = fp_alloc(p, 2);                // init_count=2
    ck_assert_int_eq(fp_get_block_refcount(blk), 2);

    fp_retain(blk);                                  // refcount -> 3
    ck_assert_int_eq(fp_get_block_refcount(blk), 3);

    // Sequential release
    fp_release(p, blk);                              // prev=3 -> now=2
    ck_assert_int_eq(fp_get_block_refcount(blk), 2);
    fp_release(p, blk);                              // prev=2 -> now=1
    ck_assert_int_eq(fp_get_block_refcount(blk), 1);

    // 마지막 release 시 free_list로 돌아가며 refcount=0
    fp_release(p, blk);                              // prev=1 -> returned
    ck_assert_int_eq(fp_get_block_refcount(blk), 0);

    frame_pool_destroy(p);
}
END_TEST

// test_block_data_ptr:
// - fp_alloc 후 반환된 FrameBlock 내 data 포인터를
//   1) Non-NULL 확인
//   2) 실제 메모리 읽기/쓰기 가능 여부 확인
START_TEST(test_block_data_ptr) {
    const size_t w = 2, h = 3;
    FramePool *p = frame_pool_create(1, w, h, GRAY);
    FrameBlock *blk = fp_alloc(p, 1);
    uint8_t *data = fp_get_block_data(blk);
    ck_assert_ptr_ne(data, NULL);

    size_t total = w * h * 1;
    for (size_t i = 0; i < total; ++i) {
        data[i] = (uint8_t)(i + 10);               // 쓰기
        ck_assert_uint_eq(data[i], (uint8_t)(i + 10)); // 읽기 검증
    }

    frame_pool_destroy(p);
}
END_TEST

// test_pool_counts_and_sizes:
// - 다양한 풀 속성 검증
//   1) fp_total_block_size: 전체 블록 수
//   2) fp_total_bytes_per_frame_size: 블록 당 바이트 크기
//   3) fp_available_count/used_count: 할당 전후 여유/사용 블록 수
START_TEST(test_pool_counts_and_sizes) {
    const size_t pool_size = 4, width = 5, height = 6;
    FramePool *p = frame_pool_create(pool_size, width, height, RGB);
    ck_assert_ptr_ne(p, NULL);

    // 풀 속성 확인
    ck_assert_uint_eq(fp_total_block_size(p), pool_size);
    ck_assert_uint_eq(fp_total_bytes_per_frame_size(p), width * height * RGB);
    ck_assert_uint_eq(fp_available_count(p), pool_size);
    ck_assert_uint_eq(fp_used_count(p), 0);

    // 두 개 블록 할당
    FrameBlock *b1 = fp_alloc(p, 1);
    FrameBlock *b2 = fp_alloc(p, 1);
    ck_assert_uint_eq(fp_available_count(p), pool_size - 2);
    ck_assert_uint_eq(fp_used_count(p), 2);

    // 반환 후 카운트 확인
    fp_release(p, b1);
    fp_release(p, b2);
    ck_assert_uint_eq(fp_available_count(p), pool_size);
    ck_assert_uint_eq(fp_used_count(p), 0);

    frame_pool_destroy(p);
}
END_TEST

// ================================
// 테스트 스위트 및 실행 함수
// ================================
Suite *frame_suite(void) {
    Suite *s = suite_create("FrameModule");          // 스위트 생성
    TCase *tc = tcase_create("Core");                // 테스트 케이스 그룹

    // TEST_CASE 등록 순서
    tcase_add_test(tc, test_frame_init_invalid);
    tcase_add_test(tc, test_frame_create_destroy);
    tcase_add_test(tc, test_pool_alloc_release);
    tcase_add_test(tc, test_pool_exhaustion);
    tcase_add_test(tc, test_pool_retain_multiple);
    tcase_add_test(tc, test_block_data_ptr);
    tcase_add_test(tc, test_pool_counts_and_sizes);

    suite_add_tcase(s, tc);                           // 스위트에 케이스 추가
    return s;
}

int main(void) {
    Suite *s = frame_suite();                         // 스위트 생성 호출
    SRunner *sr = srunner_create(s);                  // 러너 생성
    srunner_run_all(sr, CK_NORMAL);                   // 모든 테스트 실행

    int failures = srunner_ntests_failed(sr);         // 실패 테스트 개수
    srunner_free(sr);                                 // 리소스 해제
    return (failures == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
