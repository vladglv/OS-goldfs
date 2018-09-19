#include "sfs_api.h"

// - File system cached state
static super_block_t sb;
static fbm_table_t fbm_table;
static dir_entry_t dir_table[MAX_FILES];
static inode_t inode_table[MAX_FILES];
static file_entry_t file_entry_table[MAX_OPEN_FILES];

// - Super block management

int32_t sb_read(super_block_t *sb_) {
  if (sb_ == NULL)
    return MY_ERR;

  char *mem = calloc(BLOCK_SIZE, sizeof(char));

  if (read_blocks(SB_BLOCK, SB_BLOCK_NUM, mem) != SB_BLOCK_NUM) {
    free(mem);
    return MY_ERR;
  }

  memcpy(sb_, mem, sizeof(super_block_t));
  free(mem);

  return MY_OK;
}

int32_t sb_update(const super_block_t sb_) {
  char *mem = calloc(sb_.blocks_size, sizeof(char));
  memcpy(mem, &sb, sizeof(sb_));

  if (write_blocks((uint64_t)sb_.sb_block_idx, (uint64_t)sb_.sb_block_num,
                   mem) != sb_.sb_block_num) {
    free(mem);
    return MY_ERR;
  }

  free(mem);
  return MY_OK;
}

int32_t sb_init(super_block_t *sb_) {
  if (sb_ == NULL)
    return MY_ERR;

  sb_->magic = MAGIC;
  sb_->blocks = NUM_BLOCKS;
  sb_->blocks_size = BLOCK_SIZE;
  sb_->dir_block_idx = DIR_BLOCK;
  sb_->dir_block_num = DIR_BLOCK_NUM;
  sb_->fbm_block_idx = FBM_BLOCK;
  sb_->fbm_block_num = FBM_BLOCK_NUM;
  sb_->inode_block_idx = INODE_BLOCK;
  sb_->inode_block_num = INODE_BLOCK_NUM;
  sb_->sb_block_idx = SB_BLOCK;
  sb_->sb_block_num = SB_BLOCK_NUM;

  return MY_OK;
}

// - Free bit map management

int32_t fbm_read(fbm_table_t *fbm_table_) {
  if (fbm_table_ == NULL)
    return MY_ERR;

  char *mem = calloc(sb.blocks_size, sizeof(char));

  if (read_blocks((uint64_t)sb.fbm_block_idx, (uint64_t)sb.fbm_block_num,
                  mem) != sb.fbm_block_num) {
    free(mem);
    return MY_ERR;
  }

  memcpy(fbm_table_, mem, sizeof(fbm_table_t));
  free(mem);

  return MY_OK;
}

int32_t fbm_update(const fbm_table_t fbm_table_) {
  char *mem = calloc(sb.blocks_size, sizeof(char));
  memcpy(mem, &fbm_table, sizeof(fbm_table_));

  if (write_blocks((uint64_t)sb.fbm_block_idx, (uint64_t)sb.fbm_block_num,
                   mem) != sb.fbm_block_num) {
    free(mem);
    return MY_ERR;
  }

  free(mem);
  return MY_OK;
}

int32_t fbm_init(fbm_table_t *fbm_table_) {
  if (fbm_table_ == NULL)
    return MY_ERR;

  for (size_t i = 0; i < sb.blocks; i++)
    fbm_table_->block[i] = ENTRY_FREE;

  return MY_OK;
}

// - Block management (updates the free bit map table)

int32_t block_allocate(fbm_table_t *fbm_table_, int32_t idx) {
  if (fbm_table_ == NULL)
    return MY_ERR;

  if (idx >= 0 && idx < (int32_t)sb.blocks &&
      fbm_table_->block[idx] == ENTRY_FREE) {
    fbm_table_->block[idx] = ENTRY_TAKEN;
    if (fbm_update(*fbm_table_) == MY_ERR)
      return MY_ERR;

    return idx;
  }

  int32_t r = 0;
  for (size_t i = 0; i < sb.blocks; i++) {
    if (fbm_table_->block[i] == ENTRY_FREE) {
      fbm_table_->block[i] = ENTRY_TAKEN;
      r = (int32_t)i;
      break;
    }
  }

  if (r > 0) {
    if (fbm_update(*fbm_table_) == MY_ERR)
      return MY_ERR;
  }

  return r;
}

