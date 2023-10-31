#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "bf.h"
#include "hp_file.h"
#include "record.h"
#include "math.h"
#define CALL_BF(call)         \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK)        \
    {                         \
      BF_PrintError(code);    \
      return HP_ERROR;        \
    }                         \
  }

int HP_CreateFile(char *fileName)
{
  void *data;
  int fd1;
  BF_Block *block;
  BF_Block_Init(&block);
  BF_CreateFile(fileName);     // create block file with name fileName
  BF_OpenFile(fileName, &fd1); // open block file and get the file desc to it
  if (fd1 == -1)
  {
    return (-1); // could not open this file
  }

  CALL_BF(BF_AllocateBlock(fd1, block));   // initialize and allocate first block that will contain HP_block
  data = BF_Block_GetData(block); // get a pointer to the data of block
  HP_info *hp_info = data;        // hp_info pointer points to the start of the data
  int block_size = BF_BLOCK_SIZE;
  hp_info->file_desc = fd1;
  hp_info->block_record_capacity = floor((block_size - sizeof(HP_block_info *)) / sizeof(Record)); // number of records will be total size of block-sizeof hp block info pointer/sizeof record
                                                                                         // also using floorto convert floating number to its immediatly smaller
  hp_info->last_block_id = 0;            
  BF_Block_SetDirty(block);                                                                        // set the block dirty
  // dont unpin the block,it will always be pinned since it contains info for the heap file
  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);
  printf("%d\n", hp_info->block_record_capacity);

  return 0;
}

HP_info *HP_OpenFile(char *fileName)
{
  int fd;
  BF_Block *block;
  BF_Block_Init(&block);
  BF_OpenFile(fileName, &fd); // open block file and get the file desc to it
  BF_GetBlock(fd, 0, block);  // get data from the first block(contains pointer to HP_info)
  void *data;
  HP_info *hp_info = malloc(sizeof(HP_info)); // allocate memory to store the hp info data you get from the block

  data = BF_Block_GetData(block); // get a pointer to the data of the block
  printf("eeee %d\n", hp_info->block_record_capacity);

  if (data == NULL)               // if data NULL reuturn NULL
    return NULL;
    
  memcpy(hp_info, data, sizeof(HP_info));

  BF_UnpinBlock(block);
  BF_Block_Destroy(&block); // unpin block and destroy it ,return hp info

  return hp_info;
}

int HP_CloseFile(HP_info *hp_info)
{
  BF_CloseFile(hp_info->file_desc); // close the file with file desc=hp_info->file_desc
  free(hp_info);                    // free hp info
  return 0;
}


int HP_InsertEntry(HP_info *hp_info, Record record)
{
  int *blocks_num;
  void *data; 
  BF_Block *block;
  BF_Block_Init(&block);
  BF_GetBlock(hp_info->file_desc, 0, block); // load the first block 
  data = BF_Block_GetData(block);
  if(data == NULL)
    return -1;

  hp_info = data;        // hp_info pointer points to the start of the data
  int last_block = hp_info->last_block_id ;

  if (last_block == 0)
  {
    BF_Block *temp_block;
    BF_Block_Init(&temp_block);
    BF_AllocateBlock(hp_info->file_desc, temp_block);
    void *data2;
    void *rec;
    data2 = BF_Block_GetData(temp_block);
    rec = BF_Block_GetData(temp_block);
    HP_block_info *hp_block_info = data;
    // pointer indicates at the start of the block. We need to point to the end of it
    // go to the position that hp block info can be stored
    hp_block_info = find_metadata(hp_info, hp_block_info);
    int result=insert_record(hp_info, hp_block_info, rec, record);
    if(result==0){
      printf("inserted record\n");
    }
    BF_Block_SetDirty(temp_block);
    BF_UnpinBlock(temp_block);
  }

  else
  {
  }

  return 0;
}

int HP_GetAllEntries(HP_info *hp_info, int value)
{
  return 0;
}

// find the section of metadata in a block and returns a pointer to it

HP_block_info *find_metadata(HP_info *hp_info, HP_block_info *hp_block_info)
{
  int pos = BF_BLOCK_SIZE - (hp_info->block_record_capacity * sizeof(Record));
  hp_block_info = hp_block_info + pos; // go to the right position to store hp block info metadata
  hp_block_info ->records_in_block = 0;
  hp_block_info ->next_block = NULL;
  return hp_block_info;
}

int insert_record(HP_info *hp_info, HP_block_info *hp_block_info, void *block, Record record)
{
  int num = hp_block_info->records_in_block;
  if(num < hp_info->block_record_capacity)
  {
    block = block + (hp_block_info->records_in_block*sizeof(Record)); 
    memcpy(block, &record, sizeof(Record));
    hp_block_info->records_in_block++;
    return 0;
  }
  else  
    return -1;
}