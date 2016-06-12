#ifndef _OCTREE_H_
#define _OCTREE_H_
#include "Image/RGBPallette.h"

struct OctreeNode
{
   bool leaf;
   int my_index;

   unsigned char level;
   unsigned char redMidpoint;
   unsigned char greenMidpoint;
   unsigned char blueMidpoint;
   
   unsigned long redTotal;
   unsigned long greenTotal;
   unsigned long blueTotal;

   unsigned long numColors;
   unsigned long numUnique;
   
   OctreeNode *parent;
   OctreeNode *child[8];
};


class Octree
{
   typedef void (Octree::*OctreeVisit)(OctreeNode *);
   OctreeNode *_tree;
   OctreeNode **_reducibles[8];
   int        numEntries[8];

   int numLeaves;
   int numDesired;
   int tmpIndex;

   RGBPallette* _pallette;

 public:

   Octree(int k);    // k is number of desired colors
   ~Octree();

   void addColor(unsigned char red, unsigned char green, unsigned char blue);

   void assignIndexes(void);

   void getRep(unsigned char red, unsigned char green, unsigned char blue,
               unsigned char &r,  unsigned char &g,    unsigned char &b);
   int  getIndex(unsigned char red, unsigned char green, unsigned char blue);
   RGBPallette* getQuantPallette(void);
   
   void traverseVLR(void (*visit)(OctreeNode *));
   void traverseLRV(void (*visit)(OctreeNode *));

 protected:

   OctreeNode *initNode(int octant, OctreeNode *parent);

   void insertColor(OctreeNode* &tree,
                    OctreeNode* parent,
                    unsigned char red,
                    unsigned char green,
                    unsigned char blue);

   void reduceSubtree(OctreeNode* tree);
   OctreeNode *getReducible();
   void insertReducible(OctreeNode *node);
   void sortReducibles(int d);

   bool sibsAreLeaves(OctreeNode *node);
   OctreeNode* findLeaf(OctreeNode *, unsigned char, unsigned char,
                        unsigned char);

   void traverseVLR(OctreeNode *,void (*visit)(OctreeNode *));
   void traverseLRV(OctreeNode *,void (*visit)(OctreeNode *));
   void traverseVLR(OctreeNode *,OctreeVisit);
   void traverseLRV(OctreeNode *,OctreeVisit);

          void palletteVisit(OctreeNode*);
          void indexVisit(OctreeNode*);

   static int  nodeComp(const void* e1, const void* e2);
   static void deleteVisit(OctreeNode*);
};

#endif
