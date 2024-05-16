add_library(vendor
    vendor.c
    vendor_factory_reset_example.c)

target_include_directories(vendor
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE
    ${PROJECT_SOURCE_DIR}/src/
    ${PROJECT_SOURCE_DIR}/src/vendor
    ${PROJECT_SOURCE_DIR}/src/include)
