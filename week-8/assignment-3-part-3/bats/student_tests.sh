#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Runs regular command" {
    run "./dsh" <<EOF
echo hello
EOF

    # Remove all whitespace from output
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    # Format the expected output similarly (no spaces/newlines):
    expected_output="hellodsh3>dsh3>cmdloopreturned0"

    # Debug prints (only visible when the test fails)
    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    # Compare
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Runs built-in command" {
    run "./dsh" <<EOF
cd /tmp
pwd
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="/tmpdsh3>dsh3>dsh3>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Runs piped command with built-in and path binary" {
    run "./dsh" <<EOF
dragon | wc -l
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="38dsh3>dsh3>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Handle over pipe limit" {
    run "./dsh" <<EOF
echo 1 | echo 2 | echo 3 | echo 4 | echo 5 | echo 6 | echo 7 | echo 8 | echo 9
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh3>error:pipinglimitedto8commandsdsh3>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Works pipe limit max" {
    run "./dsh" <<EOF
echo 1 | echo 2 | echo 3 | echo 4 | echo 5 | echo 6 | echo 7 | echo 8
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="8dsh3>dsh3>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Handle output redirection AND appending" {
    run "./dsh" <<EOF
cd /tmp
echo "testing" > out.txt
cat out.txt
echo "testing again" >> out.txt
cat out.txt
rm out.txt
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    # We expect "testing" from the first cat, then "testing\ntesting again" from the second cat.
    expected_output="testingtestingtestingagaindsh3>dsh3>dsh3>dsh3>dsh3>dsh3>dsh3>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Handle output redirection" {
    run "./dsh" <<EOF
cd /tmp
echo "hello world" > out.txt
cat out.txt
rm out.txt
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="helloworlddsh3>dsh3>dsh3>dsh3>dsh3>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Handle input redirection" {
    run "./dsh" <<EOF
cd /tmp
echo "input works" > in.txt
grep "input works" < in.txt
rm in.txt
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="inputworksdsh3>dsh3>dsh3>dsh3>dsh3>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Exit command" {
    run "./dsh" <<EOF                
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="dsh3>cmdloopreturned0"

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
    expected_output=$(echo $(ls) | tr -d '[:space:]')"dsh3>dsh3>cmdloopreturned0"

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
    expected_output="/tmp/testdsh3>dsh3>dsh3>dsh3>dsh3>cmdloopreturned0"

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

@test "Move up directory (cd ..)" {
    # Run the commands within the dsh shell
    run "./dsh" <<EOF
mkdir /tmp/test
cd /tmp/test
pwd
cd ..
pwd
rmdir /tmp/test
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="/tmp/test/tmpdsh3>dsh3>dsh3>dsh3>dsh3>dsh3>dsh3>cmdloopreturned0"

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
    expected_output="dsh3>dsh3>CommandnotfoundinPATHdsh3>2dsh3>cmdloopreturned0"

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
    expected_output="dsh3>dsh3>Permissiondeniedtoexecutecommanddsh3>13dsh3>cmdloopreturned0"

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
    expected_output="dsh3>dsh3>Permissiondeniedtoexecutecommanddsh3>13dsh3>cmdloopreturned0"

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

@test "Ensure PATH binaries are found (ls)" {
    # Create test_dir
    mkdir -p /tmp/test_dir

    # Create test.txt inside test_dir
    touch /tmp/test_dir/test.txt

    # Run ls and check for test.txt
    run ls /tmp/test_dir

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="test.txt"

    # Print debugging information for failed tests
    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check for exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assert the exit status is 0 (success)
    [ "$status" -eq 0 ]

    # Cleanup: Remove the file and directory
    rm -f /tmp/test_dir/test.txt
    rmdir /tmp/test_dir
}

@test "Can boot shell in shell" {
    run "./dsh" <<EOF
exit
EOF

    # Expected prompt in output (assuming "dsh2>" is the prompt)
    expected_prompt="dsh3>"

    # Print debugging information for failed tests
    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"

    # Ensure the expected prompt is in the output
    [[ "$output" == *"$expected_prompt"* ]]

    # Assert that the exit status is 0 (successful execution)
    [ "$status" -eq 0 ]
}
