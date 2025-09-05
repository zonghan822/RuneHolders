#include "Utils.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>


f32 const WINDOW_WIDTH{ 1600 }, WINDOW_HEIGHT{ 900 };

AEGfxVertexList* cRectMesh = nullptr;												//Rectangle Mesh that starts from bottom left corner
AEGfxVertexList* mRectMesh = nullptr;												//Rectangle Mesh that starts from the middle
AEGfxVertexList* circleMesh = nullptr;												//Circle Mesh

float dt;																			//define dt
s32 mX{}, mY{};
AEVec2 mPos{};																		//holds mouse position in a x,y vector
float Node::radius = 0.0f;


void mouse_to_world() {								
	AEInputGetCursorPosition(&mX, &mY);
	mPos = { static_cast<f32>(mX), static_cast<f32>(mY) };
	mPos.x -= 0.5 * WINDOW_WIDTH;
	mPos.y -= 0.5 * WINDOW_HEIGHT;
}

static void create_cRect_mesh() {
	AEGfxMeshStart();

	AEGfxTriAdd(
		0.0f, 0.0f, 0x00000000, 0.0f, 1.0f,
		1.0f, 0.0f, 0x00000000, 1.0f, 1.0f,
		1.0f, 1.0f, 0x00000000, 1.0f, 0.0f 	
	);

	AEGfxTriAdd(
		0.0f, 0.0f, 0x00000000, 0.0f, 1.0f,
		1.0f, 1.0f, 0x00000000, 1.0f, 0.0f, 
		0.0f, 1.0f, 0x00000000, 0.0f, 0.0f
	);

	cRectMesh = AEGfxMeshEnd();
}

static void create_mRect_mesh() {
	AEGfxMeshStart();

	AEGfxTriAdd(
		-0.5f, -0.5f, 0x00000000, 0.0f, 1.0f,
		 0.5f, -0.5f, 0x00000000, 1.0f, 1.0f,
		 0.5f,  0.5f, 0x00000000, 1.0f, 0.0f 	
	);

	AEGfxTriAdd(
		-0.5f, -0.5f, 0x00000000, 0.0f, 1.0f,
		 0.5f,  0.5f, 0x00000000, 1.0f, 0.0f, 
		-0.5f,  0.5f, 0x00000000, 0.0f, 0.0f
	);

	mRectMesh = AEGfxMeshEnd();
}
																					//***********************TO NOTE***********************
static void create_circle_mesh() {													//UV might not work right now, to be looked into again.
	AEGfxMeshStart();																//***********************TO NOTE***********************4
		
	float sides = 30;
	float theta = (2.f * PI) / sides;

	for (int i = 0; i < static_cast<int>(sides); i++) {
		AEGfxTriAdd(
			0.0, 0.0f, 0x00000000, 0.0f, 1.0f,	
			AECos(theta * i), AESin(theta * i), 0x00000000, 1.0f, 1.0f,
			AECos(theta * (i + 1)), AESin(theta * (i + 1)), 0x00000000, 1.0f, 0.0f
		);
	}
	circleMesh = AEGfxMeshEnd();
}





void create_all_meshes() {															//call this function once to generate all the meshes needed in 1 file.
	create_mRect_mesh();
	create_cRect_mesh();
	create_circle_mesh();
}

void destroy_all_meshes() {
	AEGfxMeshFree(cRectMesh);
	AEGfxMeshFree(mRectMesh);
	AEGfxMeshFree(circleMesh);
	cRectMesh = nullptr;
	mRectMesh = nullptr;
	circleMesh = nullptr;

}



//Position and Radius. Colour auto green rn
void draw_circle(f32 x, f32 y, f32 radius, f32 alpha) {	//perhaps i could add colour in here
	AEMtx33 scale = { 0 };
	AEMtx33Scale(&scale, radius, radius);

	AEMtx33 translate = { 0 };
	AEMtx33Trans(&translate, x, y);

	AEMtx33 transform = { 0 };
	AEMtx33Concat(&transform, &translate, &scale);

	AEGfxSetTransform(transform.m);

	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	AEGfxSetTransparency(alpha);
	AEGfxMeshDraw(circleMesh, AE_GFX_MDM_TRIANGLES);
}


