#if !defined(____DISTMASTERSOCKET__H____)
#define ____DISTMASTERSOCKET__H____
#include "DistMaster.h"
#include "DistMasterMiner.h"

extern SLAVER_ADDRESS* SLAVER_ADDRESSES;
extern int NUM_SLAVER_ADDRESS;

BOOL SocketInitMaster();
VOID SocketCleanMaster();

int SocketSafeRecvBuffer(SOCKET sd, char* buff, int size);
int SocketSafeSendBuffer(SOCKET sd, const char* buff, int size);

UINT SocketThreadFuncMasterClient(LPVOID lParam);

BOOL SocketSendMasterInfo(SOCKET sd);
BOOL SocketRecvEncodedInfo(SOCKET sd,
							id_t& iid,
							bblk_t*& seq_bblk, int& seq_bblk_size,
							bblk_t*& pos_bblk, int& pos_bblk_size,
							int*& pof_buff, int& pof_buff_size);
#endif // ____DISTMASTERSOCKET__H____