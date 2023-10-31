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

  BF_CreateFile(fileName);     // create block file with name fileName
  BF_OpenFile(fileName, &fd1); // open block file and get the file desc to it
  if (fd1 == -1)
  {
    return (-1); // could not open this file
  }
  BF_Block *block;
  BF_Block_Init(&block);

  BF_AllocateBlock(fd1, block);   // initialize and allocate first block that will contain HP_block
  data = BF_Block_GetData(block); // get a pointer to the data of block
  HP_info *hp_info = data;        // hp_info pointer points to the start of the data
  int block_size = BF_BLOCK_SIZE;
  hp_info->file_desc = fd1;
  hp_info->block_record_capacity = floor((block_size - sizeof(HP_block_info *)) / sizeof(Record)); // number of records will be total size of block-sizeof hp block info pointer/sizeof record
  printf("%d\n", hp_info->block_record_capacity);                                                  // also using floorto convert floating number to its immediatly smaller
  hp_info->last_block_id = 0;                                                                      // shows the last block id
  // dont unpin the block,it will always be pinned since it contains info for the heap file
  BF_Block_SetDirty(block);
  BF_UnpinBlock(block);
  BF_Block_Destroy(&block);

  return 0;
}

HP_info *HP_OpenFile(char *fileName, int *file_desc)
{

  BF_Block *block;
  BF_Block_Init(&block);
  BF_GetBlock(*file_desc, 0, block);          // get data from the first block(contains pointer to HP_info)
  HP_info *hp_info = malloc(sizeof(HP_info)); // alocate memory to store the hp info data you get from the block

  void *data = BF_Block_GetData(block); // get a pointer to the data of the block
  if (data == NULL)                     // if data NULL reuturn NULL
    return NULL;

  memcpy(hp_info, data, sizeof(HP_info));
  printf("%d\n", hp_info->block_record_capacity);
  BF_UnpinBlock(block);
  BF_Block_Destroy(&block); // unpin block and destroy it ,return hp info
  return hp_info;
}

int HP_CloseFile(int file_desc, HP_info *hp_info)
{
  BF_CloseFile(file_desc); // close the file with file desc=hp_info->file_desc
  free(hp_info);           // free hp info
  return 0;
}

int HP_InsertEntry(int file_desc, HP_info *hp_info, Record record)
{
  int *blocks_num;
  BF_Block *block;
  BF_Block_Init(&block);
  BF_GetBlock(file_desc, 0, block); // load the first block
  void *b = BF_Block_GetData(block);

  return 0;
}

int HP_GetAllEntries(int file_desc, HP_info *hp_info, int value)
{
  return 0;
}
