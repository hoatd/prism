#include "prism.h"

#include <algorithm>
#include <cassert>

unsigned int extend_root(encoding_block_collection* ebs_items, const dataset_conf* ds_conf) {
	assert(ebs_items != NULL);
	
	unsigned int numblks = 0;
	unsigned int level = 0;

	encoding_block_collection_iterator ebs_end = ebs_items->end();
	encoding_block_collection_iterator ebs_ite = ebs_items->begin();
	
	for ( ; ebs_ite != ebs_end; ebs_ite ++) {
		numblks += extend_tree((*ebs_ite), ebs_items, ds_conf, level);
	}

	return numblks;
} 

unsigned int extend_tree_collection(encoding_block_collection* ebs, const encoding_block_collection* ebs_items, const dataset_conf* ds_conf, unsigned int level) {
	assert(ebs_items != NULL);

	unsigned int numblks = 0;

	encoding_block_collection_iterator ebs_end = ebs->end();
	encoding_block_collection_iterator ebs_ite = ebs->begin();

	for ( ; ebs_ite != ebs_end; ebs_ite ++) {
		numblks += extend_tree((*ebs_ite), ebs_items, ds_conf, level);
	}

	return numblks;
}

unsigned int extend_tree(encoding_block* eb, const encoding_block_collection* ebs_items, const dataset_conf* ds_conf, unsigned int level) {
	assert(eb != NULL);
	assert(ebs_items != NULL);

	unsigned int numblks = 0;

	eb->itemset_extensions->reserve(ebs_items->size());
	eb->sequence_extensions->reserve(ebs_items->size());

	numblks += extend_itemset(eb, ebs_items, ds_conf);
	numblks += extend_sequence(eb, ebs_items, ds_conf);

#if !defined(DATASET_SIMPLE)
	if (level != 0) {
		encoding_block_semiclean(eb);
	}
#endif /* of #if !defined(DATASET_SIMPLE) */

	numblks += extend_tree_collection(eb->itemset_extensions, ebs_items, ds_conf, level + 1);
	numblks += extend_tree_collection(eb->sequence_extensions, ebs_items, ds_conf, level + 1);

	return numblks;
}

#if !defined(USE_LINEAR_LOOKUP)
inline bool encoding_block_lesser(const encoding_block* eb1, const encoding_block* eb2) {
	return (eb1->item < eb2->item) ? true : false;
}
#endif /* of #if !defined(USE_LINEAR_LOOKUP) */

unsigned int extend_itemset(encoding_block* eb, const encoding_block_collection* ebs_items, const dataset_conf* ds_conf) {
	assert(eb != NULL);
	assert(ebs_items != NULL);

	unsigned int numblks = 0;

	encoding_block_collection* eb_itsext = eb->itemset_extensions;
	encoding_block_collection_const_iterator ebs_end = ebs_items->end();	
	encoding_block_collection_const_iterator ebs_beg = ebs_items->begin();

#if defined(USE_LINEAR_LOOKUP)
	encoding_block_collection_const_iterator ebs_ite = ebs_beg;
	item_t eb_item = eb->item;
	
	for ( ; ebs_ite != ebs_end; ebs_ite ++) {
		if ((*ebs_ite)->item == eb_item) {
			break;
		}
	}
#else /* of #if defined(USE_LINEAR_LOOKUP) */
	encoding_block_collection_const_iterator ebs_ite = lower_bound(ebs_beg, ebs_end, eb_item, encoding_block_lesser);
#endif /* of #if defined(USE_LINEAR_LOOKUP) */

	if (ebs_ite != ebs_end) {
		ebs_ite ++;
	}

	for ( ; ebs_ite != ebs_end; ebs_ite ++) {
		encoding_block* new_eb = compute_sequence_gdc(eb, (*ebs_ite), true, ds_conf);

		if (new_eb != NULL) {		
#if defined(STORE_SEQUENCE_STRING)
			new_eb->sequence_str = eb->sequence_str;
			new_eb->sequence_str += string(1, (*ebs_ite)->item);
#endif /* of #if defined(STORE_SEQUENCE_STRING) */

#if defined(STORE_SEQUENCE)
			new_eb->seq = eb->seq;
			new_eb->seq.back().push_back((*ebs_ite)->item);
#endif /* of #if defined(STORE_SEQUENCE) */

			eb_itsext->push_back(new_eb);
			
			numblks ++;
		}
	}

	return numblks;
}

