#include "stdafx.h"

#include "prism.h"

#include <cassert>

itemset* itemset_init(const dataset_conf* ds_conf) {
	assert(ds_conf != NULL);
	assert(ds_conf->miniid < ds_conf->maxiid);

	itemset* items = new itemset();

	items->reserve(ds_conf->maxiid - ds_conf->miniid + 1);

	for (item_t iid = ds_conf->miniid; iid <= ds_conf->maxiid; iid ++) {
		items->push_back(iid);
	}

	return items;
}

void itemset_clean(itemset* items) {
	assert(items != NULL);

	delete items;

	items = NULL;
}

encoding_block* encoding_block_create(item_t item, unsigned int pbslen, unsigned int sbslen) {
	encoding_block* eb = new encoding_block();
	sequence_block sb_empty = {INVALID_EVALUE};

	eb->item = item;
	eb->support = 0;

	eb->position_blocks = new eval_t[pbslen];
	eb->position_blocks_size = 0;

	sequence_block* sbs = new sequence_block[sbslen];
	for (unsigned int i = 0; i < sbslen; i ++) {
		sbs[i] = sb_empty;
	}
	eb->sequence_blocks = sbs;
	
	eb->itemset_extensions = new encoding_block_collection();

	eb->sequence_extensions = new encoding_block_collection();

	return eb;
}

void encoding_block_trim(encoding_block* eb, unsigned int pbslen) {
	assert(eb != NULL);

	if (eb->position_blocks != NULL && eb->position_blocks_size < pbslen) {
		eval_t* pbs = new eval_t[eb->position_blocks_size];
		
		memcpy(pbs, eb->position_blocks, eb->position_blocks_size * sizeof(eval_t));

		delete []eb->position_blocks;

		eb->position_blocks = pbs;
	}
}

void encoding_block_semiclean(encoding_block* eb) {
	assert(eb != NULL);

	if (eb->position_blocks != NULL) {
		delete []eb->position_blocks;

		eb->position_blocks = NULL;
		eb->position_blocks_size = 0;
	}
	
	if (eb->sequence_blocks != NULL) { 
		delete []eb->sequence_blocks;

		eb->sequence_blocks = NULL;
	}
}

void encoding_block_delete(encoding_block* eb) {
	assert(eb != NULL);

	if (eb->position_blocks != NULL) {
		delete []eb->position_blocks;
	}
	
	if (eb->sequence_blocks != NULL) { 
		delete []eb->sequence_blocks;
	}

	delete eb->itemset_extensions;
	
	delete eb->sequence_extensions;

	delete eb;

	eb = NULL;
}

void encoding_block_collection_clean(encoding_block_collection* ebs) {
	assert(ebs != NULL);

	encoding_block_collection_iterator ebs_end = ebs->end();
	encoding_block_collection_iterator ebs_ite = ebs->begin();

	for ( ; ebs_ite != ebs_end; ebs_ite ++) {
		encoding_block_delete((*ebs_ite));
	}

	delete ebs;

	ebs = NULL;
}

void store_invalid_position_blocks(	offset_block_collection* obs, bool& is_first_ofs,
									eval_t* pbs, unsigned int& pbs_size,
									unsigned int invalid_block_count) {

	assert(obs != NULL);
	assert(pbs != NULL);

	for (unsigned int i = 0; i < invalid_block_count; i ++) {
		pbs[pbs_size + i] = INVALID_EVALUE;	
	}

	if (is_first_ofs == true) {
		obs->push_back(pbs_size);

		is_first_ofs = false;
	}

	pbs_size += invalid_block_count;
}

void store_valid_position_block(offset_block_collection* obs, bool& is_first_ofs,
								eval_t* pbs, unsigned int& pbs_size,
								eval_t evalue)  {
	assert(obs != NULL);
	assert(pbs != NULL);

	pbs[pbs_size] = evalue;
				
	if (is_first_ofs == true) {
		obs->push_back(pbs_size);

		is_first_ofs = false;
	}

	pbs_size ++;
}
