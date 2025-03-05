#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

# these are some helper functions to help with the remote tests
setup_remote_server() {
  port="$1"
  mode="$2"  # "single" or "multi"
  if [ "$mode" = "multi" ]; then
    ./dsh -s -x -i 127.0.0.1 -p "$port" &
  else
    ./dsh -s -i 127.0.0.1 -p "$port" &
  fi
  SERVER_PID=$!
  sleep 1
}

teardown_remote_server() {
  # attempt a graceful exit
  echo "stop-server" | ./dsh -c -i 127.0.0.1 -p "$port" || true

  # force-kill anything left over server process wise
  kill -9 "$SERVER_PID" 2>/dev/null || true
  wait "$SERVER_PID" 2>/dev/null || true
  return 0
}

@test "[ LOCAL MODE ] Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "[ LOCAL MODE ] Runs regular command" {
    run "./dsh" <<EOF
echo hello
EOF

    # Remove all whitespace from output
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    # Format the expected output similarly (no spaces/newlines):
    expected_output="hellolocalmodedsh4>dsh4>cmdloopreturned0"

    # Debug prints (only visible when the test fails)
    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    # Compare
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "[ LOCAL MODE ] Runs built-in command" {
    run "./dsh" <<EOF
cd /tmp
pwd
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="/tmplocalmodedsh4>dsh4>dsh4>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "[ LOCAL MODE ] Runs piped command with built-in and path binary" {
    run "./dsh" <<EOF
dragon | wc -l
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="39localmodedsh4>dsh4>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "[ LOCAL MODE ] Handle over pipe limit" {
    run "./dsh" <<EOF
echo 1 | echo 2 | echo 3 | echo 4 | echo 5 | echo 6 | echo 7 | echo 8 | echo 9
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="localmodedsh4>error:pipinglimitedto8commandsdsh4>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "[ LOCAL MODE ] Works pipe limit max" {
    run "./dsh" <<EOF
echo "result" | cat | cat | cat | cat | cat | cat | cat

EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="resultlocalmodedsh4>dsh4>warning:nocommandsprovideddsh4>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "[ LOCAL MODE ] Handle output redirection AND appending" {
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
    expected_output="testingtestingtestingagainlocalmodedsh4>dsh4>dsh4>dsh4>dsh4>dsh4>dsh4>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "[ LOCAL MODE ] Handle output redirection" {
    run "./dsh" <<EOF
cd /tmp
echo "hello world" > out.txt
cat out.txt
rm out.txt
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="helloworldlocalmodedsh4>dsh4>dsh4>dsh4>dsh4>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "[ LOCAL MODE ] Handle input redirection" {
    run "./dsh" <<EOF
cd /tmp
echo "input works" > in.txt
grep "input works" < in.txt
rm in.txt
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="inputworkslocalmodedsh4>dsh4>dsh4>dsh4>dsh4>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "[ LOCAL MODE ] Exit command" {
    run "./dsh" <<EOF                
exit
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="localmodedsh4>cmdloopreturned0"

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

@test "[ LOCAL MODE ] List files in cwd" {
    run "./dsh" <<EOF                
ls
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output=$(echo $(ls) | tr -d '[:space:]')"localmodedsh4>dsh4>cmdloopreturned0"

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

@test "[ LOCAL MODE ] Change directory (cd)" {
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
    expected_output="/tmp/testlocalmodedsh4>dsh4>dsh4>dsh4>dsh4>cmdloopreturned0"

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

@test "[ LOCAL MODE ] Move up directory (cd ..)" {
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
    expected_output="/tmp/test/tmplocalmodedsh4>dsh4>dsh4>dsh4>dsh4>dsh4>dsh4>cmdloopreturned0"

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

@test "[ LOCAL MODE ] Unknown command and return code" {
    # Run the commands within the dsh shell
    run "./dsh" <<EOF
not_exists
rc
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="localmodedsh4>localmodedsh4>CommandnotfoundinPATHdsh4>2dsh4>cmdloopreturned0"

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

@test "[ LOCAL MODE ] Permission denied error and return code" {
    # Create a file without execute permissions
    touch /tmp/no_exec_file
    chmod 644 /tmp/no_exec_file  # Remove execute permissions

    run "./dsh" <<EOF
/tmp/no_exec_file
rc
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="localmodedsh4>localmodedsh4>Permissiondeniedtoexecutecommanddsh4>13dsh4>cmdloopreturned0"

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

@test "[ LOCAL MODE ] General execution failure and return code" {
    # Create a directory (trying to execute it should fail)
    mkdir -p /tmp/test_dir

    run "./dsh" <<EOF
/tmp/test_dir
rc
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="localmodedsh4>localmodedsh4>Permissiondeniedtoexecutecommanddsh4>13dsh4>cmdloopreturned0"

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

@test "[ LOCAL MODE ] Ensure PATH binaries are found (ls)" {
    # Create test_dir
    mkdir -p /tmp/test_dir

    # Create test.txt inside test_dir
    touch /tmp/test_dir/test.txt

    # Run ls and check for test.txt
    run ls /tmp/test_dir

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')
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

@test "[ LOCAL MODE ] Can boot shell in shell" {
    run "./dsh" <<EOF
exit
EOF

    # Expected prompt in output (assuming "dsh2>" is the prompt)
    expected_prompt="dsh4>"

    # Print debugging information for failed tests
    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"

    # Ensure the expected prompt is in the output
    [[ "$output" == *"$expected_prompt"* ]]

    # Assert that the exit status is 0 (successful execution)
    [ "$status" -eq 0 ]
}


@test "[ REMOTE MODE - Single Threaded ] Built-in command over server" {
    port=8888
    setup_remote_server "$port" "single"

    run ./dsh -c -i 127.0.0.1 -p "$port" <<EOF
cd /tmp
pwd
exit
EOF

    teardown_remote_server

    stripped_output=$(echo "$output" | tr -d '[:space:]\004')

    # Updated expected output based on failure output:
    expected_output="socketclientmode:addr:127.0.0.1:8888dsh4>dsh4>/tmpdsh4>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "[ REMOTE MODE - Single Threaded ] Regular command over server" {
    port=8888
    setup_remote_server "$port" "single"

    run ./dsh -c -i 127.0.0.1 -p "$port" <<EOF
echo hello
exit
EOF

    teardown_remote_server

    stripped_output=$(echo "$output" | tr -d '[:space:]\004')
    # Updated expected output based on failure output:
    expected_output="socketclientmode:addr:127.0.0.1:8888dsh4>hellodsh4>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "[ REMOTE MODE - Single Threaded ] Pipeline command over server" {
    port=8888
    setup_remote_server "$port" "single"

    run ./dsh -c -i 127.0.0.1 -p "$port" <<EOF
dragon | wc -l
exit
EOF

    teardown_remote_server

    stripped_output=$(echo "$output" | tr -d '[:space:]\004')
    # Updated expected output based on failure output:
    expected_output="socketclientmode:addr:127.0.0.1:8888dsh4>42dsh4>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "[ REMOTE MODE - Multi-threaded ] Built-in command over server" {
    port=8888
    setup_remote_server "$port" "multi"

    run ./dsh -c -i 127.0.0.1 -p "$port" <<EOF
cd /tmp
pwd
exit
EOF

    teardown_remote_server

    stripped_output=$(echo "$output" | tr -d '[:space:]\004')

    # Updated expected output based on failure output:
    expected_output="socketclientmode:addr:127.0.0.1:8888dsh4>dsh4>/tmpdsh4>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "[ REMOTE MODE - Multi-threaded ] Regular command over server" {
    port=8888
    setup_remote_server "$port" "multi"

    run ./dsh -c -i 127.0.0.1 -p "$port" <<EOF
echo hello
exit
EOF

    teardown_remote_server

    stripped_output=$(echo "$output" | tr -d '[:space:]\004')
    # Updated expected output based on failure output:
    expected_output="socketclientmode:addr:127.0.0.1:8888dsh4>hellodsh4>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "[ REMOTE MODE - Multi-threaded ] Pipeline command over server" {
    port=8888
    setup_remote_server "$port" "multi"

    run ./dsh -c -i 127.0.0.1 -p "$port" <<EOF
dragon | wc -l
exit
EOF

    teardown_remote_server

    stripped_output=$(echo "$output" | tr -d '[:space:]\004')
    # Updated expected output based on failure output:
    expected_output="socketclientmode:addr:127.0.0.1:8888dsh4>42dsh4>cmdloopreturned0"

    echo "Captured stdout:"
    echo "$output"
    echo "$stripped_output -> $expected_output"

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "[ REMOTE MODE - Multi-threaded ] Two clients can connect concurrently" {
  port=9999
  setup_remote_server "$port" "multi"

  # Create temporary files for output.
  tmp1=$(mktemp)
  tmp2=$(mktemp)

  # Launch two client sessions concurrently.
  (
    ./dsh -c -i 127.0.0.1 -p "$port" <<EOF
echo client1
exit
EOF
  ) > "$tmp1" 2>&1 &
  pid1=$!

  (
    ./dsh -c -i 127.0.0.1 -p "$port" <<EOF
echo client2
exit
EOF
  ) > "$tmp2" 2>&1 &
  pid2=$!

  wait $pid1
  wait $pid2

  output1=$(cat "$tmp1")
  output2=$(cat "$tmp2")
  rm "$tmp1" "$tmp2"

  # Strip whitespace and the EOF marker (0x04) from outputs.
  stripped1=$(echo "$output1" | tr -d '[:space:]\004')
  stripped2=$(echo "$output2" | tr -d '[:space:]\004')

  # Updated expected outputs based on failure output.
  expected1="socketclientmode:addr:127.0.0.1:${port}dsh4>client1dsh4>cmdloopreturned0"
  expected2="socketclientmode:addr:127.0.0.1:${port}dsh4>client2dsh4>cmdloopreturned0"

  echo "Client1 output:"; echo "$stripped1" | od -c
  echo "Client2 output:"; echo "$stripped2" | od -c

  [ "$stripped1" = "$expected1" ]
  [ "$stripped2" = "$expected2" ]

  teardown_remote_server
}

@test "[ REMOTE MODE - Multi-threaded ] Shutdown by one client prevents later connections" {
  port=9998
  setup_remote_server "$port" "multi"

  # Client 1 sends the stop-server command.
  run ./dsh -c -i 127.0.0.1 -p "$port" <<EOF
stop-server
exit
EOF

  # Give the server a moment to shut down.
  sleep 1

  # Client 2 attempts to connect and execute a command.
  run ./dsh -c -i 127.0.0.1 -p "$port" <<EOF
echo test
exit
EOF

  # Strip whitespace and the EOF marker.
  stripped_output=$(echo "$output" | tr -d '[:space:]\004')

  echo "Captured output from client2:"; echo "$stripped_output" | od -c

  # In our implementation, a failed connection shows an error
  [[ "$stripped_output" == *"cmdloopreturned-52"* ]]

  teardown_remote_server
}
