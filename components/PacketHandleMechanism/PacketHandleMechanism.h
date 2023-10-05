#ifndef PACKET_HANDLE_MECHANISM_H
#define PACKET_HANDLE_MECHANISM_H

#include <map>
#include <string>
#include <vector>

#include <esp_mesh.h>

typedef void (*PacketHandleFunc)(const std::vector<uint8_t> &data);

class PacketHandleMechanism
{
public:
    int addHandler(const std::string &name, const PacketHandleFunc handler);
    int removeHandler(const std::string &name);

    void run(const mesh_data_t &data);

private:
    static constexpr const char *moduleTag = "PHM";

    void stopIteration(void);

    std::map<std::string, PacketHandleFunc> m_handlers;
    bool m_stopRunning = false;
};

#endif // PACKET_HANDLE_MECHANISM_H