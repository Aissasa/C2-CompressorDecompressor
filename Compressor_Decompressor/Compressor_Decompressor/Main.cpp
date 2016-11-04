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

	//initialize the list pointer
	Node_t* listHead = NULL;

	//counter for number of coordinates
	int coordinatesCounter = 0;

	//ranges to be used for the compression
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
		//read the data
		fscanf(verticesFile, "%d: %f %f %f", &id, &(coor.x), &(coor.y), &(coor.z));

		//push the data to the list
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

		//increment counter
		coordinatesCounter++;

		//go to next line
		char eol = fgetc(verticesFile);
		while (eol != EOF && eol != '\n')
		{
			eol = fgetc(verticesFile);
		}

		//if reached the end of file, exit
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


	//launch the compression
	compressData(listHead, &ranges, maxNumBytes);

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

//todo add the bitstream and adding to the file
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
		printf("Old values: x: %f, y: %f, z: %f\n", current->coordinates.x, current->coordinates.y, current->coordinates.z);

		//get the slots the number will land on
		float xSlot = roundf((current->coordinates.x - ranges->minX) / xStep);
		float ySlot = roundf((current->coordinates.y - ranges->minY) / yStep);
		float zSlot = roundf((current->coordinates.z - ranges->minZ) / zStep);

		current->coordinates.x = xSlot;
		current->coordinates.y = ySlot;
		current->coordinates.z = zSlot;

		printf("New values: x: %f, y: %f, z: %f\n", current->coordinates.x, current->coordinates.y, current->coordinates.z);

		current = current->nextNode;
	}
}


