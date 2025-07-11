#include "fs.h"
#include <stdio.h>
#define ASSERT_EQUALS(value, expected)                                         \
	if (value == expected) {                                               \
		printf("OK\n");                                                \
	} else {                                                               \
		printf("FAIL\n");                                              \
	}

void
fs_default()
{
	fs_t fs;
	fs_init(&fs);

	printf("\nInit fs default\n");
	printf("- Un solo inodo: ");
	ASSERT_EQUALS(fs.last_inode, 0);
	printf("- El inodo es root: ");
	char path[MAX_CONTENIDO];
	strcpy(path, "/");
	ASSERT_EQUALS(strcmp(fs.inodes[0].path, path), 0);
}

void
fs_crear_archivo()
{
	fs_t fs;
	fs_init(&fs);

	printf("\nInit archivo '/nuevo_archivo'\n");
	fs_add_to_dir(&(fs.inodes[0]), "nuevo_archivo");

	inode_t *new_inode = fs_get_next_free_inode(&fs);

	fs_init_inode(new_inode, "/nuevo_archivo", __S_IFREG | 0644, 0);

	printf("- Se creó: ");
	ASSERT_EQUALS(fs.last_inode, 1);
	printf("- El nombre es nuevo_archivo: ");
	char path[MAX_CONTENIDO];
	strcpy(path, "/nuevo_archivo");
	ASSERT_EQUALS(strcmp(fs.inodes[1].path, path), 0);
	printf("- Se agregó a root: ");
	ASSERT_EQUALS((strstr(fs.inodes[0].data, "nuevo_archivo") != NULL), 1);
}

void
fs_crear_directorio()
{
	fs_t fs;
	fs_init(&fs);

	printf("\nInit directorio '/dir'\n");
	fs_add_to_dir(&(fs.inodes[0]), "dir");

	inode_t *new_inode = fs_get_next_free_inode(&fs);

	fs_init_inode(new_inode, "/dir", 0755, 1);

	printf("- Se creó: ");
	ASSERT_EQUALS(fs.last_inode, 1);
	printf("- El nombre es dir: ");
	char path[MAX_CONTENIDO];
	strcpy(path, "/dir");
	ASSERT_EQUALS(strcmp(fs.inodes[1].path, path), 0);
	printf("- Se agregó a root: ");
	ASSERT_EQUALS((strstr(fs.inodes[0].data, "dir") != NULL), 1);
	printf("- Es directorio: ");
	ASSERT_EQUALS((new_inode->mode & __S_IFDIR), (__S_IFDIR & __S_IFDIR));  // ?
}

void
fs_escribir_archivo()
{
	fs_t fs;
	fs_init(&fs);

	printf("\nEscribir archivo\n");
	fs_add_to_dir(&(fs.inodes[0]), "arch");

	inode_t *new_inode = fs_get_next_free_inode(&fs);

	fs_init_inode(new_inode, "/arch", __S_IFREG | 0644, 0);

	char text[17];
	strcpy(text, "Prueba escritura");
	fs_write_file(new_inode, text, sizeof(text), 0);

	printf("- Data correcta ('%s'): ", text);
	ASSERT_EQUALS(strcmp(new_inode->data, text), 0);
	printf("- Tamaño correcto: ");
	ASSERT_EQUALS(new_inode->size, sizeof(text));
}

void
fs_leer_archivo()
{
	fs_t fs;
	fs_init(&fs);

	printf("\nLeer archivo\n");
	fs_add_to_dir(&(fs.inodes[0]), "arch");

	inode_t *new_inode = fs_get_next_free_inode(&fs);

	fs_init_inode(new_inode, "/arch", __S_IFREG | 0644, 0);

	char text[15];
	strcpy(text, "Prueba lectura");
	fs_write_file(new_inode, text, sizeof(text), 0);

	char buf[15];
	int size = fs_read_file(new_inode, buf, sizeof(buf), 0);

	printf("- Lectura correcta ('%s'): ", text);
	ASSERT_EQUALS(strcmp(buf, text), 0);
	printf("- Tamaño correcto: ");
	ASSERT_EQUALS(size, sizeof(text));
}