int32_t block_deallocate(fbm_table_t *fbm_table_, int32_t idx) {
  if (fbm_table_ == NULL || idx < 0)
    return MY_ERR;

  if (idx < (int32_t)sb.blocks) {
    if (fbm_table_->block[idx] == ENTRY_TAKEN) {
      fbm_table_->block[idx] = ENTRY_FREE;

      if (fbm_update(*fbm_table_) == MY_ERR)
        return MY_ERR;
    }
  }

  return MY_OK;
}

// - Directory management

int32_t dir_find(dir_entry_t *d, uint32_t size, const char *name) {
  assert(size == MAX_FILES);

  if (d == NULL || name == NULL)
    return MY_ERR;

  int32_t r = MY_ERR;
  for (size_t i = 0; i < size; i++) {
    if (d[i].free == ENTRY_TAKEN && strncmp(d[i].fn, name, MAX_FN_LEN) == 0) {
      r = (int32_t)i;
      break;
    }
  }

  return r;
}

int32_t dir_remove(dir_entry_t *d, uint32_t size, const char *name) {
  assert(size == MAX_FILES);

  int32_t r = dir_find(d, size, name);
  if (r != MY_ERR && r >= 0) {
    d[r].free = ENTRY_FREE;
    d[r].linked_inode = ENTRY_INVALID;
    memset(d[r].fn, 0, MAX_FN_LEN);
  }

  return r;
}

int32_t dir_add(dir_entry_t *d, uint32_t size, const char *name,
                uint32_t node) {
  assert(size == MAX_FILES);

  int32_t r = MY_ERR;
  for (size_t i = 0; i < size; i++) {
    if (d[i].free == ENTRY_FREE) {
      d[i].free = ENTRY_TAKEN;
      d[i].linked_inode = (int32_t)node;
      strncpy(d[i].fn, name, MAX_FN_LEN);
      d[i].fn[MAX_FN_LEN - 1] = '\0';
      r = MY_OK;

      break;
    }
  }

  return r;
}

int32_t dir_read(dir_entry_t *d, uint32_t size) {
  assert(size == MAX_FILES);

  if (d == NULL ||
      read_blocks((uint64_t)sb.dir_block_idx, (uint64_t)sb.dir_block_num, d) !=
          sb.dir_block_num)
    return MY_ERR;

  return MY_OK;
}

int32_t dir_update(const dir_entry_t *d, uint32_t size) {
  assert(size == MAX_FILES);

  if (d == NULL ||
      write_blocks((uint64_t)sb.dir_block_idx, (uint64_t)sb.dir_block_num, d) !=
          sb.dir_block_num)
    return MY_ERR;

  return MY_OK;
}

int32_t dir_init(dir_entry_t *d, uint32_t size) {
  assert(size == MAX_FILES);

  if (d == NULL)
    return MY_ERR;

  for (size_t i = 0; i < size; i++) {
    d[i].free = ENTRY_FREE;
    d[i].linked_inode = ENTRY_INVALID;
    memset(d[i].fn, 0, MAX_FN_LEN);
  }

  return MY_OK;
}

// - File descriptor management

int32_t fdt_remove(file_entry_t *f, uint32_t size, int fd) {
  if (f == NULL)
    return MY_ERR;

  int32_t r = MY_ERR;
  if (fd >= 0 && fd < (int32_t)size) {
    if (f[fd].free == ENTRY_FREE)
      return MY_ERR;

    f[fd].free = ENTRY_FREE;
    f[fd].linked_inode = ENTRY_INVALID;
    f[fd].ptr_read = 0;
    f[fd].ptr_write = 0;
    r = MY_OK;
  }

  return r;
}

