#include "stdafx.h"
#include "DistMasterMiner.h"
#include "DistMAsterSocket.h"

dataset_conf* DS_CONF = NULL;

bblk_t* init_bblk_buffer(int size) {
	bblk_t* bblk = new bblk_t[size];
	
	memset(bblk, 0, size);

	return bblk;
}

char count_bit_set(bblk_t bblk) {
	char card = 0;

	if (bblk & 0x80) card ++;
	if (bblk & 0x40) card ++;
	if (bblk & 0x20) card ++;
	if (bblk & 0x10) card ++;
	if (bblk & 0x08) card ++;
	if (bblk & 0x04) card ++;
	if (bblk & 0x02) card ++;
	if (bblk & 0x01) card ++;

	return card;
}

encoding_block* encoding_block_create_from_encoded_buffers(id_t iid,
										bblk_t* seq_bblk, int seq_bblk_size,
										bblk_t* pos_bblk, int pos_bblk_size,
										int* pof_buff, int pof_buff_size) {
	encoding_block* eb = new encoding_block();

	eb->item = iid;
	eb->support = pof_buff_size;

	eb->position_blocks = new eval_t[pos_bblk_size];
	eb->position_blocks_size = pos_bblk_size;
	memcpy(eb->position_blocks, pos_bblk, sizeof(eval_t) * pos_bblk_size);

	sequence_block* sbs = new sequence_block[seq_bblk_size];

	int pof_index = 0;
	int pos_offset = 0;

 	for (int i = 0; i < seq_bblk_size; i ++) {
		sbs[i].evalue = seq_bblk[i];

		char bcount = count_bit_set(seq_bblk[i]);

		for (char p = 0 ; p < bcount; p ++) {
			sbs[i].offset_blocks.push_back(pos_offset);

			pos_offset += pof_buff[pof_index];

			pof_index ++;
		}
	}

	eb->sequence_blocks = sbs;
	
	eb->itemset_extensions = new encoding_block_collection();

	eb->sequence_extensions = new encoding_block_collection();

	return eb;
}


CRITICAL_SECTION CS_EBS;
encoding_block_collection* EBS = NULL;

void sync_encoding_block_collection_init() {
	InitializeCriticalSection(&CS_EBS);
	EBS = new encoding_block_collection();
}

void sync_encoding_block_collection_clean() {
	DeleteCriticalSection (&CS_EBS);
	encoding_block_collection_clean(EBS);
	EBS = NULL;
}

void sync_encoding_block_collection_update(encoding_block* eb) {
	EnterCriticalSection(&CS_EBS);
	EBS->push_back(eb);
	LeaveCriticalSection(&CS_EBS);
}

bool encoding_block_compare_by_item(encoding_block* eb1, encoding_block* eb2) {
	return eb1->item < eb2->item;
}

void sync_encoding_block_collection_sort_by_item() {
	sort(EBS->begin(), EBS->end(), encoding_block_compare_by_item);
}

void print_encoded_blocks(encoding_block_collection* ebs) {
	ofstream o_ebs("MASTER_ENCODED_BLOCKS.LOG");

	encoding_block_collection_iterator ebs_end = ebs->end();
	encoding_block_collection_iterator ebs_ite = ebs->begin();
	
	for ( ; ebs_ite != ebs_end; ebs_ite ++) {
		o_ebs << "ITEM " << (int)(*ebs_ite)->item << " SUPPORT " << (*ebs_ite)->support << "\n";
		o_ebs << "SEQ SIZE [" << DS_CONF->sbslen << "] ";
		for (unsigned int i = 0; i < DS_CONF->sbslen; i ++) {
			o_ebs << (int)(*ebs_ite)->sequence_blocks[i].evalue << " ";
			
			o_ebs << "{";

			const offset_block_collection& obs = (*(ebs_ite))->sequence_blocks[i].offset_blocks;
			offset_block_collection_const_iterator obs_end = obs.end();
			offset_block_collection_const_iterator obs_beg = obs.begin();
			offset_block_collection_const_iterator obs_ite = obs_beg;
			
			for ( ; obs_ite != obs_end; obs_ite ++) {
				if (obs_ite != obs_beg) {
					o_ebs << " ";
				}
				o_ebs << (*obs_ite);
			}

			o_ebs << "} ";
		}

		o_ebs << "\n";
		o_ebs << "POS SIZE [" << (*ebs_ite)->position_blocks_size << "] ";
		for (unsigned int i = 0; i < (*ebs_ite)->position_blocks_size; i ++) {
			o_ebs << ((int)(*ebs_ite)->position_blocks[i]) << " ";
		}
		o_ebs << "\n";
	}
	
	o_ebs << "rel_minsup = " << DS_CONF->rel_minsup << "\n";
	o_ebs << "abs_minsup = " << DS_CONF->abs_minsup << "\n";
	o_ebs << "numseqs = " << DS_CONF->numseqs << "\n";
	o_ebs << "miniid = " << DS_CONF->miniid << "\n";
	o_ebs << "maxiid = " << DS_CONF->maxiid << "\n";
	o_ebs << "maxslen = " << DS_CONF->maxslen << "\n";
	o_ebs << "maxilen = " << DS_CONF->maxilen << "\n";
	o_ebs << "nitems = " << DS_CONF->nitems << "\n";
	o_ebs << "pbslen = " << DS_CONF->pbslen << "\n";
	o_ebs << "sbslen = " << DS_CONF->sbslen << "\n";
	o_ebs << "trimpbs = " << DS_CONF->trimpbs << "\n";

	o_ebs.close();
}
