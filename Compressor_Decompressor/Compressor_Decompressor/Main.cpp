/*
	Written By Aissa Ben Zayed

	Compressor project: This program takes in a text file that contains vertices,
	extracts the x, y and z coordinates, compresses them and put them in a binary file.
*/

/*-------------------------------------------Includes-------------------------------------------*/
#include "stdio.h"
#include "stdlib.h"
#include "Math.h"
#include "Main.h"


/*-------------------------------------------Main-------------------------------------------*/
int main(int argc, char * argv[])
{
	/*----------------------Variables Declaration----------------------*/

	FILE * verticesFile;

	// initialize the list pointer
	Node_t* listHead = NULL;

	// counter for number of coordinates
	//int coordinatesCounter = 0;

	// ranges to be used for the compression
	Ranges_t ranges;



	/*---------------------------Retrieving Data------------------------*/

	verticesFile = fopen(VERTICES_FILE, "r");

	if (verticesFile == NULL)
	{
		printf("Error: file pointer is null");
		return -1;
	}

	int id = 0;
	Coordinates_t coor;

	while (true)
	{
		// read the data
		fscanf(verticesFile, "%d: %f %f %f", &id, &(coor.x), &(coor.y), &(coor.z));

		// urgent write these parsed data to a file for the decompresser to compare with

		// push the data to the list
		if (listHead == NULL)
		{
			listHead = createNewNode(coor);

			//initialize mins and maxs
			ranges.minX = ranges.maxX = coor.x;
			ranges.minY = ranges.maxY = coor.y;
			ranges.minZ = ranges.maxZ = coor.z;
		}
		else
		{
			pushNewNode(listHead, coor);

			//check for new mins and maxs
			updateRanges(&coor, &ranges);
		}

		// increment counter
		//coordinatesCounter++;

		// go to next line
		char eol = fgetc(verticesFile);
		while (eol != EOF && eol != '\n')
		{
			eol = fgetc(verticesFile);
		}

		// if reached the end of file, exit
		if (eol == EOF)
		{
			break;
		}
	}

	/*---------------------------Compressing Data------------------------*/

	// prepare for compression
	//		prepare the ranges
	ranges.xRange = ranges.maxX - ranges.minX;
	ranges.yRange = ranges.maxY - ranges.minY;
	ranges.zRange = ranges.maxZ - ranges.minZ;

	//		calculate the max number of bytes
	int maxNumBytes = (int)pow(2, COMPRESSION_BITS);


	// launch the compression
	compressData(listHead, &ranges, maxNumBytes);

	/*---------------------------Bit streaming Data------------------------*/
	bitStreamDataToFile(listHead);


	fclose(verticesFile);

	return 0;

}


/*---------------------------------------Linked list functions---------------------------------*/

Node_t* createNewNode(Coordinates_t coordinates)
{
	Node_t* node = (Node_t*)malloc(sizeof(Node_t));
	if (node != NULL)
	{
		node->coordinates = coordinates;
		node->nextNode = NULL;
	}
	else
	{
		printf("Node is null");
	}

	return node;
}

void pushNewNode(Node_t * headNode, Coordinates_t coordinates)
{
	Node_t* currentNode = headNode;

	while (currentNode->nextNode != NULL)
	{
		currentNode = currentNode->nextNode;
	}

	Node_t* newNode = createNewNode(coordinates);
	currentNode->nextNode = newNode;

}

/*---------------------------------------Compressing functions---------------------------------*/

void updateRanges(Coordinates_t * coordinates, Ranges_t * ranges)
{
	if (coordinates->x < ranges->minX)
	{
		ranges->minX = coordinates->x;
	}
	if (coordinates->y < ranges->minY)
	{
		ranges->minY = coordinates->y;
	}
	if (coordinates->z < ranges->minZ)
	{
		ranges->minZ = coordinates->z;
	}

	if (coordinates->x > ranges->maxX)
	{
		ranges->maxX = coordinates->x;
	}
	if (coordinates->y > ranges->maxY)
	{
		ranges->maxY = coordinates->y;
	}
	if (coordinates->z > ranges->maxZ)
	{
		ranges->maxZ = coordinates->z;
	}
}

