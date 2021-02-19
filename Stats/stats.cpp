#define _WIN32_WINNT 0x0600 /* For use WaitableTimer Object */
#include <windows.h>
#include <psapi.h>
#include <time.h>
#include <iostream>

using namespace std;

void FtSub(FILETIME* pft, const FILETIME* ft1, const FILETIME* ft2);
__int64 FtToInt(const FILETIME* pft);
__int64 FtIntervalToSeconds(const FILETIME* pft);
__int64 FtIntervalToMilliseconds(const FILETIME* pft);

#define SECOND_IN_MILI 1000
#define MINUTE_IN_SECOND 60.0
#define MINUTE_IN_MILISECOND 60000.0

char szCmdLine[1024] = "";
BOOL suppressChildConsoleOutput = FALSE;
HANDLE hChildConsoleOutput = NULL;
DOUBLE lDelayInterval = 2; /* seconds */
DOUBLE lPeriodicInterval = 1; /* seconds */
DOUBLE totalExecTimeMins = 0.0;
SIZE_T maxSnapshotMemory = 0;
MEMORYSTATUS memStatus;
BOOL exitThread = FALSE;

DWORD WINAPI MemorySnapshot_ThreadProc( LPVOID lpParam );

void parse_params(int argc, char* argv[]);

int main(int argc, char* argv[]) {
	parse_params(argc, argv);

	if (suppressChildConsoleOutput == TRUE) {
		hChildConsoleOutput = CreateFile( L"NUL", GENERIC_ALL, 0, NULL, OPEN_EXISTING, 0, NULL );
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
    
	memset( &si, 0, sizeof( si ) );
	si.cb = sizeof( si );
	if (suppressChildConsoleOutput == TRUE) {
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdInput = hChildConsoleOutput;
		si.hStdOutput = hChildConsoleOutput;
		si.hStdError = hChildConsoleOutput;
	}
	memset( &pi, 0, sizeof( pi ) );
	
	cout << "Spawn " << szCmdLine << " ";

	if (suppressChildConsoleOutput == FALSE) {
		cout << "\n";
	}

	cout.flush();

	wchar_t wszCmdLine[2048] = L"";

	mbstowcs(wszCmdLine, szCmdLine, 1024);

	clock_t start = clock();

	bool finishNormal = false;

	if( !CreateProcess( NULL, wszCmdLine, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi ) ) {
		cout << "\nCreateProcess failed (" << GetLastError( ) << ").";
	}
	else {
		exitThread = FALSE;

		SleepEx( (DWORD)(lDelayInterval * SECOND_IN_MILI), FALSE );

		maxSnapshotMemory = 0;

		HANDLE hThread = CreateThread(NULL, 0, MemorySnapshot_ThreadProc, pi.hProcess, 0, NULL);
		
		if ( !hThread ) {
			cout << "\nCreateThread failed (" << GetLastError( ) << ").";
		}
		else {
			WaitForSingleObject( pi.hProcess, INFINITE );

			exitThread = TRUE;

			WaitForSingleObject( hThread, INFINITE );

			CloseHandle( hThread );
			
			DWORD exiteCode;

			if ( GetExitCodeProcess( pi.hProcess, &exiteCode ) ) {
				if ( exiteCode == 0 ) {
					FILETIME creationFileTime, exitFileTime, kernelFileTime, userFileTime;

					if ( GetProcessTimes( pi.hProcess, &creationFileTime, &exitFileTime, &kernelFileTime, &userFileTime ) ) {
						FILETIME diffFileTime;
						FtSub(&diffFileTime, &exitFileTime, &creationFileTime);
						
						__int64 execTimeMiliSecs = FtIntervalToMilliseconds(&diffFileTime);
						DOUBLE execTimeMins = (double)execTimeMiliSecs / MINUTE_IN_MILISECOND;

						if (suppressChildConsoleOutput == TRUE) {
							cout << "\n";
						}
						cout << "\tExecutive time (Min): " << execTimeMins << ".";
						finishNormal = true;
					}
					else {
						cout << "\nBut occur an unpected error in GetProcessTimes " << GetLastError( ) << ".\n";
					}
				}
				else {
					cout << "\nBut the process report an error code " << exiteCode << ".\n";
				}
			}
			CloseHandle( pi.hProcess );
			CloseHandle( pi.hThread );

			DOUBLE maxMemMb = ((DOUBLE)maxSnapshotMemory) / (1024 * 1024);
			if (finishNormal == false) {
				clock_t finish = clock();
				double minuteDuration = ((double)(finish - start) / CLOCKS_PER_SEC) / 60;

				if (suppressChildConsoleOutput == TRUE) {
					cout << "\n";
				}
				cout << "\tExecutive time (Min): " << minuteDuration << ".";
			}
			cout << "\n\tMaximum memory used (MB): " << maxMemMb << ".";
			if (finishNormal == false) {
				cout << "\n\tMemory status before crashing (MB):";
				cout << "\n\t\tPhysical:\t" << ((DOUBLE)memStatus.dwAvailPhys / (1024 * 1024)) << " / " << ((DOUBLE)memStatus.dwTotalPhys / (1024 * 1024)) << ".";
				cout << "\n\t\tVirtual:\t" << ((DOUBLE)memStatus.dwAvailVirtual / (1024 * 1024)) << " / " << ((DOUBLE)memStatus.dwTotalVirtual / (1024 * 1024)) << ".";
			}
		}
	}

	if ( hChildConsoleOutput ) {
		CloseHandle( hChildConsoleOutput );
	}
	
	return 0;
}

DWORD WINAPI MemorySnapshot_ThreadProc( LPVOID lpParam ) {
	while( exitThread == FALSE ) {
		HANDLE hProcess = (HANDLE)(lpParam);
		PROCESS_MEMORY_COUNTERS pmc;
		
		pmc.cb = sizeof(pmc);
		
		GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));
		
		SIZE_T lMemorySize = pmc.WorkingSetSize;

		if (maxSnapshotMemory < lMemorySize) {
			maxSnapshotMemory = lMemorySize;
		}

		GlobalMemoryStatus(&memStatus);

		SleepEx( (DWORD)(lPeriodicInterval * SECOND_IN_MILI), FALSE );
	}
	
	return 0;
}

