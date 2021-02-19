#if !defined(____PRISM__H____)
#define ____PRISM__H____

#define _SECURE_SCL 0
#define _HAS_ITERATOR_DEBUGGING 0

#include <vector>
#include <fstream>
#include <iostream>

using namespace std;

//#define DATASET_SIMPLE

#define BITS_SIZE 8

//#define STORE_PRIME

#define USE_LINEAR_LOOKUP

//#define STORE_SEQUENCE

/* DEBUG ONLY */
#define STORE_SEQUENCE_STRING
#define PRINT_PRIME

#if !defined(DATASET_SIMPLE)
	#undef STORE_SEQUENCE_STRING
	#undef PRINT_PRIME
#endif /* of #if !defined(DATASET_SIMPLE) */

#if defined(DATASET_SIMPLE)
typedef unsigned char item_t; /* item type */
#else /* of #if defined(DATASET_SIMPLE) */
typedef unsigned int item_t; /* item type */
#endif /* of #if defined(DATASET_SIMPLE) */

typedef vector<item_t> itemset;		/* itemset is a collection of item type, defined as a vector */
typedef vector<item_t>::iterator itemset_iterator; /* iterator of itemset type */
typedef vector<item_t>::const_iterator itemset_const_iterator; /* const iterator of itemset type */

typedef vector<itemset> sequence;	/* sequence is a collection of itemset type, defined as a vector */
typedef vector<itemset>::iterator sequence_iterator; /* iterator of sequence type */
typedef vector<itemset>::const_iterator sequence_const_iterator; /* const iterator of sequence type */

typedef unsigned char bits_t; /* group of 4 or 8 bits type */
typedef unsigned int prime_t; /* encoded value obtained by multiplying of bits_t to G */
typedef unsigned int offs_t; /* offset to an element in a collection (vector) of position encoding blocks */
typedef unsigned char rank_t;
typedef unsigned char card_t;

#if defined(STORE_PRIME)
	#define INVALID_EVALUE 1
	#define MAX_EVALUE 9699690
	typedef prime_t eval_t;
#else /* of #if defined(STORE_PRIME) */
	#define INVALID_EVALUE 0x0
	#define MAX_EVALUE 0xFF
	typedef bits_t eval_t;
#endif /* of #if defined(STORE_PRIME) */

typedef vector<bits_t> bits_collection; /* collection of bits_t, defined as a vector */
typedef vector<bits_t>::iterator bits_collection_iterator; /* iterator of bits_collection type */
typedef vector<bits_t>::const_iterator bits_collection_const_iterator; /* const iterator of bits_collection type */

typedef vector<offs_t> offset_block_collection; /* collection of offs_t, defined as a vector */
typedef vector<offs_t>::iterator offset_block_collection_iterator; /* iterator of offset_block_collection type */
typedef vector<offs_t>::const_iterator offset_block_collection_const_iterator; /* const iterator of offset_block_collection type */

typedef struct __encoding_block encoding_block; /* an encoding block */
typedef vector<encoding_block*> encoding_block_collection; /* collection of encoding_block, defined as a vector */
typedef vector<encoding_block*>::iterator encoding_block_collection_iterator; /* iterator of encoding_block_collection type */
typedef vector<encoding_block*>::const_iterator encoding_block_collection_const_iterator; /* const iterator of encoding_block_collection type */

typedef struct __dataset_conf {
	double rel_minsup;
	unsigned int abs_minsup;
	unsigned int numseqs;
	item_t miniid;
	item_t maxiid;
	unsigned int maxslen;
	unsigned int maxilen;
	unsigned int nitems;
	unsigned int pbslen;
	unsigned int sbslen;
	bool trimpbs;
} dataset_conf;

typedef struct __dataset {
	unsigned int numseqs;
	unsigned int seqscount;
	bool binary;
	ifstream ifs;
} dataset;

typedef struct __sequence_block {
	eval_t evalue;
	offset_block_collection offset_blocks;
} sequence_block;

struct __encoding_block {
	item_t item; /* item of the last extension (last itemset in case sequence extension or last item of last itemset in itemset extension) */
	unsigned int support;

#if defined(STORE_SEQUENCE_STRING)
	string sequence_str; /* string representation for the sequence, ex.: "a->bc", debug only */
#endif /* of #if defined(STORE_SEQUENCE_STRING) */

#if defined(STORE_SEQUENCE)
	sequence seq;
#endif /* of #if defined(STORE_SEQUENCE) */

	eval_t* position_blocks;
	unsigned int position_blocks_size;

	sequence_block* sequence_blocks;

	encoding_block_collection* itemset_extensions;
	encoding_block_collection* sequence_extensions;
}; /* endcoding data structure */

dataset_conf* dataset_conf_init();
void dataset_conf_clean(dataset_conf* ds_conf);
void dataset_conf_precompute(dataset_conf* ds_conf);

