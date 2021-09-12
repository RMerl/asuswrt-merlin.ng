add_library(common
    common.c common.h)
target_link_libraries(common qrencode)

add_library(rscode
    rscode.c rscode.h)
target_link_libraries(rscode common)

macro(MAKE_TEST test_name)
    set(ADDITIONAL_LIBS "${ARGN}")
    add_executable(${test_name} ${test_name}.c)
    target_link_libraries(${test_name} common ${ADDITIONAL_LIBS})
    add_test(${test_name} ${test_name})
endmacro(MAKE_TEST)

if(TARGET PNG::PNG)
    add_executable(create_frame_pattern create_frame_pattern.c)
    target_link_libraries(create_frame_pattern common PNG::PNG)

    add_executable(create_mqr_frame_pattern create_mqr_frame_pattern.c)
    target_link_libraries(create_mqr_frame_pattern common PNG::PNG)
endif()

if(HAVE_SDL)
    add_executable(view_qrcode view_qrcode.c)
    target_link_libraries(view_qrcode common)
endif(HAVE_SDL)

if(TARGET Threads::Threads)
    if(HAVE_SYS_TIME_H)
        add_definitions(-DHAVE_SYS_TIME_H)
    endif()

    if(HAVE_TIME_H)
        add_definitions(-DHAVE_TIME_H)
    endif()

    add_executable(prof_qrencode prof_qrencode.c)
    target_link_libraries(prof_qrencode common Threads::Threads)

    if(HAVE_PTHREAD_H)
        add_executable(pthread_qrencode pthread_qrencode.c)
        target_link_libraries(pthread_qrencode common Threads::Threads)
    endif()
endif()

MAKE_TEST(test_bitstream)
MAKE_TEST(test_estimatebit)
MAKE_TEST(test_split)

if(TARGET ICONV::ICONV)
    add_library(decoder STATIC
        decoder.c decoder.h 
        datachunk.c datachunk.h 
        rsecc_decoder.c rsecc_decoder.h)
    target_link_libraries(decoder ICONV::ICONV)

    MAKE_TEST(test_qrinput decoder)
    MAKE_TEST(test_qrspec decoder)
    target_compile_definitions(test_qrspec PRIVATE -DSRCDIR="${CMAKE_CURRENT_SOURCE_DIR}/")
    MAKE_TEST(test_mqrspec decoder)
    MAKE_TEST(test_qrencode decoder)
    MAKE_TEST(test_split_urls decoder)
    MAKE_TEST(test_monkey decoder)

    include(CheckCSourceCompiles)
    check_c_source_compiles(
        "int main(){
            const int w = 1;
            char buf[w];
            return 0;
        }"
        FIXED_SIZE_BUFFER_INITIALIZATION)
    
    if(FIXED_SIZE_BUFFER_INITIALIZATION)
        MAKE_TEST(test_mask decoder)
        MAKE_TEST(test_mmask decoder)
        MAKE_TEST(test_rs rscode decoder)
    endif()
endif()

