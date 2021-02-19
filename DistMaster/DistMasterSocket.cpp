#include "stdafx.h"
#include "DistMaster.h"
#include "DistMasterSocket.h"
#include "DistMasterMiner.h"

//#define OUTPUT_TRACKING

#if defined(OUTPUT_TRACKING)
char OUTPUT_TRACKING_FILENAME[256] = "MASTER_ENCODED_BUFFERS.LOG";
#endif // defined(OUTPUT_TRACKING)

SLAVER_ADDRESS* SLAVER_ADDRESSES;
/*
	= {
	{"192.168.0.147", 9001},
	{"192.168.0.103", 9001},
	{"192.168.0.103", 9002},
	{"127.0.0.1", 9004},
	{"127.0.0.1", 9005},
	{"127.0.0.1", 9006},
	{"127.0.0.1", 9007},
	{"127.0.0.1", 9008},
	{"127.0.0.1", 9009},
	{"127.0.0.1", 9010}
};
*/
int NUM_SLAVER_ADDRESS = 0;

BOOL SocketInitMaster() {
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != NO_ERROR) {
		AfxMessageBox(L"INITIATE SOCKET ERROR.\n");
		return FALSE;
	}

	return TRUE;
}

VOID SocketCleanMaster() {
	WSACleanup();
}

int SocketSafeRecvBuffer(SOCKET sd, char* buff, int size) {
	int recvSize = 0;
	int remainSize = size;
	char* curr_buff = buff;

	while(recvSize < size) {
		int iResult = recv(sd, curr_buff, remainSize, 0);
		if (iResult == SOCKET_ERROR) {
			return iResult;
		}
		remainSize -= iResult;
		recvSize += iResult;
		curr_buff += iResult;
	}

	return recvSize;
}

int SocketSafeSendBuffer(SOCKET sd, const char* buff, int size) {
	int sendSize = 0;
	int remainSize = size;
	const char* curr_buff = buff;

	while(sendSize < size) {
		int iResult = send(sd, curr_buff, remainSize, 0);
		if (iResult == SOCKET_ERROR) {
			return iResult;
		}
		remainSize -= iResult;
		sendSize += iResult;
		curr_buff += iResult;
	}

	return sendSize;
}

INT ERROR_CODE = 0;

UINT SocketThreadFuncMasterClient(LPVOID lParam) {
	int site = *((int*)lParam);

	SetStatusMaster(L"COMMUNICATING ...");

	SetStatusSite(site, L"CONNECTING ...");
	
	SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ConnectSocket == INVALID_SOCKET) {
		SetStatusSite(site, L"ERROR ON CREATE A SOCKET.");
		
		ERROR_CODE = 0;
		return 0;
	}

	sockaddr_in clientService; 
		
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(SLAVER_ADDRESSES[site].strAddress);
	clientService.sin_port = htons(SLAVER_ADDRESSES[site].nPort);

	if ( connect( ConnectSocket, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		SetStatusSite(site, L"FAIL ON CONNECTING.");

		ERROR_CODE = 0;

		return 0;
	}

	SetStatusSite(site, L"CONNECT SUCCESS.");

	if (SocketSendMasterInfo(ConnectSocket) == FALSE) {
		closesocket(ConnectSocket);
		SetStatusSite(site, L"ERROR WHEN SEND MASTER INFORMATION.");

		ERROR_CODE = 0;

		return 0;
	}

	SetStatusSite(site, L"SEND MASTER INFORMATION SUCCESS.");

	int num_iid = 0;
	int status = 1;
	int iResult = SOCKET_ERROR;
	
	iResult = SocketSafeRecvBuffer(ConnectSocket, (char*)&status, sizeof(status));
	if (iResult == SOCKET_ERROR) {
		
		ERROR_CODE = 0;

		return 0;
	}
	assert(iResult == sizeof(status));

	if (status == 0) {
		SetStatusSite(site, L"ERROR ON SEQUENCE DATA FILE.");
	
		shutdown(ConnectSocket, SD_SEND);

		closesocket(ConnectSocket);

		ERROR_CODE = 1;

		return 1;
	}
	
	iResult = SocketSafeRecvBuffer(ConnectSocket, (char*)&num_iid, sizeof(num_iid));
	if (iResult == SOCKET_ERROR) {
		ERROR_CODE = 0;
		return 0;
	}
	assert(iResult == sizeof(num_iid));

#if defined(OUTPUT_TRACKING)
	ofstream OUTPUT_TRACKING_STREAM;
	OUTPUT_TRACKING_STREAM.open(OUTPUT_TRACKING_FILENAME);
	if (OUTPUT_TRACKING_STREAM.fail()) {
		cerr << "Fail open output tracking file stream\"" << OUTPUT_TRACKING_FILENAME << "\".";
		ERROR_CODE = 0;
		return 0;
	}
#endif // defined(OUTPUT_TRACKING)
	for (int i = 0; i < num_iid; i ++) {
		id_t iid = 0;
		int seq_bblk_size = 0;
		int pos_bblk_size = 0;
		int pof_buff_size = 0;
		bblk_t* seq_bblk = NULL;
		bblk_t* pos_bblk = NULL;
		int* pof_buff = NULL;

		if (SocketRecvEncodedInfo(ConnectSocket, iid, seq_bblk, seq_bblk_size, pos_bblk, pos_bblk_size, pof_buff, pof_buff_size) == FALSE) {
			closesocket(ConnectSocket);
			SetStatusSite(site, L"ERROR WHEN RECEIVE SLAVER ENCODED INFORMATION.");

			ERROR_CODE = 0;
			return 0;
		}

#if defined(OUTPUT_TRACKING)
		OUTPUT_TRACKING_STREAM << "ITEM " << (int)iid << "\n";
		OUTPUT_TRACKING_STREAM << "SEQ SIZE [" << seq_bblk_size << "] ";
		for (int i = 0; i < seq_bblk_size; i ++) {
			OUTPUT_TRACKING_STREAM << ((int)(seq_bblk[i])) << " ";
		}
		OUTPUT_TRACKING_STREAM << "\n";
		OUTPUT_TRACKING_STREAM << "POS SIZE [" << pos_bblk_size << "] ";
		for (int i = 0; i < pos_bblk_size; i ++) {
			OUTPUT_TRACKING_STREAM << ((int)(pos_bblk[i])) << " ";
		}
		OUTPUT_TRACKING_STREAM << "\n";
		OUTPUT_TRACKING_STREAM << "OFF SIZE [" << pof_buff_size << "] ";
		for (int i = 0; i < pof_buff_size; i ++) {
			OUTPUT_TRACKING_STREAM << ((int)(pof_buff[i])) << " ";
		}
		OUTPUT_TRACKING_STREAM << "\n";
#endif // defined(OUTPUT_TRACKING)

		assert(seq_bblk_size == DS_CONF->sbslen);

		encoding_block* eb = encoding_block_create_from_encoded_buffers(iid,
										seq_bblk, seq_bblk_size,
										pos_bblk, pos_bblk_size,
										pof_buff, pof_buff_size);

		sync_encoding_block_collection_update(eb);

		if (seq_bblk) delete []seq_bblk;
		if (pos_bblk) delete []pos_bblk;
		if (pof_buff) delete []pof_buff;
	}
#if defined(OUTPUT_TRACKING)
	OUTPUT_TRACKING_STREAM.close();
#endif // defined(OUTPUT_TRACKING)

	SetStatusSite(site, L"RECEIVED SLAVER ENCODED INFORMATION SUCCESS.");
	
	shutdown(ConnectSocket, SD_SEND);

	closesocket(ConnectSocket);

	ERROR_CODE = 2;
	return 2;
}

