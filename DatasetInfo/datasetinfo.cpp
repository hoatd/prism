#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <iostream>
#include <set>

using namespace std;

const char helps[] = \
"\
Report Sequence database information.\n\
Usage:\n\
	-ia fn : (Required) Input file in binary format, use either -ia or -ib option.\n\
	-ib fn : (Required) Input file in ASCII format, use either -ia or -ib option.\n\
	-h     : (Optional) Show this usage guide.\n\
";

char in_filename[256] = "";
bool in_binary = true;

void parse_params(int argc, char* argv[]);

int main(int argc, char* argv[]) {
	parse_params(argc, argv);

	ifstream ifs;
	
	if (in_binary == true) {
		ifs.open(in_filename, ios_base::in | ios_base::binary);
	}
	else {
		ifs.open(in_filename, ios_base::in);
	}
	
	if (ifs.fail()) {
		cerr << "Fail open input file \"" << in_filename << "\".";
		
		return -1;
	}

	unsigned int min_iid = (unsigned int)(-1);
	unsigned int max_iid = (unsigned int)(-1);
	set<unsigned int> set_iid;

	unsigned int min_seq_size = (unsigned int)(-1);
	unsigned int max_seq_size = (unsigned int)(-1);

	unsigned int min_its_size = (unsigned int)(-1);
	unsigned int max_its_size = (unsigned int)(-1);

	unsigned int avg_seq = 0;
	unsigned int avg_its = 0;

	unsigned int seq_id = 0;
	unsigned int last_sid = (unsigned int)(-1);

	unsigned int numits = 0;

	while(!ifs.eof()) {
		unsigned int sid = (unsigned int)(-1);
		unsigned int seq_size = (unsigned int)(-1);
		
		if (in_binary == true) {
			ifs.read((char *)&sid, sizeof(sid));
		}
		else {
			ifs >> sid;
		}

		if (sid == (unsigned int)(-1)) {
			break;
		}
		
		if ((last_sid != (unsigned int)(-1)) && (last_sid >= sid)) {
			cout << "Invalid order of sequence " << seq_id + 1 << "(" << sid << ").";
			ifs.close();
			return -1;
		}

		if (in_binary == true) {
			ifs.read((char *)&seq_size, sizeof(seq_size));
		}
		else {
			ifs >> seq_size;
		}

		if (seq_size == (unsigned int)(-1)) {
			break;
		}		
		
		numits += seq_size;

		for (unsigned int its_is = 0; its_is < seq_size; its_is ++) {
			unsigned int its_size = (unsigned int)(-1);

			if (in_binary == true) {
				ifs.read((char *)&its_size, sizeof(its_size));
			}
			else {
				ifs >> its_size;
			}

			if ((min_its_size == (unsigned int)(-1)) || (min_its_size > its_size)) {
				min_its_size = its_size;
			}

			if ((max_its_size == (unsigned int)(-1)) || (max_its_size < its_size)) {
				max_its_size = its_size;
			}

			avg_its += its_size;

			unsigned int last_iid = (unsigned int)(-1);

			for (unsigned int itm_id = 0; itm_id < its_size; itm_id ++) {
				unsigned int iid = (unsigned int)(-1);

				if (in_binary == true) {
					ifs.read((char *)&iid, sizeof(iid));
				}
				else {
					ifs >> iid;
				}

				if ((last_iid != (unsigned int)(-1)) && (iid <= last_iid)) {
					cout << "Invalid order of item " << itm_id + 1 << "(" << iid << ")"<< " in itemset " << its_is + 1 << ", sequence " << seq_id + 1 << "(" << sid << ").";
					ifs.close();
					return -1;
				}

				if ((min_iid == (unsigned int)(-1)) || (min_iid > iid)) {
					min_iid = iid;
				}

				if ((max_iid == (unsigned int)(-1)) || (max_iid < iid)) {
					max_iid = iid;
				}

				set_iid.insert(iid);

				last_iid = iid;
			}
		}

		if ((min_seq_size == (unsigned int)(-1)) || (min_seq_size > seq_size)) {
			min_seq_size = seq_size;
		}

		if ((max_seq_size == (unsigned int)(-1)) || (max_seq_size < seq_size)) {
			max_seq_size = seq_size;
		}

		avg_seq += seq_size;

		last_sid = sid;

		seq_id ++;
	}

	ifs.close();

	cout << "Total number of sequences: " << seq_id << ".\n\n";

	cout << "Total number of itemsets: " << numits << ".\n\n";

	cout << "Total number of items: " << set_iid.size() << ".\n";
	cout << "Min item id value: " << min_iid << ".\n";
	cout << "Max item id value: " << max_iid << ".\n\n";
	
	cout << "Min sequence size: " << min_seq_size << " (itemsets).\n";
	cout << "Max sequence size: " << max_seq_size << " (itemsets).\n";
	cout << "Avg sequence size: " << (double)avg_seq / seq_id << " (itemsets).\n\n";
	
	cout << "Min itemset size: " << min_its_size << " (items).\n";
	cout << "Max itemset size: " << max_its_size << " (items).\n";
	cout << "Avg itemset size: " << (double)avg_its / avg_seq << " (items).\n";

	return 0;
}

void parse_params(int argc, char* argv[]) {
	if (argc <= 1) {
		cout << helps;

		exit(0);
	}

	bool ia_option = false;
	bool ib_option = false;
	int len = argc;
	int i = 1;
	
	while (len > 1) {
		if (strcmp(argv[i], "-ia") == 0) {
			ia_option = true;
			if (ib_option == true) {
				cerr << "Use either -ia or -ib option. Use -h option for more detail.";

				exit(0);
			}
			else {
				in_binary = false;
				if (i < argc - 1) {
					if (argv[i + 1][0] != '-') {
						strcpy(in_filename, argv[i + 1]);
						len --;
						i ++;
					}
				}
			}
		}
		else if (strcmp(argv[i], "-ib") == 0) {
			ib_option = true;
			if (ia_option == true) {
				cerr << "Use either -a or -b option. Use -h option for more detail.";

				exit(0);
			}
			else {
				in_binary = true;
				if (i < argc - 1) {
					if (argv[i + 1][0] != '-') {
						strcpy(in_filename, argv[i + 1]);
						len --;
						i ++;
					}
				}
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
		cout << "No input file. Use -h option for help.";
		
		exit(0);
	}	
}
