#include "types.h"
#include <string.h>

#define FREE 0
#define OCCUPIED 1
#define EMPTY_DATA ""

inode_t *fs_get_inode(fs_t *fs, const char *path);

int fs_read_dir(inode_t *directory, char **entry_name);

int fs_read_file(inode_t *file, char *buffer, size_t size, off_t offset);

int fs_write_file(inode_t *file, const char *buffer, size_t size, off_t offset);

inode_t *fs_get_next_free_inode(fs_t *fs);

inode_t *fs_get_parent_dir(fs_t *fs, const char *path, char **file_path);

void fs_add_to_dir(inode_t *directory, const char *abs_path);

void fs_rm_from_dir(inode_t *directory, const char *abs_path);

void fs_init_inode(inode_t *inode, const char *path, mode_t mode, int is_dir);

void fs_free_inode(fs_t *fs, inode_t *inode);

int fs_free_dir(fs_t *fs, inode_t *directory);

void fs_init(fs_t *fs);