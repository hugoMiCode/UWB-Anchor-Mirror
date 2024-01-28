#include "TagLinkNode.h"

TagLinkNode *init_tagLinkNode()
{
#ifdef SERIAL_DEBUG
    Serial.println("init_tagLink");
#endif
    struct TagLinkNode *p = (struct TagLinkNode *)malloc(sizeof(struct TagLinkNode));
    p->next = NULL;
    p->tag_addr = 0;
    p->range = 0.0;

    return p;
}

void add_link(TagLinkNode *p, uint16_t addr)
{
#ifdef SERIAL_DEBUG
    Serial.println("add_tagLink");
#endif
    struct TagLinkNode *temp = p;

    while (temp->next != NULL)
        temp = temp->next;

    //Create a tag
    struct TagLinkNode *a = (struct TagLinkNode *)malloc(sizeof(struct TagLinkNode));
    a->tag_addr = addr;
    a->range = 0.0;
    a->dbm = 0.0;
    a->next = NULL;

    //Add tag to end of struct MyLink
    temp->next = a;

    return;
}

TagLinkNode *find_link(TagLinkNode *p, uint16_t addr)
{
#ifdef SERIAL_DEBUG
    Serial.println("find_tagLink");
#endif
    if (addr == 0) {
#ifdef SERIAL_DEBUG
        Serial.println("find_tagLink:Input addr is 0");
#endif
        return NULL;
    }

    if (p->next == NULL) {
#ifdef SERIAL_DEBUG
        Serial.println("find_tagLink:Link is empty");
#endif
        return NULL;
    }

    struct TagLinkNode *temp = p;
    while (temp->next != NULL) {
        temp = temp->next;

        if (temp->tag_addr == addr)
            return temp;
    }

#ifdef SERIAL_DEBUG
    Serial.println("find_tagLink:Can not find tag");
#endif
    return nullptr;
}

void fresh_link(TagLinkNode *p, uint16_t addr, float range, float dbm)
{
#ifdef SERIAL_DEBUG
    Serial.println("fresh_tagLink");
#endif
    struct TagLinkNode *temp = find_link(p, addr);

    if (temp == NULL) {
#ifdef SERIAL_DEBUG
        Serial.println("fresh_tagLink:Can not find tag");
#endif
        return;
    }

    temp->range = range;
    temp->dbm = dbm;

    return;
}

void print_link(TagLinkNode *p)
{
#ifdef SERIAL_DEBUG
    Serial.println("print_tagLink");
#endif
    struct TagLinkNode *temp = p;

    if (temp->next == NULL) {
#ifdef SERIAL_DEBUG
        Serial.println("print_tagLink:Link is empty");
#endif
        return;
    }

    while (temp->next != NULL) {
        temp = temp->next;
        Serial.print("tag_addr: ");
        Serial.print(temp->tag_addr, HEX);
        Serial.print(" range: ");
        Serial.print(temp->range);
        Serial.print(" dbm: ");
        Serial.println(temp->dbm);
    }

    return;
}

void delete_link(TagLinkNode *p, uint16_t addr)
{
#ifdef SERIAL_DEBUG
    Serial.println("delete_tagLink");
#endif
    if (addr == 0) {
#ifdef SERIAL_DEBUG
        Serial.println("delete_tagLink:Input addr is 0");
#endif
        return;
    }

    if (p->next == NULL) {
#ifdef SERIAL_DEBUG
        Serial.println("delete_tagLink:Link is empty");
#endif
        return;
    }

    struct TagLinkNode *temp = p;

    while (temp->next != NULL) {
        if (temp->next->tag_addr == addr) {
            struct TagLinkNode *a = temp->next;
            temp->next = temp->next->next;
            free(a);
            return;
        }

        temp = temp->next;
    }

}

void make_link_json(TagLinkNode *p, String *s)
{
#ifdef SERIAL_DEBUG
    Serial.println("make_tagLink_json");
#endif
    struct TagLinkNode *temp = p;

    *s = "{\"TagLink\":[";

    while (temp->next != NULL) {
        temp = temp->next;
        char c[100];
        sprintf(c, "{\"Ad\":\"%X\",\"R\":\"%.2f\",\"dbm\":\"%.2f\"}", temp->tag_addr, temp->range, temp->dbm);
        *s += c;

        if (temp->next != NULL)
            *s += ",";
    }

    *s += "]}";

    return;
}