int32_t fdt_add(file_entry_t *f, uint32_t size, uint32_t inode_idx) {
  if (f == NULL)
    return MY_ERR;

  int32_t r = MY_ERR;
  for (size_t i = 0; i < size; i++) {
    if (f[i].free == ENTRY_FREE) {
      f[i].free = ENTRY_TAKEN;
      f[i].linked_inode = (int32_t)inode_idx;
      f[i].ptr_read = 0;
      f[i].ptr_write = 0;
      r = (int32_t)i;

      break;
    }
  }

  return r;
}

int32_t fdt_init(file_entry_t *f, uint32_t size) {
  if (f == NULL)
    return MY_ERR;

  for (size_t i = 0; i < size; i++) {
    file_entry_table[i].free = ENTRY_FREE;
    file_entry_table[i].linked_inode = ENTRY_INVALID;
    file_entry_table[i].ptr_read = 0;
    file_entry_table[i].ptr_write = 0;
  }

  return MY_OK;
}

// I-node management

int32_t inode_find(inode_t *p, uint32_t size, int32_t idx) {
  assert(size == MAX_FILES);

  if (p == NULL && idx < 0)
    return MY_ERR;

  if ((size_t)idx < size)
    return idx;

  return MY_ERR;
}

int32_t inode_remove(inode_t *p, uint32_t size, int32_t idx) {
  assert(size == MAX_FILES);

  if (p == NULL || idx < 0)
    return MY_ERR;

  if ((size_t)idx < size) {
    p[idx].next = ENTRY_INVALID;
    p[idx].free = ENTRY_FREE;
    p[idx].size = 0;

    for (size_t j = 0; j < BLOCKS_PER_INODE; j++)
      p[idx].ptr[j] = ENTRY_INVALID;

    return MY_OK;
  }

  return MY_ERR;
}

int32_t inode_allocate(inode_t *p, uint32_t size) {
  assert(size == MAX_FILES);

  if (p == NULL)
    return MY_ERR;

  int32_t r = MY_ERR;
  for (size_t i = 0; i < size; i++) {
    if (p[i].free == ENTRY_FREE) {
      p[i].free = ENTRY_TAKEN;
      r = (int32_t)i;
      break;
    }
  }

  return r;
}

int32_t inode_init(inode_t *p, uint32_t size) {
  assert(size == MAX_FILES);

  if (p == NULL)
    return MY_ERR;

  for (size_t i = 0; i < size; i++) {
    p[i].next = ENTRY_INVALID;
    p[i].free = ENTRY_FREE;
    p[i].size = 0;

    for (size_t j = 0; j < BLOCKS_PER_INODE; j++)
      p[i].ptr[j] = ENTRY_INVALID;
  }

  return MY_OK;
}

int32_t inode_read(inode_t *p, uint32_t size) {
  assert(size == MAX_FILES);
  assert(size / INODE_BLOCK_NUM * sizeof(inode_t) == BLOCK_SIZE);

  if (p == NULL ||
      read_blocks((uint64_t)sb.inode_block_idx, (uint64_t)sb.inode_block_num,
                  p) != sb.inode_block_num)
    return MY_ERR;

  return MY_OK;
}

int32_t inode_update(const inode_t *p, uint32_t size) {
  assert(size == MAX_FILES);
  assert(size / INODE_BLOCK_NUM * sizeof(inode_t) == BLOCK_SIZE);

  if (p == NULL ||
      write_blocks((uint64_t)sb.inode_block_idx, (uint64_t)sb.inode_block_num,
                   p) != sb.inode_block_num)
    return MY_ERR;

  return MY_OK;
}

int32_t *inode_get_block_list(const inode_t p, uint32_t *size) {
  if (size == NULL)
    return NULL;

  int32_t *ptr = calloc(MAX_BLOCKS_PER_FILE, sizeof(int32_t));
  *size = MAX_BLOCKS_PER_FILE;

  for (size_t i = 0; i < BLOCKS_PER_INODE; i++)
    ptr[i] = p.ptr[i];

  int32_t *iptr = (int32_t *)malloc(sb.blocks_size);

  assert(p.next != ENTRY_INVALID);
  assert(read_blocks((uint64_t)p.next, 1, iptr) == 1);

  for (size_t i = BLOCKS_PER_INODE, j = 0; i < *size; i++, j++)
    ptr[i] = iptr[j];

  free(iptr);

  return ptr;
}

