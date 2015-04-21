#include "btree.h"

static btree_node_t *btree_creat_node(btree_t *btree);
static int _btree_insert(btree_t *btree, btree_node_t *node, int key, int idx);
static int btree_split(btree_t *btree, btree_node_t *node);
static int _btree_remove(btree_t *btree, btree_node_t *node, int idx);
static int btree_merge(btree_t *btree, btree_node_t *node);
static void _btree_print(const btree_node_t *node, int deep);
static int _btree_merge(btree_t *btree, btree_node_t *left, btree_node_t *right, int idx);


#define btree_print(btree) \
{ \
    if (NULL != btree->root) \
    { \
        _btree_print(btree->root, 0); \
    } \
}

/******************************************************************************
 **��������: btree_creat 
 **��    ��: ����B��
 **�������: 
 **     _btree: B��
 **     m: ��m>=3
 **�������: NONE
 **��    ��: 0:�ɹ� !0:ʧ��
 **ʵ������: 
 **ע������: 
 **��    ��: # Qifeng.zou # 2014.03.12 #
 ******************************************************************************/
int btree_creat(btree_t **_btree, int m)
{
    btree_t *btree = NULL;

    if (m < 3)
    {
        fprintf(stderr, "[%s][%d] Parameter 'm' must geater than 2!\n", __FILE__, __LINE__);
        return -1;
    }

    btree = (btree_t *)calloc(1, sizeof(btree_t));
    if (NULL == btree)
    {
        fprintf(stderr, "[%s][%d] errmsg:[%d] %s!\n", __FILE__, __LINE__, errno, strerror(errno));
        return -1;
    }

    btree->max= m - 1;
    btree->min = m / 2;
    if (0 != m%2)
    {
        btree->min++;
    }
    btree->min--;
    btree->sidx = m/2;
    btree->root = NULL;
    fprintf(stderr, "max:%d min:%d sidx:%d\n", btree->max, btree->min, btree->sidx);

    *_btree = btree;
    return 0;
}

/******************************************************************************
 **��������: btree_insert
 **��    ��: ��B���в���һ���ؼ���
 **�������: 
 **     btree: B��
 **     key: ��������Ĺؼ���
 **�������: NONE
 **��    ��: 0:�ɹ� !0:ʧ��
 **ʵ������: 
 **ע������: 
 **��    ��: # Qifeng.zou # 2014.03.12 #
 ******************************************************************************/
int btree_insert(btree_t *btree, int key)
{
    int idx;
    btree_node_t *node = btree->root;

    /* 1. �������� */
    if (NULL == node)
    {
        node = btree_creat_node(btree);
        if (NULL == node)
        {
            fprintf(stderr, "[%s][%d] Create node failed!\n", __FILE__, __LINE__);
            return -1;
        }

        node->num = 1; 
        node->key[0] = key;
        node->parent = NULL;

        btree->root = node;
        return 0;
    }

    /* 2. ���ҹؼ��ֵĲ���λ�� */
    while(NULL != node)
    {
        for(idx=0; idx<node->num; idx++)
        {
            if (key == node->key[idx])
            {
                fprintf(stderr, "[%s][%d] The node is exist!\n", __FILE__, __LINE__);
                return 0;
            }
            else if (key < node->key[idx])
            {
                break;
            }
        }

        if (NULL != node->child[idx])
        {
            node = node->child[idx];
        }
        else
        {
            break;
        }
    }

    /* 3. ִ�в������ */
    return _btree_insert(btree, node, key, idx);
}

/******************************************************************************
 **��������: _btree_insert
 **��    ��: ����ؼ��ֵ�ָ���ڵ�
 **�������: 
 **     btree: B��
 **     node: ָ���ڵ�
 **     key: �豻����Ĺؼ���
 **     idx: ����λ��
 **�������: NONE
 **��    ��: 0:�ɹ� !0:ʧ��
 **ʵ������: 
 **ע������: 
 **��    ��: # Qifeng.zou # 2014.03.12 #
 ******************************************************************************/
static int _btree_insert(btree_t *btree, btree_node_t *node, int key, int idx)
{
    int i;

    /* 1. ������ײ�Ľڵ�: ���ӽڵ㶼�ǿ�ָ�� */
    for(i=node->num; i>idx; i--)
    {
        node->key[i] = node->key[i-1];
        /* node->child[i+1] = node->child[i]; */
    }

    node->key[idx] = key;
    node->num++;

    /* 2. �ֻ��ڵ� */
    if (node->num > btree->max)
    {
        return btree_split(btree, node);
    }

    return 0;
}

