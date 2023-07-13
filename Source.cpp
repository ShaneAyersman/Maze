#include <iostream>
#include <fstream>
#include <string>
using namespace std;

enum terrain
{
	Open,		// A position that you can move to in a maze
	Visited,	// A position that has already been moved to in a maze (cannot revisit)
	Wall,		// A wall of the maze
	Exit,		// An exit of the maze
};

struct stack
{
	int arrayStack[20];	// Declare stack as an array
	int top = -1;		// Declare top index of stack, initialize to -1 (empty)
};

void clearArray(char mazeArray[][15]);													// Initializes the array for the maze
void fillArray(string line, char mazeArray[][15], int& row, int& col, ifstream& fin);	// Creates maze from input file
void printMaze(char maze[][15], int& row, int& col, int& mazeNumber, ofstream& fout);	// Prints the array (maze)

void clearTerrain(int mazeTerrain[][15]);																// Initializes terrain array
void getTerrain(char mazeArray[][15], int mazeTerrain[][15], int& row, int& col, int& numberOfExits);	// Assigns different terrain of the maze to an array
void printTerrain(int mazeTerrain[][15], int& row, int& col);											// Prints constants that represent the terrain of the maze (check if getTerrain works)

int clearStack(stack& stack);		// Clears a stack by setting top of stack to -1 (empty)
bool isEmpty(stack& stack);			// Checks if stack is empty
bool isFull(stack& stack);			// Checks if stack is full
int pop(stack& stack);				// Deletes and returns the top element in stack
void push(stack& stack, int num);	// Adds new element to top of stack

void depthFirstSearch(int mazeTerrain[][15], int row, int col, int numRowsInMaze, int numColumnsInMaze, stack& rowStack, stack& colStack);	// Finds paths through maze
void printPath(char maze[][15], stack rowStack, stack colStack, int row, int col, int& pathDistance,
	string& shortestPath, int& shortestPathDistance, ofstream& fout);	// Prints path through maze (+ ordered pairs), distance, and stores shortest path/distance

int main()
{
	string line = " ";					// Variable to store a single line from input file
	char mazeArray[15][15];				// 2 dimensional array to store maze from input file
	int mazeTerrain[15][15];			// 2 dimensional array to store maze terrain

	int mazeNumber = 0;					// Formatting for which maze you are before outputing the maze and its paths
	int row = 0, col = 0;				// Row and collumn for the maze
	int numRowsInMaze;					// TOTAL rows in maze
	int numColumnsInMaze;				// TOTAL columns in maze

	int pathDistance = 0;				// Variable to store distance of a path through the maze
	string shortestPath;				// Variable to hold the shortest path (ordered pairs) through a maze
	int shortestPathDistance = 0;		// Variable to store distance of the shortest path through the maze

	int row_start = 1, col_start = 0;	// Every maze starts at index (1, 0) which will be the second row, first column
	int numberOfExits;					// Counts exits in the maze

	stack rowStack;						// Holds rows for path positions (ordered pairs)
	stack colStack;						// Holds columns for path positions (ordered pairs)

	ifstream fin;
	ofstream fout;

	//Open files
	fin.open("mazeData.txt");
	if (fin.fail())
	{
		cout << "File could not be opened.\n";
		exit(-1);
	}
	fout.open("MazeOutput.txt");

	getline(fin, line);
	fout << line;
	getline(fin, line);

	while (!fin.eof())
	{
		mazeNumber++;					// Increment mazeNumber for every new maze
		numberOfExits = 0;				// Reset numberOfExits to 0 for every new maze
		shortestPath = "";				// Reset shortestPath for every new maze
		shortestPathDistance = 1000;	// Initialize shortestPath distance to be greater than the pathDistance of any maze (in the case a maze has only 1 path, that path will be the shortest)

		clearArray(mazeArray);
		fin >> row >> col;				// Get row/column for the maze

		//Storage for total rows/columns in maze that will not be changed (for bounds checking when finding paths)
		numRowsInMaze = row;
		numColumnsInMaze = col;

		getline(fin, line);
		fillArray(line, mazeArray, row, col, fin);
		printMaze(mazeArray, row, col, mazeNumber, fout);

		clearTerrain(mazeTerrain);
		getTerrain(mazeArray, mazeTerrain, row, col, numberOfExits);

		cout << "Checking getTerrain function for maze #" << mazeNumber << "\n";
		printTerrain(mazeTerrain, row, col);

		clearStack(rowStack);
		clearStack(colStack);

		for (int i = numberOfExits, pathNumber = 1; i > 0; i--, pathNumber++)
		{
			fout << "\nPath " << pathNumber << "\n";
			depthFirstSearch(mazeTerrain, row_start, col_start, numRowsInMaze, numColumnsInMaze, rowStack, colStack);
			printPath(mazeArray, rowStack, colStack, row, col, pathDistance, shortestPath, shortestPathDistance, fout);
		}
		fout << "\nShortest path for this maze: " << shortestPath << "\n" << "Shortest path distance: " << shortestPathDistance << "\n";	//Output shortest path/distance for a maze
	}

	//Close files
	fin.close();
	fout.close();
	system("pause");
}