int32_t inode_set_block_list(inode_t *p, int32_t *block_list) {
  if (p == NULL || block_list == NULL)
    return MY_ERR;

  for (size_t i = 0; i < BLOCKS_PER_INODE; i++)
    p->ptr[i] = block_list[i];

  assert(p->next != ENTRY_INVALID);
  assert(write_blocks((uint64_t)p->next, 1, &block_list[BLOCKS_PER_INODE]) ==
         1);

  return MY_OK;
}

int32_t inode_free_block_list(int32_t *block_list) {
  if (block_list == NULL)
    return MY_ERR;

  free(block_list);

  return MY_OK;
}

// - ssfs

void mkssfs(int fresh) {
  if (fresh) {
    assert(sb_init(&sb) == MY_OK);
    assert(inode_init(inode_table, MAX_FILES) == MY_OK);
    assert(dir_init(dir_table, MAX_FILES) == MY_OK);
    assert(fdt_init(file_entry_table, MAX_OPEN_FILES) == MY_OK);
    assert(fbm_init(&fbm_table) == MY_OK);

    assert(init_fresh_disk(MY_NAME, sb.blocks_size, sb.blocks) == 0);
    assert(block_allocate(&fbm_table, sb.fbm_block_idx) ==
           (int32_t)sb.fbm_block_idx);
    assert(block_allocate(&fbm_table, sb.sb_block_idx) ==
           (int32_t)sb.sb_block_idx);

    for (int32_t i = sb.dir_block_idx; i < sb.dir_block_idx + sb.dir_block_num;
         i++)
      assert(block_allocate(&fbm_table, i) == i);

    for (int32_t i = sb.inode_block_idx;
         i < sb.inode_block_idx + sb.inode_block_num; i++)
      assert(block_allocate(&fbm_table, i) == i);

    assert(sb_update(sb) == MY_OK);
    assert(inode_update(inode_table, MAX_FILES) == MY_OK);
    assert(dir_update(dir_table, MAX_FILES) == MY_OK);
    assert(fbm_update(fbm_table) == MY_OK);
  } else {
    assert(init_disk(MY_NAME, sizeof(super_block_t), 1) == 0);
    assert(sb_init(&sb) == MY_OK);
    assert(read_blocks(0, 1, &sb) == 1);
    assert(close_disk() == 0);
    assert(sb.magic == MAGIC);

    assert(init_disk(MY_NAME, sb.blocks_size, sb.blocks) == 0);
    assert(sb_read(&sb) == MY_OK);
    assert(sb.magic == MAGIC);
    assert(inode_read(inode_table, MAX_FILES) == MY_OK);
    assert(dir_read(dir_table, MAX_FILES) == MY_OK);
    assert(fbm_read(&fbm_table) == MY_OK);
  }
}

