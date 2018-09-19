#include "disk_emu.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static FILE *fp = NULL;
static u64 BLOCK_SIZE = 0;
static u64 MAX_BLOCK = 0;

/*----------------------------------------------------------*/
/*Close the disk file filled when you don't need it anymore. */
/*----------------------------------------------------------*/
i64 close_disk(void) {
  if (fp != NULL)
    fclose(fp);

  return 0;
}

/*---------------------------------------*/
/*Initializes a disk file filled with 0's*/
/*---------------------------------------*/
i64 init_fresh_disk(char *filename, u64 block_size, u64 num_blocks) {
  u64 i, j;

  BLOCK_SIZE = block_size;
  MAX_BLOCK = num_blocks;

  /*Creates a new file*/
  fp = fopen(filename, "w+b");

  if (fp == NULL) {
    printf("Could not create new disk file %s\n\n", filename);
    return -1;
  }

  /*Fills the file with 0's to its given size*/
  for (i = 0; i < MAX_BLOCK; i++) {
    for (j = 0; j < BLOCK_SIZE; j++) {
      fputc(0, fp);
    }
  }

  return 0;
}
/*----------------------------*/
/*Initializes an existing disk*/
/*----------------------------*/
i64 init_disk(char *filename, u64 block_size, u64 num_blocks) {
  BLOCK_SIZE = block_size;
  MAX_BLOCK = num_blocks;

  /*Opens a file*/
  fp = fopen(filename, "r+b");

  if (fp == NULL) {
    printf("Could not open %s\n\n", filename);
    return -1;
  }

  return 0;
}

/*-------------------------------------------------------------------*/
/*Reads a series of blocks from the disk into the buffer             */
/*-------------------------------------------------------------------*/
i64 read_blocks(u64 start_address, u64 nblocks, void *buffer) {
  u64 i, j;
  i64 e = 0;
  i64 s = 0;

  /*Sets up a temporary buffer*/
  void *blockRead = (void *)malloc(BLOCK_SIZE);

  /*Checks that the data requested is within the range of addresses of the
   * disk*/
  if (start_address + nblocks > MAX_BLOCK) {
    printf("out of bound error %ld\n", start_address);
    return -1;
  }

  /*Goto the data requested from the disk*/
  i64 offset = (i64)(start_address * BLOCK_SIZE);
  fseek(fp, offset, SEEK_SET);

  /*For every block requested*/
  for (i = 0; i < nblocks; ++i) {
    s++;
    fread(blockRead, BLOCK_SIZE, 1, fp);

    for (j = 0; j < BLOCK_SIZE; j++) {
      memcpy(((char *)buffer) + (i * BLOCK_SIZE), blockRead, BLOCK_SIZE);
    }
  }

  free(blockRead);

  /*If no failure return the number of blocks read, else return the negative
   * number of failures*/
  if (e == 0)
    return s;
  else
    return e;
}

/*------------------------------------------------------------------*/
/*Writes a series of blocks to the disk from the buffer             */
/*------------------------------------------------------------------*/
i64 write_blocks(u64 start_address, u64 nblocks, const void *buffer) {
  u64 i;
  i64 e = 0;
  i64 s = 0;

  void *blockWrite = (void *)malloc(BLOCK_SIZE);

  /*Checks that the data requested is within the range of addresses of the
   * disk*/
  if (start_address + nblocks > MAX_BLOCK) {
    printf("out of bound error\n");
    return -1;
  }

  /*Goto where the data is to be written on the disk*/
  i64 offset = (i64)(start_address * BLOCK_SIZE);
  fseek(fp, offset, SEEK_SET);

  /*For every block requested*/
  for (i = 0; i < nblocks; ++i) {
    memcpy(blockWrite, ((const char *)buffer) + (i * BLOCK_SIZE), BLOCK_SIZE);

    fwrite(blockWrite, BLOCK_SIZE, 1, fp);
    fflush(fp);
    s++;
  }
  free(blockWrite);

  /*If no failure return the number of blocks written, else return the
   * negative number of failures*/
  if (e == 0)
    return s;
  else
    return e;
}
