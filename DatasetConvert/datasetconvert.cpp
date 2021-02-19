#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

const char helps[] = \
"\
Convert from IBM AssocGen for sequence format to Sequence database format.\n\
Usage:\n\
	-ia fn : (Required) Input file in binary format, use either -ia or -ib option.\n\
	-ib fn : (Required) Input file in ASCII format, use either -ia or -ib option.\n\
	-oa fn : (Required) ASCII output file, use -oa or ob or both.\n\
	-ob fn : (Required) Binary output file, use -oa or ob or both.\n\
	-h     : (Optional) Show this usage guide.\n\
";

char in_filename[256] = "";
bool in_binary = true;
char oa_filename[256] = "";
bool ou_ascii = false;
char ob_filename[256] = "";
bool ou_binary = false;

void parse_params(int argc, char* argv[]);

int main(int argc, char* argv[]) {
	
	parse_params(argc, argv);

	ifstream in_file;
	ofstream oa_file;
	ofstream ob_file;

	if (in_binary == true) {
		in_file.open(in_filename, ios_base::in | ios_base::binary);
	}
	else {
		in_file.open(in_filename, ios_base::in);
	}
	
	if (ou_ascii == true) {
		oa_file.open(oa_filename, ios_base::out);
	}

	if (ou_binary == true) {
		ob_file.open(ob_filename, ios_base::out | ios_base::binary);
	}

	if (in_file.fail()) {
		cerr << "Fail open input file \"" << in_filename << "\".";
		
		return -1;
	}

	if ((ou_ascii == true) && oa_file.fail()) {
		cerr << "Fail open output ASCII file \"" << oa_filename << "\".";
		in_file.close();
		
		return -1;
	}

	if ((ou_binary == true) && ob_file.fail()) {
		cerr << "Fail open output binary file \"" << ob_filename << "\".";
		in_file.close();
		
		return -1;
	}

	unsigned int last_cid = (unsigned int)(-1);
	vector< vector<unsigned int> > itss; /* itemsets*/

	while(!in_file.eof()) {
		unsigned int cid = (unsigned int)(-1);
		unsigned int tid = (unsigned int)(-1);
		unsigned int nit = 0;
		
		if (in_binary == true) {
			in_file.read((char *)&cid, sizeof(cid));
			in_file.read((char *)&tid, sizeof(tid));
			in_file.read((char *)&nit, sizeof(nit));
		}
		else {
			in_file >> cid;
			in_file >> tid;
			in_file >> nit;
		}
		if (cid != last_cid) {
			if (last_cid != ((unsigned int)(-1))) {
				unsigned int itss_size = (unsigned int)itss.size();

				if (ou_binary == true) {
					ob_file.write((char *)&last_cid, sizeof(cid));
					ob_file.write((char *)&itss_size, sizeof(itss_size));
				}
				if (ou_ascii == true) {
					oa_file << last_cid << " " << itss_size;
				}

				vector< vector<unsigned int> >::const_iterator itss_end = itss.end();
				vector< vector<unsigned int> >::const_iterator itss_ite = itss.begin();

				for ( ; itss_ite != itss_end; itss_ite ++) {
					unsigned int its_size = (unsigned int)itss_ite->size();
					
					if (ou_binary == true) {
						ob_file.write((char *)&its_size, sizeof(its_size));
					}
					if (ou_ascii == true) {
						oa_file << " " << its_size;
					}

					vector<unsigned int>::const_iterator its_end = itss_ite->end();
					vector<unsigned int>::const_iterator its_ite = itss_ite->begin();

					for ( ; its_ite != its_end; its_ite ++) {
						unsigned int curr_iid = (*its_ite);

						if (ou_binary == true) {
							ob_file.write((char *)&curr_iid, sizeof(curr_iid));
						}
						if (ou_ascii == true) {
							oa_file << " " << curr_iid;
						}
					}					
				}
				
				if (ou_ascii == true) {
					oa_file << "\n";
				}
			} 
			
			last_cid = cid;

			itss.clear();
		}

		itss.push_back(vector<unsigned int>());
		vector<unsigned int>& its = itss.back();

		for (unsigned int i = 0; i < nit; i ++) {
			unsigned int iid = (unsigned int)(-1);

			if (in_binary == true) {
				in_file.read((char *)&iid, sizeof(iid));
			}
			else {
				in_file >> iid;
			}
			its.push_back(iid);
		}
	}
		
	oa_file.close();
	in_file.close();
	
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
		else if (strcmp(argv[i], "-oa") == 0) {
			if (i < argc - 1) {
				if (argv[i + 1][0] != '-') {
					strcpy(oa_filename, argv[i + 1]);
					len --;
					i ++;
					ou_ascii = true;
				}
			}
		}
		else if (strcmp(argv[i], "-ob") == 0) {
			if (i < argc - 1) {
				if (argv[i + 1][0] != '-') {
					strcpy(ob_filename, argv[i + 1]);
					len --;
					i ++;
					ou_binary = true;
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
	
	if ((ou_ascii == false) && (ou_binary == false)) {
		cout << "Must have output options. Use -h option for help.";

		exit(0);
	}

	if ((strcmp(oa_filename, "") == 0) && (strcmp(ob_filename, "") == 0)) {
		cout << "Must have output filename. Use -h option for help.";

		exit(0);
	}
}
