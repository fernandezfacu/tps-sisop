#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include "fs.h"

char fisopfs_file[MAX_PATH] = "fs.fisopfs";
fs_t fs;

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	inode_t *inode = fs_get_inode(&fs, path);

	if (!inode) {
		printf("[debug] fisopfs_getattr - path: %s no se encontró\n",
		       path);
		return -ENOENT;
	}

	st->st_uid = inode->uid;
	st->st_mode = inode->mode;
	st->st_nlink = inode->nlink;
	st->st_size = inode->size;
	st->st_atime = inode->last_access;
	st->st_mtime = inode->last_mod;
	st->st_uid = inode->uid;
	st->st_gid = inode->gid;

	return 0;
}

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);

	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	inode_t *inode = fs_get_inode(&fs, path);

	if (!inode) {
		printf("[debug] fisopfs_readdir - path: %s no se encontró\n",
		       path);
		return -ENOENT;
	}

	if (!(inode->mode & __S_IFDIR)) {
		printf("[debug] fisopfs_readdir - path: %s no es un "
		       "directiorio\n",
		       path);
		return -ENOTDIR;
	}

	inode->last_access = time(NULL);

	char *entry_name = NULL;
	int res = fs_read_dir(inode, &entry_name);
	while (res > 0) {
		filler(buffer, entry_name, NULL, 0);
		res = fs_read_dir(inode, &entry_name);
	}

	return 0;
}

static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	inode_t *inode = fs_get_inode(&fs, path);
	if (!inode) {
		printf("[debug] fisopfs_read - path: %s no se encontró\n", path);
		return -ENOENT;
	}

	if (inode->mode & __S_IFDIR) {
		printf("[debug] fisopfs_read - path: %s es un "
		       "directiorio\n",
		       path);
		return -EISDIR;
	}

	if (offset > inode->size) {
		fprintf(stderr, "[debug] fisopfs_read: offset inválido");
		return -EINVAL;
	}

	inode->last_access = time(NULL);
	return fs_read_file(inode, buffer, size, offset);
}

static int
fisopfs_utimens(const char *path, const struct timespec tv[2])
{
	inode_t *inode = fs_get_inode(&fs, path);
	if (!inode) {
		printf("[debug] fisopfs_read - path: %s no se encontró\n", path);
		return -ENOENT;
	}

	inode->last_access = tv[0].tv_sec;
	inode->last_mod = tv[1].tv_sec;

	return 0;
}

static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[debug] fisopfs_mkdir - path: %s\n", path);

	if (strlen(path) > MAX_PATH) {
		printf("[debug] fisopfs_mkdir - path too long");
		return -EINVAL;
	}

	inode_t *inode = fs_get_inode(&fs, path);
	if (!inode) {
		char *dir_abs_path;
		inode_t *root = fs_get_inode(&fs, "/");
		inode_t *directory = fs_get_parent_dir(&fs, path, &dir_abs_path);
		if (!directory ||
		    root != directory) {  // directorio dentro de directorio no soportado
			printf("[debug] fisopfs_mkdir - error de path: %s\n",
			       path);
			return -EINVAL;
		}

		if (strlen(root->data) + strlen(dir_abs_path) + 1 > root->size) {
			printf("[debug] fisopfs_mkdir - directory content too "
			       "long\n");
			return -EFBIG;
		}
		fs_add_to_dir(root, dir_abs_path);

		inode_t *new_inode = fs_get_next_free_inode(&fs);
		if (!new_inode) {
			printf("[debug] fisopfs_mkdir - no space left on "
			       "device\n");
			return -ENOSPC;
		}

		fs_init_inode(new_inode, path, mode, 1);

	} else {
		printf("[debug] fisopfs_mkdir - path: %s already exists\n", path);
		return -EEXIST;
	}

	return 0;
}

static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *)
{
	printf("[debug] fisopfs_create - path: %s\n", path);

	if (strlen(path) > MAX_PATH) {
		printf("[debug] fisopfs_create - path too long");
		return -EINVAL;
	}

	inode_t *inode = fs_get_inode(&fs, path);
	if (!inode) {
		char *file_abs_path;
		inode_t *directory = fs_get_parent_dir(&fs, path, &file_abs_path);
		if (!directory) {
			printf("[debug] fisopfs_create - directorio: %s no se "
			       "encontró\n",
			       path);
			return -EINVAL;
		}

		if (strlen(directory->data) + strlen(file_abs_path) + 1 >
		    directory->size) {
			printf("[debug] fisopfs_create - directory content too "
			       "long\n");
			return -EFBIG;
		}
		fs_add_to_dir(directory, file_abs_path);

		inode_t *new_inode = fs_get_next_free_inode(&fs);
		if (!new_inode) {
			printf("[debug] fisopfs_create - no space left on "
			       "device\n");
			return -ENOSPC;
		}

		fs_init_inode(new_inode, path, mode, 0);

	} else {
		printf("[debug] fisopfs_create - path: %s already exists\n", path);
		return -EEXIST;
	}

	return 0;
}