/******************************************************************************
 **��������: btree_split
 **��    ��: ����ؼ��ֵ�ָ���ڵ�, �����з��Ѵ���
 **�������: 
 **     btree: B��
 **     node: ָ���ڵ�
 **     key: �豻����Ĺؼ���
 **     idx: ����λ��
 **�������: NONE
 **��    ��: 0:�ɹ� !0:ʧ��
 **ʵ������: 
 **ע������: 
 **��    ��: # Qifeng.zou # 2014.03.12 #
 ******************************************************************************/
static int btree_split(btree_t *btree, btree_node_t *node)
{
    int idx, total, sidx = btree->sidx;
    btree_node_t *parent, *node2;

    while(node->num > btree->max)
    {
        /* Split node */
        total = node->num;

        node2 = btree_creat_node(btree);
        if (NULL == node2)
        {
            fprintf(stderr, "[%s][%d] Create node failed!\n", __FILE__, __LINE__);
            return -1;
        }

        /* Copy data */
        memcpy(node2->key, node->key+sidx+1, (total-sidx-1) * sizeof(int));
        memcpy(node2->child, node->child+sidx+1, (total-sidx) * sizeof(btree_node_t *));

        node2->num = (total - sidx - 1);
        node2->parent  = node->parent;

        node->num = sidx;

        /* Insert into parent */
	    parent  = node->parent;
        if (NULL == parent)
        {
            /* Split root node */
            parent = btree_creat_node(btree);
            if (NULL == parent)
            {
                fprintf(stderr, "[%s][%d] Create root failed!", __FILE__, __LINE__);
                return -1;
            }

            btree->root = parent;
            parent->child[0] = node;
            node->parent = parent;
            node2->parent = parent;

            parent->key[0] = node->key[sidx];
            parent->child[1] = node2;
            parent->num++;
        }
        else
        {
            /* Insert into parent node */
            for(idx=parent->num; idx>0; idx--)
            {
                if (node->key[sidx] < parent->key[idx-1])
                {
                    parent->key[idx] = parent->key[idx-1];
                    parent->child[idx+1] = parent->child[idx];
                }
                else
                {
                    parent->key[idx] = node->key[sidx];
                    parent->child[idx+1] = node2;
                    node2->parent = parent;
                    parent->num++;
                    break;
                }
            }

            if (0 == idx)
            {
                parent->key[0] = node->key[sidx];
                parent->child[1] = node2;
                node2->parent = parent;
                parent->num++;               
            }
        }

        memset(node->key+sidx, 0, (total - sidx) * sizeof(int));
        memset(node->child+sidx+1, 0, (total - sidx) * sizeof(btree_node_t *));

        /* Change node2's child->parent */
        for(idx=0; idx<=node2->num; idx++)
        {
            if (NULL != node2->child[idx])
            {
                node2->child[idx]->parent = node2;
            }
        }
        node = parent;
    }

    return 0;
}

/******************************************************************************
 **��������: btree_remove
 **��    ��: ɾ��ָ���ؼ���
 **�������: 
 **     btree: B��
 **     key: �ؼ���
 **�������: NONE
 **��    ��: 0:�ɹ� !0:ʧ��
 **ʵ������: 
 **ע������: 
 **��    ��: # Qifeng.zou # 2014.03.12 #
 ******************************************************************************/
int btree_remove(btree_t *btree, int key)
{
    int idx;
    btree_node_t *node = btree->root;

    while(NULL != node)
    {
        for(idx=0; idx<node->num; idx++)
        {
            if (key == node->key[idx])
            {
                return _btree_remove(btree, node, idx);
            }
            else if (key < node->key[idx])
            {
                break;
            }
        }

        node = node->child[idx];
    }

    return 0;
}

/******************************************************************************
 **��������: _btree_remove
 **��    ��: ��ָ�����ɾ��ָ���ؼ���
 **�������: 
 **     btree: B��
 **     node: ָ�����
 **     idx: ����ɾ���Ĺؼ����ڽ��node��λ��(0 ~ node->num - 1)
 **�������: NONE
 **��    ��: 0:�ɹ� !0:ʧ��
 **ʵ������: 
 **     ʹ��node->child[idx]�е����ֵ�����ɾ���Ĺؼ���, 
 **     ���������´���ֱ����ײ���, 
 **     -- ��ʵ�����䴦������൱����ɾ����ײ���Ĺؼ���
 **ע������: 
 **��    ��: # Qifeng.zou # 2014.03.12 #
 ******************************************************************************/
