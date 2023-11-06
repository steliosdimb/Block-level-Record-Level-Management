#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "hp_file.h"

#define RECORDS_NUM 500 // you can change it if you want
#define FILE_NAME "data.db"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK)        \
    {                         \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

int main()
{
  BF_Init(LRU);

  HP_CreateFile(FILE_NAME);
  int file_desc;

  HP_info *hp_info2 = HP_OpenFile(FILE_NAME, &file_desc);

  Record record;
  srand(1782872);
  int r;
  printf("Insert Entries\n");
  for (int id = 0; id < RECORDS_NUM; ++id)
  {
    record = randomRecord();
    HP_InsertEntry(file_desc, hp_info2, record);
  }
  printf("RUN PrintAllEntries\n");
  //HP_Print_All_Records(file_desc, hp_info2);

  int id = rand() % RECORDS_NUM;
  printf("\nSearching for: %d\n", id);
  int num = 0; 
  num = HP_GetAllEntries(file_desc, hp_info2, id);
  printf("\nNumber of checked Blocks is: %d\n", num);
  HP_CloseFile(file_desc, hp_info2);
  BF_Close();
  remove("data.db");
}