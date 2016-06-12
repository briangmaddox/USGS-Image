#include "Image/Octree.h"
#include "Image/RGBPixel.h"
#include <iostream.h>
#include <stdlib.h>


Octree::Octree(int k)
{
   int i;

   numDesired = k;
   numLeaves  = 0;
   tmpIndex = 0;
   _pallette = NULL;
   for (i=0;i<8;i++)
   {
      _reducibles[i] = new (OctreeNode *)[k+1];
      numEntries[i] = 0;
   }
   _tree = new OctreeNode;

   _tree->leaf  = false;
   _tree->my_index = 0;
   _tree->level = 0;

   _tree->redMidpoint   = 128;
   _tree->greenMidpoint = 128;
   _tree->blueMidpoint  = 128;
   
   _tree->redTotal   = 0;
   _tree->greenTotal = 0;
   _tree->blueTotal  = 0;
   
   _tree->parent = NULL;
   for (i=0;i<8;i++) _tree->child[i] = NULL;
}

Octree::~Octree()
{
   int i;
   traverseLRV(_tree,deleteVisit);
   for (i=0;i<8;i++) delete[] _reducibles[i];
   if (_pallette != NULL) delete _pallette;
}   

OctreeNode* Octree::initNode(int octant, OctreeNode *parent)
{
   OctreeNode *temp = new OctreeNode; 
   int i;

   temp->leaf = false;
   if (parent!=NULL)
   {
      temp->parent = parent;
      temp->level = parent->level + 1;
      temp->my_index = 0;
      temp->numColors = 0;
      temp->redTotal = 0;
      temp->greenTotal = 0;
      temp->blueTotal = 0;
      if (temp->level == 8)
         temp->numUnique = 1;
      else
         temp->numUnique = 0;
      for (i=0;i<8;i++) temp->child[i] = NULL;      
      if (temp->level != 8)
      {
         if ((octant&4) != 0) 
            temp->redMidpoint = parent->redMidpoint + (1 << (7-temp->level));
         else
            temp->redMidpoint = parent->redMidpoint - (1 << (7-temp->level));
         if ((octant&2) != 0) 
            temp->greenMidpoint=parent->greenMidpoint+(1<<(7-temp->level));
         else
            temp->greenMidpoint=parent->greenMidpoint-(1<<(7-temp->level));
         if ((octant&1) != 0) 
            temp->blueMidpoint = parent->blueMidpoint + (1 << (7-temp->level));
         else
            temp->blueMidpoint = parent->blueMidpoint - (1 << (7-temp->level));
      }
      else
      {
         if ((octant&4) != 0) 
            temp->redMidpoint = parent->redMidpoint;
         else
            temp->redMidpoint = parent->redMidpoint - 1;
         if ((octant&2) != 0)
            temp->greenMidpoint=parent->greenMidpoint;
         else
            temp->greenMidpoint=parent->greenMidpoint-1;
         if ((octant&1) != 0)
            temp->blueMidpoint = parent->blueMidpoint;
         else
            temp->blueMidpoint = parent->blueMidpoint - 1;
         temp->leaf = true;
         numLeaves++;
         insertReducible(temp->parent);
      }
   }
   else
   {
      cout << "We got trouble. Parent is NULL.  This shouldn't happen." << endl;
   }
   return temp;
}
      

void Octree::insertColor(OctreeNode* &tree,
                        OctreeNode* parent,
                        unsigned char red,
                        unsigned char green,
                        unsigned char blue)
{
   int octant;

   if (tree == NULL) 
   {
      octant = (red   >= parent->redMidpoint)*4 +
               (green >= parent->greenMidpoint)*2 +
               (blue  >= parent->blueMidpoint)*1;     
      tree = initNode(octant,parent);
   }
   if (tree->leaf)
   {
      //cout << "Found a leaf at level " << (int) tree->level << " ";
      //cout << "Inserting <" << (int) red << " ";
      //cout << (int) green << " " << (int) blue << ">" << endl;
      tree->numColors++;
      tree->redTotal   += red;
      tree->greenTotal += green;
      tree->blueTotal  += blue;
   }
   else
   {
      octant = (red   >= tree->redMidpoint)*4 +
               (green >= tree->greenMidpoint)*2 +
               (blue  >= tree->blueMidpoint)*1;
      insertColor(tree->child[octant],tree,red,green,blue);
   } 
}

