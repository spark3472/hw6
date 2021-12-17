#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "defrag.h"
#define BOOTSIZE 512


superblock *super_block;
void* buffer;
void* inodes;
FILE *defragged;
int block_count;
int file_size;
int block_size;
int num_inodes;

int m_error;
enum{E_FNF, E_BADARGS, E_SIZE, E_SEEK, E_MEM, E_READ, E_OFFSET};

int main(int argc, char *argv[]){
  char *file;
  if (argc != 2)
  {
    printf("Try './defrag -h' for more information.\n");
    exit(1);
  }else if (strcmp(argv[1], "-h") == 0)
  {
      printf("Usage: ./defrag [fragmented disk]\nFunction: Defragments a fragmented disk\n");
      exit(1);
  }else 
  {
      file = argv[1];
  }
  block_count = 0; 
  FILE *fp = fopen(file, "r");
  if (fp == NULL) {
    m_error = E_BADARGS;
    exit(EXIT_FAILURE);
  }
  char* name = (char*)malloc(strlen(file) + strlen("-defrag") + 1);
  strcpy(name, file);
  strcat(name, "-defrag");
  defragged = fopen(name, "w+");
  free(name);

  if (defragged == NULL)
  {
    m_error = E_FNF;
    exit(EXIT_FAILURE);
  }
  fseek(fp, 0, SEEK_END);
  file_size = ftell(fp);
  if (file_size == -1){
    m_error = E_SIZE;
    exit(EXIT_FAILURE);
  }
  fseek(fp, 0, SEEK_SET);
  buffer = malloc(file_size);
  if (buffer == NULL){
    m_error = E_MEM;
    exit(EXIT_FAILURE);
  }
  fread(buffer, 1, file_size, fp);
  super_block = (superblock*) (buffer + 512);
  block_size = super_block->size;
 
  if (super_block->inode_offset >= super_block->data_offset)
  {
    m_error = E_SIZE;
    exit(EXIT_FAILURE);

  };
  fseek(defragged, 1024 + block_size * (super_block->data_offset), SEEK_SET);
  fclose(fp);
  int num_inodes = (super_block->data_offset - super_block->inode_offset) * block_size / (sizeof(struct inode) + 0.0);
  inodes = malloc(sizeof(struct inode) * num_inodes);
  memcpy(inodes, buffer + 1024 + block_size * super_block->inode_offset, sizeof(struct inode) * num_inodes);
  
  for (int i = 0; i < num_inodes; i++){
    inode *node = (inode*) (buffer + 1024 + block_size * super_block->inode_offset + i * sizeof(struct inode));
    if (node->nlink == 1){
      inode *nade = (inode*)(inodes + i * sizeof(struct inode));
      int file_size = node->size;
      for (int i = 0; i < N_DBLOCKS; i++)
      {
          if (file_size  <= 0) break;
          void *block = buffer + 1024 + block_size * super_block->data_offset + block_size * node->dblocks[i];
          fwrite(block, 1, block_size, defragged);
          nade->dblocks[i] = block_count;
          block_count++;
          file_size = file_size - block_size;
      }
      long to_data = 1024 + block_size * super_block->inode_offset + i * sizeof(inode);
      fseek(defragged, to_data, SEEK_SET);
      fwrite(nade, 1, sizeof(struct inode), defragged);
      fseek(defragged, 0, SEEK_END);
    }
  }
  super_block->free_block = block_count;
  rewind(defragged);
  fwrite(buffer, 1, 1024, defragged);
  fseek(defragged, 0, SEEK_END);
  
  int boolean = 1;
  void* to_swap = buffer + 1024 + block_size * super_block->swap_offset;
  void* data_to_add = to_swap - 1;
  if (to_swap < buffer+block_size*2 || to_swap >= buffer+file_size)
  {
    boolean = 0;
    data_to_add = buffer + file_size - 1;
  }

  void* data_block = buffer + block_size*2 + block_size * super_block->data_offset + block_size * block_count;
  int numFreeBlocks = (data_to_add - (buffer+block_size*2+block_size*(super_block->data_offset + block_count - 1) + 1)) /block_size;
  for (int i = 0; i < numFreeBlocks-1; i++){
    *(int*)data_block = block_count;
    fwrite(data_block, 1, block_size, defragged);
    block_count++;
    data_block = data_block + block_size;
  }
  *(int*)(data_block) = -1;
  fwrite(data_block, 1, block_size, defragged);
  if (boolean == 1)
  {
    fwrite(to_swap, 1, buffer + file_size - to_swap - 1, defragged);
  }
  free(buffer);
  free(inodes);
  fclose(defragged);
  return 0;
}