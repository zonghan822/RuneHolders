#pragma once
#include "AEEngine.h"
#include <string>
#include <vector>
													//include utils to access
extern const f32 WINDOW_WIDTH, WINDOW_HEIGHT;			//	Window Width & Height
extern float dt;										//	delta time (between frames)
extern AEVec2 mPos;										//	mousePos (relative to screen at 0,0 being in the middle)

extern AEGfxVertexList* cRectMesh;
extern AEGfxVertexList* mRectMesh;
extern AEGfxVertexList* circleMesh;

void mouse_to_world();									//this function ensures that the mouse is always where the middle of the screen is

void create_all_meshes();
void destroy_all_meshes();

void draw_circle(f32 x, f32 y, f32 radius, f32 alpha);
void draw_rect(f32 x, f32 y, f32 length, f32 width, f32 alpha);


double rk4_step(double (*f)(double, double),
	double t, double y, double h);


class Node {
public:
	Node(int x, int y);

	int x, y;
	float gCost, hCost;
	Node* parent = nullptr;
	bool isWalkable;

	bool inOpenList = false;
	bool inClosedList = false;

	float fCost() const { return gCost + hCost; };

	static float radius;
};



#pragma region A* Pathfinding
//Implementation of A* Pathfinding
extern std::vector<std::vector<int>> BinaryMap;
void loadMap(const std::string& filename);
void printMap();


extern std::vector<std::vector<Node>> NodeGrid;
void generateNodeGrid();
void printNodeGrid();

//choose your start and ending later...
//extern Node* StartNode; 
//extern Node* EndNode;
std::vector<Node*> getNeighbours(Node* node); 
void printNeighbours(Node* node);

void runAStar();
void printPath();
void DrawPath();
#pragma endregion

#pragma region Physics
/*

collision checks / apply collision on objects?




void applyGravity(Entity* thisGuy);

void entityJump(Entity* thisGuy);			//maybe member function?

void applyAcceleration(Entity* thisGuy); 

void applyVelocity(Entity* thisGuy); 

*/



#pragma endregion