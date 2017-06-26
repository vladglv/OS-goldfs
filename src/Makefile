# Compiler options
CC=clang -g -O0 -fPIC -D_XOPEN_SOURCE=700 -std=c11 -Wmain -W -Wall -Wthread-safety -pedantic -pedantic-errors
CCS=-fsanitize=address,undefined,leak

# Executable names
EXECUTABLE1=sfs_t1
EXECUTABLE1_VG=sfs_t1_vg
EXECUTABLE2=sfs_t2
EXECUTABLE2_VG=sfs_t2_vg

# Sources
SOURCES_TEST1= disk_emu.c sfs_api.c sfs_test1.c tests.c
SOURCES_TEST2= disk_emu.c sfs_api.c sfs_test2.c tests.c

# Targets
all:
	$(CC) $(CCS) -o $(EXECUTABLE1) $(SOURCES_TEST1)
	$(CC) -o $(EXECUTABLE1_VG) $(SOURCES_TEST1)
	$(CC) $(CCS) -o $(EXECUTABLE2) $(SOURCES_TEST2)
	$(CC) -o $(EXECUTABLE2_VG) $(SOURCES_TEST2)

clean:
	rm $(EXECUTABLE1) $(EXECUTABLE1_VG) $(EXECUTABLE2) $(EXECUTABLE2_VG)
