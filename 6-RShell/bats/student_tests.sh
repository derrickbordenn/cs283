#!/usr/bin/env bats

########################################################################################
# File: student_tests.sh
# 
# Student tests for the remote shell client and server implementation
# These tests verify both the client and server components function correctly
########################################################################################

# Setup function to prepare the environment before each test
setup() {
    # Create any necessary test files
    echo "test content" > test_file.txt 2>/dev/null || true
}

# Teardown function to clean up after each test
teardown() {
    # Kill any remaining server processes
    pkill -f "./dsh -s" || true
    
    # Clean up test files
    rm -f test_file.txt output.txt
    rm -rf test_dir
}

@test "Remote: Basic server startup and client connection" {
    # Start the server in background
    ./dsh -s > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1
    
    # Run client with exit command
    echo "exit" | ./dsh -c 127.0.0.1:1234 > client_output.txt 2>&1
    
    # Verify connection was successful
    grep -q "dsh4>" client_output.txt
    
    # Clean up
    rm -f client_output.txt
    kill $SERVER_PID || true
}

@test "Remote: Simple command execution" {
    # Start the server in background
    ./dsh -s > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1
    
    # Run echo test command
    echo "echo test" | ./dsh -c 127.0.0.1:1234 > client_output.txt 2>&1
    
    # Verify output contains the expected result
    grep -q "test" client_output.txt
    
    # Clean up
    rm -f client_output.txt
    kill $SERVER_PID || true
}

@test "Remote: Pipe command execution" {
    # Start the server in background
    ./dsh -s > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1
    
    # Run pipe command
    echo "echo hello | grep hello" | ./dsh -c 127.0.0.1:1234 > client_output.txt 2>&1
    
    # Verify output contains the expected result
    grep -q "hello" client_output.txt
    
    # Clean up
    rm -f client_output.txt
    kill $SERVER_PID || true
}

@test "Remote: Multiple commands in sequence" {
    # Start the server in background
    ./dsh -s > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1
    
    # Create a temporary script to feed multiple commands
    cat > cmd_script.txt << EOF
echo first
echo second
exit
EOF
    
    # Run multiple commands
    cat cmd_script.txt | ./dsh -c 127.0.0.1:1234 > client_output.txt 2>&1
    
    # Verify output contains both expected results
    grep -q "first" client_output.txt
    grep -q "second" client_output.txt
    
    # Clean up
    rm -f client_output.txt cmd_script.txt
    kill $SERVER_PID || true
}

@test "Remote: Command with complex pipe" {
    # Start the server in background
    ./dsh -s > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1
    
    # Run complex pipe command
    echo "ls -la | grep -i dsh | wc -l" | ./dsh -c 127.0.0.1:1234 > client_output.txt 2>&1
    
    # Verify output contains a number (count result from wc)
    grep -q "[0-9]" client_output.txt
    
    # Clean up
    rm -f client_output.txt
    kill $SERVER_PID || true
}

@test "Remote: Error handling for nonexistent command" {
    # Start the server in background
    ./dsh -s > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1
    
    # Run nonexistent command
    echo "nonexistent_command" | ./dsh -c 127.0.0.1:1234 > client_output.txt 2>&1
    
    # Verify error message is returned
    grep -q "not found\|No such file" client_output.txt || [ -s client_output.txt ]
    
    # Clean up
    rm -f client_output.txt
    kill $SERVER_PID || true
}

@test "Remote: Long output handling" {
    # Start the server in background
    ./dsh -s > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1
    
    # Run command that generates long output
    echo "ls -la / | cat" | ./dsh -c 127.0.0.1:1234 > client_output.txt 2>&1
    
    # Verify output is substantial (more than minimal size)
    [ $(wc -c < client_output.txt) -gt 100 ]
    
    # Clean up
    rm -f client_output.txt
    kill $SERVER_PID || true
}

@test "Remote: Server termination with stop-server command" {
    # Start the server in background
    ./dsh -s > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1
    
    # Store the PID
    original_pid=$SERVER_PID
    
    # Send stop-server command
    echo "stop-server" | ./dsh -c 127.0.0.1:1234 > /dev/null 2>&1
    
    # Wait a moment for server to terminate
    sleep 1
    
    # Check if server process is still running
    ! ps -p $original_pid > /dev/null 2>&1
}

@test "Remote: Client exit with exit command" {
    # Start the server in background
    ./dsh -s > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1
    
    # Send exit command and capture exit code
    echo "exit" | ./dsh -c 127.0.0.1:1234 > /dev/null 2>&1
    EXIT_CODE=$?
    
    # Verify client exited successfully
    [ $EXIT_CODE -eq 0 ]
    
    # Server should still be running
    ps -p $SERVER_PID > /dev/null 2>&1
    
    # Clean up
    kill $SERVER_PID || true
}

@test "Remote: Custom port specification" {
    # Use a non-default port
    CUSTOM_PORT=4567
    
    # Start the server on custom port
    ./dsh -s 127.0.0.1:$CUSTOM_PORT > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1
    
    # Connect client to custom port
    echo "echo custom_port_test" | ./dsh -c 127.0.0.1:$CUSTOM_PORT > client_output.txt 2>&1
    
    # Verify connection and command execution was successful
    grep -q "custom_port_test" client_output.txt
    
    # Clean up
    rm -f client_output.txt
    kill $SERVER_PID || true
}
