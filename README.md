# OS-goldfs

A simple file system implementation. It is limited in many ways:
1. 11 characters for file names
2. 1024 data blocks
3. No multi-user access or file protection
4. Only a single root directory
5. A file is fully read from disk on read/write operations
6. No commit/restore functionality for shadowing
7. 256 files can be stored
8. 32 files can be open at the same time
9. Maximum file size is 270 data blocks
10 A data block contains 1024 bytes of data
