#pragma once

#include <stdint.h>

typedef int64_t i64;

i64 init_fresh_disk(char *filename, i64 block_size, i64 num_blocks);
i64 init_disk(char *filename, i64 block_size, i64 num_blocks);
i64 read_blocks(i64 start_address, i64 nblocks, void *buffer);
i64 write_blocks(i64 start_address, i64 nblocks, const void *buffer);
i64 close_disk(void);
