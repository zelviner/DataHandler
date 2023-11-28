#include "hsmbase.h"
// #include "comm/CommUtils.h"
#include "public/utility/string.h"

CIHSMBase::CIHSMBase()
{    
    iError = NO_ERROR;
    m_socket = INVALID_SOCKET;
}

CIHSMBase::~CIHSMBase()
{
    if (m_socket != INVALID_SOCKET)
       disConnect();
    m_socket = INVALID_SOCKET;
}

bool CIHSMBase::connectHSM(const char *ip, unsigned int port, char *err)
{
    WSADATA wsaData;

    // Initialize Windows socket library
    WORD wVer = MAKEWORD(1, 1); //第一个参数为低位字节；第二个参数为高位字节
    if (WSAStartup(wVer, &wsaData) != 0)
    {
        sprintf(err, "Initialize error: %s.", "WSAStartup");
        return false;
    }

    if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) //LOBYTE（）取得16进制数最低位；HIBYTE（）取得16进制数最高（最左边）那个字节的内容
    {
        WSACleanup();
        sprintf(err, "WSADATA version %d.%d error.", wsaData.wVersion, wsaData.wVersion);
        return false;
    }

    // 创建客户端套节字
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == SOCKET_ERROR)
    {
        iError = WSAGetLastError();
        sprintf(err, "socket() error: %d!", iError);
        return false;
    }
/*
    sockaddr_in clientSockAddr;
    clientSockAddr.sin_family     = AF_INET;
    clientSockAddr.sin_port       = 0;
    clientSockAddr.sin_addr.s_addr= inet_addr((char *)"127.0.0.1");
    if(bind(m_socket,(LPSOCKADDR)&clientSockAddr,sizeof(clientSockAddr)) == SOCKET_ERROR)
    {
        iError = WSAGetLastError();
        closesocket(m_socket);
        WSACleanup();
        return false;
    }
//*/
    SOCKADDR_IN server;
    memset(&server, 0x00, sizeof(SOCKADDR_IN));
    server.sin_family      = PF_INET;
    server.sin_port        = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if (connect(m_socket, (LPSOCKADDR)&server, sizeof(server)) == SOCKET_ERROR)
    {
        iError = WSAGetLastError();
        WSACleanup();
        sprintf(err, "connect server error, server -- %s:%d!", ip, port);
        return false;
    }

    return true;
}

bool CIHSMBase::disConnect()
{
    // 释放连接和进行结束工作
    closesocket(m_socket);
    WSACleanup();

    return true;
}