static int _btree_remove(btree_t *btree, btree_node_t *node, int idx)
{
    btree_node_t *orig = node, *child = node->child[idx];

    /* ʹ��node->child[idx]�е����ֵ�����ɾ���Ĺؼ��� */
    while(NULL != child)
    {
        node = child;
        child = node->child[child->num];
    }

	orig->key[idx] = node->key[node->num - 1];

    /* �����䴦������൱����ɾ����ײ���Ĺؼ��� */
    node->key[--node->num] = 0;
    if (node->num < btree->min)
    {
        return btree_merge(btree, node);
    }

    return 0;
}

/******************************************************************************
 **��������: btree_merge
 **��    ��: �ϲ����
 **�������: 
 **     btree: B��
 **     node: �ý��ؼ�����num<min
 **�������: NONE
 **��    ��: 0:�ɹ� !0:ʧ��
 **ʵ������: 
 **    �����������:
 **     1) �ϲ��������: node->num + brother->num + 1 <= max
 **     2) ���ý������: node->num + brother->num + 1 >  max
 **ע������: 
 **     node��ʱΪ��ײ���
 **��    ��: # Qifeng.zou # 2014.03.12 #
 ******************************************************************************/
static int btree_merge(btree_t *btree, btree_node_t *node)
{
    int idx = 0, m = 0, mid = 0;
    btree_node_t *parent = node->parent, *right = NULL, *left = NULL;

    /* 1. node�Ǹ����, ���ؽ��кϲ����� */
    if (NULL == parent)
    {
        if (0 == node->num)
        {
            if (NULL != node->child[0])
            {
                btree->root = node->child[0];
                node->child[0]->parent = NULL;
            }
            else
            {
                btree->root = NULL;
            }
            free(node);
        }
        return 0;
    }

    /* 2. ����node���丸���ĵڼ������ӽ�� */
    for(idx=0; idx<=parent->num; idx++)
    {
        if (parent->child[idx] == node)
        {
            break;
        }
    }

    if (idx > parent->num)
    {
        fprintf(stderr, "[%s][%d] Didn't find node in parent's children array!\n", __FILE__, __LINE__);
        return -1;
    }
    /* 3. node: ���һ�����ӽ��(left < node)
     * node as right child */
    else if (idx == parent->num)
    {
        mid = idx - 1;
        left = parent->child[mid];

        /* 1) �ϲ���� */
        if ((node->num + left->num + 1) <= btree->max)
        {
            return _btree_merge(btree, left, node, mid);
        }

        /* 2) ���ý��:brother->key[num-1] */
        for(m=node->num; m>0; m--)
        {
            node->key[m] = node->key[m - 1];
            node->child[m+1] = node->child[m];
        }
        node->child[1] = node->child[0];

        node->key[0] = parent->key[mid];
        node->num++;
        node->child[0] = left->child[left->num];
        if (NULL != left->child[left->num])
        {
            left->child[left->num]->parent = node;
        }

        parent->key[mid] = left->key[left->num - 1];
        left->key[left->num - 1] = 0;
        left->child[left->num] = NULL;
        left->num--;
        return 0;
    }
    
    /* 4. node: �����һ�����ӽ��(node < right)
     * node as left child */
    mid = idx;
    right = parent->child[mid + 1];

    /* 1) �ϲ���� */
    if ((node->num + right->num + 1) <= btree->max)
    {
        return _btree_merge(btree, node, right, mid);
    }

    /* 2) ���ý��: right->key[0] */
    node->key[node->num++] = parent->key[mid];
    node->child[node->num] = right->child[0];
    if (NULL != right->child[0])
    {
        right->child[0]->parent = node;
    }

    parent->key[mid] = right->key[0];
    for(m=0; m<right->num; m++)
    {
        right->key[m] = right->key[m+1];
        right->child[m] = right->child[m+1];
    }
    right->child[m] = NULL;
    right->num--;
    return 0;
}

/******************************************************************************
 **��������: _btree_merge
 **��    ��: �ϲ����
 **�������: 
 **     btree: B��
 **     node: 
 **     brother:
 **�������: NONE
 **��    ��: 0:�ɹ� !0:ʧ��
 **ʵ������: 
 **ע������: 
 **��    ��: # Qifeng.zou # 2014.03.12 #
 ******************************************************************************/
