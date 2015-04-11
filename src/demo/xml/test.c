#include "slab.h"
#include "xml_tree.h"

#define MEM_SIZE (10 * 1024 * 1024)

int main(int argc, char *argv[])
{
    void *addr;
    slab_pool_t *slab;
    xml_tree_t *xml;
    xml_option_t option;

    memset(&option, 0, sizeof(option));

    syslog_init(7, "test.log");

    addr = calloc(1, MEM_SIZE);
    if (NULL == addr)
    {
        fprintf(stdout, "errmsg:[%d] %s!", errno, strerror(errno));
        return -1;
    }

    slab = slab_init(addr, MEM_SIZE);
    if (NULL == slab)
    {
        return -1;
    }

    option.pool = (void *)slab;
    option.alloc = (mem_alloc_cb_t)slab_alloc;
    option.dealloc = (mem_dealloc_cb_t)slab_dealloc;

    xml = xml_creat(argv[1], &option);
    if (NULL == xml)
    {
        fprintf(stdout, "Create XML failed!");
        return -1;
    }

    xml_fwrite(xml, "output.xml");

    xml_destroy(xml);
    fprintf(stderr,
            "....        .........................  "
            " ....      .... .......................  "
            "  ....    ....  ........................  "
            "   ....  ....   ....                ....  "
            "    ... ....    ....                ....  "
            "     ......     ....                ....  "                          
            "      ....      ....                ....  "
            "     ......     ....                ....  "
            "    ... ....    ....                ....  "
            "   ....  ....   ....                ....  "
            "  ....    ....  ........................  "
            " ....      .... .......................   "
            "....        ..........................    ");
    return 0;
}