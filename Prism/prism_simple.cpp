#include "prism.h"

#include <sstream>
#include <cassert>

const unsigned char GENERATOR[] = {2, 3, 5, 7, 11, 13, 17, 19};

static int tree_tabs = 0;

void print_subtree(const sequence* prefix, const encoding_block* eb, const dataset_conf* ds_conf, bool print_sequence_detail, ostream& os) {
	assert(prefix != NULL);

	os << eb->item;

	os << " [" << eb->support << " (" << (eb->support/(double)ds_conf->numseqs) * 100 << "%)]";

	if (print_sequence_detail == true) {
		os << " (";
#if defined(STORE_SEQUENCE_STRING)
		os << eb->sequence_str;
#else /* of #if defined(STORE_SEQUENCE_STRING) */
	#if defined(STORE_SEQUENCE)
		print_sequence(&eb->seq, os);
	#else /* of #if defined(STORE_SEQUENCE) */
		print_sequence(prefix, os);
	#endif /* of #if defined(STORE_SEQUENCE) */
#endif /* of #if defined(STORE_SEQUENCE_STRING) */
		os << ")";
	}

#if defined(DATASET_SIMPLE)
	os << " S{";

	const sequence_block* sbs = eb->sequence_blocks;

	for (unsigned int i = 0 ; i < ds_conf->sbslen; i ++) {
		if (i != 0) {
			os << " ";
		}
#if defined(STORE_PRIME)
	#if defined(PRINT_PRIME)
			os << (unsigned int)sbs[i].evalue;
	#else /* of #if defined(PRINT_PRIME) */
			os << compute_bits_from_prime(sbs[i].evalue);
	#endif /* of #if defined(PRINT_PRIME) */
#else /* of #if defined(STORE_PRIME) */
	#if defined(PRINT_PRIME)
			os << compute_prime_from_bits(sbs[i].evalue);
	#else /* of #if defined(PRINT_PRIME) */
			os << (unsigned int)sbs[i].evalue;
	#endif /* of #if defined(PRINT_PRIME) */
#endif /* of #if defined(STORE_PRIME) */
	}
	os << "}";

	os << " P{";

	const eval_t* pbs = eb->position_blocks;
	
	for (unsigned int i = 0; i < eb->position_blocks_size; i ++) {
		if (i != 0) {
			os << " ";
		}
#if defined(STORE_PRIME)
	#if defined(PRINT_PRIME)
			os << (unsigned int)pbs[i];
	#else /* of #if defined(PRINT_PRIME) */
			os << compute_bits_from_prime(pbs[i]);
	#endif /* of #if defined(PRINT_PRIME) */
#else /* of #if defined(STORE_PRIME) */
	#if defined(PRINT_PRIME)
			os << compute_prime_from_bits(pbs[i]);
	#else /* of #if defined(PRINT_PRIME) */
			os << (unsigned int)pbs[i];
	#endif /* of #if defined(PRINT_PRIME) */
#endif /* of #if defined(STORE_PRIME) */
	}
	
	os << "}";
	
	os << " O{";

	int ofs_index = 0;

	for (unsigned int i = 0 ; i < ds_conf->sbslen; i ++) {
		if (i != 0) {
			os << " ";
		}

		const offset_block_collection& obs = sbs[i].offset_blocks;

		os << ofs_index << ":";
#if defined(STORE_PRIME)
	#if defined(PRINT_PRIME)
			os << (unsigned int)sbs[i].evalue;
	#else /* of #if defined(PRINT_PRIME) */
			os << compute_bits_from_prime(sbs[i].evalue);
	#endif /* of #if defined(PRINT_PRIME) */
#else /* of #if defined(STORE_PRIME) */
	#if defined(PRINT_PRIME)
			os << compute_prime_from_bits(sbs[i].evalue);
	#else /* of #if defined(PRINT_PRIME) */
			os << (unsigned int)sbs[i].evalue;
	#endif /* of #if defined(PRINT_PRIME) */
#endif /* of #if defined(STORE_PRIME) */

		os << "->{";
		
		offset_block_collection_const_iterator obs_end = obs.end();
		offset_block_collection_const_iterator obs_beg = obs.begin();
		offset_block_collection_const_iterator obs_ite = obs_beg;

		for (; obs_ite != obs_end; obs_ite ++) {
			if (obs_ite != obs_beg) {
				os << " ";
			}

			os << (*obs_ite);
		}
		os << "}";

		ofs_index ++;
	}
	os << "}";
#endif /* of #if !defined(DATASET_SIMPLE) */

	os << "\n";
	
	tree_tabs ++;

	encoding_block_collection_const_iterator isxebs_end = eb->itemset_extensions->end();
	encoding_block_collection_const_iterator isxebs_ite = eb->itemset_extensions->begin();

	for ( ; isxebs_ite != isxebs_end; isxebs_ite ++) {
		for (int i = 0; i < tree_tabs; i ++) {
			os << "  ";
		}

		os << " *";

		sequence seq;

#if (!defined(STORE_SEQUENCE_STRING)) && (!defined(STORE_SEQUENCE))		
		if (print_sequence_detail == true) {
			seq = (*prefix);

			seq.back().push_back((*isxebs_ite)->item);
		}
#endif /* of #if (!defined(STORE_SEQUENCE_STRING)) && (!defined(STORE_SEQUENCE)) */
		
		print_subtree(&seq, (*isxebs_ite), ds_conf, print_sequence_detail, os);
	}

	encoding_block_collection_const_iterator sqxebs_end = eb->sequence_extensions->end();
	encoding_block_collection_const_iterator sqxebs_ite = eb->sequence_extensions->begin();
	
	for ( ; sqxebs_ite != sqxebs_end; sqxebs_ite ++) {
		for (int i = 0; i < tree_tabs; i ++) {
			os << "  ";
		}

		os << " +";
		
		sequence seq;

#if (!defined(STORE_SEQUENCE_STRING)) && (!defined(STORE_SEQUENCE))
		if (print_sequence_detail == true) {
			seq = (*prefix);
		
			seq.push_back(itemset());
			seq.back().push_back((*sqxebs_ite)->item);
		}
#endif /* of #if (!defined(STORE_SEQUENCE_STRING)) && (!defined(STORE_SEQUENCE)) */

		print_subtree(&seq, (*sqxebs_ite), ds_conf, print_sequence_detail, os);
	}

	tree_tabs --;
}

