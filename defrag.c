#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "defrag.h"
#define BOOTSIZE 512

struct superblock *super_block;
FILE *defragged;
void* buffer;
void* inodes;
int block_count = 0;
int size;
int block_size;
int num_inodes;
void* data_block;

void seg_handler(int signo)
{
    printf("E_FREE_BLOCK\n");
    exit(EXIT_FAILURE);
}


void write_index(int start, int end)
{
    int arr[block_size/4];
    int write = end - start;
    for (int i = 0; i < write; i++)
    {
        arr[i] = start + i;
    }
    fwrite(arr, 1, block_size, defragged);
    fseek(defragged, 0, SEEK_END);
}

void write(struct inode *inode, struct inode *new, int index)
{
    int data = inode->size;
    int decrement = N_DBLOCKS;
    int increment = 0;
    while(decrement > 0 && data > 0)
    {
        void *block = data_block + block_size * inode->dblocks[increment];
        fwrite(block, 1, block_size, defragged);
        new->dblocks[increment] = block_count;
        block_count++;
        data = data - block_size;
        increment++;
        decrement--;
    }

    decrement = N_IBLOCKS;
    increment = 0;

    while(decrement > 0 && data > 0)
    {
        
        write_index(block_count + 1, block_count + block_size/4);
        new->iblocks[increment] = block_count;
        block_count++;
        int loop = block_size;
        int count = 0;
        while(loop > 0 && data > 0)
        {
            int *new_buffer = data_block + block_size * inode->iblocks[increment] + count;
            void *block = data_block + block_size * *new_buffer;
            fwrite(block, 1, block_size, defragged);
            block_count++;
            data = data - block_size;
            count += 4;
            loop -= 4;

        }
        decrement--;
        increment++;
    }

    if (data > 0)
    {
        write_index(block_count+1, block_count+block_size/4);
        new->i2block = block_count;
        block_count++;
        int i2_data = data;
        increment = 0;
        decrement = block_size;
        while (decrement > 0 && i2_data > 0)
        {
            write_index(block_count+((increment/4)+1)*block_size/4, block_count+((increment/4)+2)*((block_size/4)-1));
            block_count++;
            i2_data -= block_size;
            increment += 4;
            decrement -= 4;
        }
        decrement = block_size;
        increment = 0;
        int i2_decrement = block_size;
        while(decrement > 0 && data > 0)
        {
            int* new_buffer = data_block+block_size*inode->i2block + increment;
            while(i2_decrement > 0 && data > 0)
            {
                int* new_new_buffer = data_block + block_size* *new_buffer+increment;
                void *block = data_block + block_size * *new_new_buffer;
                fwrite(block, 1, block_size, defragged);
                block_count++;
                data -= block_size;
            }
            decrement -= 4;
            increment += 4;

        }

        
    }

  if(data > 0)
  {
      write_index(block_count+1, block_count+block_size/4);
      new->i3block = block_count;
      block_count++;
      int i3_data = data;
      int curr_block = block_count;
      decrement = block_size;
      increment = 0;
      while(decrement > 0 && i3_data > 0)
      {
          write_index(curr_block + (increment/4+1)*block_size/4, curr_block + (increment/4+2)*block_size/4-1);
          block_count++;
          i3_data -= block_size;
          decrement -= 4;
          increment += 4;
      }
      int block = (curr_block+(block_size/4+1)*block_size/4);
      decrement = block_size * block_size;
      
      while (decrement > 0 && i3_data > 0)
      {
          write_index(block, block+block_size/4-1);
          block += block_size/4;
          block_count++;
          i3_data -= block_size;
          decrement -= 4;
      }

      decrement = block_size;
      increment = 0;
      while(decrement > 0 && data > 0)
      {
          int* new_buffer = data_block + block_size * inode->i3block + increment;
          int i2_decrement = block_size;

          while(i2_decrement > 0 && data > 0)
          {
              int* new_new_buffer = data_block + block_size * *new_buffer + increment;
              int i3_decrement = block_size;
              while(i3_decrement > 0 && data > 0)
              {
                  int* i3_buffer = data_block+block_size * *new_new_buffer + increment;
                  void *block = data_block + block_size * *i3_buffer;
                  fwrite(block, 1, block_size, defragged);
                  block_count++;
                  data -= block_size;
                  i3_decrement -= 4;
              }
              i2_decrement -= 4;
          }
          decrement -= 4;
          increment += 4;
      }
      

  }

  long place = BOOTSIZE + super_block->size + block_size * super_block->inode_offset + index * sizeof(struct inode);
  fseek(defragged, place, SEEK_SET);
  fwrite(new, 1, sizeof(struct inode), defragged);
  fseek(defragged, 0, SEEK_END);
  return;

}