static int
fisopfs_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink(%s)\n", path);

	inode_t *inode = fs_get_inode(&fs, path);
	if (!inode) {
		printf("[debug] fisopfs_unlink - path: %s no se encontró\n", path);
		return -ENOENT;
	}

	if (inode->mode & __S_IFDIR) {
		printf("[debug] fisopfs_unlink - path: %s es un "
		       "directiorio\n",
		       path);
		return -EISDIR;
	}

	char *file_abs_path;
	inode_t *parent_dir = fs_get_parent_dir(&fs, path, &file_abs_path);
	if (!parent_dir) {
		printf("[debug] fisopfs_unlink - directorio: %s no se "
		       "encontró\n",
		       path);
		return -EINVAL;
	}

	fs_rm_from_dir(parent_dir, file_abs_path);
	fs_free_inode(&fs, inode);

	return 0;
}

static int
fisopfs_rmdir(const char *path)
{
	printf("[debug] fisopfs_rmdir - path %s\n", path);

	inode_t *inode = fs_get_inode(&fs, path);
	if (!inode) {
		printf("[debug] fisopfs_rmdir - path: %s no se encontró\n", path);
		return -ENOENT;
	}

	if (!(inode->mode & __S_IFDIR)) {
		printf("[debug] fisopfs_rmdir - path: %s no es un "
		       "directiorio\n",
		       path);
		return -ENOTDIR;
	}

	char *dir_abs_path;
	inode_t *root = fs_get_inode(&fs, "/");
	inode_t *parent_dir = fs_get_parent_dir(&fs, path, &dir_abs_path);
	if (!parent_dir ||
	    root != parent_dir) {  // directorio dentro de directorio no soportado
		printf("[debug] fisopfs_rmdir - error path: %s\n", path);
		return -EINVAL;
	}

	fs_rm_from_dir(parent_dir, dir_abs_path);
	return fs_free_dir(&fs, inode);
}

static int
fisopfs_write(const char *path,
              const char *buffer,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_write - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	if (offset + size > MAX_CONTENIDO) {
		printf("[debug] fisopfs_write - file too long\n");
		return -EFBIG;
	}

	inode_t *inode = fs_get_inode(&fs, path);
	if (!inode) {  // si no existe se crea
		int res_create =
		        fisopfs_create(path, __S_IFREG | 0644, fi);  // mode?
		if (res_create != 0) {
			return res_create;
		}
		inode = fs_get_inode(&fs, path);
		if (!inode) {
			printf("[debug] fisopfs_write - path: %s no se "
			       "encontró y no se pudo crear\n",
			       path);
			return -ENOENT;
		}
	}

	if (inode->mode & __S_IFDIR) {
		printf("[debug] fisopfs_write - path: %s es un "
		       "directiorio\n",
		       path);
		return -EISDIR;
	}

	if (inode->size < offset) {
		printf("[debug] fisopfs_write - offset invalido\n");
		return -EINVAL;
	}

	inode->last_access = time(NULL);
	inode->last_mod = time(NULL);
	return fs_write_file(inode, buffer, size, offset);
}

static int
fisopfs_truncate(const char *path, off_t size)
{
	printf("[debug] fisopfs_truncate - path: %s, size: %lu\n", path, size);

	if (size > MAX_CONTENIDO) {
		printf("[debug] fisopfs_truncate - size invalido\n");
		return -EINVAL;  // EFBIG?
	}

	inode_t *inode = fs_get_inode(&fs, path);
	if (!inode) {
		printf("[debug] fisopfs_truncate - path: %s no se encontró\n",
		       path);
		return -ENOENT;
	}

	inode->size = size;
	inode->last_mod = time(NULL);
	// inode->last access??

	return 0;
}

static void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] fisopfs_init\n");

	FILE *file = fopen(fisopfs_file, "r");
	if (!file) {
		fs_init(&fs);
	} else {
		size_t res = fread(&fs, sizeof(fs), 1, file);
		if (res != 1) {
			fprintf(stderr,
			        "[debug] init - error al cargar el filesystem");
			return NULL;
		}

		fclose(file);
	}

	return NULL;
}

static void
fisopfs_destroy()
{
	printf("[debug] fisopfs_destroy\n");

	FILE *file = fopen(fisopfs_file, "w");
	if (!file)
		fprintf(stderr,
		        "[debug] destroy - error al guardar el filesystem");

	size_t res = fwrite(&fs, sizeof(fs), 1, file);
	if (res != 1)
		fprintf(stderr,
		        "[debug] destroy - error al guardar el filesystem");

	fflush(file);
	fclose(file);
}

static int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_flush\n");

	fisopfs_destroy();

	return 0;
}

static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,
	.write = fisopfs_write,
	.truncate = fisopfs_truncate,
	.create = fisopfs_create,
	.utimens = fisopfs_utimens,
	.unlink = fisopfs_unlink,
	.mkdir = fisopfs_mkdir,
	.rmdir = fisopfs_rmdir,
	.init = fisopfs_init,
	.destroy = fisopfs_destroy,
	.flush = fisopfs_flush,
};

int
main(int argc, char *argv[])
{
	if (strcmp(argv[1], "-f") == 0) {
		if (argc == 4) {
			strcpy(fisopfs_file, argv[3]);
			argv[3] = NULL;
			argc--;
		}
	} else {
		if (argc == 3) {
			strcpy(fisopfs_file, argv[2]);
			argv[2] = NULL;
			argc--;
		}
	}

	return fuse_main(argc, argv, &operations, NULL);
}