void print_tree(const encoding_block_collection* ebs, const dataset_conf* ds_conf, bool print_sequence_detail, ostream& os) {
	encoding_block_collection_const_iterator ebs_end = ebs->end();
	encoding_block_collection_const_iterator ebs_ite = ebs->begin();

	for ( ; ebs_ite != ebs_end; ebs_ite ++) {
		sequence seq;

#if (!defined(STORE_SEQUENCE_STRING)) && (!defined(STORE_SEQUENCE))
		if (print_sequence_detail == true) {
			seq.push_back(itemset()); 
			seq.back().push_back((*ebs_ite)->item);
		}
#endif /* of #if (!defined(STORE_SEQUENCE_STRING)) && (!defined(STORE_SEQUENCE)) */

		print_subtree(&seq, (*ebs_ite), ds_conf, print_sequence_detail, os);
	}
}

#if defined(DATASET_SIMPLE)

void print_encoding_blocks(const encoding_block_collection* ebs, const dataset_conf* ds_conf, bool printbits, ostream& os) {
	encoding_block_collection_const_iterator ebs_end = ebs->end();
	encoding_block_collection_const_iterator ebs_ite = ebs->begin();

	os << "Item encoding: G = { ";
	for (int i = 0; i < BITS_SIZE; i ++) {
		os << ((int)GENERATOR[i]);
		if (i < BITS_SIZE - 1) {
			os << ", ";
		}
	}
	os << " }, |G| = " << BITS_SIZE << endl << "--------------";

	unsigned int icount = -1;

	for (; ebs_ite != ebs_end; ebs_ite ++) {
		icount ++;

		os << endl << "(" << icount << ") Item " << (*ebs_ite)->item << ":\n";

		os << "\tPos.:\t";

		const eval_t* pbs = (*ebs_ite)->position_blocks;

		for (unsigned int i = 0; i < (*ebs_ite)->position_blocks_size; i++) {
			if (printbits == true) {
				print_bits(COMPUTE_BITS_OF_EVALUE(pbs[i]), os);
				os << "(";
			}
#if defined(STORE_PRIME)
	#if defined(PRINT_PRIME)
			os << (unsigned int)pbs[i];
	#else /* of #if defined(PRINT_PRIME) */
			os << compute_bits_from_prime(pbs[i]);
	#endif /* of #if defined(PRINT_PRIME) */
#else /* of #if defined(STORE_PRIME) */
	#if defined(PRINT_PRIME)
			os << compute_prime_from_bits(pbs[i]);
	#else /* of #if defined(PRINT_PRIME) */
			os << (unsigned int)pbs[i];
	#endif /* of #if defined(PRINT_PRIME) */
#endif /* of #if defined(STORE_PRIME) */
			if (printbits == true) {
				os << ")\t";
			}
			else {
				os << " ";
			}
		}
		
		os << endl << "\tSeq.:\t";
		
		const sequence_block* sbs = (*ebs_ite)->sequence_blocks;
		
		unsigned int idoffs  = 0;
		
		for (unsigned int i = 0 ; i < ds_conf->sbslen; i ++) {
			idoffs ++;

			cout << idoffs - 1 << ":";

			if (printbits == true) {
				print_bits(COMPUTE_BITS_OF_EVALUE(sbs[i].evalue), os);
				os << "(";
			}
#if defined(STORE_PRIME)
	#if defined(PRINT_PRIME)
			os << (unsigned int)sbs[i].evalue;
	#else /* of #if defined(PRINT_PRIME) */
			os << compute_bits_from_prime(sbs[i].evalue);
	#endif /* of #if defined(PRINT_PRIME) */
#else /* of #if defined(STORE_PRIME) */
	#if defined(PRINT_PRIME)
			os << compute_prime_from_bits(sbs[i].evalue);
	#else /* of #if defined(PRINT_PRIME) */
			os << (unsigned int)sbs[i].evalue;
	#endif /* of #if defined(PRINT_PRIME) */
#endif /* of #if defined(STORE_PRIME) */
			os << "->";
			
			os << "{";

			const offset_block_collection& obs = sbs[i].offset_blocks;
			offset_block_collection_const_iterator obs_end = obs.end();
			offset_block_collection_const_iterator obs_beg = obs.begin();
			offset_block_collection_const_iterator obs_ite = obs_beg;
			
			for ( ; obs_ite != obs_end; obs_ite ++) {
				if (obs_ite != obs_beg) {
					os << " ";
				}
				os << (*obs_ite);
			}

			os << "}";
			
			if (printbits == true) {
				os << ") ";
			}
			else {
				os << " ";
			}
		}

		os << endl << "\tSupport = " << (*ebs_ite)->support << "\n";
	} /* end encoding all item */
}