static int _btree_merge(btree_t *btree, btree_node_t *left, btree_node_t *right, int mid)
{
    int m = 0;
    btree_node_t *parent = left->parent;

    left->key[left->num++] = parent->key[mid];

    memcpy(left->key + left->num, right->key, right->num*sizeof(int));
    memcpy(left->child + left->num, right->child, (right->num+1)*sizeof(int));
    for(m=0; m<=right->num; m++)
    {
        if (NULL != right->child[m])
        {
            right->child[m]->parent = left;
        }
    }
    left->num += right->num;

    for(m=mid; m<parent->num-1; m++)
    {
        parent->key[m] = parent->key[m+1];
        parent->child[m+1] = parent->child[m+2];
    }

    parent->key[m] = 0;
    parent->child[m+1] = NULL;
    parent->num--;
    free(right);

    /* Check */
    if (parent->num < btree->min)
    {
        return btree_merge(btree, parent);
    }

    return 0;
}

/******************************************************************************
 **��������: btree_destroy
 **��    ��: ����B��
 **�������: 
 **     btree: B��
 **�������: NONE
 **��    ��: 0:�ɹ� !0:ʧ��
 **ʵ������: 
 **ע������: 
 **��    ��: # Qifeng.zou # 2014.03.12 #
 ******************************************************************************/
int btree_destroy(btree_t **btree)
{
    if (NULL != (*btree)->root)
    {
        free((*btree)->root);
    }

    free(*btree);
    *btree = NULL;
    return 0;
}

/******************************************************************************
 **��������: _btree_print
 **��    ��: ��ӡB���ṹ
 **�������: 
 **     node: B�����
 **     deep: ������
 **�������: NONE
 **��    ��: �ڵ��ַ
 **ʵ������: 
 **ע������: 
 **��    ��: # Qifeng.zou # 2014.03.12 #
 ******************************************************************************/
static void _btree_print(const btree_node_t *node, int deep)
{
    int idx, d, flag = 0;

    /* 1. Print Start */
    for(d=0; d<deep; d++)
    {
        if (d == deep-1)
        {
            fprintf(stderr, "|-------");
        }
        else
        {
            fprintf(stderr, "|        ");
        }
    }

    fprintf(stderr, "<%d| ", node->num);
    for(idx=0; idx<node->num; idx++)
    {
        fprintf(stderr, "%d ", node->key[idx]);
    }

    fprintf(stderr, ">\n");

    /* 2. Print node's children */
    for(idx=0; idx<node->num+1; idx++)
    {
        if (NULL != node->child[idx])
        { 
            _btree_print(node->child[idx], deep+1);
            flag = 1;
        }
    }

    if (1 == flag)
    {
        for(d=0; d<deep; d++)
        {
            fprintf(stderr, "|        ");
        }

        fprintf(stderr, "</%d| ", node->num);
        for(idx=0; idx<node->num; idx++)
        {
            fprintf(stderr, "%d ", node->key[idx]);
        }

        fprintf(stderr, ">\n");
    }
}

/******************************************************************************
 **��������: btree_creat_node
 **��    ��: ����һ���ڵ�
 **�������: 
 **     btree: B��
 **�������: NONE
 **��    ��: �ڵ��ַ
 **ʵ������: 
 **ע������: 
 **��    ��: # Qifeng.zou # 2014.03.12 #
 ******************************************************************************/
static btree_node_t *btree_creat_node(btree_t *btree)
{
    btree_node_t *node;

    node = (btree_node_t *)calloc(1, sizeof(btree_node_t));
    if (NULL == node)
    {
        fprintf(stderr, "[%s][%d] errmsg:[%d] %s\n", __FILE__, __LINE__, errno, strerror(errno));
        return NULL;
    }

    node->num = 0;

    /* More than (max) is for move */
    node->key = (int *)calloc(btree->max+1, sizeof(int));
    if (NULL == node->key)
    {
        free(node), node=NULL;
        fprintf(stderr, "[%s][%d] errmsg:[%d] %s\n", __FILE__, __LINE__, errno, strerror(errno));
        return NULL;
    }

    /* More than (max+1) is for move */
    node->child = (btree_node_t **)calloc(btree->max+2, sizeof(btree_node_t *));
    if (NULL == node->child)
    {
        free(node->key);
        free(node), node=NULL;
        fprintf(stderr, "[%s][%d] errmsg:[%d] %s\n", __FILE__, __LINE__, errno, strerror(errno));
        return NULL;
    }

    return node;
}
