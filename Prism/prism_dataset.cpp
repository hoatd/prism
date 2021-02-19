#include "prism.h"

#include <cmath>
#include <cassert>

#define LINE_SIZE 256

dataset_conf* dataset_conf_init() {
	dataset_conf* ds_conf = new dataset_conf();

	ds_conf->rel_minsup = 0.05;
	ds_conf->abs_minsup = 0;
	ds_conf->numseqs = 0;
	ds_conf->miniid = 0;
	ds_conf->maxiid = 0;
	ds_conf->maxslen = 0;
	ds_conf->maxilen = 0;
	ds_conf->nitems = 0;
	ds_conf->pbslen = 1;
	ds_conf->sbslen = 1;
	ds_conf->trimpbs = false;
	return ds_conf;
}

void dataset_conf_precompute(dataset_conf* ds_conf) {
	assert(ds_conf != NULL);

	ds_conf->nitems = ds_conf->maxiid - ds_conf->miniid + 1;
	ds_conf->abs_minsup = (unsigned int)ceil(ds_conf->rel_minsup * ds_conf->numseqs);
	ds_conf->pbslen = (ds_conf->numseqs * ds_conf->maxslen * ds_conf->maxilen) / BITS_SIZE + (((ds_conf->numseqs * ds_conf->maxslen * ds_conf->maxilen) % BITS_SIZE == 0) ? 0 : 1);
	ds_conf->sbslen = ds_conf->numseqs / BITS_SIZE + ((ds_conf->numseqs % BITS_SIZE == 0) ? 0 : 1);
}

void dataset_conf_clean(dataset_conf* ds_conf) {
	assert(ds_conf != NULL);

	delete ds_conf;

	ds_conf = NULL;
}

dataset* dataset_init(const char* filename, bool in_binary, unsigned int in_numseqs) {
	dataset* ds = new dataset();

	if (in_binary == true) {
		ds->ifs.open(filename, ios_base::in | ios_base::binary);
	}
	else {
		ds->ifs.open(filename, ios_base::in);
	}
	
	if (ds->ifs.fail() == true) {
		return NULL;
	}

	ds->binary = in_binary;

	ds->numseqs = in_numseqs;
	ds->seqscount = 0;

	return ds;
}

void dataset_clean(dataset* ds) {
	assert(ds != NULL);

	ds->ifs.close();

	delete ds;

	ds = NULL;
}

unsigned int dataset_next_sequence(dataset* ds, item_t** itss, unsigned int* itss_size) {
	assert(ds != NULL);
	assert(itss != NULL);
	assert(itss_size != NULL);

#if defined(DATASET_SIMPLE)
	return dataset_next_simple_sequence(ds, itss, itss_size);
#else /* of #if defined(DATASET_SIMPLE) */
	return dataset_next_normal_sequence(ds, itss, itss_size);
#endif /* of #if defined(DATASET_SIMPLE) */
}

#if defined(DATASET_SIMPLE)

unsigned int dataset_next_simple_sequence(dataset* ds, item_t** seq, unsigned int* itss_size) {
	assert(ds != NULL);
	assert(seq != NULL);
	assert(itss_size != NULL);

	if (ds->seqscount >= ds->numseqs) {
		return 0;
	}

	char buff[LINE_SIZE];			
	char* ite = buff;
	
	if(!ds->ifs.eof()) {
		ds->ifs.getline(buff, LINE_SIZE);

		unsigned int its_idx = 0;
		
		while(*ite) {
			item_t item = (*ite);

			if (((item >= 'a') && (item <= 'z')) || ((item >= 'A') && (item <= 'Z'))) {
				
				unsigned int it_idx = 0;
				
				do {
					seq[its_idx][it_idx] = item;

					ite ++;
					it_idx ++;

					item = (*ite);
				}
				while (((item >= 'a') && (item <= 'z')) || ((item >= 'A') && (item <= 'Z')));

				itss_size[its_idx] = it_idx;

				its_idx ++;
			}

			ite ++;
		}

		ds->seqscount ++;

		return its_idx;
	}

	return 0;
}
 
#else /* of #if defined(DATASET_SIMPLE) */

unsigned int dataset_next_normal_sequence(dataset* ds, item_t** seq, unsigned int* itss_size) {
	assert(ds != NULL);
	assert(seq != NULL);
	assert(itss_size != NULL);

	if (ds->seqscount >= ds->numseqs) {
		return 0;
	}

	if(!ds->ifs.eof()) {
		unsigned int sid = (unsigned int)(-1);
		unsigned int seq_size = (unsigned int)(-1);
		
		if (ds->binary == true) {
			ds->ifs.read((char *)&sid, sizeof(sid));
			ds->ifs.read((char *)&seq_size, sizeof(seq_size));
		}
		else {
			ds->ifs >> sid;
			ds->ifs >> seq_size;
		}

		if (sid == (unsigned int)(-1)) {
			return 0;
		}

		for (unsigned int i = 0; i < seq_size; i ++) {
			unsigned int its_size = (unsigned int)(-1);

			if (ds->binary == true) {
				ds->ifs.read((char *)&its_size, sizeof(its_size));
			}
			else {
				ds->ifs >> its_size;
			}

			itss_size[i] = its_size;

			for (unsigned int j = 0; j < its_size; j ++) {
				unsigned int iid = (unsigned int)(-1);

				if (ds->binary == true) {
					ds->ifs.read((char *)&iid, sizeof(iid));
				}
				else {
					ds->ifs >> iid;
				}

				seq[i][j] = iid;
			}
		}

		ds->seqscount ++;

		return seq_size;
	}

	return 0;
}

#endif /* of #if defined(DATASET_SIMPLE) */