void print_bits(unsigned char n, ostream& os) {
	unsigned char i, step;

	if (0 == n) { /* For simplicity's sake, I treat 0 as a special case*/
		os << "00000000";
		return;
	}

	i = 1<<(sizeof(n) * 8 - 1);

	step = -1; /* Only print the relevant digits */
	step >>= 4; /* In groups of 4 */
	while (step >= n) {
		i >>= 4;
		step >>= 4;
	}

	/* At this point, i is the smallest power of two larger or equal to n */
	if (n <= 0xF) os << "0000";
	while (i > 0) {
		if (n & i)
			os << '1';
		else
			os << '0';
		i >>= 1;
	}
}

#endif /* of #if defined(DATASET_SIMPLE) */

void print_sequence(const sequence* seq, ostream& os) {
	assert(seq != NULL);

	sequence_const_iterator seq_end = seq->end();
	sequence_const_iterator seq_beg = seq->begin();

	for(sequence_const_iterator seq_ite = seq_beg; seq_ite != seq_end; seq_ite ++) {
		if (seq_ite != seq_beg) {
			os << " ";
		}
		print_itemset(seq_ite, os);
	}
}

void print_itemset(sequence_const_iterator is, ostream& os) {
	itemset_const_iterator is_end = is->end();
	itemset_const_iterator is_beg = is->begin();

	os << "<";

	for(itemset_const_iterator is_ite = is_beg; is_ite != is_end; is_ite ++) {
		if (is_ite != is_beg) {
			os << " ";
		}
		
		os << (*is_ite);
	}

	os << ">";
}
