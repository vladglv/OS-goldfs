#include "disk_emu.h"
#include <stdio.h>

extern i64 vl_close_disk(FILE*);
extern i64 vl_init_fresh_disk(FILE**, char*, i64, i64, i64*, i64*);
extern i64 vl_init_disk(FILE**, char*, i64, i64, i64*, i64*);
extern i64 vl_write_blocks(FILE*, i64, i64, i64, i64, const void*);
extern i64 vl_read_blocks(FILE*, i64, i64, i64, i64, void*);

static FILE* fp = NULL;
static i64 BLOCK_SIZE = 0;
static i64 MAX_BLOCK = 0;

i64 close_disk(void) {
  return vl_close_disk(fp);
}

i64 init_fresh_disk(char *filename, i64 block_size, i64 num_blocks) {
  return vl_init_fresh_disk(&fp, filename, block_size, num_blocks, &BLOCK_SIZE, &MAX_BLOCK);
}

i64 init_disk(char *filename, i64 block_size, i64 num_blocks) {
  return vl_init_disk(&fp, filename, block_size, num_blocks, &BLOCK_SIZE, &MAX_BLOCK);
}

i64 read_blocks(i64 start_address, i64 nblocks, void *buffer) {
  return vl_read_blocks(fp, start_address, nblocks, BLOCK_SIZE, MAX_BLOCK, buffer);
}

i64 write_blocks(i64 start_address, i64 nblocks, const void *buffer) {
  return vl_write_blocks(fp, start_address, nblocks, BLOCK_SIZE, MAX_BLOCK, buffer);
}
