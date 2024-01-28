#include <Arduino.h>

struct TagLinkNode
{
    uint16_t tag_addr;
    float range;
    float dbm;
    struct TagLinkNode *next;
};

struct TagLinkNode *init_tagLinkNode();
void add_link(struct TagLinkNode *p, uint16_t addr);
struct TagLinkNode *find_link(struct TagLinkNode *p, uint16_t addr);
void fresh_link(struct TagLinkNode *p, uint16_t addr, float range, float dbm);
void print_link(struct TagLinkNode *p);
void delete_link(struct TagLinkNode *p, uint16_t addr);
void make_link_json(struct TagLinkNode *p,String *s);