void draw_rect(f32 x, f32 y, f32 length, f32 width, f32 alpha) {
	AEMtx33 scale = { 0 };
	AEMtx33Scale(&scale, width, length);

	AEMtx33 translate = { 0 };
	AEMtx33Trans(&translate, x, y);

	AEMtx33 transform = { 0 };
	AEMtx33Concat(&transform, &translate, &scale);

	AEGfxSetTransform(transform.m);

	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	AEGfxSetTransparency(alpha);
	AEGfxMeshDraw(cRectMesh, AE_GFX_MDM_TRIANGLES);


}


/*
in order to use rk4, call it using the function pointer.

double function(double t, double y) {
	return y; // example: dy/dt = y
}

int main() {
	double y1 = rk4_step(function, 0.0, 1.0, 0.1);
	std::cout << y1 << "\n";
}
*/

double rk4_step(double (*f)(double, double), double t, double y, double h)
{
	double k1 = f(t, y);
	double k2 = f(t + h * 0.5, y + h * 0.5 * k1);
	double k3 = f(t + h * 0.5, y + h * 0.5 * k2);
	double k4 = f(t + h, y + h * k3);
	return y + (h / 6.0) * (k1 + 2 * k2 + 2 * k3 + k4);
}


#pragma region A*PathFinding
//A star Pathfinding
std::vector<std::vector<int>> BinaryMap;

void loadMap(const std::string& filename) {
	BinaryMap.clear();
	std::ifstream file{ filename };
	std::string line;

	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << filename << "\n";
		return;
	}

	while (std::getline(file, line)) {
		std::stringstream ss(line);
		std::vector<int> row{};
		int val{};
		//assign ss next word into val. ss will auto concatenate itself, while val gets updated to the next number
		while (ss >> val) {
			row.push_back(val);
		}
		BinaryMap.push_back(row);
	}
}

void printMap() {

	for (const std::vector<int>& dog : BinaryMap) {
		for (const int& papa : dog) {
			std::cout << papa << " ";
		}
		std::cout << "\n";
	}
}


//Nodes
std::vector<std::vector<Node>> NodeGrid;
//Node ctor
Node::Node(int x, int y):
x{ x }, 
y{ y }, 
isWalkable{(BinaryMap[y][x] == 0)}, 
gCost{ INFINITY },
hCost{ INFINITY },
parent{ nullptr },
inOpenList{ false },
inClosedList{ false }
{}


void generateNodeGrid() {
	NodeGrid.clear();

	for (int y = 0; y < BinaryMap.size(); ++y) {
		std::vector<Node> row;

		for (int x = 0; x < BinaryMap[y].size(); ++x) {
			row.push_back(Node(x,y));
		}
		NodeGrid.push_back(row);
	}
}

void printNodeGrid() {
	for (const std::vector<Node>& dog : NodeGrid) {
		for (const Node& papa : dog) {
			std::cout << papa.y << papa.x << " ";
		}
		std::cout << "\n";
	}
}


//using euclidean distance (to allow for diagonal movement) -> simply a way to "getHcost()"
float heuristic(Node* a, Node* b) {
	float distX = abs(static_cast<float>(a->x - b->x));
	float distY = abs(static_cast<float>(a->y - b->y));
	return std::sqrt(distX * distX + distY * distY);
}


//movement cost to the next node
float movement_cost(Node* from, Node* to) {		
	float distX = abs(from->x - to->x);
	float distY = abs(from->y - to->y);
	return (distX == 1 && distY == 1) ? 1.4142f : 1.0f; //1.4142 is sqrt(1+1) where 1 is dist of x and the other 1 is dist of y
}



std::vector<Node*> getNeighbours(Node* node) {
	std::vector<Node*> neighbourList;

	for (int distY = -1; distY <= 1; ++distY) {
		for (int distX = -1; distX <= 1; distX++) {
			if (distX == 0 && distY==0) {
				continue;
			}	
			int NodeX = node->x + distX;
			int NodeY = node->y + distY;

			//checking for boundaries
			if (NodeY < 0 || NodeY >= NodeGrid.size() || NodeX < 0 || NodeX >= NodeGrid[0].size()) continue;
		
			Node* neighbour = &NodeGrid[NodeY][NodeX];

			
			if (neighbour->isWalkable) {
				neighbourList.push_back(neighbour);
			}
		}
	
	}
	return neighbourList;
}


void printNeighbours(Node* node) {
	
	std::vector<Node*> neighbourList = getNeighbours(node);
	while(!neighbourList.empty()) {
		std::cout << neighbourList.front()->x << neighbourList.front()->y << " ";
		neighbourList.erase(neighbourList.begin());
	}

}