void compressData(Node_t * listHead, Ranges_t * ranges, int maxNumBytes)
{
	//	calculate the step for each coordinate
	float xStep = ranges->xRange / maxNumBytes;
	float yStep = ranges->yRange / maxNumBytes;
	float zStep = ranges->zRange / maxNumBytes;

	printf("Mins: %f %f %f \n", ranges->minX, ranges->minY, ranges->minZ);
	printf("Maxs: %f %f %f \n", ranges->maxX, ranges->maxY, ranges->maxZ);
	printf("Ranges: %f %f %f \n", ranges->xRange, ranges->yRange, ranges->zRange);
	printf("Steps: %f %f %f\n", xStep, yStep, zStep);

	Node_t* current = listHead;
	while (current != NULL)
	{
		printf("\nOld values: x: %f, y: %f, z: %f\n", current->coordinates.x, current->coordinates.y, current->coordinates.z);


		//get the slots the number will land on
		float xSlot = roundf((current->coordinates.x - ranges->minX) / xStep);		
		if (xSlot >= maxNumBytes)
		{
			xSlot--;		// make sure the max wont overflow
		}

		float ySlot = roundf((current->coordinates.y - ranges->minY) / yStep);
		if (ySlot >= maxNumBytes)
		{
			ySlot--;		// make sure the max wont overflow
		}

		float zSlot = roundf((current->coordinates.z - ranges->minZ) / zStep);
		if (zSlot >= maxNumBytes)
		{
			zSlot--; 		// make sure the max wont overflow
		}


		current->coordinates.x = xSlot;
		current->coordinates.y = ySlot;
		current->coordinates.z = zSlot;

		printf("New values: x: %f, y: %f, z: %f\n", current->coordinates.x, current->coordinates.y, current->coordinates.z);

		current = current->nextNode;
	}
}

/*------------------------------------Bit streaming functions------------------------------------*/

void bitStreamDataToFile(Node_t * listHead)
{
	FILE * compressedVertsFile = fopen(COMPRESSED_VERTS_FILE, "wb");
	// urgent add the header to the binary file

	// initialize bit stream
	Bitstream_t bs = { 0, 0, COMPRESSION_BITS };

	Node_t * currentNode = listHead;

	// go through all of the data and bit stream them
	while (currentNode->nextNode != NULL)
	{
		// put the current node data into slots data structure
		Slots_t slots;
		slots.xSlot = (short)currentNode->coordinates.x;
		slots.ySlot = (short)currentNode->coordinates.y;
		slots.zSlot = (short)currentNode->coordinates.z;

		bitStreamCurrentSlots(&slots, &bs, compressedVertsFile, false);

		currentNode = currentNode->nextNode;
	}

	Slots_t slots;
	slots.xSlot = (short)currentNode->coordinates.x;
	slots.ySlot = (short)currentNode->coordinates.y;
	slots.zSlot = (short)currentNode->coordinates.z;

	bitStreamCurrentSlots(&slots, &bs, compressedVertsFile, true);

	fclose(compressedVertsFile);

}

void bitStreamCurrentSlots(Slots_t* slots, Bitstream_t* bs, FILE * compressedVertsFile, bool lastCoordinates)
{
	// encode x
	bitStreamASlot(slots->xSlot, bs, compressedVertsFile, false);
	// encode y
	bitStreamASlot(slots->ySlot, bs, compressedVertsFile, false);
	// encode z
	bitStreamASlot(slots->zSlot, bs, compressedVertsFile, lastCoordinates);

}

void bitStreamASlot(short slot, Bitstream_t* bs, FILE * compressedVertsFile, bool lastSlot)
{

	for (char i = 0; i < bs->numberOfCompressionBits; i++)
	{
		short bit = slot & 1;      // get the bit to add to the stream
		bit <<= bs->currentBit;    // add the offset to the bit
		bs->data += bit;           // add the offset bit to the stream

		if (++bs->currentBit > 15) // going to overflow
		{
			// write the current data in bs to the file
			fwrite(&bs->data, sizeof(bs->data), 1, compressedVertsFile); 

		    // reset the bit stream
			bs->data = 0;
			bs->currentBit = 0;
		}

		// update buffer
		slot >>= 1;
	}

	if (lastSlot)
	{
		// write the current data in bs to the file
		fwrite(&bs->data, sizeof(bs->data), 1, compressedVertsFile);

		// reset the bit stream
		bs->data = 0;
		bs->currentBit = 0;

	}
}






