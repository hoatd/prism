#include "prism.h"

#include <time.h>
#include <cassert>

char in_filename[256] = "";
bool in_binary = true;
bool ou_stf = false;
char ou_stf_filename[256] = "";

void parse_params(int argc, char* argv[], dataset_conf* ds_conf);

int main(int argc, char* argv[]) {
	dataset_conf* ds_conf = dataset_conf_init();
	
	parse_params(argc, argv, ds_conf);
	
	dataset_conf_precompute(ds_conf);
	
	cout << "Encoding " << ds_conf->nitems << " items in " << ds_conf->numseqs << " sequences with minsup = " << ds_conf->abs_minsup << " (" << ds_conf->rel_minsup * 100 << "%) ... ";
	
	dataset* ds = dataset_init(in_filename, in_binary, ds_conf->numseqs);
	
	if (ds == NULL) {
		cerr << "Fail to open dataset file " << in_filename << ".";
		return -1;
	}
	

	itemset* items = itemset_init(ds_conf);
	
	clock_t start_encoding = clock();
	
	encoding_block_collection* ebs = encode_all_sequences(items, ds, ds_conf);

	clock_t finish_encoding = clock();
	
	double encoding_duration = ((double)(finish_encoding - start_encoding) / CLOCKS_PER_SEC);
	
	cout << encoding_duration << " seconds.\n";

/**/
	ofstream o_ebs("PRISM_ENCODED_BLOCKS.LOG");

	encoding_block_collection_iterator ebs_end = ebs->end();
	encoding_block_collection_iterator ebs_ite = ebs->begin();
	for ( ; ebs_ite != ebs_end; ebs_ite ++) {
		o_ebs << "ITEM " << (int)(*(ebs_ite))->item << " SUPPORT " << (*ebs_ite)->support << "\n";
		o_ebs << "SEQ SIZE [" << ds_conf->sbslen << "] ";
		for (unsigned int i = 0; i < ds_conf->sbslen; i ++) {
			o_ebs << (int)(*(ebs_ite))->sequence_blocks[i].evalue << " ";

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
		o_ebs << "POS SIZE [" << (*(ebs_ite))->position_blocks_size << "] ";
		for (unsigned int i = 0; i < (*(ebs_ite))->position_blocks_size; i ++) {
			o_ebs << ((int)(*(ebs_ite))->position_blocks[i]) << " ";
		}

		o_ebs << "\n";
	}

	o_ebs << "rel_minsup = " << ds_conf->rel_minsup << "\n";
	o_ebs << "abs_minsup = " << ds_conf->abs_minsup << "\n";
	o_ebs << "numseqs = " << ds_conf->numseqs << "\n";
	o_ebs << "miniid = " << ds_conf->miniid << "\n";
	o_ebs << "maxiid = " << ds_conf->maxiid << "\n";
	o_ebs << "maxslen = " << ds_conf->maxslen << "\n";
	o_ebs << "maxilen = " << ds_conf->maxilen << "\n";
	o_ebs << "nitems = " << ds_conf->nitems << "\n";
	o_ebs << "pbslen = " << ds_conf->pbslen << "\n";
	o_ebs << "sbslen = " << ds_conf->sbslen << "\n";
	o_ebs << "trimpbs = " << ds_conf->trimpbs << "\n";

	o_ebs.close();

	//return 1;
/**/

	
	
	dataset_clean(ds);
	
	itemset_clean(items);
	
#if defined(DATASET_SIMPLE)
	print_encoding_blocks(ebs, ds_conf, true, cout);
#endif /* of #if defined(DATASET_SIMPLE) */
	
	cout << "Extending " << ebs->size() << " encoded items satisfied the minsup ... ";
	
	clock_t start_extension = clock();
	
	unsigned int numblks = (unsigned int)ebs->size() + extend_root(ebs, ds_conf);
	
	clock_t finish_extension = clock();
	
	double extension_duration = ((double)(finish_extension - start_extension) / CLOCKS_PER_SEC);
	
	cout << extension_duration << " seconds.\n";
	
	cout << "Result " << numblks << " frequent sequences in total " << encoding_duration + extension_duration << " seconds.\n";
	
	if (ou_stf == true) {
		if (strcmp(ou_stf_filename, "") == 0) {
			print_tree(ebs, ds_conf, true, cout);
		}
		else {
			ofstream stf_ofs(ou_stf_filename);
			
			if (stf_ofs.fail() == true) {
				cerr << "IO fail. Can not open for write out STF file '" << ou_stf_filename << "'.";
				return -1;
			}
			
			cout << "Write out search tree result to file " << ou_stf_filename << " ... ";
			
			print_tree(ebs, ds_conf, true, stf_ofs);
			
			cout << "ok.\n";
		}
	}
	
	encoding_block_collection_clean(ebs);
	
	dataset_conf_clean(ds_conf);
	
	return 0;
}


const char helps[] = \
"Usage:\n\
	-ia fn : (Required) Sequence database file in ASCII format, use either -ia or -ib option.\n\
	-ib fn : (Required) Sequence database file in binary format, use either -ia or -ib option.\n\
	-ns #  : (Required) Total number of sequences (positive interger value).\n\
	-im #  : (Required) Minimun item id value (positive interger).\n\
	-ix #  : (Required) Maximun item id value (positive interger).\n\
	-sl #  : (Optional) Maximun sequence length (positive interger), default 0 for unknown.\n\
	-il #  : (Optional) Maximun itemset length (positive interger), default 0 for unknown.\n\
	-s  ms : (Optional) Input minimum support threshold, [0, 1], default minsup = 50%.\n\
	-ot fn : (Optional) File for output the search tree result. Use -ot without filename for printing out result to stdout.\n\
	-h     : (Optional) Show this usage guide.\n\
";

void parse_params(int argc, char* argv[], dataset_conf* ds_conf) {
	if (argc <= 1) {
		cout << helps;
		exit(0);
	}

	bool a_option = false;
	bool b_option = false;
	int len = argc;
	int i = 1;
	
	ds_conf->maxslen =
	ds_conf->maxilen = 0;

	while (len > 1) {
		if (strcmp(argv[i], "-ia") == 0) {
			a_option = true;
			if (b_option == true) {
				cerr << "MUST use either -ia or -ib option. Use -h option for more detail.";
				exit(0);
			}
			else {
				in_binary = false;
				if (i < argc - 1) {
					strcpy(in_filename, argv[i + 1]);
					len --;
					i ++;
				}
			}
		}
		else if (strcmp(argv[i], "-ib") == 0) {
			b_option = true;
			if (a_option == true) {
				cerr << "MUST use either -ia or -ib option. Use -h option for more detail.";
				exit(0);
			}
			else {
				in_binary = true;
				if (i < argc - 1) {
					strcpy(in_filename, argv[i + 1]);
					len --;
					i ++;
				}
			}
		}
		else if (strcmp(argv[i], "-ns") == 0) {
			if (i < argc - 1) {
				ds_conf->numseqs = atoi(argv[i + 1]);
				if (ds_conf->numseqs == 0) {
					cerr << "Total number of sequences in database MUST greater than ZERO. Use -h option for more detail.";
					exit(0);
				}
				len --;
				i ++;
			}
		}
		else if (strcmp(argv[i], "-im") == 0) {
			if (i < argc - 1) {
#if defined(DATASET_SIMPLE)
				ds_conf->miniid = argv[i + 1][0];
#else /* of #if defined(DATASET_SIMPLE) */
				ds_conf->miniid = atoi(argv[i + 1]);
#endif /* of #if defined(DATASET_SIMPLE) */
				len --;
				i ++;
			}
		}
		else if (strcmp(argv[i], "-ix") == 0) {
			if (i < argc - 1) {
#if defined(DATASET_SIMPLE)
				ds_conf->maxiid = argv[i + 1][0];
#else /* of #if defined(DATASET_SIMPLE) */
				ds_conf->maxiid = atoi(argv[i + 1]);
#endif /* of #if defined(DATASET_SIMPLE) */
				len --;
				i ++;
			}
		}
		else if (strcmp(argv[i], "-sl") == 0) {
			if (i < argc - 1) {
				ds_conf->maxslen = atoi(argv[i + 1]);
				len --;
				i ++;
			}
		}
		else if (strcmp(argv[i], "-il") == 0) {
			if (i < argc - 1) {
				ds_conf->maxilen = atoi(argv[i + 1]);
				len --;
				i ++;
			}
		}
		else if (strcmp(argv[i], "-s") == 0) {
			if (i < argc - 1) {
				ds_conf->rel_minsup = atof(argv[i + 1]);
				if ((ds_conf->rel_minsup < 0) || (ds_conf->rel_minsup > 1)) {
					cerr << "Minimum support must be in [0, 1]. Use -h option for more detail.";

					exit(0);
				}
				len --;
				i ++;
			}
		}
		else if (strcmp(argv[i], "-ot") == 0) {
			ou_stf = true;
			if (i < argc - 1) {
				strcpy(ou_stf_filename, argv[i + 1]);
				len --;
				i ++;
			}
		}
		else if (strcmp(argv[i], "-h") == 0) {
			cout << helps;

			exit(0);
		}

		len --;
		i ++;
	}

	if (strcmp(in_filename, "") == 0) {
		cerr << "No input SDB. Use -h option for help.";
		
		exit(0);
	}

	if (ds_conf->numseqs == 0) {
		cerr << "MUST input the total number of sequences in database. Use -h option for more detail.";

		exit(0);
	}

	if (ds_conf->maxiid <= ds_conf->miniid) {
		cerr << "MUST input the min & max item id. Use -h option for more detail.";

		exit(0);
	}

	if (ds_conf->maxslen == 0 || ds_conf->maxilen == 0) {
		cerr << "MUST input the minslen & maxilen. Use -h option for more detail.";

		exit(0);
	}
}
