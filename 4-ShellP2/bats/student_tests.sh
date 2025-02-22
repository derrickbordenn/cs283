#!/usr/bin/env bats

# Basic command execution tests
@test "Execute ls" {
    run ./dsh <<EOF
ls
EOF
    [ "$status" -eq 0 ]
}

# Built-in cd command tests
@test "cd with no arguments does nothing" {
    current_dir=$(pwd)
    run ./dsh <<EOF
cd
pwd
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "$current_dir" ]]
}

@test "cd to specific directory works" {
    run ./dsh <<EOF
cd /tmp
pwd
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "/tmp" ]]
}

@test "cd to invalid directory shows error" {
    run ./dsh <<EOF
cd /nonexistent_directory
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "No such file or directory" ]]
}

# Quote handling tests
@test "handle quoted arguments" {
    run ./dsh <<EOF
echo "  hello    world  "
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "  hello    world  " ]]
}

@test "handle multiple quoted arguments" {
    run ./dsh <<EOF
echo "first quote" "second quote"
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "first quote second quote" ]]
}

# Extra credit: Return code handling
@test "rc command returns 0 after successful command" {
    run ./dsh <<EOF
true
rc
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "0" ]]
}

@test "rc command shows failure after failed command" {
    run ./dsh <<EOF
nonexistent_command
rc
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "Command not found in PATH" ]]
    [[ "$output" =~ "2" ]]
}

@test "rc command shows permission denied" {
    run ./dsh <<EOF
touch testfile
chmod 000 testfile
./testfile
rc
rm -f testfile
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "Permission denied" ]]
    [[ "$output" =~ "13" ]]
}

# Command line parsing tests
@test "handle empty command" {
    run ./dsh <<EOF

EOF
    [ "$status" -eq 0 ]
}

@test "handle multiple spaces between arguments" {
    run ./dsh <<EOF
echo    test    argument
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "test argument" ]]
}

# Exit command test
@test "exit command works" {
    run ./dsh <<EOF
exit
EOF
    [ "$status" -eq 0 ]
}