unsigned int extend_sequence(encoding_block* eb, const encoding_block_collection* ebs_items, const dataset_conf* ds_conf) {
	assert(eb != NULL);
	assert(ebs_items != NULL);

	unsigned int numblks = 0;

	encoding_block_collection* eb_seqext = eb->sequence_extensions;
	encoding_block_collection_const_iterator ebs_end = ebs_items->end();	
	encoding_block_collection_const_iterator ebs_ite = ebs_items->begin();
	
	for (; ebs_ite != ebs_end; ebs_ite ++) {
		encoding_block* new_eb = compute_sequence_gdc(eb, (*ebs_ite), false, ds_conf);

		if (new_eb != NULL) {
#if defined(STORE_SEQUENCE_STRING)
			new_eb->sequence_str = eb->sequence_str;
			new_eb->sequence_str += "->";
			new_eb->sequence_str += string(1, (*ebs_ite)->item);
#endif /* of #if defined(STORE_SEQUENCE_STRING) */

#if defined(STORE_SEQUENCE)
			new_eb->seq = eb->seq;
			new_eb->seq.push_back(itemset());
			new_eb->seq.back().push_back((*ebs_ite)->item);
#endif /* of #if defined(STORE_SEQUENCE) */

			eb_seqext->push_back(new_eb);

			numblks ++;
		}
	}

	return numblks;
}

offs_t next_position_offset(size_t obs_size, 
							size_t obs_index,
							size_t pbs_size,
							sequence_block* sbs, unsigned int seqblks_index, unsigned int seqblks_size) {
	offs_t obs_next = ((offs_t)-1);

	if (obs_index + 1 < obs_size) {
		obs_next = sbs[seqblks_index].offset_blocks[obs_index + 1];
	}
	else {
		unsigned int seqblks_index_next = seqblks_index;
			
		seqblks_index_next ++;

		do {
			if (seqblks_index_next != seqblks_size) {
				const offset_block_collection* obs = &(sbs[seqblks_index_next].offset_blocks);

				if (obs->size() > 0) { /* else case is empty sequence block */
					obs_next = *(obs->begin()); /* ofs[0]; */
					
					break;
				}
			}
			else { /* end of offsets, must extend to the last position block */
				obs_next = (offs_t)pbs_size;

				break;
			}
			
			seqblks_index_next ++;

		} while (true);
	}

	assert(obs_next != ((offs_t)-1));

	return obs_next;
}

bits_t unset_bit_by_index(bits_t bits, int bits_idx) {
	assert((bits_idx >= 0) && (bits_idx < BITS_SIZE));

	bits_t new_bits = bits;

	int bits_index_counter = -1;	
	
	for (bits_t i = 0; i < BITS_SIZE; i ++) {
		bits_t bit_mask = 0x80 >> i;
		/*
		 *	bits_mask =
		 *		10000000;	01000000;	00100000;	00010000;	00001000;	00000100;	00000010;	00000001
		 *		or
		 *		0x80(128);	0x40(64);	0x20(32);	0x10(16);	0x08(8);	0x04(4);	0x02(2);	0x01(1)
		 */

		/* sequence i * 8 + bit_index */
		if ((bits & bit_mask) != 0) {
			bits_index_counter ++;

			if (bits_index_counter == bits_idx) {
				new_bits = bits & (~bit_mask);

				break;
			}
		}
	}

	return new_bits;
}