bool CIHSMBase::sendData(const char *head, const char *data, char *buff, int &bufl, char *err)
{
//*
    // TG 1=enc 01=des_ecb  000 0=signle_key key_data(16)
    //    2=dec 02=des_cbc      1=two_key    key_data(32)
    unsigned char dest[1024*10] = { 0 };

    int hedLen = 0, datLen = 0, inpLen = 0;
    int offset = 2, revLen = 1;
    char dstBuf[1024*10], datTmp[5];
    memset(dstBuf, 0x00, sizeof(dstBuf));

    hedLen = strlen(head);
    memcpy(dstBuf + offset, head, hedLen);
    offset += hedLen;

    inpLen = strlen(data);
    datLen = zel::utility::String::hex2bin(data, dest, inpLen);

    memset(datTmp, 0x00, sizeof(datTmp));
    sprintf(datTmp, "%04d", datLen);
    memcpy(dstBuf + offset, datTmp, 4);
    offset += 4;

    memcpy(dstBuf + offset, dest, datLen);
    offset += datLen;

    dstBuf[0] = (offset - 2) / 256;
    dstBuf[1] = (offset - 2) % 256;

    if (send(m_socket, dstBuf, offset, 0) == SOCKET_ERROR)
    {
        iError = WSAGetLastError();
        closesocket(m_socket);
        WSACleanup();
        sprintf(err, "send data failed: %s, %s!", head, data);
        return false;
    }

    memset(dstBuf, 0x00, sizeof(dstBuf));
    if ((revLen = recv(m_socket, dstBuf, 10240, 0)) == SOCKET_ERROR)
    {
        iError = WSAGetLastError();
        closesocket(m_socket);
        WSACleanup();
        sprintf(err, "receive response failed: %d!", iError);
        return false;
    }     

    char TAG[3];
    memset(TAG, 0x00, sizeof(TAG));
    if (memicmp(head, "TG", 2) == 0)
    {
        memcpy(TAG, "TH", 2);
    }
    else if (memicmp(head, "TS", 2) == 0)
    {
        memcpy(TAG, "TT", 2);
    }
    else if (memicmp(head, "TU", 2) == 0)
    {
        memcpy(TAG, "TV", 2);
    }
    else if (memicmp(head, "UE", 2) == 0)
    {
        memcpy(TAG, "UF", 2);
    }
    else if (memicmp(head, "WA", 2) == 0)
    {
        memcpy(TAG, "WB", 2);
    }

    if (memicmp(dstBuf + 2, TAG, 2) != 0)
    {
        iError = WSAGetLastError();
        closesocket(m_socket);
        WSACleanup();
        sprintf(err, "verify response failed, response head error: %s!", dstBuf);
        return false;
    }

    if (memicmp(dstBuf + 4, "00", 2) != 0)
    {
        iError = WSAGetLastError();
        closesocket(m_socket);
        WSACleanup();
        if (memicmp(dstBuf + 4, "10", 2) == 0)
        {
            sprintf(err, "verify response failed: %s!", "ZMK Parity error");
            return false;
        }
        else if (memicmp(dstBuf + 4, "12", 2) == 0)
        {
            sprintf(err, "verify response failed: %s!", "No keys loaded in user storage");
            return false;
        }
        else if (memicmp(dstBuf + 4, "13", 2) == 0)
        {
            sprintf(err, "verify response failed: %s!", "LMK error:report to supervisor");
            return false;
        }
        else if (memicmp(dstBuf + 4, "15", 2) == 0)
        {
            sprintf(err, "verify response failed: %s!", "Error in input data");
            return false;
        }
        else if (memicmp(dstBuf + 4, "21", 2) == 0)
        {
            sprintf(err, "verify response failed: %s!", "Invalid user storage index");
            return false;
        }
        else
        {
            sprintf(err, "verify response failed, unknown error code: %s!", dstBuf);
            return false;
        }
    }

    memset(datTmp, 0x00, sizeof(datTmp));
    if ((memicmp(head, "TG", 2) == 0) || (memicmp(head, "WA", 2) == 0) || (memicmp(head, "UE", 2) == 0))
    {
        memcpy(datTmp, dstBuf + 6, 4);
    }
    else if (memicmp(head, "TS", 2) == 0)
    {
        memcpy(datTmp, dstBuf + 6, 2);
    }
    else if (memicmp(head, "TU", 2) == 0)
    {
        sprintf(datTmp, "%02d", revLen-5); // TV00 data...
    }
    //qDebug() << revLen << QString::fromLocal8Bit(dstBuf+2, 2) << QString::fromLocal8Bit(dstBuf, revLen) << QString::fromLocal8Bit(datTmp, 4);
    if (atoi(datTmp) != datLen)
    {
        iError = WSAGetLastError();
        closesocket(m_socket);
        WSACleanup();
        sprintf(err, "output data length error: %s!", dstBuf);
        return false;
    }

    char ascOut[2048];
    unsigned char bcdSrc[2048];

    memset(bcdSrc, 0x00, sizeof(bcdSrc));
    memset(ascOut, 0x00, sizeof(ascOut));

    memcpy(bcdSrc, dstBuf + 10, datLen);
    bufl = zel::utility::String::bin2hex(bcdSrc, ascOut, datLen);
    memcpy(buff, ascOut, bufl);
    buff[bufl] = '\0';

//*/
    return true;
}

int CIHSMBase::getIpAddrSize(char *IpAddr[])
{
    //PIP_ADAPTER_INFO结构体指针存储本机网卡信息
    PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
    //得到结构体大小,用于GetAdaptersInfo参数
    unsigned long stSize = sizeof(IP_ADAPTER_INFO);
    //调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量;其中stSize参数既是一个输入量也是一个输出量
    int nRel = GetAdaptersInfo(pIpAdapterInfo,&stSize);
    //记录网卡数量
    int netCardNum = 0;
    //记录每张网卡上的IP地址数量
    int IPnumPerNetCard = 0;
    if (ERROR_BUFFER_OVERFLOW == nRel)
    {
        //如果函数返回的是ERROR_BUFFER_OVERFLOW
        //则说明GetAdaptersInfo参数传递的内存空间不够,同时其传出stSize,表示需要的空间大小
        //这也是说明为什么stSize既是一个输入量也是一个输出量
        //释放原来的内存空间
        delete pIpAdapterInfo;
        //重新申请内存空间用来存储所有网卡信息
        pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
        //再次调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量
        nRel=GetAdaptersInfo(pIpAdapterInfo,&stSize);
    }
    if (ERROR_SUCCESS == nRel)
    {
        //输出网卡信息
        //可能有多网卡,因此通过循环去判断
        while (pIpAdapterInfo)
        {
            //可能网卡有多IP,因此通过循环去判断
            IP_ADDR_STRING *pIpAddrString =&(pIpAdapterInfo->IpAddressList);
            do
            {
                    IpAddr[netCardNum]=pIpAddrString->IpAddress.String;
                    netCardNum++;
                    pIpAddrString=pIpAddrString->Next;
            } while (pIpAddrString);
            pIpAdapterInfo = pIpAdapterInfo->Next;
        }

    }
    //释放内存空间
    if (pIpAdapterInfo)
    {
        delete pIpAdapterInfo;
    }

    return netCardNum;
}

void CIHSMBase::setErrorMessage(const char *fmt, ...)
{
    char err[1024*10];
    memset(err, 0x00, sizeof(err));

    va_list args;
    va_start(args, err);
    vsprintf(err, fmt, args);
    va_end(args);

    m_errMsg = std::string(err);
}
