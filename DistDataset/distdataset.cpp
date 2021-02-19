#define _CRT_SECURE_NO_WARNINGS 1
#define _SECURE_SCL 0
#define _HAS_ITERATOR_DEBUGGING 0

#include <fstream>
#include <iostream>
#include <map>
#include <list>

using namespace std;

typedef struct __ctid {
	unsigned int cid;
	unsigned int tid;
} ctid;

typedef list<ctid> ctids;

typedef map<unsigned int, void*> item_ctid_map;

const char helps[] = \
"\
Convert from IBM AssocGen for sequence format to Sequence database format.\n\
Usage:\n\
	-ia fn : (Required) Input file in binary format, use either -ia or -ib option.\n\
	-ib fn : (Required) Input file in ASCII format, use either -ia or -ib option.\n\
	-oa fn : (Required) ASCII output file, use -oa or ob or both.\n\
	-ob fn : (Required) Binary output file, use -oa or ob or both.\n\
	-ns #s : (Optional) Number of sites, default 5, 0 for separate file.\n\
	-h     : (Optional) Show this usage guide.\n\
";

char in_filename[256] = "";
bool in_binary = true;
char oa_filename[256] = "";
bool ou_ascii = false;
char ob_filename[256] = "";
bool ou_binary = false;
int numsites = 5;

void parse_params(int argc, char* argv[]);

int main(int argc, char* argv[]) {
	
	parse_params(argc, argv);

	ifstream in_file;
	ofstream oa_file;
	ofstream ob_file;

	if (in_binary == true) {
		in_file.open(in_filename, ios_base::binary);
	}
	else {
		in_file.open(in_filename);
	}

	if (in_file.fail()) {
		cerr << "Fail open input file \"" << in_filename << "\".";
		
		return -1;
	}

	unsigned int last_cid = (unsigned int)(-1);
	item_ctid_map ictmap;

	while(!in_file.eof()) {
		unsigned int cid = (unsigned int)(-1);
		unsigned int tid = (unsigned int)(-1);
		unsigned int tsz = 0;
		
		if (in_binary == true) {
			in_file.read((char *)&cid, sizeof(cid));
			in_file.read((char *)&tid, sizeof(tid));
			in_file.read((char *)&tsz, sizeof(tsz));
		}
		else {
			in_file >> cid;
			in_file >> tid;
			in_file >> tsz;
		}
		
		ctid ct = {cid, tid};

		for (unsigned int i = 0; i < tsz; i ++) {
			unsigned int iid = (unsigned int)(-1);

			if (in_binary == true) {
				in_file.read((char *)&iid, sizeof(iid));
			}
			else {
				in_file >> iid;
			}

			item_ctid_map::iterator iter = ictmap.find(iid);
			if( iter != ictmap.end() ) {
				((ctids*)(iter->second))->push_back(ct);
			}
			else {
				ctids* l = new ctids();
				l->push_back(ct);
				ictmap.insert(make_pair(iid, l));
			}
			
			//cout << cid << " " << tid << " " << iid << "\n";
		}
	}
	
	in_file.close();

	int site = 1;
	size_t numitems = ictmap.size() / numsites;
	int countitems = 0;
	bool newfile = true;

	for(item_ctid_map::iterator m_iter = ictmap.begin(); m_iter != ictmap.end(); ++ m_iter ) {
		unsigned int iid = m_iter->first;

		char outputfilename[256];
		char outputfilename_b[256];

		if (countitems == 0) {
			if (ou_ascii == true) {
				sprintf(outputfilename, "%s%s%02i%s", oa_filename, "-", site, ".SITE");
			}
			
			if (ou_binary == true) {
				sprintf(outputfilename_b, "%s%s%02i%s", ob_filename, "-", site, ".SITE");
			}
			
			if (ou_ascii == true) {
				oa_file.open(outputfilename);
			}

			if (ou_binary == true) {
				ob_file.open(outputfilename_b, ios_base::binary);
			}

			if ((ou_ascii == true) && oa_file.fail()) {
				cerr << "Fail open output ASCII file \"" << outputfilename << "\".";
				
				return -1;
			}

			if ((ou_binary == true) && ob_file.fail()) {
				cerr << "Fail open output binary file \"" << outputfilename << "\".";
				
				return -1;
			}

			if (ou_ascii == true) {
				cout << "Write out " << outputfilename << "\n";
			}
			
			if (ou_binary == true) {
				cout << "Write out " << outputfilename_b << "\n";
			}

			newfile = true;
		}

		countitems ++;

		ctids* l = (ctids*)m_iter->second;

		for(ctids::iterator l_iter = l->begin(); l_iter != l->end(); l_iter ++ ) {
			unsigned int cid = l_iter->cid;
			unsigned int tid = l_iter->tid;

			if (ou_binary == true) {
				ob_file.write((char *)&cid, sizeof(cid));
				ob_file.write((char *)&tid, sizeof(tid));
				ob_file.write((char *)&iid, sizeof(iid));
			}

			if (ou_ascii == true) {
				if (newfile == false) {
					oa_file << "\n";
				}
				else {
					newfile = false;
				}
				oa_file << cid << " " << tid << " " << iid;
			}
		}

		if (countitems == numitems && site < numsites) {
			if (ou_binary == true) {
				ob_file.close();
			}

			if (ou_ascii == true) {
				oa_file.close();
			}

			countitems = 0;
			site ++;
		}

		delete l;
	}

	if (ou_binary == true) {
		ob_file.close();
	}

	if (ou_ascii == true) {
		oa_file.close();
	}

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
		else if (strcmp(argv[i], "-ns") == 0) {
			if (i < argc - 1) {
				if (argv[i + 1][0] != '-') {
					numsites = atoi(argv[i + 1]);
					if (numsites <= 0) {
						numsites = 5;
					}
					len --;
					i ++;
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
