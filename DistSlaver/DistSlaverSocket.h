#if !defined(____DISTSLAVERSOCKET__H____)
#define ____DISTSLAVERSOCKET__H____

#include "DistSlaverMiner.h"

BOOL SocketInitSlaver();
VOID SocketCleanSlaver();
BOOL InitSocketInfo();

int SocketSafeRecvBuffer(SOCKET sd, char* buff, int size);
int SocketSafeSendBuffer(SOCKET sd, const char* buff, int size);

UINT SocketThreadFuncSlaverServer(LPVOID lParam);
BOOL SocketSendEncodedInfo(SOCKET sd, id_t iid, bblk_t* seq_bblk, int seq_bblk_size, bblk_t* pos_bblk, int pos_bblk_size, int* pof_buff, int pof_buff_size);

extern CString SocketAddress;
extern int SocketPort;
extern int SiteId;

#endif // ____DISTSLAVERSOCKET__H____