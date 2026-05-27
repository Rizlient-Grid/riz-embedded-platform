add_library(riz_core OBJECT
    container/detail/ring_queue.cpp
    container/detail/lockfree/spsc_byte_ring_queue.cpp
    container/intrusive/delta_queue.cpp
    container/intrusive/fifo_queue.cpp
    coro/execution/scheduler.cpp
    timer/timer_service.cpp
)

target_include_directories(riz_core PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

add_library(riz
    $<TARGET_OBJECTS:riz_core>
    ${PROJECT_SOURCE_DIR}/src/port/${RIZ_HAL}/tick_source.cpp
)

target_include_directories(riz PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

add_library(riz_test
    $<TARGET_OBJECTS:riz_core>
    ${PROJECT_SOURCE_DIR}/src/port/mock/tick_source.cpp
)

target_include_directories(riz_test PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
