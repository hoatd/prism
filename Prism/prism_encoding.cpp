#include "prism.h"

#include <cmath>
#include <algorithm>
#include <cassert>

encoding_block_collection* encode_all_sequences(const itemset* items, dataset* ds, const dataset_conf* ds_conf) {
	assert(items != NULL);
	assert(ds != NULL);

	encoding_block_collection* ebs = new encoding_block_collection(); 
	
	size_t ebs_size = items->size();
	
	ebs->reserve(ebs_size);

	itemset_const_iterator items_end = items->end();
	itemset_const_iterator items_ite = items->begin();
	for ( ; items_ite != items_end; items_ite ++) {
		encoding_block* eb = encoding_block_create((*items_ite), ds_conf->pbslen, ds_conf->sbslen);
#if defined(STORE_SEQUENCE_STRING)
		eb->sequence_str = (*items_ite);
#endif /* of #if defined(STORE_SEQUENCE_STRING) */

#if defined(STORE_SEQUENCE)
		eb->seq.push_back(itemset());
		eb->seq.back().push_back((*items_ite));
#endif /* of #if defined(STORE_SEQUENCE) */
		
		ebs->push_back(eb);
	}

	bits_collection last_sequence_bits(ebs_size, 0); /* last sequence offset all items */
	
	unsigned int* itss_size = new unsigned int[ds_conf->maxslen];
	item_t** seq = new item_t*[ds_conf->maxslen];

	for (size_t i = 0; i < ds_conf->maxslen; i ++) {
		seq[i] = new item_t[ds_conf->maxilen];
		/* no need itss_size[i] = 0; */
	}
	
	unsigned int seqbits_index = 0; /* index of current bit in current block */
	unsigned int seqblks_index = 0;
	unsigned int seqscount = 0;
	unsigned int numseqs_remain = ds_conf->numseqs;
	unsigned int seqscount_eliminate = ds_conf->numseqs - ds_conf->abs_minsup;

	unsigned int seq_size = 0;

	while((seq_size = dataset_next_sequence(ds, seq, itss_size)) > 0) {
		seqbits_index ++; /* next bit in sequence bits block */

		encode_all_items_in_sequence(ebs, seq, itss_size, seq_size, seqbits_index, seqblks_index, &last_sequence_bits);

		if (seqbits_index == BITS_SIZE) { /* end a sequence bits block */
			seqscount += BITS_SIZE;
			numseqs_remain -= BITS_SIZE;

			bits_collection_iterator last_sequence_bits_ite = last_sequence_bits.begin();
			encoding_block_collection_iterator ebs_ite = ebs->begin();

			while (ebs_ite != ebs->end()) {
				encoding_block* curr_eb = (*ebs_ite);
				sequence_block* curr_sb = &(curr_eb->sequence_blocks[seqblks_index]);
				bits_t& sequence_bits = (*last_sequence_bits_ite);

				curr_sb->evalue = COMPUTE_EVALUE_OF_BITS(sequence_bits);
				curr_eb->support += COMPUTE_SUPPORT_OF_EVALUE(curr_sb->evalue);

				if ((seqscount >= seqscount_eliminate) && ((curr_eb->support + numseqs_remain) < ds_conf->abs_minsup)) {
					ebs_ite = ebs->erase(ebs_ite);
					
					encoding_block_delete(curr_eb);

					last_sequence_bits_ite = last_sequence_bits.erase(last_sequence_bits_ite);
				}
				else {
					sequence_bits = 0;

					ebs_ite ++;
					last_sequence_bits_ite ++;
				}
			}

			assert(ebs->size() == last_sequence_bits.size());

			seqblks_index ++;
			
			seqbits_index = 0;
		}
/* no need
		for (size_t i = 0; i < ds_conf->maxslen; i ++) {
			itss_size[i] = 0;
		}
*/
	}

	for (size_t i = 0; i < ds_conf->maxslen; i ++) {
		delete []seq[i];
	}
	delete []seq;
	delete []itss_size;

	if (seqbits_index > 0 && seqbits_index < BITS_SIZE) { /* completly end a sequence bits block */
		assert(seqscount + seqbits_index == ds_conf->numseqs);
		assert(numseqs_remain - seqbits_index == 0);

		bits_collection_iterator last_sequence_bits_ite = last_sequence_bits.begin();
		encoding_block_collection_iterator ebs_ite = ebs->begin();

		while (ebs_ite != ebs->end()) {
			encoding_block* curr_eb = (*ebs_ite);
			sequence_block* curr_sb = &(curr_eb->sequence_blocks[seqblks_index]);
			bits_t& sequence_bits = (*last_sequence_bits_ite);				
			
			curr_sb->evalue = COMPUTE_EVALUE_OF_BITS(sequence_bits);
			curr_eb->support += COMPUTE_SUPPORT_OF_EVALUE(curr_sb->evalue);

			if (curr_eb->support < ds_conf->abs_minsup) {
				ebs_ite = ebs->erase(ebs_ite);

				encoding_block_delete(curr_eb);

				last_sequence_bits_ite = last_sequence_bits.erase(last_sequence_bits_ite);
			}
			else {
				ebs_ite ++;
				last_sequence_bits_ite ++;
			}
		}

		assert(ebs->size() == last_sequence_bits.size());
	}
	
	if (ds_conf->trimpbs == true) {
		encoding_block_collection_iterator ebs_end = ebs->end();
		encoding_block_collection_iterator ebs_ite = ebs->begin();

		while (ebs_ite != ebs_end) {
			encoding_block_trim((*ebs_ite), ds_conf->pbslen);

			ebs_ite ++;
		}
	}
	return ebs;
}

