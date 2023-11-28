#ifndef CIHSMBASE_H
#define CIHSMBASE_H

#include <WinSock2.h>
#include <windows.h>
#include <MSWSock.h>
#include <string>
#include <iphlpapi.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "AdvAPI32.lib")


#define  HSM_DEFAULT_IP      "127.0.0.1"
#define  HSM_DEFAULT_PORT    9001

class CIHSMBase
{
public:
  CIHSMBase();
  ~CIHSMBase();

  /**
   * @brief connect
   * @param ip
   * @param port
   * @param err
   * @return
   */
  bool connectHSM(const char* ip, unsigned int port, char* err);
  bool disConnect();

  /**
   * @brief sendData
   * @param head
   * @param data
   * @param buff
   * @param bufl
   * @param err
   * @return
   */
  bool sendData(const char* head, const char* data, char* buff, int& bufl, char* err);

  inline std::string getErrorMessage() { return m_errMsg; };

private:
  /**
   * @brief getIpAddrSize
   * @param IpAddr
   * @return
   */
  int  getIpAddrSize(char* IpAddr[]);

  /**
   * @brief setErrorMessage
   * @param fmt
   */
  void setErrorMessage(const char* fmt, ...);

public:
  int           iError;

private:
  SOCKET        m_socket;
  std::string   m_errMsg;
};

#endif // CIHSMBASE_H
