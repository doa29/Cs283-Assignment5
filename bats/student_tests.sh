#!/usr/bin/env bats

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF
ls
EOF
    [ "$status" -eq 0 ]
}

@test "Pipes: ls piped to grep for dshlib.c" {
    run ./dsh <<EOF
ls | grep "dshlib.c"
EOF
    # We check that output contains dshlib.c (case-sensitive)
    echo "$output" | grep -q "dshlib.c"
    [ "$status" -eq 0 ]
}

@test "Redirection: write and read file" {
    # Write to file using redirection
    run ./dsh <<EOF
echo "hello, class" > test_out.txt
cat test_out.txt
EOF
    echo "$output" | grep -q "hello, class"
    [ "$status" -eq 0 ]
}

@test "Append redirection: append text to file" {
    # Ensure file is created first
    run ./dsh <<EOF
echo "line1" > test_out.txt
echo "line2" >> test_out.txt
cat test_out.txt
EOF
    echo "$output" | grep -q "line1"
    echo "$output" | grep -q "line2"
    [ "$status" -eq 0 ]
}

@test "Built-in: cd command" {
    run ./dsh <<EOF
cd /
pwd
EOF
    # Assuming the prompt displays a '/' when in the root directory.
    echo "$output" | grep -q "^/$"
    [ "$status" -eq 0 ]
}