void
fs_crear_archivo_en_directorio()
{
	fs_t fs;
	fs_init(&fs);

	printf("\nArchivo nuevo en directorio '/dir'\n");
	fs_add_to_dir(&(fs.inodes[0]), "dir");

	inode_t *new_inode = fs_get_next_free_inode(&fs);

	fs_init_inode(new_inode, "/dir", 0755, 1);

	fs_add_to_dir(&(fs.inodes[1]), "nuevo_archivo");

	inode_t *new_inode_file = fs_get_next_free_inode(&fs);

	fs_init_inode(new_inode_file, "/dir/nuevo_archivo", __S_IFREG | 0644, 0);

	printf("- Se creó: ");
	ASSERT_EQUALS(fs.last_inode, 2);
	printf("- El nombre es nuevo_archivo: ");
	char path[MAX_CONTENIDO];
	strcpy(path, "/dir/nuevo_archivo");
	ASSERT_EQUALS(strcmp(fs.inodes[2].path, path), 0);
	printf("- Se agregó a dir: ");
	ASSERT_EQUALS((strstr(fs.inodes[1].data, "nuevo_archivo") != NULL), 1);
}

void
fs_leer_directorio()
{
	fs_t fs;
	fs_init(&fs);

	printf("\nLeer directorio\n");
	fs_add_to_dir(&(fs.inodes[0]), "dir");

	inode_t *new_inode_dir = fs_get_next_free_inode(&fs);

	fs_init_inode(new_inode_dir, "/dir", 0755, 1);

	fs_add_to_dir(&(fs.inodes[0]), "arch");

	inode_t *new_inode_file = fs_get_next_free_inode(&fs);

	fs_init_inode(new_inode_file, "/arch", __S_IFREG | 0644, 0);

	char buf[MAX_CONTENIDO];

	char *entry_name = NULL;
	int res = fs_read_dir(&(fs.inodes[0]), &entry_name);
	while (res > 0) {
		strcat(buf, entry_name);
		strcat(buf, " ");
		res = fs_read_dir(&(fs.inodes[0]), &entry_name);
	}

	printf("- Lectura correcta: ");
	char lect[MAX_CONTENIDO];
	strcpy(lect, "dir arch ");
	ASSERT_EQUALS(strcmp(buf, lect), 0);
}

void
fs_borrar_archivo()
{
	fs_t fs;
	fs_init(&fs);

	printf("\nBorrar archivo\n");
	fs_add_to_dir(&(fs.inodes[0]), "archivo");

	inode_t *new_inode = fs_get_next_free_inode(&fs);

	fs_init_inode(new_inode, "/archivo", __S_IFREG | 0644, 0);
	fs_rm_from_dir(&(fs.inodes[0]), "archivo");
	fs_free_inode(&fs, new_inode);

	printf("- Se borró del directorio: ");
	ASSERT_EQUALS((strstr(fs.inodes[0].data, "archivo") == NULL), 1);
	printf("- Se borró del fs: ");
	ASSERT_EQUALS(fs_get_inode(&fs, "/archivo"), NULL);
	printf("- Último inodo es root: ");
	ASSERT_EQUALS(fs.last_inode, 0);
}

void
fs_borrar_directorio()
{
	fs_t fs;
	fs_init(&fs);

	printf("\nBorrar directorio\n");
	fs_add_to_dir(&(fs.inodes[0]), "dir");

	inode_t *new_inode = fs_get_next_free_inode(&fs);

	fs_init_inode(new_inode, "/dir", 0755, 1);

	fs_rm_from_dir(&(fs.inodes[0]), "dir");
	fs_free_dir(&fs, new_inode);

	printf("- Se borró del directorio: ");
	ASSERT_EQUALS((strstr(fs.inodes[0].data, "dir") == NULL), 1);
	printf("- Se borró del fs: ");
	ASSERT_EQUALS(fs_get_inode(&fs, "/dir"), NULL);
	printf("- Último inodo es root: ");
	ASSERT_EQUALS(fs.last_inode, 0);
}

int
main()
{
	fs_default();
	fs_crear_archivo();
	fs_crear_directorio();
	fs_escribir_archivo();
	fs_leer_archivo();
	fs_crear_archivo_en_directorio();
	fs_leer_directorio();
	fs_borrar_archivo();
	fs_borrar_directorio();
}