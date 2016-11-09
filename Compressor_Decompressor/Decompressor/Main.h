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
	float minX;
	float minY;
	float minZ;

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
	unsigned short data;
	char currentBit;

}Bitstream_t;

//define buffer structure
//		it is used as a temporary vessel for data read from the compressed data file
typedef struct Buffer
{
	unsigned short data;
	char bitsLeft;

}Buffer_t;

// define the slots node structure for the linked list
//		a slot represents an encoded coordinate
typedef struct SlotNode
{
	unsigned short slot;
	struct SlotNode* nextNode;

} SlotNode_t;



#define COMPRESSED_DATA_FILE "../Data/compressedData"
#define DOT_BIN ".bin"
#define COORDINATES_FILE "../Data/coordinates.bin"

#define STATS_FILE "../Data/stats.csv"

#define MIN_COMPRESSION_BITS 5

#define DEBUG 0


/*------------------------------------Linked lists functions-------------------------------------*/
CoordinatesNode_t* createNewCoorNode(Coordinates_t coordinates);
void pushNewCoorNode(CoordinatesNode_t* headNode, Coordinates_t coordinates);

SlotNode_t* createNewSlotNode(short slots);
void pushNewSlotNode(SlotNode_t* headNode, short slots);

/*-------------------------------Reading Original data functions----------------------------------*/
CoordinatesNode_t * readOriginalData();

/*-----------------------------------Decompression functions--------------------------------------*/
SlotNode_t * decompressData(Header_t* header, unsigned int* compressedDataFileSize, char compressionBits);
unsigned int GetFileSize(FILE* compressedDataFile);
Header_t readHeader(FILE * compressedDataFile);

/*--------------------------------------Decoding functions----------------------------------------*/
CoordinatesNode_t * decodeData(SlotNode_t * slotsListHead, Header_t* header);

/*------------------------------------RMS Error functions-----------------------------------------*/
float calculateRMSError(CoordinatesNode_t* originalDataHead, CoordinatesNode_t* decompressedDataHead);

/*-----------------------------------Write Stats functions-----------------------------------------*/
void writeToStatsFile(Header_t header, float rmsError, unsigned int compressedDataFileSize);

