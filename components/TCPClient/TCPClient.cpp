#include "TCPClient.h"
#include "MeshDataExchangeModule.h"

#include <esp_log.h>

TCPClient &TCPClient::getInstance()
{
    static TCPClient instance;

    return instance;
}

TCPClient::TCPClient()
{
    xTaskCreatePinnedToCore(TCPClient::connectionObserverTask,
                            TCP_CONNECTION_OBSERVER_TASK_NAME,
                            TCP_CONNECTION_OBSERVER_TASK_STACK_SIZE,
                            NULL,
                            TCP_CONNECTION_OBSERVER_TASK_PRIORITY,
                            NULL,
                            TCP_CONNECTION_OBSERVER_TASK_CORE);
}

void TCPClient::connectionObserverTask(void * /*arg*/)
{
    int socket;
    while (1)
    {
        socket = TCPClient::getInstance().getSocket();
        // Check if any errors
        if (socket <= 0)
        {
            TCPClient::getInstance().tryConnect();

            socket = TCPClient::getInstance().getSocket();
            if (socket > 0)
            {
                TaskHandle_t handle = MeshDataExchangeModule::getInstance().getReceiveIPTaskHandle();
                if (handle)
                    vTaskResume(handle);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(CHECK_CONNECTION_DELAY));
    }
}

int TCPClient::getSocket() const
{
    return m_socket_fd;
}

void TCPClient::closeSocket()
{
    close(m_socket_fd);
    m_socket_fd = -1;

    ESP_LOGW(moduleTag, "The socket was closed.");
}

void TCPClient::tryConnect()
{
    if (m_socket_fd > 0)
        return;

    // Create socket
    m_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket_fd < 0)
    {
        ESP_LOGE(moduleTag, "Failed to create socket");
        return;
    }

    // TODO: change to the Kconfig or to the default values via macroses
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("192.168.99.112");
    server_address.sin_port = htons(5300);

    if (connect(m_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        ESP_LOGE(moduleTag, "TCP Connection failed.");
        closeSocket();
    }
    else
        ESP_LOGI(moduleTag, "The connection with a server has established.");
}