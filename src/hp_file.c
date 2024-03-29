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

  CALL_BF(BF_CreateFile(fileName));     // create block file with name fileName
  CALL_BF(BF_OpenFile(fileName, &fd1)); // open block file and get the file desc to it
  if (fd1 == -1)
  {
    return (-1); // could not open this file
  }
  BF_Block *block;
  BF_Block_Init(&block);

  CALL_BF(BF_AllocateBlock(fd1, block)); // initialize and allocate first block that will contain HP_block
  data = BF_Block_GetData(block);        // get a pointer to the data of block
  HP_info *hp_info = data;               // hp_info pointer points to the start of the data
  int block_size = BF_BLOCK_SIZE;
  hp_info->file_desc = fd1;
  hp_info->block_record_capacity = floor((block_size - sizeof(HP_block_info *)) / sizeof(Record)); // number of records will be total size of block-sizeof hp block info pointer/sizeof record
                                                                                                   // also using floor to convert floating number to its immediatly smaller
  hp_info->last_block_id = 0;

  BF_Block_SetDirty(block); // set the block dirty
  CALL_BF(BF_UnpinBlock(block));
  CALL_BF(BF_CloseFile(fd1));

  BF_Block_Destroy(&block);

  return 0;
}

HP_info *HP_OpenFile(char *fileName, int *file_desc)
{
  CALL_BF(BF_OpenFile(fileName, file_desc));
  BF_Block *block;
  BF_Block_Init(&block);
  CALL_BF(BF_GetBlock(*file_desc, 0, block)); // get data from the first block(contains pointer to HP_info)
  HP_info *hp_info = malloc(sizeof(HP_info)); // alocate memory to store the hp info data you get from the block

  void *data = BF_Block_GetData(block); // get a pointer to the data of the block

  if (data == NULL) // if data NULL reuturn NULL
    return NULL;

  memcpy(hp_info, data, sizeof(HP_info));

  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block); // unpin block and destroy it ,return hp info

  return hp_info;
}

int HP_CloseFile(int file_desc, HP_info *hp_info)
{
  CALL_BF(BF_CloseFile(file_desc)); // close the file with file desc=hp_info->file_desc
  free(hp_info);                    // free hp info
  return 0;
}

int HP_InsertEntry(int file_desc, HP_info *hp_info, Record record)

{
  int last_block = hp_info->last_block_id;

  // if we only have one block (the initial block with meta data) then we need to allocate a new block and insert a record in it
  if (last_block == 0)
  {
    BF_Block *temp_block;
    BF_Block_Init(&temp_block);
    CALL_BF(BF_AllocateBlock(hp_info->file_desc, temp_block));
    hp_info->last_block_id++;
    void *rec;
    rec = BF_Block_GetData(temp_block);

    // pointer indicates at the start of the block. We need to point to the end of it
    // go to the position of metadata
    HP_block_info *hp_block_info = get_metadata(temp_block);
    hp_block_info->records_in_block = 0;
    int result = insert_record(hp_info, hp_block_info, rec, record);
    if (result == 0)
    {
      // printf("inserted record\n");
    }
    else
    {
      printf("couldnt insert record\n");
      exit(-1);
    }
    BF_Block_SetDirty(temp_block);
    CALL_BF(BF_UnpinBlock(temp_block));
    BF_Block_Destroy(&temp_block);
  }
  else // option where block is not the initial block
  {
    BF_Block *temp_block;
    BF_Block_Init(&temp_block);
    CALL_BF(BF_GetBlock(file_desc, hp_info->last_block_id, temp_block)); // find last block and pin it
    void *rec;
    rec = BF_Block_GetData(temp_block);

    HP_block_info *hp_block_info = get_metadata(temp_block);
    if (hp_block_info->records_in_block < hp_info->block_record_capacity)
    { // option when the block can store one more record
      int result = insert_record(hp_info, hp_block_info, rec, record);
      if (result == -1)
      {
        // printf("couldnt insert record\n");
        exit(-1);
      }
      else
      {
        // printf("inserted record  \n");
        BF_Block_SetDirty(temp_block);
        CALL_BF(BF_UnpinBlock(temp_block));
        BF_Block_Destroy(&temp_block);
      }
    }
    else
    { // option where block is full so we need to allocate new one
      BF_Block *new_block;
      BF_Block_Init(&new_block);
      BF_AllocateBlock(hp_info->file_desc, new_block);
      hp_info->last_block_id++;
      void *rec;
      rec = BF_Block_GetData(temp_block);

      HP_block_info *hp_block_info = get_metadata(temp_block);
      hp_block_info->records_in_block = 0;
      int result = insert_record(hp_info, hp_block_info, rec, record);
      if (result == 0)
      {

        // printf("inserted record\n");
      }
      else
      {
        printf("couldnt insert record\n");
        exit(-1);
      }
      BF_Block_SetDirty(new_block);
      CALL_BF(BF_UnpinBlock(new_block));
      BF_Block_Destroy(&new_block);
      BF_Block_SetDirty(temp_block);
      CALL_BF(BF_UnpinBlock(temp_block));
      BF_Block_Destroy(&temp_block);
    }
  }

  return 0;
}