void clearArray(char mazeArray[][15])
{
	for (int row = 0; row < 15; row++)
		for (int col = 0; col < 15; col++)
			mazeArray[row][col] = '0';
}

//Creates maze from input file
void fillArray(string line, char mazeArray[][15], int& row, int& col, ifstream& fin)
{
	for (int r = 0; r < row; r++)
	{
		getline(fin, line);

		for (int c = 0; c < col; c++)
			mazeArray[r][c] = line[c];
	}
}

void printMaze(char maze[][15], int& row, int& col, int& mazeNumber, ofstream& fout)
{
	fout << "\n\n##### Maze " << mazeNumber << " #####";
	fout << "\n" << row << "	" << col << "\n";

	for (int r = 0; r < row; r++)
	{
		for (int c = 0; c < col; c++)
			fout << maze[r][c];

		fout << "\n";
	}
}

void clearTerrain(int mazeTerrain[][15])
{
	for (int row = 0; row < 15; row++)
		for (int col = 0; col < 15; col++)
			mazeTerrain[row][col] = -1;
}

void getTerrain(char mazeArray[][15], int mazeTerrain[][15], int& row, int& col, int& numberOfExits)
{
	for (int r = 0; r < row; r++)
		for (int c = 0; c < col; c++)
		{
			if (mazeArray[r][c] == 'X')		// If character is 'X', it is a wall in the maze
				mazeTerrain[r][c] = Wall;	// Set terrain to wall

			// If any borders in the maze have a space as opposed to an X (wall), set it to an exit of the maze
			else if (mazeArray[r][c] == ' ' && (r == 0 || r == row - 1 || (c == 0 && r != 1) || c == col - 1))
			{
				mazeTerrain[r][c] = Exit;
				numberOfExits++;
			}
			else							// If it is not a wall or an exit, set to open
				mazeTerrain[r][c] = Open;
		}
	mazeTerrain[1][0] = Visited;			// Set row 1, col 0 (second row, first column) to visited because that will be the starting point for every maze
}

// Prints constants that represent the terrain of the maze (just for checking if getTerrain function worked properly)
void printTerrain(int mazeTerrain[][15], int& row, int& col)
{
	cout << row << "	" << col << endl;

	for (int r = 0; r < row; r++)
	{
		for (int c = 0; c < col; c++)
			cout << mazeTerrain[r][c];

		cout << endl;
	}
	cout << "\n";
}

