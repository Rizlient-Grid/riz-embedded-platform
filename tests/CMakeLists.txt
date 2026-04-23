include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(task_test coro/task_test.cpp)
target_link_libraries(task_test GTest::gtest_main)
target_include_directories(task_test PRIVATE ${PROJECT_SOURCE_DIR}/include)

add_executable(schedulable_task_test coro/schedulable_task_test.cpp)
target_link_libraries(schedulable_task_test GTest::gtest_main riz)
target_include_directories(schedulable_task_test PRIVATE ${PROJECT_SOURCE_DIR}/include)

add_executable(delta_queue_test container/delta_queue_test.cpp)
target_link_libraries(delta_queue_test GTest::gtest_main riz)
target_include_directories(delta_queue_test PRIVATE ${PROJECT_SOURCE_DIR}/include)

add_executable(fifo_queue_test container/fifo_queue_test.cpp)
target_link_libraries(fifo_queue_test GTest::gtest_main riz)
target_include_directories(fifo_queue_test PRIVATE ${PROJECT_SOURCE_DIR}/include)

include(GoogleTest)
gtest_discover_tests(task_test)
gtest_discover_tests(schedulable_task_test)
gtest_discover_tests(delta_queue_test)
gtest_discover_tests(fifo_queue_test)
