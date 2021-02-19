#if !defined(____DISTSLAVERMINER__H____)
#define ____DISTSLAVERMINER__H____

#define _CRT_SECURE_NO_WARNINGS 1
#define _SECURE_SCL 0
#define _HAS_ITERATOR_DEBUGGING 0

#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

#define BBLK_SIZE 8

typedef unsigned int id_t;
typedef map<id_t, void*> id_map;
typedef pair<id_t, void*> id_pair;
typedef vector<id_t> id_vector;

typedef unsigned char bblk_t; // bit block - 1 byte - 8 bit

void bblk_set_bit_buffer(int bit_pos, bblk_t* bitmap);

void compute_minsup();

int compute_sequence_blocks_size_in_byte();
int compute_position_blocks_size_in_byte(id_map* cid_map);
bblk_t* init_bblk_buffer(int size);
bool encode_blocks(id_map* cid_map, int minsup,
				   bblk_t*& seq_bblk, int seq_bblk_size,
				   bblk_t*& pos_bblk, int& pos_bblk_size,
				   int*& pof_buff, int& pof_buff_size);
void clean_encoded_blocks(bblk_t* seq_bblk, bblk_t* pos_bblk, int* pof_buff);
void next_record(ifstream& RECORD_STREAM, id_t& cid, id_t& tid, id_t& iid);

typedef struct __encode_info {
	id_t iid;

	bblk_t* seq_bblk;
	int seq_bblk_size;

	bblk_t* pos_bblk;
	int pos_bblk_size;

	int* pof_buff;
	int pof_buff_size;
} encode_info;

typedef vector<encode_info> encode_info_vector;

int slaver_mining(encode_info_vector& eis, char* seqfilename);

//extern bool RECORD_ASCII;
extern int MAX_CID;
extern double MINSUP_IN_PERCENT;

#endif // ____DISTSLAVERMINER__H____