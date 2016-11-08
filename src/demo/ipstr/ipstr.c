#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "str.h"
#include "comm.h"

#define PORT (80)

int main(int argc, void *argv[])
{
    ip_port_t data;
    const char *ip = "127.0.0.1:80";

    if (str_to_ip_port(ip, &data)) {
        fprintf(stderr, "Convert to ip and port failed!");
        return -1;
    }

    fprintf(stderr, "ipaddr:%s port:%d\n", data.ipaddr, data.port);

    return 0;
}
