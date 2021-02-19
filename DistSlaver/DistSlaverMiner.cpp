#include "stdafx.h"
#include "DistSlaverMiner.h"
#include "DistSlaverSocket.h"

const bblk_t BBLK_MASK[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

bool RECORD_ASCII = false;
int MAX_CID = 0;
char SEQDATA_FILENAME[256] = "";
double MINSUP_IN_PERCENT = -1;
int MIN_SUP = 1;

void compute_minsup() {
	MIN_SUP = (int)ceil(MINSUP_IN_PERCENT * MAX_CID);
}

void bblk_set_bit_buffer(int bit_pos, bblk_t* bitmap) {
    int n, x;

    x = bit_pos / BBLK_SIZE;	// Index to byte.
    n = bit_pos % BBLK_SIZE;	// Specific bit in byte.

    bitmap[x] |= BBLK_MASK[n];  // Set bit.
}

int slaver_mining(encode_info_vector& eis, char* seqfilename) {
	ifstream RECORD_STREAM;

	if (RECORD_ASCII) {
		RECORD_STREAM.open(seqfilename);
	}
	else {
		RECORD_STREAM.open(seqfilename, ios_base::binary);
	}
	if (RECORD_STREAM.fail()) {
		cerr << "Fail open input file stream\"" << seqfilename << "\".";
		
		return -1;
	}

	int seq_bblk_size = compute_sequence_blocks_size_in_byte();

	id_map* IID_MAP = new id_map();
	id_map* CID_MAP = NULL;
	id_vector* TID_VECTOR = NULL;

	id_t curr_cid = (id_t)(-1);
	id_t curr_tid = (id_t)(-1);
	id_t curr_iid = (id_t)(-1);

	int iid_count = 0;

	while(!RECORD_STREAM.eof()) {
		id_t cid = (id_t)(-1);
		id_t tid = (id_t)(-1);
		id_t iid = (id_t)(-1);

		next_record(RECORD_STREAM, cid, tid, iid);

		if ((cid == (id_t)(-1)) || (tid == (id_t)(-1)) || (iid == (id_t)(-1))) {
			continue;
		}

		iid_count ++;

		if (iid != curr_iid) {
			if (CID_MAP != NULL) {
				//ENCODE FOR PREV-ITEM
				//cout << "---------- " << curr_iid << " -------------------------------\n";
				int pos_bblk_size = 0;
				int pof_buff_size = 0;
				bblk_t* seq_bblk = NULL;
				bblk_t* pos_bblk = NULL;
				int* pof_buff = NULL;				 

				if (encode_blocks(CID_MAP, MIN_SUP, seq_bblk, seq_bblk_size, pos_bblk, pos_bblk_size, pof_buff, pof_buff_size) == true) {		
					
					encode_info ei = {curr_iid, seq_bblk, seq_bblk_size, pos_bblk, pos_bblk_size, pof_buff, pof_buff_size};

					eis.push_back(ei);
				}
			}
			
			// NEXT IID
			curr_iid = iid;

			CID_MAP = new id_map();

			IID_MAP->insert(id_pair(iid, CID_MAP));

			curr_cid = (id_t)(-1);
			curr_tid = (id_t)(-1);
		}
		
		assert(CID_MAP != NULL);

		if (cid != curr_cid) {
			curr_cid = cid;

			TID_VECTOR = new id_vector();

			CID_MAP->insert(id_pair(cid, TID_VECTOR));

			curr_tid = (id_t)(-1);
		}

		assert(TID_VECTOR != NULL);
		assert(tid > 0 && cid > 0);

		TID_VECTOR->push_back(tid);

		//cout << cid << " " << tid << " " << iid << "\n";
	}

	//ENCODE FOR LAST ITEM
	//cout << "---------- " << curr_iid << " -------------------------------\n";
	int pos_bblk_size = 0;
	int pof_buff_size = 0;
	bblk_t* seq_bblk = NULL;
	bblk_t* pos_bblk = NULL;
	int* pof_buff = NULL;				 

	if (encode_blocks(CID_MAP, MIN_SUP, seq_bblk, seq_bblk_size, pos_bblk, pos_bblk_size, pof_buff, pof_buff_size) == true) {

		encode_info ei = {curr_iid, seq_bblk, seq_bblk_size, pos_bblk, pos_bblk_size, pof_buff, pof_buff_size};

		eis.push_back(ei);
	}
	
	RECORD_STREAM.close();

	// CLEAN UP
	for (id_map::iterator iid_iter = IID_MAP->begin(); iid_iter != IID_MAP->end(); iid_iter ++ ) {
		CID_MAP = (id_map*)iid_iter->second;

		for (id_map::iterator cid_iter = CID_MAP->begin(); cid_iter != CID_MAP->end(); cid_iter ++ ) {
			TID_VECTOR = (id_vector*)cid_iter->second;

			delete TID_VECTOR;
		}

		delete CID_MAP;
	}

	delete IID_MAP;

	//cout << "END ENCODING\n";
	return iid_count;
}

void next_record(ifstream& RECORD_STREAM, id_t& cid, id_t& tid, id_t& iid) {
	if (RECORD_ASCII) {
		RECORD_STREAM >> cid;
		RECORD_STREAM >> tid;
		RECORD_STREAM >> iid;
	}
	else {
		RECORD_STREAM.read((char*)&cid, sizeof(id_t));
		RECORD_STREAM.read((char*)&tid, sizeof(id_t));
		RECORD_STREAM.read((char*)&iid, sizeof(id_t));
	}
}

bblk_t* init_bblk_buffer(int size) {
	bblk_t* bblk = new bblk_t[size];
	
	memset(bblk, 0, size);

	return bblk;
}

bool encode_blocks(id_map* cid_map, int minsup,
				   bblk_t*& seq_bblk, int seq_bblk_size,
				   bblk_t*& pos_bblk, int& pos_bblk_size,
				   int*& pof_buff, int& pof_buff_size) {

	if ((int)cid_map->size() < minsup) {
		return false;
	}

	pos_bblk_size = compute_position_blocks_size_in_byte(cid_map);
	pof_buff_size = (int)cid_map->size();
	seq_bblk = init_bblk_buffer(seq_bblk_size);
	pos_bblk = init_bblk_buffer(pos_bblk_size);
	pof_buff = new int[pof_buff_size];
	memset(pof_buff, 0, pof_buff_size);

	bblk_t* curr_pos_bblk = pos_bblk;

	int i = 0;

	for (id_map::iterator cid_iter = cid_map->begin(); cid_iter != cid_map->end(); cid_iter ++ ) {
		
		bblk_set_bit_buffer(cid_iter->first - 1, seq_bblk);

		id_vector* tid_vector = (id_vector*)cid_iter->second;
		
		for (id_vector::iterator tid_iter = tid_vector->begin(); tid_iter != tid_vector->end(); tid_iter ++ ) {
			id_t tid = *(tid_iter);

			bblk_set_bit_buffer(tid - 1, curr_pos_bblk);
		}

		id_t max_tid = tid_vector->back();

		pof_buff[i] = max_tid / BBLK_SIZE + (max_tid % BBLK_SIZE > 0 ? 1 : 0); //(int)tid_vector->size();

		i ++;

		curr_pos_bblk += max_tid / BBLK_SIZE + (max_tid % BBLK_SIZE > 0 ? 1 : 0);
	}

	return true;
}

void clean_encoded_blocks(bblk_t* seq_bblk, bblk_t* pos_bblk, int* pof_buff) {
	delete []pos_bblk;
	delete []seq_bblk;
	delete []pof_buff;
}

int compute_sequence_blocks_size_in_byte() {
	return MAX_CID / BBLK_SIZE + (MAX_CID % BBLK_SIZE > 0 ? 1 : 0);
}

int compute_position_blocks_size_in_byte(id_map* cid_map) {
	int pos_bblk_size = 0;

	for (id_map::iterator cid_iter = cid_map->begin(); cid_iter != cid_map->end(); cid_iter ++ ) {
		id_vector* tid_vector = (id_vector*)cid_iter->second;
		id_t max_tid = tid_vector->back();
		int bblk_size = max_tid / BBLK_SIZE + (max_tid % BBLK_SIZE > 0 ? 1 : 0);

		pos_bblk_size += bblk_size;
	}

	return pos_bblk_size;
}