void encode_all_items_in_sequence(	encoding_block_collection* ebs,
									item_t** seq, unsigned int* itss_size, unsigned int seq_size,
									unsigned int seqbits_index, unsigned int seqblks_index,
									bits_collection* last_sequence_bits) {
	assert(ebs != NULL);
	assert(seq != NULL);
	assert(itss_size != NULL);
	assert(last_sequence_bits != NULL);
	assert(ebs->size() == last_sequence_bits->size());
	
	encoding_block_collection_iterator ebs_end = ebs->end();
	encoding_block_collection_iterator ebs_beg = ebs->begin();	

	bits_collection_iterator last_sequence_bits_ite = last_sequence_bits->begin();

	for(encoding_block_collection_iterator ebs_ite = ebs_beg; ebs_ite != ebs_end;
		ebs_ite ++,
		last_sequence_bits_ite ++) {
		
		bits_t& sequence_bits_mask = (*last_sequence_bits_ite); /* mask of current sequence block */
		
		encoding_block* eb = (*ebs_ite);
		
		bool is_found_new = encode_item_in_sequence(
			eb->item,
			eb->position_blocks, eb->position_blocks_size,
			&(eb->sequence_blocks[seqblks_index].offset_blocks),
			seq, itss_size, seq_size);

		if (is_found_new == true) {
			sequence_bits_mask |= (1 << (8 - seqbits_index));
		}
	}
}

bool encode_item_in_sequence(item_t item,
							 eval_t* pbs, unsigned int& pbs_size,
							 offset_block_collection* ofs,
							 item_t** seq, unsigned int* itss_size, unsigned int seq_size) {
	assert(pbs != NULL);
	assert(ofs != NULL);
	assert(seq != NULL);
	assert(itss_size != NULL);

	bool is_first_offs = true; /* consider first offset in a sequence only */
	bool is_found_new = false; /* found item in sequence */

	bits_t position_bits = 0; /* current block */
	bits_t position_bit_index = 0; /* index of current bit in current block */
	
	int invalid_block_count = 0;

	for (unsigned int seq_idx = 0; seq_idx < seq_size; seq_idx ++) {
		position_bit_index ++;
		
#if defined(USE_LINEAR_LOOKUP)
		bool found = (find(&seq[seq_idx][0], &seq[seq_idx][itss_size[seq_idx]], item) != &seq[seq_idx][itss_size[seq_idx]]);
#else /* of #if defined(USE_LINEAR_LOOKUP) */
		bool found = binary_search(&seq[seq_idx][0], &seq[seq_idx][itss_size[seq_idx]], item);
#endif /* of #if defined(USE_LINEAR_LOOKUP) */		
		
		if (found == true) { /* found eb.item in current itemset */
			position_bits |= (1 << (8 - position_bit_index));
		}

		if (position_bit_index == BITS_SIZE) {	/* checking if next block then reset block_mask */
			if (position_bits != 0) { /* compact endcoding contains valid block only */
				if (is_found_new == false) {
					is_found_new = true;
				}

				if (invalid_block_count > 0) {
					store_invalid_position_blocks(ofs, is_first_offs, pbs, pbs_size, invalid_block_count);
				}
				store_valid_position_block(ofs, is_first_offs, pbs, pbs_size, COMPUTE_EVALUE_OF_BITS(position_bits));

				invalid_block_count = 0;
			}
			else {
				invalid_block_count ++;
			}
			/* reset bits for next block */
			position_bit_index = 0;
			position_bits = 0;
		}
	} /* end for iterate sequence ~ last itemset */

	/* must complete the last bits block */
	if (position_bit_index < BITS_SIZE) {
		if (position_bits != 0) {
			if (is_found_new == false) {
				is_found_new = true;
			}
						
			if (invalid_block_count > 0) {
				store_invalid_position_blocks(ofs, is_first_offs, pbs, pbs_size, invalid_block_count);
			}
			store_valid_position_block(ofs, is_first_offs, pbs, pbs_size, COMPUTE_EVALUE_OF_BITS(position_bits));
		}
	}

	return is_found_new;
}