dataset* dataset_init(const char* filename, bool in_binary, unsigned int in_numseqs);
void dataset_clean(dataset* ds);
unsigned int dataset_next_sequence(dataset* ds, item_t** seq, unsigned int* itss_size);
#if defined(DATASET_SIMPLE)
unsigned int dataset_next_simple_sequence(dataset* ds, item_t** seq, unsigned int* itss_size);
#else /* of #if defined(DATASET_SIMPLE) */
unsigned int dataset_next_normal_sequence(dataset* ds, item_t** seq, unsigned int* itss_size);
#endif /* of #if defined(DATASET_SIMPLE) */

itemset* itemset_init(const dataset_conf* ds_conf);
void itemset_clean(itemset* items);

encoding_block* encoding_block_create(item_t item, unsigned int pbslen, unsigned int sbslen);
void encoding_block_trim(encoding_block* eb, unsigned int pbslen);
void encoding_block_semiclean(encoding_block* eb);
void encoding_block_delete(encoding_block* eb);

void encoding_block_collection_clean(encoding_block_collection* ebs);

void encode_all_items_in_sequence(	encoding_block_collection* ebs,
									item_t** seq, unsigned int* itss_size, unsigned int seq_size,
									unsigned int seqbits_index, unsigned int seqblks_index,
									bits_collection* last_sequence_bits);
bool encode_item_in_sequence(item_t item,
							 eval_t* pbs, unsigned int& pbs_size,
							 offset_block_collection* obs,
							 item_t** seq, unsigned int* itss_size, unsigned int seq_size);
encoding_block_collection* encode_all_sequences(const itemset* items, dataset* ds, const dataset_conf* ds_conf);

encoding_block* compute_sequence_gdc(const encoding_block* eb1, const encoding_block* eb2, bool is_itsext, const dataset_conf* ds_conf);
offs_t next_position_offset(	size_t ofs_size, 
								size_t current_ofs_index,
								size_t pbs_size,
								sequence_block* sbs, unsigned int seqblks_index, unsigned int seqblks_size);

unsigned int extend_itemset(encoding_block* eb, const encoding_block_collection* ebs_items, const dataset_conf* ds_conf);
unsigned int extend_sequence(encoding_block* eb, const encoding_block_collection* ebs_items, const dataset_conf* ds_conf);
unsigned int extend_tree_collection(encoding_block_collection* ebs, const encoding_block_collection* ebs_items, const dataset_conf* ds_conf, unsigned int level);
unsigned int extend_tree(encoding_block* eb, const encoding_block_collection* ebs_items, const dataset_conf* ds_conf, unsigned int level);
unsigned int extend_root(encoding_block_collection* ebs, const dataset_conf* ds_conf);

void print_tree(const encoding_block_collection* ebs, const dataset_conf* ds_conf, bool print_sequence_detail, ostream& os = cout);
void print_subtree(const sequence* prefix, const encoding_block* eb, const dataset_conf* ds_conf, bool print_sequence_detail, ostream& os = cout);
void print_sequence(const sequence* seq, ostream& os = cout);
void print_itemset(sequence_const_iterator is, ostream& os = cout);
#if defined(DATASET_SIMPLE)
void print_encoding_blocks(const encoding_block_collection* ebs, const dataset_conf* ds_conf, bool printbists, ostream& os = cout);
void print_bits(unsigned char n, ostream& os=cout);
#endif /* of #if defined(DATASET_SIMPLE) */

/* Common */
void store_invalid_position_blocks(	offset_block_collection* obs, bool& is_first_offs,
									eval_t* pbs, unsigned int& pbs_size,
									unsigned int invalid_block_count);
void store_valid_position_block(offset_block_collection* obs, bool& is_first_offs,
								eval_t* pbs, unsigned int& pbs_size,
								eval_t evalue);

/* Base */
prime_t compute_prime_from_bits(bits_t bits);

/* Pre-Computed tables */
#if defined(STORE_PRIME)

#include "prism_tables.h"

#define COMPUTE_GDC_EVALUE(eval1, eval2) GDC_OF((eval1), (eval2))
#define COMPUTE_MASK_OF_EVALUE(eval) MASK_OF(eval)
#define COMPUTE_BITS_OF_EVALUE(eval) BITS_OF(eval)
#define COMPUTE_EVALUE_OF_BITS(bits) compute_prime_from_bits(bits)
#define COMPUTE_SUPPORT_OF_EVALUE(eval) CARD_OF(eval)
#else /* of #if !defined(STORE_PRIME) */

bits_t compute_bits_from_prime(prime_t prime);
card_t compute_card_from_bits(bits_t bits);
bits_t compute_mask_from_bits(bits_t bits);

#define COMPUTE_GDC_EVALUE(eval1, eval2) ((eval1) & (eval2))
#define COMPUTE_MASK_OF_EVALUE(eval) compute_mask_from_bits(eval)
#define COMPUTE_BITS_OF_EVALUE(eval) (eval)
#define COMPUTE_EVALUE_OF_BITS(bits) (bits)
#define COMPUTE_SUPPORT_OF_EVALUE(eval) compute_card_from_bits(eval)
#endif /* of #if defined(STORE_PRIME) */

#endif /* of #if !defined(____PRISM__H____) */