int ssfs_fopen(char *name) {
  int32_t dir_idx = dir_find(dir_table, MAX_FILES, name);
  if (dir_idx == MY_ERR) {
    int32_t inode_idx = inode_allocate(inode_table, MAX_FILES);
    if (inode_idx == MY_ERR)
      return -1;

    inode_t *p = &inode_table[inode_idx];
    p->next = (int16_t)block_allocate(&fbm_table, -1);
    if (p->next == MY_ERR) {
      p->next = ENTRY_INVALID;
      return -1;
    }

    int32_t *iptr = (int32_t *)calloc(sb.blocks_size, 1);

    assert(sb.blocks_size == INDIRECT_BLOCKS * INDIRECT_BLOCK_ENTRY_SIZE);

    for (size_t i = 0; i < INDIRECT_BLOCKS; i++)
      iptr[i] = ENTRY_INVALID;

    assert(write_blocks((uint64_t)p->next, 1, iptr) == 1);

    free(iptr);

    assert(inode_update(inode_table, MAX_FILES) == MY_OK);
    assert(dir_add(dir_table, MAX_FILES, name, (uint32_t)inode_idx) == MY_OK);
    assert(dir_update(dir_table, MAX_FILES) == MY_OK);

    return fdt_add(file_entry_table, MAX_OPEN_FILES, (uint32_t)inode_idx);
  } else {
    int32_t inode_idx = dir_table[dir_idx].linked_inode;
    if (inode_idx == MY_ERR)
      return -1;

    for (size_t i = 0; i < MAX_OPEN_FILES; i++) {
      if (file_entry_table[i].linked_inode == inode_idx)
        return (int32_t)i;
    }

    int32_t fd = fdt_add(file_entry_table, MAX_OPEN_FILES, (uint32_t)inode_idx);
    if (fd == MY_ERR)
      return -1;

    file_entry_table[fd].ptr_read = 0;
    file_entry_table[fd].ptr_write = (int32_t)inode_table[inode_idx].size;

    return fd;
  }
}

int ssfs_fclose(int fileID) {
  if (fileID >= 0 && fileID < MAX_OPEN_FILES)
    return fdt_remove(file_entry_table, MAX_OPEN_FILES, fileID);

  return MY_ERR;
}

int ssfs_frseek(int fileID, int loc) {
  if (fileID >= 0 && fileID < MAX_OPEN_FILES && loc >= 0) {
    int32_t inode_idx = file_entry_table[fileID].linked_inode;
    if (inode_idx == ENTRY_INVALID ||
        loc > (int32_t)inode_table[inode_idx].size)
      return MY_ERR;

    file_entry_table[fileID].ptr_read = loc;

    return MY_OK;
  }

  return MY_ERR;
}

int ssfs_fwseek(int fileID, int loc) {
  if (fileID >= 0 && fileID < MAX_OPEN_FILES && loc >= 0) {
    int32_t inode_idx = file_entry_table[fileID].linked_inode;
    if (inode_idx == ENTRY_INVALID ||
        loc > (int32_t)inode_table[inode_idx].size)
      return MY_ERR;

    file_entry_table[fileID].ptr_write = loc;

    return MY_OK;
  }

  return MY_ERR;
}

int ssfs_fwrite(int fileID, char *buf, int length) {
  if (buf == NULL || length <= 0)
    return MY_ERR;

  int32_t written_bytes = MY_ERR;

  if (fileID >= 0 && fileID < MAX_OPEN_FILES) {
    int32_t inode_idx = file_entry_table[fileID].linked_inode;
    if (inode_idx == ENTRY_INVALID)
      return MY_ERR;

    inode_t *node = &inode_table[inode_idx];
    file_entry_t *fd = &file_entry_table[fileID];

    int32_t avail = FILE_SIZE_MAX - fd->ptr_write;
    if (avail <= 0)
      return MY_ERR;

    int32_t len = 0;
    if (avail >= length)
      len = length;
    else
      len = avail;

    int32_t cur_block = (fd->ptr_write + len) / (int32_t)sb.blocks_size;

    uint32_t block_list_size = 0;
    int32_t *block_list = inode_get_block_list(*node, &block_list_size);
    if (cur_block >= (int32_t)block_list_size) {
      cur_block = (int32_t)(block_list_size - 1);
    }

    for (int32_t i = 0; i <= cur_block; i++) {
      if (block_list[i] == ENTRY_INVALID) {
        block_list[i] = block_allocate(&fbm_table, -1);
        assert(block_list[i] != MY_ERR);
      }
    }

    assert(inode_set_block_list(node, block_list) == MY_OK);

    if ((int32_t)node->size < fd->ptr_write + len)
      node->size = (uint32_t)(fd->ptr_write + len);

    assert(inode_update(inode_table, MAX_FILES) == MY_OK);

    char *file_buf = calloc(FILE_SIZE_MAX, sizeof(char));
    char *file_cursor = file_buf;

    for (int32_t i = 0; i <= cur_block; i++) {
      assert(read_blocks((uint64_t)block_list[i], 1, file_cursor) == 1);

      file_cursor += sb.blocks_size;
    }

    memcpy(&file_buf[fd->ptr_write], buf, (size_t)len);
    fd->ptr_write += len;
    written_bytes = len;
    file_cursor = file_buf;

    for (int32_t i = 0; i <= cur_block; i++) {
      assert(write_blocks((uint64_t)block_list[i], 1, file_cursor) == 1);

      file_cursor += sb.blocks_size;
    }

    free(file_buf);
    assert(inode_free_block_list(block_list) == MY_OK);

    if (len == avail)
      written_bytes = -1;
  }

  return written_bytes;
}

