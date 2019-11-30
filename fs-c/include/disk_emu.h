#pragma once

#include <stdint.h>
#include <stdio.h>

typedef int64_t i64;

i64 init_fresh_disk(char *filename, i64 block_size, i64 num_blocks);
i64 init_disk(char *filename, i64 block_size, i64 num_blocks);
i64 read_blocks(i64 start_address, i64 nblocks, void *buffer);
i64 write_blocks(i64 start_address, i64 nblocks, const void *buffer);
i64 close_disk(void);

extern i64 vl_close_disk(FILE *);
extern i64 vl_init_fresh_disk(FILE **, char *, i64, i64, i64 *, i64 *);
extern i64 vl_init_disk(FILE **, char *, i64, i64, i64 *, i64 *);
extern i64 vl_write_blocks(FILE *, i64, i64, i64, i64, const void *);
extern i64 vl_read_blocks(FILE *, i64, i64, i64, i64, void *);