int main(int argc, char *argv[]){
    if (argc > 2 || argc < 2)
    {
        printf("E_BAD_ARGS\n");
        exit(EXIT_FAILURE);
    }else if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0)
        {
            printf("Usage: ./defrag [fragmented disk]\nFunction: Defragments a fragmented disk\n");
            exit(EXIT_SUCCESS);
        }
    }

    //read disk and make disk of same size
    char* file = argv[1];
    FILE *fp;
    if ((fp = fopen(file, "r")) == NULL) 
    {
        printf("E_FILE_NOT_FOUND\n");
        exit(EXIT_FAILURE);
    }
    char* newname = (char*)malloc(strlen(file)+strlen("-defrag")+1);
    strcpy(newname, file);
    strcat(newname, "-defrag");
    defragged = fopen(newname, "w+");
    free(newname); 
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    if (size < 0)
    {
        printf("E_FILE_SIZE\n");
        exit(EXIT_FAILURE);
    }
    fseek(fp, 0, SEEK_SET);
    buffer = malloc(size);
    fread(buffer, 1, size, fp);
    super_block = (struct superblock*)(buffer+512);
    block_size = super_block->size;
    fclose(fp);

    //get the inode array and start writing into new disk the data blocks
    num_inodes = (super_block->data_offset - super_block->inode_offset) * block_size/(sizeof(struct inode));
    inodes = malloc(sizeof(struct inode) * num_inodes);
    memcpy(inodes, buffer + 512 + super_block->size + block_size * super_block->inode_offset, sizeof(struct inode) * num_inodes);
    fwrite(buffer, 1, BOOTSIZE + super_block->size, defragged);
    data_block = buffer + BOOTSIZE + super_block->size + block_size* super_block->data_offset;
    for (int i = 0; i < num_inodes; i++)
    {
        struct inode *ptr = (struct inode*) (buffer + 512 + super_block->size + block_size * super_block->inode_offset + i * sizeof(struct inode));
        if (ptr->nlink == 1)
        {
            struct inode *new = (struct inode*) (inodes + i * sizeof(struct inode));
            write(ptr, new, i);
        }
    }
  
    //write in free-blocks
    signal(SIGSEGV, seg_handler);
    rewind(defragged);
    super_block->free_block = block_count;

    fseek(defragged, 0, SEEK_END);
    int swap = 1;
    void* add = buffer +  BOOTSIZE + super_block->size + block_size * super_block->swap_offset;
    void* data = add;
    if (add >= buffer+size||add < buffer+BOOTSIZE + super_block->size)
    {
        swap = 0;
        data = buffer + size;
    }
    
    void* end = data_block+ block_size * block_count;
    int num_free = (data - (buffer+BOOTSIZE + super_block->size+block_size*(super_block->data_offset + block_count - 1) + 2))/BOOTSIZE;
    
    for (int i = 0; i < num_free-1; i++)
    {
        int* ptr = (int*)end;
        *ptr = block_count;
        fwrite(ptr, 1, block_size, defragged);
        block_count++;
        end += block_size;

    }
    *(int*)(end) = -1;
    int* ptr = (int*)end;
    *ptr = -1;
    fwrite(end, 1, block_size, defragged);
    if (swap == 1)
    {
        fwrite(add, 1, buffer + size - add - 1, defragged);
    }

    //free everything
    free(buffer);
    free(inodes);
    fclose(defragged);

}


