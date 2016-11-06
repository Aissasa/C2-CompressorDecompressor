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



	/*-------------------------Retrieving Data--------------------------*/

	// read original data and put it in a linked list
	CoordinatesNode_t * originalDataHead = readOriginalData();

#if DEBUG
	CoordinatesNode_t * current = originalDataHead;
	while (current != NULL)
	{
		printf("x: %f, y: %f, z: %f\n", current->coordinates.x, current->coordinates.y, current->coordinates.z);

		current = current->nextNode;
	}
#endif

	// read the header of the compressed data file
	// read data and decompress it and put it in a linked list
	Header_t header;
	SlotNode_t * slotsListHead = decompressData(&header);

	// decompress the decoded data
	CoordinatesNode_t * decompressedDataHead = decodeData(slotsListHead, &header);

	// compare old data with new data

	return 0;

}


/*---------------------------------------Linked list functions---------------------------------*/

CoordinatesNode_t* createNewCoorNode(Coordinates_t coordinates)
{
	CoordinatesNode_t* node = (CoordinatesNode_t*)malloc(sizeof(CoordinatesNode_t));
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

void pushNewCoorNode(CoordinatesNode_t * headNode, Coordinates_t coordinates)
{
	CoordinatesNode_t* currentNode = headNode;

	while (currentNode->nextNode != NULL)
	{
		currentNode = currentNode->nextNode;
	}

	CoordinatesNode_t* newNode = createNewCoorNode(coordinates);
	currentNode->nextNode = newNode;

}

SlotNode_t * createNewSlotNode(short slot)
{
	SlotNode_t* node = (SlotNode_t*)malloc(sizeof(SlotNode_t));
	if (node != NULL)
	{
		node->slot = slot;
		node->nextNode = NULL;
	}
	else
	{
		printf("Node is null");
	}

	return node;

}

void pushNewSlotNode(SlotNode_t * headNode, short slot)
{
	SlotNode_t* currentNode = headNode;

	while (currentNode->nextNode != NULL)
	{
		currentNode = currentNode->nextNode;
	}

	SlotNode_t* newNode = createNewSlotNode(slot);
	currentNode->nextNode = newNode;
}



CoordinatesNode_t * readOriginalData()
{
	CoordinatesNode_t * head = NULL;
	FILE * coordinatesFile = fopen(COORDINATES_FILE, "rb");

	while (true)
	{
		Coordinates_t coor;
		fread(&coor.x, sizeof(float), 1, coordinatesFile);
		fread(&coor.y, sizeof(float), 1, coordinatesFile);
		fread(&coor.z, sizeof(float), 1, coordinatesFile);

		// if reached end of file
		if (feof(coordinatesFile))
		{
			fclose(coordinatesFile);
			break;
		}

		if (head == NULL)
		{
			head = createNewCoorNode(coor);
		}
		else
		{
			pushNewCoorNode(head, coor);
		}
	}

	return head;
}


SlotNode_t * decompressData(Header_t* header)
{
	FILE * compressedDataFile = fopen(COMPRESSED_DATA_FILE, "rb");

	// get header
	*header = readHeader(compressedDataFile);

	//decompress data
	SlotNode_t * slotsListHead = NULL;
	int coordinatesCounter = 0;

	Bitstream_t bs = { 0, 0 };

	Buffer_t buffer = { 0, 0 };

	while (true)
	{
		if (buffer.bitsLeft <= 0)
		{
			fread(&buffer.data, sizeof(buffer.data), 1, compressedDataFile);

			// if reached end of file
			if (feof(compressedDataFile))
			{
				fclose(compressedDataFile);
				break;
			}
			buffer.bitsLeft = sizeof(buffer.data) * 8;
		}

		while (buffer.bitsLeft > 0 && bs.currentBit < header->numberOfCompressionBits)
		{
			short bit = buffer.data & 1;      // get the bit to add to the stream
			bit <<= bs.currentBit;            // add the offset to the bit
			bs.data += bit;                   // add the offset bit to the stream

			// update buffer
			buffer.data >>= 1;
			buffer.bitsLeft--;

			// update bit stream
			bs.currentBit++;
		}

		if (bs.currentBit >= header->numberOfCompressionBits)
		{
#if DEBUG
			printf("%d\n", bs.data);
#endif // DEBUG

			if (slotsListHead == NULL)
			{
				slotsListHead = createNewSlotNode(bs.data);
			}
			else
			{
				pushNewSlotNode(slotsListHead, bs.data);
			}

			coordinatesCounter++;

			// reached last coordinate needed
			if (coordinatesCounter >= header->coordinatesCount)
			{
				fclose(compressedDataFile);
				break;
			}

			//reset bit stream
			bs = { 0,0 };
		}
	}

	return slotsListHead;
}

Header_t readHeader(FILE * compressedDataFile)
{
	Header_t header;

	// read and store header
	fread(&header.numberOfCompressionBits, sizeof(int), 1, compressedDataFile);
	fread(&header.coordinatesCount, sizeof(int), 1, compressedDataFile);
	fread(&header.ranges.minX, sizeof(float), 1, compressedDataFile);
	fread(&header.ranges.maxX, sizeof(float), 1, compressedDataFile);
	fread(&header.ranges.minY, sizeof(float), 1, compressedDataFile);
	fread(&header.ranges.maxY, sizeof(float), 1, compressedDataFile);
	fread(&header.ranges.minZ, sizeof(float), 1, compressedDataFile);
	fread(&header.ranges.maxZ, sizeof(float), 1, compressedDataFile);

	// set ranges
	header.ranges.xRange = header.ranges.maxX - header.ranges.minX;
	header.ranges.yRange = header.ranges.maxY - header.ranges.minY;
	header.ranges.zRange = header.ranges.maxZ - header.ranges.minZ;

	return header;
}


CoordinatesNode_t * decodeData(SlotNode_t * slotsListHead, Header_t* header)
{
	CoordinatesNode_t * decodedDataHead = NULL;

	//max number of bytes
	int maxNumBytes = (int)pow((double)2, (double)header->numberOfCompressionBits);

	//	calculate the step for each coordinate
	float xStep = header->ranges.xRange / maxNumBytes;
	float yStep = header->ranges.yRange / maxNumBytes;
	float zStep = header->ranges.zRange / maxNumBytes;

	SlotNode_t * currentSlotPtr = slotsListHead;
	Coordinates_t coor = { 0, 0, 0 };
	char counter = 0;

	while (currentSlotPtr != NULL)
	{
#if DEBUG
		printf("\nCoded %d: %d, ", counter, currentSlotPtr->slot);
#endif // DEBUG

		switch (counter)
		{
			case 0:
				coor.x = (currentSlotPtr->slot * xStep) + header->ranges.minX;
				counter++;
				currentSlotPtr = currentSlotPtr->nextNode;
				break;

			case 1:
				coor.y = (currentSlotPtr->slot * yStep) + header->ranges.minY;
				counter++;
				currentSlotPtr = currentSlotPtr->nextNode;
				break;

			case 2:
				coor.z = (currentSlotPtr->slot * zStep) + header->ranges.minZ;
				counter++;
				currentSlotPtr = currentSlotPtr->nextNode;
#if DEBUG
				printf("\nDecoded numbers: %f, %f, %f\n", coor.x, coor.y, coor.z);
#endif // DEBUG

				break;
			
			default:

				if (decodedDataHead == NULL)
				{
					createNewCoorNode(coor);
				}
				else
				{
					pushNewCoorNode(decodedDataHead, coor);
				}
				coor = { 0, 0, 0 };
				counter = 0;

				break;
		}
	}

	return decodedDataHead;

}


