#pragma once
/*-------------------------------------------Defines-------------------------------------------*/
// define the coordinates structure
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

// define the bit stream structure
typedef struct Bitstream
{
	short data;
	char currentBit;
	char numberOfCompressionBits;

}Bitstream_t;

// define the slots structure
typedef struct Slots
{
	short xSlot;
	short ySlot;
	short zSlot;

} Slots_t;


#define VERTICES_FILE "../Data/verts.txt"
#define COMPRESSED_VERTS_FILE "../Data/compressedData"
#define DOT_BIN ".bin"
#define COORDINATES_FILE "../Data/coordinates.bin"

#define DEFAULT_COMPRESSION_BITS 5
#define DEBUG 0

char compressionBits;


/*------------------------------------Linked list functions-------------------------------------*/
Node_t* createNewNode(Coordinates_t coordinates);
void pushNewNode(Node_t* headNode, Coordinates_t coordinates);

/*------------------------------------Encoding functions-------------------------------------*/
void updateRanges(Coordinates_t* coordinates, Ranges_t* ranges);
void encodeData(Node_t* listHead, Ranges_t* ranges, int maxNumBytes);

/*------------------------------------Bit streaming functions------------------------------------*/
void compressDataToFile(Node_t * listHead, Ranges_t * ranges, int coordinatesCounter);
void writeHeader(FILE * compressedVertsFile, Ranges_t * ranges, int coordinatesCounter);
void compressCurrentSlots(Slots_t* slots, Bitstream_t* bs, FILE * compressedVertsFile, bool lastCoordinates);
void compressASlot(short slot, Bitstream_t* bs, FILE * compressedVertsFile, bool lastSlot);



