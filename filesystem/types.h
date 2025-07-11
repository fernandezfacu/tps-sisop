#define MAX_INODES 100
#define MAX_CONTENIDO 4096
#define MAX_PATH 256

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct inode {
	char path[MAX_PATH];
	char data[MAX_CONTENIDO];
	mode_t mode;
	time_t last_mod;
	time_t last_access;
	nlink_t nlink;
	uid_t uid;
	gid_t gid;
	off_t size;
} inode_t;

typedef struct fs {
	inode_t inodes[MAX_INODES];
	u_int8_t inodes_bitmap[MAX_INODES];
	int last_inode;
} fs_t;