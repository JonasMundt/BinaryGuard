
# Execute BinaryGuard and verify its result.

execute_process(
    COMMAND "${PROGRAM}" "${INPUT}"
    RESULT_VARIABLE actual_exit
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr
)

# Verify exit code
if(NOT "${actual_exit}" STREQUAL "${EXPECT_EXIT}")
    message(FATAL_ERROR
        "Unexpected exit code: ${actual_exit}\n"
        "Expected: ${EXPECT_EXIT}\n"
        "Output: ${stdout}${stderr}"
    )
endif()

# Verify expected output
set(output "${stdout}${stderr}")

string(FIND "${output}" "${EXPECT_TEXT}" position)

if(position EQUAL -1)
    message(FATAL_ERROR
        "Expected text not found: ${EXPECT_TEXT}\n"
        "Actual output: ${output}"
    )
endif()

message(STATUS "CLI test passed")
