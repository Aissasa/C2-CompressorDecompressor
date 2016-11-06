#pragma once
/*-------------------------------------------Defines-------------------------------------------*/
// define the coordinates structure
typedef struct Coordinates
{
	float x;
	float y;
	float z;

} Coordinates_t;

// define the coordinates node structure for the linked list
typedef struct CoordinatesNode
{
	Coordinates_t coordinates;
	struct CoordinatesNode* nextNode;

} CoordinatesNode_t;

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

// define the header structure
typedef struct Header
{
	int numberOfCompressionBits;
	int coordinatesCount;
	Ranges_t ranges;

}Header_t;

// define the bit stream structure
typedef struct Bitstream
{
	short data;
	char currentBit;

}Bitstream_t;

//define buffer structure
typedef struct Buffer
{
	short data;
	char bitsLeft;

}Buffer_t;

// define the slots node structure for the linked list
typedef struct SlotNode
{
	short slot;
	struct SlotNode* nextNode;

} SlotNode_t;



#define COMPRESSED_DATA_FILE "compressedData.bin"
#define COORDINATES_FILE "coordinates.bin"

#define DEBUG 1


/*------------------------------------Linked lists functions-------------------------------------*/
CoordinatesNode_t* createNewCoorNode(Coordinates_t coordinates);
void pushNewCoorNode(CoordinatesNode_t* headNode, Coordinates_t coordinates);

SlotNode_t* createNewSlotNode(short slots);
void pushNewSlotNode(SlotNode_t* headNode, short slots);


CoordinatesNode_t * readOriginalData();

SlotNode_t * decompressData(Header_t* header);
Header_t readHeader(FILE * compressedDataFile);

CoordinatesNode_t * decodeData(SlotNode_t * slotsListHead, Header_t* header);

