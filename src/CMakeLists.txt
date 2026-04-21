add_library(riz
    container/intrusive/delta_queue.cpp
    container/intrusive/fifo_queue.cpp
)

target_include_directories(riz PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
