#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "fs.h"

inode_t *
fs_get_inode(fs_t *fs, const char *path)
{
	inode_t *inode = NULL;
	int i = 0;
	while (!inode && i < fs->last_inode + 1) {
		if ((fs->inodes_bitmap[i] == OCCUPIED) &&
		    (strcmp(fs->inodes[i].path, path) == 0))
			inode = &(fs->inodes[i]);
		i++;
	}

	return inode;
}

int
fs_read_dir(inode_t *directory, char **entry_name)
{
	if (!(*entry_name)) {
		char data[MAX_CONTENIDO];
		strcpy(data, directory->data);
		*entry_name = strtok(data, " ");
	} else {
		*entry_name = strtok(NULL, " ");
	}

	if (!(*entry_name))
		return 0;
	else
		return 1;
}

int
fs_read_file(inode_t *file, char *buffer, size_t size, off_t offset)
{
	if (offset + size > file->size)
		size = file->size - offset;
	memcpy(buffer, file->data + offset, size);

	return size;
}

int
fs_write_file(inode_t *file, const char *buffer, size_t size, off_t offset)
{
	memcpy(file->data + offset, buffer, size);
	if (file->size < offset + size)
		file->size = offset + size;

	return size;
}

inode_t *
fs_get_next_free_inode(fs_t *fs)
{
	inode_t *inode = NULL;
	int i = 0;
	while (!inode && i < MAX_INODES) {
		if (fs->inodes_bitmap[i] == FREE) {
			inode = &(fs->inodes[i]);
			fs->inodes_bitmap[i] = OCCUPIED;
		}
		i++;
	}

	if (i - 1 > fs->last_inode)
		fs->last_inode = i - 1;

	return inode;
}

inode_t *
fs_get_parent_dir(fs_t *fs, const char *path, char **file_path)
{
	inode_t *parent_inode = NULL;

	int slashes = 0;
	char *aux_path = strchr(path, '/');
	while (aux_path != NULL) {
		slashes++;
		*file_path = aux_path;
		aux_path = strchr(aux_path + 1, '/');
	}

	*file_path = strtok(*file_path, "/");

	if (slashes == 1) {  // nuevo archivo en root
		parent_inode = &(fs->inodes[0]);
	} else if (slashes == 2) {  // nuevo archivo en otro directorio
		char aux_path[MAX_PATH];
		strcpy(aux_path, path);
		char *dir_abs_path = strtok(aux_path, "/");
		char dir_rel_path[MAX_PATH];
		strcpy(dir_rel_path, "");
		strcat(dir_rel_path, "/");
		strcat(dir_rel_path, dir_abs_path);
		parent_inode = fs_get_inode(fs, dir_rel_path);
	}

	return parent_inode;
}

void
fs_add_to_dir(inode_t *directory, const char *abs_path)
{
	strcat(directory->data, abs_path);
	strcat(directory->data, " ");

	directory->last_mod = time(NULL);
	directory->last_access = time(NULL);
}

void
fs_rm_from_dir(inode_t *directory, const char *abs_path)
{
	char *token = strtok(directory->data, " ");
	char new_directory_data[MAX_CONTENIDO] = EMPTY_DATA;

	while (token != NULL) {
		if (strcmp(token, abs_path) != 0) {
			strcat(new_directory_data, token);
			strcat(new_directory_data, " ");
		}
		token = strtok(NULL, " ");
	}

	strcpy(directory->data, new_directory_data);

	directory->last_mod = time(NULL);
	directory->last_access = time(NULL);
}

void
fs_init_inode(inode_t *inode, const char *path, mode_t mode, int is_dir)
{
	strcpy(inode->data, EMPTY_DATA);
	strcpy(inode->path, path);
	inode->nlink = 1;
	inode->uid = getuid();
	inode->gid = getgid();
	inode->last_mod = time(NULL);
	inode->last_access = time(NULL);

	if (is_dir) {
		inode->mode = mode | __S_IFDIR;
		inode->size = MAX_CONTENIDO;
	} else {
		inode->mode = mode;
		inode->size = 0;
	}
}

void
fs_free_inode(fs_t *fs, inode_t *inode)
{
	int i = 0;
	int new_last_inode = 0;
	while (i < fs->last_inode + 1) {
		if ((fs->inodes_bitmap[i] == OCCUPIED)) {
			if (strcmp(fs->inodes[i].path, inode->path) == 0) {
				fs->inodes_bitmap[i] = FREE;
				memset(inode, 0, sizeof(struct inode));
			} else {
				new_last_inode = i;
			}
		}
		i++;
	}

	fs->last_inode = new_last_inode;
}

int
fs_free_dir(fs_t *fs, inode_t *directory)
{
	// elimina cada archivo dentro del directorio, considerando que no hay directorios en un directorio.
	char data[MAX_CONTENIDO];
	strcpy(data, directory->data);
	char *file_abs_path = strtok(data, " ");
	while (file_abs_path != NULL) {
		char file_path[MAX_CONTENIDO];
		strcpy(file_path, directory->path);
		strcat(file_path, "/");
		strcat(file_path, file_abs_path);
		inode_t *inode = fs_get_inode(fs, file_path);
		if (!inode) {
			printf("[debug] fisopfs_rmdir - path: %s no se "
			       "encontró\n",
			       file_path);
			return -ENOENT;
		}
		fs_free_inode(fs, inode);
		file_abs_path = strtok(NULL, " ");
	}

	fs_free_inode(fs, directory);

	return 0;
}

void
fs_init(fs_t *fs)
{
	memset(fs->inodes, 0, sizeof(fs->inodes));
	memset(fs->inodes_bitmap, FREE, sizeof(fs->inodes_bitmap));

	inode_t *root_inode = &(fs->inodes[0]);

	strcpy(root_inode->data, EMPTY_DATA);
	strcpy(root_inode->path, "/");
	root_inode->mode = __S_IFDIR | 0755;
	root_inode->nlink = 2;
	root_inode->uid = 1717;  // getuid???
	root_inode->gid = getgid();
	root_inode->last_mod = time(NULL);
	root_inode->last_access = time(NULL);
	root_inode->size = MAX_CONTENIDO;
	fs->inodes_bitmap[0] = OCCUPIED;

	fs->last_inode = 0;
}