BOOL SocketSendMasterInfo(SOCKET sd) {
	char seqdataname[256];

	int sz = GetSeqDataName(seqdataname);

	int iResult = SOCKET_ERROR;
	
	iResult = SocketSafeSendBuffer(sd, (const char*)&sz, sizeof(sz));
    if (iResult == SOCKET_ERROR) {
        return FALSE;
    }
	assert(iResult == sizeof(sz));

	iResult = SocketSafeSendBuffer(sd, (char*)seqdataname, sizeof(char) * sz);
    if (iResult == SOCKET_ERROR) {
        return FALSE;
    }
	assert(iResult == sizeof(char) * sz);

	iResult = SocketSafeSendBuffer(sd, (const char*)&DS_CONF->numseqs, sizeof(DS_CONF->numseqs));
    if (iResult == SOCKET_ERROR) {
        return FALSE;
    }
	assert(iResult == sizeof(DS_CONF->numseqs));

	iResult = SocketSafeSendBuffer(sd, (const char*)&DS_CONF->rel_minsup, sizeof(DS_CONF->rel_minsup));
    if (iResult == SOCKET_ERROR) {
        return FALSE;
    }
	assert(iResult == sizeof(DS_CONF->rel_minsup));

	return TRUE;
}

BOOL SocketRecvEncodedInfo(SOCKET sd,
						 id_t& iid,
						 bblk_t*& seq_bblk, int& seq_bblk_size,
						 bblk_t*& pos_bblk, int& pos_bblk_size,
						 int*& pof_buff, int& pof_buff_size) {
	int iResult = SOCKET_ERROR;
	
	iResult = SocketSafeRecvBuffer(sd, (char*)&iid, sizeof(iid));
    if (iResult == SOCKET_ERROR) {
        return FALSE;
    }
	assert(iResult == sizeof(iid));

	iResult = SocketSafeRecvBuffer(sd, (char*)&seq_bblk_size, sizeof(seq_bblk_size));
    if (iResult == SOCKET_ERROR) {
        return FALSE;
    }
	assert(iResult == sizeof(seq_bblk_size));

	seq_bblk = init_bblk_buffer(seq_bblk_size);
	
	iResult = SocketSafeRecvBuffer(sd, (char*)seq_bblk, sizeof(bblk_t) * seq_bblk_size);
    if (iResult == SOCKET_ERROR) {
        return FALSE;
    }
	assert(iResult == sizeof(bblk_t) * seq_bblk_size);

	iResult = SocketSafeRecvBuffer(sd, (char*)&pos_bblk_size, sizeof(pos_bblk_size));
    if (iResult == SOCKET_ERROR) {
        return FALSE;
    }
	assert(iResult == sizeof(pos_bblk_size));

	pos_bblk = init_bblk_buffer(pos_bblk_size);

	iResult = SocketSafeRecvBuffer(sd, (char*)pos_bblk, sizeof(bblk_t) * pos_bblk_size);
    if (iResult == SOCKET_ERROR) {
        return FALSE;
    }
	assert(iResult == sizeof(bblk_t) * pos_bblk_size);

	iResult = SocketSafeRecvBuffer(sd, (char*)&pof_buff_size, sizeof(pof_buff_size));
    if (iResult == SOCKET_ERROR) {
        return FALSE;
    }
	assert(iResult == sizeof(pof_buff_size));

	pof_buff = new int[pof_buff_size];
	memset(pof_buff, 0, sizeof(pof_buff_size) * pof_buff_size);

	iResult = SocketSafeRecvBuffer(sd, (char*)pof_buff, (int)sizeof(int) * pof_buff_size);
    if (iResult == SOCKET_ERROR) {
        return FALSE;
    }
	assert(iResult == sizeof(int) * pof_buff_size);

	return TRUE;
}

