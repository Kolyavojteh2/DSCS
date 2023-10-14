#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"

// TODO: move into the Kconfig
#define CHECK_CONNECTION_DELAY (2000)
#define TCP_CONNECTION_OBSERVER_TASK_CORE (0)
// TODO: check priority
#define TCP_CONNECTION_OBSERVER_TASK_PRIORITY (0)
#define TCP_CONNECTION_OBSERVER_TASK_STACK_SIZE (2048)
#define TCP_CONNECTION_OBSERVER_TASK_NAME "connectionObserver"

class TCPClient
{
public:
    static TCPClient &getInstance(void);

    void tryConnect();

    int getSocket() const;
    void closeSocket();

private:
    static constexpr const char *moduleTag = "TCPClient";

    TCPClient();
    TCPClient(const TCPClient &) = delete;
    TCPClient &operator=(const TCPClient &) = delete;

    static void connectionObserverTask(void *arg);

    int m_socket_fd = -1;
};

#endif // TCP_CLIENT_H