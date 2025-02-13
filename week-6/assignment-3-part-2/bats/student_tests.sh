#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file
@test "Exit command" {
    run "./dsh" <<EOF                
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="dsh2>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    # if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "List files in cwd" {
    run "./dsh" <<EOF                
ls
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output=$(echo $(ls) | tr -d '[:space:]')"dsh2>dsh2>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    # if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Change directory (cd)" {
    # Run the commands within the dsh shell
    run "./dsh" <<EOF
mkdir /tmp/test
cd /tmp/test
pwd
rmdir /tmp/test
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="/tmp/testdsh2>dsh2>dsh2>dsh2>dsh2>cmdloopreturned0"

    # Print debugging information for failed tests
    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check for exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assert the exit status is 0 (success)
    [ "$status" -eq 0 ]

}

@test "Unknown command and return code" {
    # Run the commands within the dsh shell
    run "./dsh" <<EOF
not_exists
rc
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="dsh2>CommandnotfoundinPATHdsh2>dsh2>2dsh2>cmdloopreturned0"

    # Print debugging information for failed tests
    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check for exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assert the exit status is 0 (success)
    [ "$status" -eq 0 ]
}

@test "Permission denied error and return code" {
    # Create a file without execute permissions
    touch /tmp/no_exec_file
    chmod 644 /tmp/no_exec_file  # Remove execute permissions

    run "./dsh" <<EOF
/tmp/no_exec_file
rc
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="dsh2>Permissiondeniedtoexecutecommanddsh2>dsh2>13dsh2>cmdloopreturned0"

    # Print debugging information for failed tests
    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check for exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assert the exit status is 0 (success)
    [ "$status" -eq 0 ]

    # Cleanup
    rm /tmp/no_exec_file
}

@test "General execution failure and return code" {
    # Create a directory (trying to execute it should fail)
    mkdir -p /tmp/test_dir

    run "./dsh" <<EOF
/tmp/test_dir
rc
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="dsh2>Permissiondeniedtoexecutecommanddsh2>dsh2>13dsh2>cmdloopreturned0"

    # Print debugging information for failed tests
    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check for exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assert the exit status is 0 (success)
    [ "$status" -eq 0 ]

    # Cleanup
    rmdir /tmp/test_dir
}







