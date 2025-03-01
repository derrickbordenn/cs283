#!/usr/bin/env bats

# Basic command execution tests
@test "Simple command execution works" {
    run ./dsh <<EOF
echo "Hello World"
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "Hello World" ]]
}

# Built-in command tests
@test "Exit command works" {
    run ./dsh <<EOF
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "exiting" ]]
}

@test "cd command with valid path works" {
    run ./dsh <<EOF
cd /tmp
pwd
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "/tmp" ]]
}

@test "cd command with invalid path shows error" {
    run ./dsh <<EOF
cd /nonexistent_directory
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "No such file or directory" ]]
}

@test "rc command shows last return code" {
    run ./dsh <<EOF
true
rc
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "0" ]]
}

# Basic pipe tests
@test "Simple pipe with two commands works" {
    run ./dsh <<EOF
echo "hello world" | grep hello
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "hello world" ]]
}

@test "Pipe with non-existent command shows error" {
    run ./dsh <<EOF
echo "test" | nonexistent_command
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "Command not found" ]]
}

@test "Multiple pipes work" {
    run ./dsh <<EOF
echo -e "line1\nline2\nline3" | grep line | grep 2
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "line2" ]]
}

@test "Pipe with ls and grep works" {
    run ./dsh <<EOF
ls | grep ".c"
EOF
    [ "$status" -eq 0 ]
    # Check that at least one .c file is found
    [[ "$output" =~ ".c" ]]
}

@test "Pipe with three commands works" {
    run ./dsh <<EOF
echo "hello world" | wc -w | grep 2
EOF
    [ "$status" -eq 0 ]
    # The word count of "hello world" is 2, so grep should find "2" in the output
    [[ "$output" =~ "2" ]]
}