int ssfs_fread(int fileID, char *buf, int length) {
  if (buf == NULL || length <= 0)
    return MY_ERR;

  int32_t read_bytes = MY_ERR;

  if (fileID >= 0 && fileID < MAX_OPEN_FILES) {
    int32_t inode_idx = file_entry_table[fileID].linked_inode;
    if (inode_idx == ENTRY_INVALID)
      return MY_ERR;

    inode_t *node = &inode_table[inode_idx];
    file_entry_t *fd = &file_entry_table[fileID];

    int32_t avail = (int32_t)node->size - fd->ptr_read;
    if (avail <= 0)
      return MY_ERR;

    int32_t len = 0;
    if (avail >= length)
      len = length;
    else
      len = avail;

    char *file_buf = calloc(FILE_SIZE_MAX, sizeof(char));
    char *file_cursor = file_buf;

    int32_t cur_block = (fd->ptr_read + len) / (int32_t)sb.blocks_size;

    uint32_t block_list_size = 0;
    int32_t *block_list = inode_get_block_list(*node, &block_list_size);
    if (cur_block >= (int32_t)block_list_size) {
      cur_block = (int32_t)(block_list_size - 1);
    }

    for (int32_t i = 0; i <= cur_block; i++) {
      assert(block_list[i] != ENTRY_INVALID);
      assert(read_blocks((uint64_t)block_list[i], 1, file_cursor) == 1);

      file_cursor += sb.blocks_size;
    }

    assert(inode_free_block_list(block_list) == MY_OK);

    memcpy(buf, &file_buf[fd->ptr_read], (size_t)len);
    fd->ptr_read += len;
    read_bytes = len;

    free(file_buf);
  }

  return read_bytes;
}

int ssfs_remove(char *file) {
  int32_t dir_idx = dir_find(dir_table, MAX_FILES, file);
  if (dir_idx == MY_ERR)
    return MY_ERR;

  int32_t inode_idx = dir_table[dir_idx].linked_inode;
  assert(inode_idx != ENTRY_INVALID);
  inode_t node = inode_table[inode_idx];

  for (size_t i = 0; i < MAX_OPEN_FILES; i++) {
    if (file_entry_table[i].free == ENTRY_TAKEN &&
        file_entry_table[i].linked_inode == inode_idx)
      assert(ssfs_fclose((int)i) == 0);
  }

  uint32_t block_list_size = 0;
  int32_t *block_list = inode_get_block_list(node, &block_list_size);

  for (size_t i = 0; i < block_list_size; i++) {
    if (block_list[i] != ENTRY_INVALID)
      assert(block_deallocate(&fbm_table, block_list[i]) == MY_OK);
  }

  assert(inode_free_block_list(block_list) == MY_OK);
  assert(block_deallocate(&fbm_table, node.next) == MY_OK);

  assert(inode_remove(inode_table, MAX_FILES, inode_idx) == MY_OK);
  assert(dir_remove(dir_table, MAX_FILES, file) != MY_ERR);
  assert(inode_update(inode_table, MAX_FILES) == MY_OK);
  assert(dir_update(dir_table, MAX_FILES) == MY_OK);

  return MY_OK;
}
