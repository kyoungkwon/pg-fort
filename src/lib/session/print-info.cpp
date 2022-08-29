#include <cstring>
#include <iostream>

enum SentBy
{
    BACKEND,
    FRONTEND
};

void PrintInfo(SentBy src, char* data, int size)
{
    auto type = data[0];
    std::cout << "type = " << type << std::endl;

    if (type == 'Q')
    {
        std::cout << "\t\"Query\"" << std::endl;

        int32_t len = (uint32_t(data[1]) << 24) + (uint32_t(data[2]) << 16) +
                      (uint32_t(data[3]) << 8) + uint32_t(data[4]);

        std::cout << "\ttotal len = " << len << std::endl;

        len -= 4;

        std::cout << "\tquery len = " << len << std::endl;

        auto query = data + 5;

        std::cout << "\n\tquery =\n\t" << query << std::endl;
    }
    else if (type == 'E' && src == BACKEND)
    {
        std::cout << "\t\033[1;35m\"ErrorResponse\"\033[0m" << std::endl;

        uint32_t len = (uint32_t(data[1]) << 24) + (uint32_t(data[2]) << 16) +
                       (uint32_t(data[3]) << 8) + uint32_t(data[4]);

        std::cout << "\ttotal len = " << len << std::endl;
        std::cout << "\tbody len = " << len - 4 << std::endl;

        uint32_t pos = 5;
        while (pos < len)
        {
            std::cout << "\tfield type = " << data[pos] << std::endl;

            pos += 1;
            char* v = data + pos;

            std::cout << "\tfield value = " << v;
            std::cout << "\t[pos: " << pos << "]" << std::endl;

            pos += strlen(v) + 1;
        }
        std::cout << "\tdone [pos: " << pos << "]" << std::endl;

        for (int i = pos; i < size; i++)
        {
            std::cout << "\t\tdata[" << i << "] = " << data[i];
            printf(" [%x]", data[i]);
            std::cout << (data[i] == '\0' ? " (zero)" : "") << std::endl;
        }
    }
    else if (type == 'T' && src == BACKEND)
    {
        std::cout << "\t\033[1;33m\"RowDescription\"\033[0m" << std::endl;

        uint32_t len = (uint32_t(data[1]) << 24) + (uint32_t(data[2]) << 16) +
                       (uint32_t(data[3]) << 8) + uint32_t(data[4]);

        std::cout << "\ttotal len = " << len << std::endl;

        uint32_t nfield = (uint32_t(data[5]) << 8) + uint32_t(data[6]);

        std::cout << "\tnum fields = " << nfield << std::endl;

        uint32_t pos = 7;
        for (int32_t i = 0; i < nfield; i++)
        {
            char* name = data + pos;

            std::cout << "\t\tfields[" << i << "].name = " << name;
            std::cout << "\t[pos: " << pos << "]" << std::endl;

            pos += strlen(name) + 1 + 4 + 2 + 4 + 2 + 4 + 2;
        }
        std::cout << "\t\tdone [pos: " << pos << "]" << std::endl;
    }

    // check last 6 bytes
    if (size > 6)
    {
        uint32_t i = size - 6;
        std::cout << "\tlast 6 bytes:" << std::endl;
        std::cout << "\t\tdata[" << i << "] = " << data[i++] << std::endl;
        std::cout << "\t\tdata[" << i << "] = " << data[i++] << std::endl;
        std::cout << "\t\tdata[" << i << "] = " << data[i++] << std::endl;
        std::cout << "\t\tdata[" << i << "] = " << data[i++] << std::endl;
        std::cout << "\t\tdata[" << i << "] = " << data[i++] << std::endl;
        std::cout << "\t\tdata[" << i << "] = " << data[i++] << std::endl;
    }

    std::cout << std::endl;
}