int HP_GetAllEntries(int file_desc, HP_info *hp_info, int value)
{
  int total_blocks = hp_info->last_block_id; // get number of blocks
  int count = 0;
  for (int i = 0; i < total_blocks; i++) // for each block
  {
    BF_Block *temp_block;
    BF_Block_Init(&temp_block);
    CALL_BF(BF_GetBlock(file_desc, i, temp_block));
    void *rec;
    rec = BF_Block_GetData(temp_block);
    count++;
    for (int j = 0; j < hp_info->block_record_capacity; j++) // for each record in each block
    {
      Record *r = rec + (j * sizeof(Record)); // get record
      if (r->id == value)                     // if record id equal to value print it and return
      {
        printf("\nId: %d\nName: %s\nSurname: %s\nCity: %s\n", r->id, r->name, r->surname, r->city);
      }
    }
    CALL_BF(BF_UnpinBlock(temp_block));
    BF_Block_Destroy(&temp_block);
  }
  printf("blocks red till last record found %d\n", count);
  if (count == 0)
  {
    return 0;
  }
  else
  {
    return -1;
  }
}

// print all records from a file in file desc

int HP_Print_All_Records(int file_desc, HP_info *hp_info)
{
  int block_num = hp_info->last_block_id;
  printf("block counter: %d\n", block_num);
  if (block_num > 0)
  {
    for (int i = 1; i < block_num; i++)
    {
      printf("Block number %d\n", i);
      BF_Block *temp;
      BF_Block_Init(&temp);
      CALL_BF(BF_GetBlock(file_desc, i, temp));
      void *data = BF_Block_GetData(temp);

      HP_Print_Block_Records(data, hp_info);
      CALL_BF(BF_UnpinBlock(temp));
      BF_Block_Destroy(&temp);
    }
    return 0;
  }
  else
    printf("No blocks found!\n");
  return -1;
}

// Prints all records from a block

int HP_Print_Block_Records(void *rec, HP_info *hp_info)
{

  int num = hp_info->block_record_capacity;

  if (num != 0)
  {
    for (int i = 0; i < num; i++)
    {
      Record *r = rec + (i * sizeof(Record));
      printf("Record number %d\n", i);
      printf("\nId: %d\nName: %s\nSurname: %s\nCity: %s\n", r->id, r->name, r->surname, r->city);
    }
    return 0;
  }
  else
    return -1;
}

HP_block_info *get_metadata(BF_Block *block)
{
  char *data = BF_Block_GetData(block); // get data from block
  void *end = data + BF_BLOCK_SIZE;     // go to the end of the block
  end = end - sizeof(HP_block_info);    // go to the end minus the size of metadata
  return (HP_block_info *)end;
}

int insert_record(HP_info *hp_info, HP_block_info *hp_block_info, void *block_data, Record record)
{
  int num = hp_block_info->records_in_block;
  if (num < hp_info->block_record_capacity) // if there is free space in block
  {
    block_data = block_data + (hp_block_info->records_in_block * sizeof(Record)); // go to the position where the free space for record is
    memcpy(block_data, &record, sizeof(Record));                                  // memcpy it
    hp_block_info->records_in_block++;                                            // add one more record to block's metadata

    return 0;
  }
  else
    return -1;
}