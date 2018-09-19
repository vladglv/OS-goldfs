#pragma once

#include <stdint.h>

#define i64 int64_t
#define u64 uint64_t

i64 init_fresh_disk(char *filename, u64 block_size, u64 num_blocks);
i64 init_disk(char *filename, u64 block_size, u64 num_blocks);
i64 read_blocks(u64 start_address, u64 nblocks, void *buffer);
i64 write_blocks(u64 start_address, u64 nblocks, const void *buffer);
i64 close_disk(void);