void depthFirstSearch(int mazeTerrain[][15], int row, int col, int numRowsInMaze, int numColumnsInMaze, stack& rowStack, stack& colStack)
{
	if (mazeTerrain[row][col] == Exit)		// If current spot is an exit
	{
		mazeTerrain[row][col] = Visited;	// Mark current spot as visited
		if (!isFull(rowStack) && !isFull(colStack))
		{
			push(rowStack, row);			// Push row onto stack
			push(colStack, col);			// Push collumn onto stack
		}
		else
		{
			cout << "\nStack is full\n";
			return;
		}
	}
	else									// If an exit/path has not been found
	{
		mazeTerrain[row][col] = Visited;	// Mark current spot as visited

		if (mazeTerrain[row - 1][col] == Open || mazeTerrain[row - 1][col] == Exit)	// If up is open
		{
			if (row > 0)
			{
				if (!isFull(rowStack) && !isFull(colStack))
				{
					push(rowStack, row);	// Push row onto stack
					push(colStack, col);	// Push collumn onto stack
				}
				else
				{
					cout << "\nStack is full\n";
					return;
				}
				depthFirstSearch(mazeTerrain, row - 1, col, numRowsInMaze, numColumnsInMaze, rowStack, colStack);	// Go up
			}
		}

		else if (mazeTerrain[row][col + 1] == Open || mazeTerrain[row][col + 1] == Exit)	// If right is open
		{
			if (col < numColumnsInMaze - 1)
			{
				if (!isFull(rowStack) && !isFull(colStack))
				{
					push(rowStack, row);	// Push row onto stack
					push(colStack, col);	// Push collumn onto stack
				}
				else
				{
					cout << "\nStack is full\n";
					return;
				}
				depthFirstSearch(mazeTerrain, row, col + 1, numRowsInMaze, numColumnsInMaze, rowStack, colStack);	// Go right
			}
		}

		else if (mazeTerrain[row + 1][col] == Open || mazeTerrain[row + 1][col] == Exit)	// If down is open
		{
			if (row < numRowsInMaze - 1)
			{
				if (!isFull(rowStack) && !isFull(colStack))
				{
					push(rowStack, row);	// Push row onto stack
					push(colStack, col);	// Push collumn onto stack
				}
				else
				{
					cout << "\nStack is full\n";
					return;
				}
				depthFirstSearch(mazeTerrain, row + 1, col, numRowsInMaze, numColumnsInMaze, rowStack, colStack);	// Go down
			}
		}

		else if (mazeTerrain[row][col - 1] == Open || mazeTerrain[row][col - 1] == Exit)	// If left is open
		{
			if (col > 0)
			{
				if (!isFull(rowStack) && !isFull(colStack))
				{
					push(rowStack, row);	// Push row onto stack
					push(colStack, col);	// Push collumn onto stack
				}
				else
				{
					cout << "\nStack is full\n";
					return;
				}
				depthFirstSearch(mazeTerrain, row, col - 1, numRowsInMaze, numColumnsInMaze, rowStack, colStack);	// Go left
			}
		}
		else
			depthFirstSearch(mazeTerrain, pop(rowStack), pop(colStack), numRowsInMaze, numColumnsInMaze, rowStack, colStack);	//Back track
	}
}

// Clears a stack by setting top of stack to -1 (empty)
int clearStack(stack& stack)
{
	return stack.top = -1;		// Top of stack = -1 (empty)
}

// Checks if stack is empty
bool isEmpty(stack& stack)
{
	return stack.top == -1;		// True is top equals -1, otherwise returns false
}

// Checks if stack is full
bool isFull(stack& stack)
{
	return stack.top >= 15;		// True if top is greater or equal to 50 (size of stack), otherwise returns false
}

// Deletes and returns the top element in stack
int pop(stack& stack)
{
	int popped = stack.arrayStack[stack.top];	// Set popped equal to the value of the top of the stack
	stack.top--;								// Decrement top (logically removing the value at that index)
	return popped;								// Return the value of the popped index (top before it was popped)
}

// Adds new element to top of stack
void push(stack& stack, int num)
{
	stack.top++;						// Increment top index of the stack
	stack.arrayStack[stack.top] = num;	// Add num to top index of the stack
}

// Prints path through maze (ordered pairs) and distance
void printPath(char maze[][15], stack rowStack, stack colStack, int row, int col, int& pathDistance,
	string& shortestPath, int& shortestPathDistance, ofstream& fout)
{
	pathDistance = 0;		// Reset path distance

	string path;			// Holds path (ordered pairs)
	int pathRow, pathCol;	// Holds popped rows/columns from stacks
	stack rowPrintStack;
	stack colPrintStack;

	while (!isEmpty(rowStack) && !isEmpty(colStack))
	{
		push(rowPrintStack, pop(rowStack));		// Dump row/col stacks to new stacks so they are popped in order to print
		push(colPrintStack, pop(colStack));
	}

	while (!isEmpty(rowPrintStack) && !isEmpty(colPrintStack))
	{
		pathRow = pop(rowPrintStack);
		pathCol = pop(colPrintStack);

		maze[pathRow][pathCol] = '-';			// Change maze positions to a dash to represent the path

		if (pathRow != 1 || pathCol != 0)
		{
			path.append(", ");
			pathDistance++;
		}
		path.append("(" + to_string(pathRow + 1) + ", " + to_string(pathCol + 1) + ")");    // Append path (ordered pairs) to a string
	}
	fout << path << endl;	// Print path (ordered pairs)

	// Track shortest path and its distance
	if (pathDistance < shortestPathDistance)
	{
		shortestPathDistance = pathDistance;
		shortestPath = path;
	}

	fout << "Distance = " << pathDistance << "\n\n";

	// Print new maze to include the path
	for (int r = 0; r < row; r++)
	{
		for (int c = 0; c < col; c++)
			fout << maze[r][c];

		fout << "\n";
	}
}