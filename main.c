#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

char tmpcode[33]; //32-bit encoding should be enough;

typedef struct abc_item
{
    char chr;
    int cnt;
    double prb;
    struct abc_item *nxt;
    struct node *parent;
    char *code;
}abc_item;

typedef struct abc_list
{
    int length;
    double entropy;
    struct abc_item *nxt;
}abc_list;

typedef struct node
{
    double prb;
    abc_item* abc;
    struct node *l;
    struct node *r;
    struct node *parent;
}node;

int add_abc_item(abc_item **abc, char item)
{
    if(*abc==NULL) // reached the end of the list...
    {
        *abc = (abc_item*)malloc(sizeof(abc_item));
        (*abc)->chr = item;
        (*abc)->cnt = 1;
        (*abc)->nxt = NULL;
        //printf("%c\n", (*abc)->chr);
        return 1;
    }

    if((*abc)->chr==item) // found the item...
    {
        (*abc)->cnt++;
        //printf("%c\n", (*abc)->chr);
        return 0;
    }

    // didn't find the item yet...
    //printf("%c ", (*abc)->chr);
    return add_abc_item(&((*abc)->nxt), item); // recursive awesomeness...
}

void render_abc(abc_list *abc, int total)
{
    int cnt;
    abc->entropy = 0;
    abc_item* i;

    for(i=abc->nxt,cnt=0;i!=NULL;i=i->nxt,cnt++)
    {
        ;
    }

    abc->length = cnt;

    for(i=abc->nxt;i!=NULL;i=i->nxt)
    {
        i->prb = (i->cnt /(float)total);
        //printf("%c: %d / %d: %g\n", i->chr, i->cnt, total, i->prb);
        abc->entropy += (i->prb)*(log2(i->prb));
    }
    abc->entropy *= -1;
}

node **build_tree_leaves(abc_list abc)
{
    node **node_group = (node*)malloc(sizeof(node*) * abc.length);
    node *tmpleaf = NULL;
    abc_item *i;
    int j;
    //fill node_group with pointers of leaf nodes
    for(j=0,i=abc.nxt;j<abc.length;j++,i=i->nxt)
    {
        tmpleaf            = (node*)malloc(sizeof(node));
        node_group[j]      = tmpleaf;
        node_group[j]->l   = NULL;
        node_group[j]->r   = NULL;
        node_group[j]->prb = i->prb;
        node_group[j]->abc = i;
        node_group[j]->abc->parent = tmpleaf; // abc link to leaves
    }
    return node_group;
}

node *recurse_tree(node **node_group, int n)
{
    int i, pomx1, pomx2;
    double min1 = 10, min2 = 10;
    node *min1ptr = NULL, *min2ptr = NULL;
    node *new_node;

    // search for 2 minimums
    for(i=0;i<n;i++)
    {
        if(node_group[i]!=NULL)
        {
            if(node_group[i]->prb<min1)
            {
                if(node_group[i]->prb>=min2)
                {
                    min1    = node_group[i]->prb;
                    min1ptr = node_group[i];
                    pomx1   = i;
                }
                else
                {
                    min1    = min2;
                    min1ptr = min2ptr;
                    pomx1   = pomx2;
                    min2    = node_group[i]->prb;
                    min2ptr = node_group[i];
                    pomx2   = i;
                }
            }
        }
    }

    // stop if there is only one node left
    if(min1ptr == NULL)
    {
        return min2ptr;
    }

    // unite two minimums in one higher level node
    else
    {
        //printf("Uniting: %lf and %lf -> ", min1ptr->prb, min2ptr->prb);
        new_node          = (node*)malloc(sizeof(node));
        new_node->l       = min2ptr;
        new_node->r       = min1ptr;
        new_node->prb     = min1ptr->prb + min2ptr->prb;
        node_group[pomx1] = NULL;
        node_group[pomx2] = new_node;
        //printf("%g\n", node_group[pomx2]->prb);
    }

    // now recurse!
    return recurse_tree(node_group, n);
}

void dump_tree(node *root, FILE *fp)
{
    if(root->l == NULL)
    {
        root->abc->code = (char*)malloc(sizeof(char)*strlen(tmpcode)+1);
        strcpy(root->abc->code, tmpcode);
        printf("%c -> %s\n",root->abc->chr, root->abc->code);
        fprintf(fp,"%c%s\n", root->abc->chr, root->abc->code);
    }
    else
    {
        tmpcode[strlen(tmpcode)] = '0';
        dump_tree(root->l, fp);
        tmpcode[strlen(tmpcode)-1] = '\0';
    }

    if(root->r == NULL)
    {
        ;
    }
    else
    {
        tmpcode[strlen(tmpcode)] = '1';
        dump_tree(root->r, fp);
        tmpcode[strlen(tmpcode)-1] = '\0';
    }
}

int main()
{
    tmpcode[0] = '\0';
    FILE *fp, *fp_ct;
    int i=0, buf;
    abc_list lst;
    lst.nxt = NULL;

    /* Read file and build abc*/
    fp = fopen("file.txt", "r");
    while  ( (buf = fgetc(fp)) != EOF )
    {
        add_abc_item(&(lst.nxt), (char)buf);
        i++;
    }

    /* Render abc */
    render_abc(&lst, i);

    /* Build HUFFMAN binary tree */
    node **node_group = build_tree_leaves(lst);
    node *tree = recurse_tree(node_group, lst.length);
    free(node_group);

    /* Print input file info */
    printf("::::::SOURCE INFO::::::\n");
    printf("Read %d characters from file.\n", i);
    printf("File contains %d unique characters.\n", lst.length);
    printf("Data entropy: %lf.\n", lst.entropy);

    /* Generate and dump code table */
    fp_ct = fopen("code_table.txt", "w");
    dump_tree(tree, fp_ct);
    fclose(fp);
    return 0;
}
