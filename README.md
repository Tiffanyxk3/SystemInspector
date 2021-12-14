# System Inspector

## Description
- A Unix utility that inspects the system it runs on and creates a summarized report for the user. Similar to the `top` command from a shell.

## Usage
```bash
make
./inspector
```
Command options:
```
Usage: ./inspector [-ho] [-i interval] [-p procfs_dir]

Options:
    * -h              Display help/usage information
    * -i interval     Set the update interval (default: 1000ms)
    * -p procfs_dir   Set the expected procfs mount point (default: /proc)
    * -o              Operate in one-shot mode (no curses or live updates)
```

## Included files

- `Makefile`: Used to compile and run the program.
- inspector.c: Driver of the program.
- procfs.c: Getting system information.
- display.c: Displaying the system information.
- util.c: Includes helper functions for other files.
- Header files of procfs.c, display.c, util.c.

## Testing

To compile and run:
```
make
./inspector
```

To execute the test cases, use `make test`. To pull in updated test cases, run `make testupdate`. You can also run a specific test case instead of all of them:

```
# Run all test cases:
make test

# Run a specific test case:
make test run=4

# Run a few specific test cases (4, 8, and 12 in this case):
make test run='4 8 12'
```
