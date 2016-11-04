#pragma once
/*-------------------------------------------Defines-------------------------------------------*/
// define a coordinates structure
typedef struct Coordinates
{
	float x;
	float y;
	float z;
} Coordinates_t;

// define the node structure for the linked list
typedef struct Node
{
	Coordinates_t coordinates;
	struct Node* nextNode;
} Node_t;

// define the ranges structure that will be used for storing data used in the compression
typedef struct Ranges
{
	//mins
	float minX;
	float minY;
	float minZ;

	//maxs
	float maxX;
	float maxY;
	float maxZ;

	//ranges
	//		maxX - minX
	float xRange;
	//		maxY - minY
	float yRange;
	//		maxZ - minZ
	float zRange;

} Ranges_t;

// define the slots structure
typedef struct Slots
{
	int xSlot;
	int ySlot;
	int zSlot;
} Slots_t;


#define VERTICES_FILE "verts.txt"

#define COMPRESSION_BITS 6


/*------------------------------------Linked list functions-------------------------------------*/
Node_t* createNewNode(Coordinates_t coordinates);
void pushNewNode(Node_t* headNode, Coordinates_t coordinates);


/*------------------------------------Compressing functions-------------------------------------*/
void updateRanges(Coordinates_t* coordinates, Ranges_t* ranges);
void compressData(Node_t* listHead, Ranges_t* ranges, int maxNumBytes);

