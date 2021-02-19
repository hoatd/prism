#include "stdafx.h"
#include "DistSlaver.h"
#include "DistSlaverSocket.h"
#include "DistSlaverMiner.h"
#define _CRT_SECURE_NO_WARNINGS 1

CString SocketAddress("127.0.0.1");
int SocketPort = 9001;
int SiteId = 1;
extern char SEQDATA_FILENAME[];
extern int MAX_CID;
extern double MINSUP_IN_PERCENT;
int SocketSafeRecvBuffer(SOCKET sd, char* buff, int size);
BOOL SocketRecvMasterInfo(SOCKET sd);

BOOL SocketInitSlaver() {
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != NO_ERROR) {
		MessageBox(NULL, L"INITIATE SOCKET ERROR.\n", strCaption, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	return TRUE;
}

BOOL InitSocketInfo() {
	char localname[256];
	
	if (gethostname(localname, 255)) {
		MessageBox(NULL, L"NETWORK ERROR IN gethostname() FUNCTION.\n", strCaption, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	struct hostent* localhost = gethostbyname(localname);
	if(!localhost) {
		MessageBox(NULL, L"NETWORK ERROR IN gethostbyname() FUNCTION.\n", strCaption, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	struct sockaddr_in SocketAddr;
	char localipaddr[255];

	memcpy(&SocketAddr.sin_addr, localhost->h_addr_list[0], localhost->h_length);
	strcpy(localipaddr, inet_ntoa(SocketAddr.sin_addr));

	SocketAddress = localipaddr;

	return TRUE;
}

VOID SocketCleanSlaver() {
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

void print_slaver_encoded_buffers(encode_info_vector& eis) {
	ofstream o_ebs("SLAVER_ENCODED_BUFFERS.LOG");
	for (encode_info_vector::iterator eis_iter = eis.begin(); eis_iter != eis.end(); eis_iter ++) {
		o_ebs << "ITEM " << (int)eis_iter->iid << "\n";
		o_ebs << "SEQ SIZE [" << eis_iter->seq_bblk_size << "] ";
		for (int i = 0; i < eis_iter->seq_bblk_size; i ++) {
			o_ebs << ((int)(eis_iter->seq_bblk[i])) << " ";
		}
		o_ebs << "\n";
		o_ebs << "POS SIZE [" << eis_iter->pos_bblk_size << "] ";
		for (int i = 0; i < eis_iter->pos_bblk_size; i ++) {
			o_ebs << ((int)(eis_iter->pos_bblk[i])) << " ";
		}
		o_ebs << "\n";	
		o_ebs << "OFF SIZE [" << eis_iter->pof_buff_size << "] ";
		for (int i = 0; i < eis_iter->pof_buff_size; i ++) {
			o_ebs << ((int)(eis_iter->pof_buff[i])) << " ";
		}
		o_ebs << "\n";	
	}
	o_ebs.close();
}

UINT SocketThreadFuncSlaverServer(LPVOID lParam) {
	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	if (ListenSocket == INVALID_SOCKET) {
		MessageBox(NULL, L"NETWORK ERRORS: SLAVER SITE CAN NOT MAKE A SOCKET FOR COMMUNICATE WITH MASTER.", strCaption, MB_OK | MB_ICONERROR);

		return 0;
	}
		
	sockaddr_in sinInterface;
	
	char SocketAddressInChar[256];

	wcstombs(SocketAddressInChar, (LPCTSTR)SocketAddress, 256);

	sinInterface.sin_family = AF_INET;
    sinInterface.sin_addr.s_addr = inet_addr(SocketAddressInChar);
    sinInterface.sin_port = htons(SocketPort);

    if (bind(ListenSocket, (sockaddr*)&sinInterface, sizeof(sockaddr_in)) == SOCKET_ERROR) {
		closesocket(ListenSocket);
		
		CString sPort;

		sPort.Format(L"%i", SocketPort);
		
		MessageBox(NULL, L"NETWORK ERRORS: SLAVER SITE CAN NOT BIND THE SOCKET TO " + SocketAddress + L":" + sPort, strCaption, MB_OK | MB_ICONERROR);

		return 0;
    }

	if (listen(ListenSocket, 1) == SOCKET_ERROR) {
		closesocket(ListenSocket);
		
		CString sPort;

		sPort.Format(L"%i", SocketPort);

		MessageBox(NULL, L"NETWORK ERRORS: SLAVER SITE CAN NOT LISTEN ON THE SOCKET AT " + SocketAddress + L":" + sPort, strCaption, MB_OK | MB_ICONERROR);

		return 0;
	}

	SetMasterInfo();

	SetStatusSlaver(L"WAITING FOR MASTER SITE ...");
	
	SOCKET AcceptSocket = accept(ListenSocket, NULL, NULL);
	
	if (AcceptSocket != INVALID_SOCKET) {
		SetStatusSlaver(L"RECEIVING MASTER INFORMATION ...");

		if (SocketRecvMasterInfo(AcceptSocket) == FALSE) {
			SetStatusSlaver(L"ERROR ON RECEIVING MASTER INFORMATION.");

			closesocket(AcceptSocket);
			closesocket(ListenSocket);

			return FALSE;
		}
		else {
			SetMasterInfo();

			SetStatusSlaver(L"SLAVER SITE MINING IN PROCESS ...");

			compute_minsup();
			
			encode_info_vector eis;
			
			clock_t start_encoding = clock();
			
			int result = slaver_mining(eis, SEQDATA_FILENAME);
			
			clock_t finish_encoding = clock();

			if (result == -1) {
				int status = 0;

				int iResult = SOCKET_ERROR;

				iResult = SocketSafeSendBuffer(AcceptSocket, (const char*)&status, sizeof(status));
				if (iResult == SOCKET_ERROR) {
					return FALSE;
				}
				assert(iResult == sizeof(status));

				SetStatusSlaver(L"ERROR ON SLAVER SEQUENCE DATA FILE.");

				closesocket(AcceptSocket);
				closesocket(ListenSocket);

				return FALSE;
			}
			else {
				int status = 1;

				SetStatusSlaver(L"SENDING ENCODED INFORMATION TO MASTER SITE ...");

				int num_iid = (int)eis.size();
				int iResult = SOCKET_ERROR;
	
				clock_t start_sending = clock();

				iResult = SocketSafeSendBuffer(AcceptSocket, (const char*)&status, sizeof(status));
				if (iResult == SOCKET_ERROR) {
					return FALSE;
				}
				assert(iResult == sizeof(status));

				iResult = SocketSafeSendBuffer(AcceptSocket, (const char*)&num_iid, sizeof(num_iid));
				if (iResult == SOCKET_ERROR) {
					return FALSE;
				}
				assert(iResult == sizeof(num_iid));

				for (encode_info_vector::iterator eis_iter = eis.begin(); eis_iter != eis.end(); eis_iter ++) {
					BOOL sResult = SocketSendEncodedInfo(
						AcceptSocket,
						eis_iter->iid,
						eis_iter->seq_bblk, eis_iter->seq_bblk_size,
						eis_iter->pos_bblk, eis_iter->pos_bblk_size,
						eis_iter->pof_buff, eis_iter->pof_buff_size);
					
					if (sResult == FALSE) {	
						for (encode_info_vector::iterator eis_iter1 = eis.begin(); eis_iter1 != eis.end(); eis_iter1 ++) {
							delete []eis_iter1->seq_bblk;
							delete []eis_iter1->pos_bblk;
							delete []eis_iter1->pof_buff;
						}
				
						int err = WSAGetLastError();
						CString strErr;

						strErr.Format(L"%i", err);

						SetStatusSlaver(L"ERROR ON SENDING ENCODED INFORMATION (" + strErr + L").");

						shutdown(AcceptSocket, SD_SEND);
						closesocket(AcceptSocket);

						closesocket(ListenSocket);

						return 0;
					}
				}

				clock_t finish_sending = clock();

				//print_slaver_encoded_buffers(eis);

				for (encode_info_vector::iterator eis_iter = eis.begin(); eis_iter != eis.end(); eis_iter ++) {
					delete []eis_iter->seq_bblk;
					delete []eis_iter->pos_bblk;
					delete []eis_iter->pof_buff;
				}

				SetStatusSlaver(L"SEND SLAVER ENCODED INFORMATION SUCCESS.");

				closesocket(AcceptSocket);
				closesocket(ListenSocket);

				CString msg;

				double encoding_sec = ((double)(finish_encoding - start_encoding) / CLOCKS_PER_SEC);
				double encoding_min = encoding_sec / 60;
				double sending_sec = ((double)(finish_sending - start_sending) / CLOCKS_PER_SEC);
				double sending_min = sending_sec / 60;

				msg.Format(L"ENCODED AND SENT TO MASTERSITE %i RECORDS.\n\nENCODING TIME:\t %f SEC (%f MIN)\nSENDING TIME:\t %f SEC (%f MIN)",
					result, encoding_sec, encoding_min, sending_sec, sending_min);

				MessageBox(NULL, msg, strCaption, MB_OK | MB_ICONINFORMATION);	

				return 1;
			}
		}
	}
	else {
		MessageBox(NULL, L"NETWORK ERRORS: SLAVER SITE CAN NOT ACCTEPT CONNECTION FORM MASTER SITE", strCaption, MB_OK | MB_ICONERROR);

		closesocket(ListenSocket);

		return 0;
	}
}

BOOL SocketRecvMasterInfo(SOCKET sd) {
	int sz = 256;
	char seqdataname[256];
	int iResult = SOCKET_ERROR;
	
	iResult = SocketSafeRecvBuffer(sd, (char*)&sz, sizeof(sz));
    if (iResult <= 0) {
        return FALSE;
    }
	assert(iResult == sizeof(sz));

	iResult = SocketSafeRecvBuffer(sd, (char*)seqdataname, sizeof(char) * sz);
    if (iResult == SOCKET_ERROR) {
        return FALSE;
    }
	assert(iResult == sizeof(char) * sz);
	seqdataname[sz] = NULL;

	sprintf(SEQDATA_FILENAME, "%s-%02i.SITE", seqdataname, SiteId);
	
	iResult = SocketSafeRecvBuffer(sd, (char*)&MAX_CID, sizeof(MAX_CID));
    if (iResult <= 0) {
        return FALSE;
    }
	assert(iResult == sizeof(MAX_CID));

	iResult = SocketSafeRecvBuffer(sd, (char*)&MINSUP_IN_PERCENT, (int)sizeof(MINSUP_IN_PERCENT));
    if (iResult <= 0) {
        return FALSE;
    }
	assert(iResult == sizeof(MINSUP_IN_PERCENT));

	return TRUE;
}

BOOL SocketSendEncodedInfo(SOCKET sd,
						 id_t iid,
						 bblk_t* seq_bblk, int seq_bblk_size,
						 bblk_t* pos_bblk, int pos_bblk_size,
						 int* pof_buff, int pof_buff_size) {
	int iResult = SOCKET_ERROR;
	
	iResult = SocketSafeSendBuffer(sd, (const char*)&iid, sizeof(iid));
    if (iResult == SOCKET_ERROR) {
        return FALSE;
    }
	assert(iResult == sizeof(iid));

	iResult = SocketSafeSendBuffer(sd, (const char*)&seq_bblk_size, sizeof(seq_bblk_size));
    if (iResult == SOCKET_ERROR) {
        return FALSE;
    }
	assert(iResult == sizeof(seq_bblk_size));

	iResult = SocketSafeSendBuffer(sd, (const char*)seq_bblk, sizeof(bblk_t) * seq_bblk_size);
    if (iResult == SOCKET_ERROR) {
        return FALSE;
    }
	assert(iResult == sizeof(bblk_t) * seq_bblk_size);

	iResult = SocketSafeSendBuffer(sd, (const char*)&pos_bblk_size, sizeof(pos_bblk_size));
    if (iResult == SOCKET_ERROR) {
        return FALSE;
    }
	assert(iResult == sizeof(pos_bblk_size));

	iResult = SocketSafeSendBuffer(sd, (const char*)pos_bblk, sizeof(bblk_t) * pos_bblk_size);
    if (iResult == SOCKET_ERROR) {
        return FALSE;
    }
	assert(iResult == sizeof(bblk_t) * pos_bblk_size);

	iResult = SocketSafeSendBuffer(sd, (const char*)&pof_buff_size, sizeof(pof_buff_size));
    if (iResult == SOCKET_ERROR) {
        return FALSE;
    }
	assert(iResult == sizeof(pof_buff_size));

	iResult = SocketSafeSendBuffer(sd, (const char*)pof_buff, sizeof(int) * pof_buff_size);
    if (iResult == SOCKET_ERROR) {
		return FALSE;
    }
	assert(iResult == sizeof(int) * pof_buff_size);

	return TRUE;
}