encoding_block* compute_sequence_gdc(const encoding_block* eb1, const encoding_block* eb2, bool is_itsext, const dataset_conf* ds_conf) {
	assert(eb1 != NULL);
	assert(eb2 != NULL);

	encoding_block* new_eb = encoding_block_create(eb2->item, ds_conf->pbslen, ds_conf->sbslen);

	sequence_block* eb1_sbs = eb1->sequence_blocks;
	eval_t* eb1_pbs = eb1->position_blocks;
	unsigned int pbs1_size = eb1->position_blocks_size;

	sequence_block* eb2_sbs = eb2->sequence_blocks;
	eval_t* eb2_pbs = eb2->position_blocks;
	unsigned int pbs2_size = eb2->position_blocks_size;

	sequence_block* new_eb_sbs = new_eb->sequence_blocks;
	eval_t* new_eb_pbs = new_eb->position_blocks;
	unsigned int& new_eb_pbs_size = new_eb->position_blocks_size;
	
	unsigned int seqscount = 0;
	unsigned int numseqs_remain = ds_conf->numseqs;
	unsigned int seqscount_eliminate = ds_conf->numseqs - ds_conf->abs_minsup;

	unsigned int seqblks_size = ds_conf->sbslen;
	unsigned int seqblks_index = 0;

	for ( ; seqblks_index < seqblks_size; seqblks_index ++) {
		sequence_block* new_sb = &(new_eb_sbs[seqblks_index]);
		offset_block_collection* new_sb_obs = &(new_sb->offset_blocks);

		bits_t sbs1_ite_bits = COMPUTE_BITS_OF_EVALUE(eb1_sbs[seqblks_index].evalue);
		bits_t sbs2_ite_bits = COMPUTE_BITS_OF_EVALUE(eb2_sbs[seqblks_index].evalue);

		bits_t new_sb_bits_org = sbs1_ite_bits & sbs2_ite_bits;
		bits_t new_sb_bits = new_sb_bits_org;

		if (seqblks_index + 1 == seqblks_size) {
			unsigned int last_block_size = ds_conf->numseqs % BITS_SIZE;

			seqscount += last_block_size;
			numseqs_remain -= last_block_size;

			//assert(seqscount == ds_conf->numseqs);
			//assert(numseqs_remain == 0);
		}
		else {
			seqscount += BITS_SIZE;
			numseqs_remain -= BITS_SIZE;
		}

		if (new_sb_bits_org != 0x0) {
			const offset_block_collection& obs1 = eb1_sbs[seqblks_index].offset_blocks;
			const offset_block_collection& obs2 = eb2_sbs[seqblks_index].offset_blocks;
			
			size_t obs1_size = obs1.size();
			size_t obs2_size = obs2.size();

			int set_bit_counter = 0;

			offs_t obs1_idx = ((offs_t)-1);
			offs_t obs2_idx = ((offs_t)-1);
		
			for (bits_t bits_idx = 0; bits_idx < BITS_SIZE; bits_idx ++) { /* BITS_SIZE = 4 or 8 */
				bits_t bits_mask = 0x80 >> bits_idx;
				/*
				 *	bits_mask =
				 *		10000000;	01000000;	00100000;	00010000;	00001000;	00000100;	00000010;	00000001
				 *		or
				 *		0x80(128);	0x40(64);	0x20(32);	0x10(16);	0x08(8);	0x04(4);	0x02(2);	0x01(1)
				 */
				bits_t bits1_mask = sbs1_ite_bits & bits_mask;
				bits_t bits2_mask = sbs2_ite_bits & bits_mask;

				if (bits1_mask != 0x0) {
					obs1_idx ++;
				}

				if (bits2_mask != 0x0) {
					obs2_idx ++;
				}

				if ((bits1_mask & bits2_mask) != 0x0) {
					assert(obs1_idx != ((offs_t)-1));
					assert(obs2_idx != ((offs_t)-1));
					
					offs_t obs1_first = obs1[obs1_idx];
					offs_t obs2_first = obs2[obs2_idx];
					offs_t obs1_next = next_position_offset(obs1_size, obs1_idx, pbs1_size, eb1_sbs, seqblks_index, seqblks_size);
					offs_t obs2_next = next_position_offset(obs2_size, obs2_idx, pbs2_size, eb2_sbs, seqblks_index, seqblks_size);
					size_t obs1_len = obs1_next - obs1_first;
					size_t obs2_len = obs2_next - obs2_first;
					size_t ofs_len = (obs1_len < obs2_len) ? obs1_len : obs2_len;
					
					if (is_itsext == false) {
						ofs_len = obs2_len;
					}
					
					int invalid_block_count = 0;
					bool is_first_ofs = true; /* consider first offset in a sequence only */
					bool is_found_valid = false; /* found that new pbs is valid */
					bool is_first_mask = true; /* first block masked in mask operator in the case sequence extension */

					/* position */
					for (size_t pbs_idx = 0; pbs_idx < ofs_len; pbs_idx ++) {
						eval_t new_eval = INVALID_EVALUE;
						
						if (is_itsext == false) { /* sequence extension */
							eval_t eval1 = INVALID_EVALUE;
							
							if (pbs_idx < obs1_len) {
								eval1 = eb1_pbs[obs1_first + pbs_idx];
							} /* else -> eval1 = INVALID_EVALUE */
							
							eval_t eval1_mask = MAX_EVALUE;

							if (is_first_mask == true) { /* compute the mask for first valid position block only, follow blocks result the mask 0xFF*/
								if (eval1 == INVALID_EVALUE) {
									eval1_mask = INVALID_EVALUE;
								}
								else {
									eval1_mask = COMPUTE_MASK_OF_EVALUE(eval1);

									is_first_mask = false;
								}
							} /* else -> eval1_mask = MAX_EVALUE */

							eval_t eval2 = eb2_pbs[obs2_first + pbs_idx];
							
							new_eval = COMPUTE_GDC_EVALUE(eval1_mask, eval2);							
						}
						else { /* itemset extension */
							eval_t eval1 = eb1_pbs[obs1_first + pbs_idx];
							eval_t eval2 = eb2_pbs[obs2_first + pbs_idx];

							new_eval = COMPUTE_GDC_EVALUE(eval1, eval2);
						}						

						if (new_eval != INVALID_EVALUE) {
							if (is_found_valid == false) {
								is_found_valid = true;
							}

							if (invalid_block_count > 0) {
								store_invalid_position_blocks(new_sb_obs, is_first_ofs, new_eb_pbs, new_eb_pbs_size, invalid_block_count);
							}
							store_valid_position_block(new_sb_obs, is_first_ofs, new_eb_pbs, new_eb_pbs_size, new_eval);

							invalid_block_count = 0;
						}
						else {
							invalid_block_count ++;
						}
					}

					if (is_found_valid == false) {
						new_sb_bits &= unset_bit_by_index(new_sb_bits_org, set_bit_counter);
					}

					set_bit_counter ++;
				}
			}

			new_sb->evalue = COMPUTE_EVALUE_OF_BITS(new_sb_bits);

			new_eb->support += COMPUTE_SUPPORT_OF_EVALUE(new_sb->evalue);

			if ((seqscount >= seqscount_eliminate) && ((new_eb->support + numseqs_remain) < ds_conf->abs_minsup)) {
				encoding_block_delete(new_eb);

				return NULL;
			}
		}
	}

	if ((new_eb->support == 0) || (new_eb->support < ds_conf->abs_minsup)) {
		encoding_block_delete(new_eb);

		return NULL;
	}

	if (ds_conf->trimpbs == true) {
		encoding_block_trim(new_eb, ds_conf->pbslen);
	}

	return new_eb;
}
