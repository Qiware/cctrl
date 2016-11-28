#if !defined(__TEST_MESG_H__)
#define __TEST_MESG_H__

#define URL_MAX_LEN (1024)

typedef enum
{
    MSG_SEARCH_REQ = 1
} mesg_type_e;

////////////////////////////////////////////////////////////////////////////////
/* 搜索消息结构 */
#define SRCH_WORD_LEN       (128)
typedef struct
{
    char words[SRCH_WORD_LEN];          /* 搜索关键字 */
} mesg_search_req_t;

////////////////////////////////////////////////////////////////////////////////
/* 插入关键字请求 */
typedef struct
{
    char word[SRCH_WORD_LEN];           /* 关键字 */
    char url[URL_MAX_LEN];              /* 关键字对应的URL */
    int freq;                           /* 频率 */
} mesg_insert_word_req_t;

/* 插入关键字应答 */
typedef struct
{
#define MESG_INSERT_WORD_FAIL   (0)
#define MESG_INSERT_WORD_SUCC   (1)
    int code;                           /* 应答码 */
    char word[SRCH_WORD_LEN];           /* 关键字 */
} mesg_insert_word_rsp_t;

#define mesg_insert_word_resp_hton(rsp) do { \
    (rsp)->code = htonl((rsp)->code); \
} while(0)

#define mesg_insert_word_resp_ntoh(rsp) do { \
    (rsp)->code = ntohl((rsp)->code); \
} while(0)

////////////////////////////////////////////////////////////////////////////////
/* 订阅请求 */
typedef struct
{
    uint32_t type;                      /* 订阅类型(消息类型) */
    int nid;                            /* 结点ID */
} mesg_sub_req_t;

#define mesg_sub_req_hton(req) do { /* 主机 > 网络 */\
    (req)->type = htonl((req)->type); \
    (req)->nid = htonl((req)->nid); \
} while(0)

#define mesg_sub_req_ntoh(req) do { /* 网络 > 主机 */\
    (req)->type = ntohl((req)->type); \
    (req)->nid = ntohl((req)->nid); \
} while(0)

/* 订阅应答 */
typedef struct
{
    int code;                           /* 应答码 */
    uint32_t type;                      /* 订阅类型(消息类型) */
} mesg_sub_rsp_t;

#define mesg_sub_rsp_hton(rsp) do { /* 主机 > 网络 */\
    (rsp)->code = htonl((rsp)->code); \
    (rsp)->type = htonl((rsp)->type); \
} while(0)

#define mesg_sub_rsp_ntoh(rsp) do { /* 网络 > 主机 */\
    (rsp)->code = ntohl((rsp)->code); \
    (rsp)->type = ntohl((rsp)->type); \
} while(0)


#endif /*__TEST_MESG_H__*/
