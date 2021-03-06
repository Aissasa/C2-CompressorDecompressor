/*
	Written By Aissa Ben Zayed

	Decompresser project: This program takes in a number as an arg that represents 
	a binary file that contains compressed data, extracts the data, decompresses it 
	and then compares it to the original data.
*/

/*-------------------------------------------Includes-------------------------------------------*/
#include "stdio.h"
#include "stdlib.h"
#include "Math.h"
#include "String.h"
#include "Main.h"


/*-------------------------------------------Main-------------------------------------------*/
int main(int argc, char * argv[])
{
	/*----------------------Variables Declaration----------------------*/
	// get the number of compression bits
	char compressionBits;
	if (argc == 2)
	{
		compressionBits = atoi(argv[1]);
	}
	else if (argc > 2)
	{
		compressionBits = MIN_COMPRESSION_BITS;
		printf("Too many arguments supplied.\n");
	}
	else
	{
		compressionBits = MIN_COMPRESSION_BITS;
		printf("One argument expected.\n");
	}


	// read original data and store it in a linked list
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
	unsigned int compressedDataFileSize = 0;
	SlotNode_t * slotsListHead = decompressData(&header, &compressedDataFileSize, compressionBits);

	// decompress the decoded data
	CoordinatesNode_t * decompressedDataHead = decodeData(slotsListHead, &header);

	// compare old data with new data
	float rmsError = calculateRMSError(originalDataHead, decompressedDataHead);
#if DEBUG
	printf("\nRMS Error is: %f", rmsError);

#endif // DEBUG

	// write the data to the csv file
	writeToStatsFile(header, rmsError, compressedDataFileSize);

	return 0;

}


/*---------------------------------------Linked list functions---------------------------------*/
// linked list for coordinates
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

// linked list for slots, which represent an encoded coordinate
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

/*-------------------------------Reading Original data functions----------------------------------*/

CoordinatesNode_t * readOriginalData()
{
	CoordinatesNode_t * head = NULL;
	FILE * coordinatesFile = fopen(COORDINATES_FILE, "rb");

	while (true)
	{
		// read coordinates
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

		// push the coordinates to the list
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

/*-----------------------------------Decompression functions--------------------------------------*/

// this method decompress data read from the compressed data file and returns an encoded data list,
// and the header of the file 
SlotNode_t * decompressData(Header_t* header, unsigned int* compressedDataFileSize, char compressionBits)
{
	// build the file name
	char fileName[30] = COMPRESSED_DATA_FILE;
	char bitsBuffer[5];

	sprintf(bitsBuffer, "%d", compressionBits);

	strcat(fileName, bitsBuffer);
	strcat(fileName, DOT_BIN);

	FILE * compressedDataFile = fopen(fileName, "rb");

	*compressedDataFileSize = GetFileSize(compressedDataFile);

	// get header
	*header = readHeader(compressedDataFile);

	//decompress data
	SlotNode_t * slotsListHead = NULL;
	int coordinatesCounter = 0;

	Bitstream_t bs = { 0, 0 };

	Buffer_t buffer = { 0, 0 };

	while (true)
	{
		// refill buffer if it is empty 
		if (buffer.bitsLeft <= 0)
		{
			fread(&buffer.data, sizeof(buffer.data), 1, compressedDataFile);

			// if reached end of file, exit the loop
			if (feof(compressedDataFile))
			{
				fclose(compressedDataFile);
				break;
			}
			buffer.bitsLeft = sizeof(buffer.data) * 8;
		}

		// loop until either the buffer is empty or the bit stream is full
		while (buffer.bitsLeft > 0 && bs.currentBit < header->numberOfCompressionBits)
		{
			unsigned short bit = buffer.data & 1;      // get the bit to add to the stream
			bit <<= bs.currentBit;                     // add the offset to the bit
			bs.data += bit;                            // add the offset bit to the stream

			// update buffer
			buffer.data >>= 1;
			buffer.bitsLeft--;

			// update bit stream
			bs.currentBit++;
		}

		// if bit stream is full, add the data to the list of encoded data
		if (bs.currentBit >= header->numberOfCompressionBits)
		{
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

// get file size for statistics
unsigned int GetFileSize(FILE* compressedDataFile)
{
	fseek(compressedDataFile, 0, SEEK_END); // seek to end of file
	unsigned int size = ftell(compressedDataFile); // get current file pointer
	fseek(compressedDataFile, 0, SEEK_SET); // seek back to beginning of file

	return size;
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

/*--------------------------------------Decoding functions----------------------------------------*/

// this methods decodes encoded data and return a decoded data list
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


	// go through the encoded data list and decoded its coordinates, 
	// and add the latter to the decoded data list
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

				if (decodedDataHead == NULL)
				{
					decodedDataHead = createNewCoorNode(coor);
				}
				else
				{
					pushNewCoorNode(decodedDataHead, coor);
				}

				break;

			default:

				coor = { 0, 0, 0 };
				counter = 0;

				break;
		}
	}

	return decodedDataHead;

}

/*------------------------------------RMS Error functions-----------------------------------------*/

float calculateRMSError(CoordinatesNode_t* originalDataHead, CoordinatesNode_t* decompressedDataHead)
{
	CoordinatesNode_t* currentOriginalData = originalDataHead;
	CoordinatesNode_t* currentDecompressedData = decompressedDataHead;

	float square = 0;
	int counter = 0;

	while (currentOriginalData != NULL)
	{
		square += (float)pow((double)currentOriginalData->coordinates.x - (double)currentDecompressedData->coordinates.x, 2);
		square += (float)pow((double)currentOriginalData->coordinates.y - (double)currentDecompressedData->coordinates.y, 2);
		square += (float)pow((double)currentOriginalData->coordinates.z - (double)currentDecompressedData->coordinates.z, 2);

		counter += 3;

		currentOriginalData = currentOriginalData->nextNode;
		currentDecompressedData = currentDecompressedData->nextNode;
	}

	return sqrtf(square / counter);
}

/*-----------------------------------Write Stats functions-----------------------------------------*/

void writeToStatsFile(Header_t header, float rmsError, unsigned int compressedDataFileSize)
{
	char compressionBits = (char)header.numberOfCompressionBits;

	FILE* statsFile;

	if (compressionBits == 5)
	{
		// create a new one
		statsFile = fopen(STATS_FILE, "w");

		// add headings
		fprintf(statsFile, "Number_Of_Bits, RMS_Error, File_Size\n");

	}
	else
	{
		// append to file
		statsFile = fopen(STATS_FILE, "a+");
	}

	// output data to the file
	fprintf(statsFile, "%d, %f, %d\n", compressionBits, rmsError, compressedDataFileSize);

	fclose(statsFile);

}