void Octree::reduceSubtree(OctreeNode* tree)
{
   int i;
   //cout << "Reducing subtree with children: ";
   for (i=0;i<8;i++)
   {
      if (tree->child[i] != NULL)
      {
         //cout << i << " ";
         tree->redTotal   += tree->child[i]->redTotal;
         tree->greenTotal += tree->child[i]->greenTotal;
         tree->blueTotal  += tree->child[i]->blueTotal;
         tree->numColors  += tree->child[i]->numColors;
         tree->numUnique  += tree->child[i]->numUnique;
         numLeaves--;
         delete tree->child[i];
         tree->child[i] = NULL;
      }
   }
   //cout << endl;
   tree->leaf = true;
   numLeaves++;
   if (sibsAreLeaves(tree)) insertReducible(tree->parent);
}

OctreeNode* Octree::getReducible(void)
{
   OctreeNode* temp;
   int i,j;
   i = 7;
   //cout << "NumEntries[" << i << "]: " << numEntries[i] << endl;
   while (numEntries[i] == 0)
      i--;
   //cout << "Found reducible at level " << i << endl;
   temp = _reducibles[i][0];
   for (j=1;j<numEntries[i];j++)
       _reducibles[i][j-1] = _reducibles[i][j];
   _reducibles[i][j] = NULL;
   numEntries[i] -= 1;
   return temp;
}
   

void Octree::addColor(unsigned char red, unsigned char green,
                      unsigned char blue)
{
   OctreeNode *temp;
   //cout << "NumLeaves = " << numLeaves << endl;
   insertColor(_tree,NULL,red,green,blue);
   while (numLeaves > numDesired)
   {
      temp = getReducible();
      reduceSubtree(temp);
      //cout << "NumLeaves = " << numLeaves << endl;
   }
}

void Octree::insertReducible(OctreeNode* node)
{
   int d = node->level;
   int i;
   bool inThere = false;
   for (i=0;i<numEntries[d];i++)
       if (node ==  _reducibles[d][i])
          inThere = true;
   if (!inThere)
   {
      _reducibles[d][numEntries[d]] = node;
      numEntries[d] += 1;
      //cout << "Inserting a reducible at level " << d << endl;
      //cout << "numEntries[" << d << "] = " << numEntries[d] << endl;
      if (numEntries[d] > 1) sortReducibles(d);
   }
/*
   else
   {
      cout << "This node already in list " << d << endl;
   }
*/
} 

void Octree::sortReducibles(int d)
{
   qsort(_reducibles[d],numEntries[d],sizeof(OctreeNode *),Octree::nodeComp);
}

int Octree::nodeComp(const void* e1, const void* e2)
{
   OctreeNode* foo1 = *((OctreeNode **) e1);
   OctreeNode* foo2 = *((OctreeNode **) e2);
   
   int i;
   int sum1=0;
   int sum2=0;
   

   if (foo1->level > foo2->level) return -1;
   if (foo1->level < foo2->level) return 1;
   for (i=0;i<8;i++)
   {
       if (foo1->child[i] != NULL)
          sum1 += foo1->child[i]->numColors;
       if (foo2->child[i] != NULL)
          sum2 += foo2->child[i]->numColors;
   }
   if (sum1 < sum2)  return -1;
   if (sum1 > sum2)  return  1;
   return 0;
}

bool Octree::sibsAreLeaves(OctreeNode *node)
{
   bool ret = true;
   if (node->parent != NULL)
   {
      int i;
      for (i=0;i<8;i++)
          if (node->parent->child[i] != NULL)
             ret = ret && (node->parent->child[i]->leaf);
   }
   else
      ret = false;

   return ret;
} 