Node* StartNode = nullptr;
Node* EndNode = nullptr;


bool comparefCost(Node* a, Node* b) {
	return a->fCost() < b->fCost();
}


//Define your startpoint later. For now this is hardcoded.
void runAStar() {
	StartNode = &NodeGrid[0][0];	//start at top left
	EndNode = &NodeGrid[NodeGrid.size()-1][NodeGrid[0].size()-1];

	StartNode->gCost = 0;
	StartNode->hCost = heuristic(StartNode, EndNode);

	std::vector<Node*> openList;
	//i will now be exploring the StartNode
	openList.push_back(StartNode);
	StartNode->inOpenList = true;

	while (!openList.empty()) {
		std::sort(openList.begin(), openList.end(), comparefCost);			//using fCost as a function pointer, im comparing each of the Nodes in the OpenList's fcost. 
		//After sorting all the openList, I want to find the cheapest fCost, which is now at the front of the open list.
		Node* current = openList.front();

		if (current == EndNode) {
			std::cout << "Path found!\n";
			return;
		}

		//now that we have found the cheapest way here and its not the end node, we move on to the next node
		//move this node to the closed list so that we do not revisit this again
		openList.erase(openList.begin());
		current->inOpenList = false;
		current->inClosedList = true;

		
		for (Node* neighbour: getNeighbours(current)) {
			if (neighbour->inClosedList) {
				continue;
			}
				
			float tentativeGcost = current->gCost + movement_cost(current,neighbour);
			//The tentative g - cost lets you compare two scenarios for reaching the same neighbor without committing:
			//1. The cost you already know from a previous path.
			//2. This new possible cost via the current node.

			if (!neighbour->inOpenList || tentativeGcost < neighbour->gCost) {
				neighbour->gCost = tentativeGcost; //we will update the gcost if we find a cheaper way to reach this same node
				neighbour->hCost = heuristic(neighbour, EndNode);
				neighbour->parent = current;

				//if we discover a new node, we'll add it to the open list
				if (!neighbour->inOpenList) {
					openList.push_back(neighbour);
					neighbour->inOpenList = true;
				}
			
			}

		}
	}

	std::cout << "no path found\n";
}



void printPath() {
	Node* current = EndNode;
	while (current != nullptr) {
		std::cout << current->y << current->x << " "; //printing it in the opposite direction
		current = current->parent;
	}
}

//up to here, we are able to read from a file, evaluate and assign values to the map by creating nodes, and use A* to generate a enemy path.



/*
THE MAP IS WRITTEN AS 
00 01 02 03
10 11 12 13
20 21 22 23
30 31 32 33

BUT THE WAY THE MAP IS DRAWN IS 

30 31 32 33
20 21 22 23
10 11 12 13
00 01 02 03

Y INVERT ALLOWS THEM TO FLIP TO THE OTHER
*/
int yInvert(int y) {
	int toflip = NodeGrid.size() - 1;
	return toflip - y;
}

void DrawPath() {
	//i want a 600 by 600 window, where i am able to store 8x8 circles
	const float pathW = 600.f;
	const float pathH = 600.f;
	Node::radius = (pathW / NodeGrid.size()) / 2;	//this returns radius downwards(y radius). if wanna do square need to do Nodegrid[0].size() for x radius

	AEGfxSetColorToAdd(0, 0, 1, 0);
	draw_rect(-300, -300, pathW, pathH, 0.2);


	for (int y = 0; y < NodeGrid.size(); y++) {
		for (int x = 0; x < NodeGrid[y].size(); x++) {			

			if (NodeGrid[y][x].isWalkable) {
			AEGfxSetColorToAdd(1, 0, 1, 0);
			}
			else {
			AEGfxSetColorToAdd(0, 0, 0, 0);
			}
			draw_circle((- 300 + Node::radius) + x* (2* Node::radius), (-300 + Node::radius) + yInvert(y) * (2 * Node::radius), Node::radius, 1);
		
		}
	}

	Node* current = EndNode;
	
	while (current != nullptr) {
		AEGfxSetColorToAdd(1, 1, 1, 0);
		draw_circle((-300 + Node::radius) + current->x * (2 * Node::radius), (-300 + Node::radius) + yInvert(current->y) * (2 * Node::radius), Node::radius, 1);
		current = current->parent;
	}
}




#pragma endregion