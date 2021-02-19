#if !defined(____DISTMASTERMINER__H____)
#define ____DISTMASTERMINER__H____

#define _CRT_SECURE_NO_WARNINGS 1
#define _SECURE_SCL 0
#define _HAS_ITERATOR_DEBUGGING 0

#include "prism.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <set>
#include <map>
#include <cmath>

using namespace std;

#define BBLK_SIZE 8

typedef unsigned int id_t;
//typedef map<id_t, void*> id_map;
//typedef pair<id_t, void*> id_pair;
//typedef vector<id_t> id_vector;

typedef unsigned char bblk_t; // bit block - 1 byte - 8 bit

bblk_t* init_bblk_buffer(int size);

extern dataset_conf* DS_CONF;

encoding_block* encoding_block_create_from_encoded_buffers(id_t iid,
										bblk_t* seq_bblk, int seq_bblk_size,
										bblk_t* pos_bblk, int pos_bblk_size,
										int* pof_buff, int pof_buff_size);
void sync_encoding_block_collection_init();
void sync_encoding_block_collection_clean();
void sync_encoding_block_collection_update(encoding_block* eb);
void sync_encoding_block_collection_sort_by_item();

void print_encoded_blocks(encoding_block_collection* ebs);
#endif // ____DISTMASTERMINER__H____