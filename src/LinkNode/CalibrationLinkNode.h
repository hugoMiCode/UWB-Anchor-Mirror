#include <Arduino.h>

struct CalibrationLinkNode
{
    uint16_t anchor_addr;
    float averageRange;
    float averageDbm;
    uint32_t numberOfValues;
    struct CalibrationLinkNode *next;
};

struct CalibrationLinkNode *init_calibrationLinkNode();
void add_link(struct CalibrationLinkNode *p, uint16_t addr);
struct CalibrationLinkNode *find_link(struct CalibrationLinkNode *p, uint16_t addr);
void update_link(struct CalibrationLinkNode *p, uint16_t addr, float range, float dbm);
void print_link(struct CalibrationLinkNode *p);
void delete_link(struct CalibrationLinkNode *p, uint16_t addr);
void make_link_json(struct CalibrationLinkNode *p,String *s);


