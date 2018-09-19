#include "disk_emu.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static FILE *fp = NULL;
static i64 BLOCK_SIZE = 0;
static i64 MAX_BLOCK = 0;

/*----------------------------------------------------------*/
/*Close the disk file filled when you don't need it anymore. */
/*----------------------------------------------------------*/
i64 close_disk(void) {
  int r = -1;

  if (fp != NULL) {
    r = fclose(fp);
    assert(r == 0);
  }

  return r;
}

/*---------------------------------------*/
/*Initializes a disk file filled with 0's*/
/*---------------------------------------*/
i64 init_fresh_disk(char *filename, i64 block_size, i64 num_blocks) {
  if (block_size <= 0 || num_blocks <= 0 || filename == NULL) {
    return -1;
  }

  BLOCK_SIZE = block_size;
  MAX_BLOCK = num_blocks;

  /*Creates a new file*/
  fp = fopen(filename, "w+be");
  if (fp == NULL) {
    printf("Could not create new disk file %s\n\n", filename);
    return -1;
  }

  void *buf = calloc((size_t)block_size, 1);
  if (buf == NULL) {
    return -1;
  }

  /*Fills the file with 0's to its given size*/
  for (i64 i = 0; i < num_blocks; i++) {
    assert(fwrite(buf, (size_t)block_size, 1, fp) == 1);
  }

  free(buf);

  return 0;
}
/*----------------------------*/
/*Initializes an existing disk*/
/*----------------------------*/
i64 init_disk(char *filename, i64 block_size, i64 num_blocks) {
  if (block_size <= 0 || num_blocks <= 0 || filename == NULL) {
    return -1;
  }

  BLOCK_SIZE = block_size;
  MAX_BLOCK = num_blocks;

  /*Opens a file*/
  fp = fopen(filename, "r+be");
  if (fp == NULL) {
    printf("Could not open %s\n\n", filename);
    return -1;
  }

  return 0;
}

/*-------------------------------------------------------------------*/
/*Reads a series of blocks from the disk into the buffer             */
/*-------------------------------------------------------------------*/
i64 read_blocks(i64 start_address, i64 nblocks, void *buffer) {
  if (start_address < 0 || nblocks <= 0 || buffer == NULL) {
    return -1;
  }

  i64 s = 0;

  assert(BLOCK_SIZE > 0);
  assert(MAX_BLOCK > 0);

  /*Checks that the data requested is within the range of addresses of the
   * disk*/
  if (start_address + nblocks > MAX_BLOCK) {
    printf("out of bound error %ld\n", start_address);
    return -1;
  }

  /*Goto the data requested from the disk*/
  i64 offset = start_address * BLOCK_SIZE;
  if (fseek(fp, offset, SEEK_SET) != 0) {
    return -1;
  }

  /*For every block requested*/
  /*Sets up a temporary buffer*/
  void *blockRead = malloc((size_t)BLOCK_SIZE);
  if (blockRead == NULL) {
    return -1;
  }

  for (i64 i = 0; i < nblocks; ++i) {
    s++;
    assert(fread(blockRead, (size_t)BLOCK_SIZE, 1, fp) == 1);
    memcpy(((char *)buffer) + (i * BLOCK_SIZE), blockRead, (size_t)BLOCK_SIZE);
  }

  free(blockRead);
  return s;
}

/*------------------------------------------------------------------*/
/*Writes a series of blocks to the disk from the buffer             */
/*------------------------------------------------------------------*/
i64 write_blocks(i64 start_address, i64 nblocks, const void *buffer) {
  if (start_address < 0 || nblocks <= 0 || buffer == NULL) {
    return -1;
  }

  i64 s = 0;

  assert(BLOCK_SIZE > 0);
  assert(MAX_BLOCK > 0);

  /*Checks that the data requested is within the range of addresses of the
   * disk*/
  if (start_address + nblocks > MAX_BLOCK) {
    printf("out of bound error\n");
    return -1;
  }

  /*Goto where the data is to be written on the disk*/
  i64 offset = start_address * BLOCK_SIZE;
  if (fseek(fp, offset, SEEK_SET) != 0) {
    return -1;
  }

  void *blockWrite = malloc((size_t)BLOCK_SIZE);
  /*For every block requested*/
  for (i64 i = 0; i < nblocks; ++i) {
    memcpy(blockWrite, ((const char *)buffer) + (i * BLOCK_SIZE),
           (size_t)BLOCK_SIZE);

    assert(fwrite(blockWrite, (size_t)BLOCK_SIZE, 1, fp) == 1);
    assert(fflush(fp) == 0);
    s++;
  }

  free(blockWrite);

  return s;
}
