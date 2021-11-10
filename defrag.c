#include <stdio.h>
#include <stdlib.h>

#define N_DBLOCKS 10
#define N_IBLOCKS 4

struct superblock {
    int size; /* size of blocks in bytes */
    int inode_offset; /* offset of inode region in blocks */
    int data_offset; /* data region offset in blocks */
    int swap_offset; /* swap region offset in blocks */
    int free_inode; /* head of free inode list, index, if disk is full, -1 */
    int free_block; /* head of free block list, index, if disk is full, -1 */
};

struct inode {
    int next_inode; /* index of next free inode */
    int protect; /* protection field */
    int nlink; /* number of links to this file */
    int size; /* numer of bytes in file */
    int uid; /* owner’s user ID */
    int gid; /* owner’s group ID */
    int ctime; /* change time */
    int mtime; /* modification time */
    int atime; /* access time */
    int dblocks[N_DBLOCKS]; /* pointers to data blocks */
    int iblocks[N_IBLOCKS]; /* pointers to indirect blocks */
    int i2block; /* pointer to doubly indirect block */
    int i3block; /* pointer to triply indirect block */
};

void read()
{

}

int main(int argc, char** argv)
{
    if (argc < 1)
    {
        printf("BAD ARGS\n");
        
    }

    struct superblock* super = (struct superblock*)malloc(sizeof(struct superblock));

    FILE *fp = fopen(argv[1], "r");
    int i = 0;
    
    fseek(fp, 512, SEEK_SET);

    fread(&super, sizeof(struct superblock), 1, fp);
    
    printf("%i\n", i);
}