void Octree::assignIndexes(void)
{
    traverseVLR(_tree,indexVisit);
}

void Octree::indexVisit(OctreeNode *node)
{
   if (node->leaf)
      node->my_index = tmpIndex++;
}

void Octree::palletteVisit(OctreeNode *node)
{
   if (node->leaf)
   {
      RGBPixel* tmp = new RGBPixel(node->redTotal/node->numColors,
                                   node->greenTotal/node->numColors,
                                   node->blueTotal/node->numColors);
      _pallette->setEntry(node->my_index,tmp);
      delete tmp;
   }
}

void Octree::deleteVisit(OctreeNode* node)
{
   delete node;
}

   
RGBPallette* Octree::getQuantPallette(void)
{
   RGBPallette* pal;
   if (_pallette != NULL) delete _pallette;
   _pallette = new RGBPallette(numLeaves);

   traverseVLR(_tree,palletteVisit);
  
   pal = (RGBPallette *) _pallette->copyMe();
   return pal;
}

OctreeNode* Octree::findLeaf(OctreeNode *tree, unsigned char red,
                             unsigned char green, unsigned char blue)
{
   int kid;
   OctreeNode *ret;
   if (tree->leaf)
   {
      ret = tree;
   }
   else
   {
      kid = (red   >= tree->redMidpoint)*4 +
            (green >= tree->greenMidpoint)*2 +
            (blue  >= tree->blueMidpoint)*1;
      if (tree->child[kid] != NULL)
         ret = findLeaf(tree->child[kid],red,green,blue);
      else
         ret = NULL;
   }
   return ret;
}

void Octree::getRep(unsigned char red, unsigned char green, unsigned char blue,
                    unsigned char &r,  unsigned char &g,   unsigned char &b)
{
   OctreeNode *temp = findLeaf(_tree,red,green,blue);

   if (temp != NULL)
   {
      r = temp->redTotal/temp->numColors;
      g = temp->greenTotal/temp->numColors;
      b = temp->blueTotal/temp->numColors;
   }
   else
   {
      cout << "Color <" << (int) red << " " << (int) green << " ";
      cout << (int) blue << "> not found..." << endl;
   }
}
     
int Octree::getIndex(unsigned char red,unsigned char green,unsigned char blue)
{
   OctreeNode *temp = findLeaf(_tree,red,green,blue);
      
   if (temp!=NULL)
      return temp->my_index;
   else
   {
      cout << "No index for <" << (int) red << " " << (int) green << " ";
      cout << (int) blue << "> not found..." << endl;
      return -1;
   }
}

void Octree::traverseVLR(void (*visit)(OctreeNode *))
{
    traverseVLR(_tree,visit);
}

void Octree::traverseLRV(void (*visit)(OctreeNode *))
{
    traverseLRV(_tree,visit);
}

void Octree::traverseVLR(OctreeNode *tree,void (*visit)(OctreeNode *))
{
   int i;
   visit(tree);
   for (i=0;i<8;i++)
       if (tree->child[i] != NULL)
          traverseVLR(tree->child[i],visit);
}

void Octree::traverseLRV(OctreeNode *tree,void (*visit)(OctreeNode *))
{
   int i;
   for (i=0;i<8;i++)
       if (tree->child[i] != NULL)
          traverseLRV(tree->child[i],visit);
   visit(tree);
}

void Octree::traverseVLR(OctreeNode *tree,OctreeVisit visit)
{
   int i;
   visit(tree);
   for (i=0;i<8;i++)
       if (tree->child[i] != NULL)
          traverseVLR(tree->child[i],visit);
}

void Octree::traverseLRV(OctreeNode *tree,OctreeVisit visit)
{
   int i;
   for (i=0;i<8;i++)
       if (tree->child[i] != NULL)
          traverseLRV(tree->child[i],visit);
   visit(tree);
}