const char helps[] = \
"Usage:\n\
	-p : (Required) Process command line and params.\n\
	-d : (Optional) Delay interval before start to monitor, in second, default 2 seconds.\n\
	-v : (Optional) Periodic interval, in second, default 1 second.\n\
	-r : (Optional) Suppress child process stdout.\n\
	-h : (Optional) Show this usage guide.\n\
";

void parse_params(int argc, char* argv[]) {
	if (argc <= 1) {
		cout << helps;

		exit(0);
	}

	int len = argc;
	int i = 1;
	
	while (len > 1) {
		if (strcmp(argv[i], "-p") == 0) {
			if (i < argc - 1) {
				strcpy(szCmdLine, argv[i + 1]);
				len --;
				i ++;
			}
		}
		else if (strcmp(argv[i], "-d") == 0) {
			if (i < argc - 1) {
				lDelayInterval = atof(argv[i + 1]);
				if (lDelayInterval < 0) {
					cerr << "Delay interval MUST greater than ZERO. Use -h option for more detail.";

					exit(0);
				}
				len --;
				i ++;
			}
		}
		else if (strcmp(argv[i], "-v") == 0) {
			if (i < argc - 1) {
				lPeriodicInterval = atof(argv[i + 1]);
				if (lPeriodicInterval < 0) {
					cerr << "Periodic interval MUST greater than ZERO. Use -h option for more detail.";

					exit(0);
				}
				len --;
				i ++;
			}
		}
		else if (strcmp(argv[i], "-r") == 0) {
			suppressChildConsoleOutput = TRUE;
		}
		else if (strcmp(argv[i], "-h") == 0) {
			cout << helps;

			exit(0);
		}

		len --;
		i ++;
	}
}

void FtSub(FILETIME* pft, const FILETIME* ft1, const FILETIME* ft2) {
    LARGE_INTEGER l1, l2;

    l1.LowPart = ft1->dwLowDateTime;
    l1.HighPart = ft1->dwHighDateTime;
    l2.LowPart = ft2->dwLowDateTime;
    l2.HighPart = ft2->dwHighDateTime;

    l1.QuadPart -= l2.QuadPart;

    pft->dwLowDateTime = l1.LowPart;
    pft->dwHighDateTime = l1.HighPart;
}

__int64 FtToInt(const FILETIME* pft) {
    LARGE_INTEGER ll;
    
	ll.LowPart = pft->dwLowDateTime;
    ll.HighPart = pft->dwHighDateTime;
    
	return ll.QuadPart;
}

__int64 FtIntervalToSeconds(const FILETIME* pft) {
    __int64 i = FtToInt(pft);
    
	return (i / 10000000i64);
}

__int64 FtIntervalToMilliseconds(const FILETIME* pft) {
    __int64 i = FtToInt(pft);
    
	return (i / 10000i64);
}