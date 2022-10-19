#include <raylib.h>
#include <raymath.h>

#define RLIGHTS_IMPLEMENTATION
#include <rlights.h>

#include <chrono>
#include <string>
#include <iostream>
#include <stdlib.h>

#define MAX_INSTANCES 300000
#define GET_TIME std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()

int main()
{
	// Window Initialization
	InitWindow(1280, 720, "Instanced vs Non-Instanced Rendering");

	// Generate basic 1x1x1 mesh
	Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);

	// Define transforms to be uploaded to GPU for instances
	Matrix* transforms = (Matrix*)RL_CALLOC(MAX_INSTANCES, sizeof(Matrix));

	// Translate and rotate cubes randomly
	for (int i = 0; i < MAX_INSTANCES; i++)
	{
		Matrix translation = MatrixTranslate((float)GetRandomValue(-100, 100), (float)GetRandomValue(-100, 100), (float)GetRandomValue(-100, 100));
		Vector3 axis = Vector3Normalize({ (float)GetRandomValue(0, 360), (float)GetRandomValue(0, 360), (float)GetRandomValue(0, 360) });
		float angle = (float)GetRandomValue(0, 10) * DEG2RAD;
		Matrix rotation = MatrixRotate(axis, angle);

		transforms[i] = MatrixMultiply(rotation, translation);
	}

	// Load lighting shader
	Shader instancingShader = LoadShader(TextFormat("shaders/lighting_instancing.vs", 330), TextFormat("shaders/lighting.fs", 330));
	Shader shader = LoadShader(TextFormat("shaders/lighting.vs", 330), TextFormat("shaders/lighting.fs", 330));

	// Get shader locations
	instancingShader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(instancingShader, "mvp");
	instancingShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(instancingShader, "viewPos");
	instancingShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(instancingShader, "instanceTransform");
	shader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(shader, "mvp");
	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
	shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(shader, "matModel");

	// Set shader value: ambient light level
	Vector4 ambience = { 0.2f, 0.2f, 0.2f, 1.0f };
	SetShaderValue(instancingShader, GetShaderLocation(instancingShader, "ambient"), &ambience, SHADER_UNIFORM_VEC4);
	SetShaderValue(shader, GetShaderLocation(shader, "ambient"), &ambience, SHADER_UNIFORM_VEC4);

	// Create one light
	CreateLight(LIGHT_DIRECTIONAL, { 100.0f, 100.0f, 0.0f }, Vector3Zero(), WHITE, instancingShader);
	CreateLight(LIGHT_DIRECTIONAL, { 100.0f, 100.0f, 0.0f }, Vector3Zero(), WHITE, shader);

	// Create Cube Material
	Material matInstances = LoadMaterialDefault();
	matInstances.shader = instancingShader;
	matInstances.maps[MATERIAL_MAP_DIFFUSE].color = RED;

	Material mat = LoadMaterialDefault();
	mat.shader = shader;
	mat.maps[MATERIAL_MAP_DIFFUSE].color = BLUE;


	// Create Orbital Camera
	Camera3D camera = { 0 };
	camera.fovy = 90;
	camera.position = { 0, 0, -200 };
	camera.target = { 0, 0, 0 };
	camera.up = { 0, 1, 0 };
	camera.projection = CAMERA_PERSPECTIVE;

	SetCameraMode(camera, CAMERA_ORBITAL);

	// Setup Timer
	long lastUpdate = GET_TIME;
	long frames = 0;
	double time = 0;
	double milliseconds = 0;
	int instances = 30000;
	bool instancing = false;

	std::string avgMilliseconds = "Average Frame Time: " + std::to_string(0) + " ms";
	std::string currentInstances = "Instances: " + std::to_string(instances);
	std::string maxInstances = "Max Instances: " + std::to_string(MAX_INSTANCES);
	std::string isInstancing = (instancing) ? "true" : "false";
	std::string instancingText = "Using Instancing: " + isInstancing;

	// Loop every frame
	while (!WindowShouldClose())
	{
		long timeDiff = (GET_TIME - lastUpdate);
		double delta = timeDiff / 1000000000.0;

		lastUpdate += timeDiff;
		time += delta;
		frames++;

		milliseconds += delta * 1000;

		UpdateCamera(&camera);

		// Set camera positions in shader
		float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
		SetShaderValue(instancingShader, instancingShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
		SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

		// Update Key
		if (IsKeyPressed(KEY_UP) && instances < MAX_INSTANCES)
		{
			instances += 1000;
			currentInstances = "Instances: " + std::to_string(instances);
			milliseconds = 0;
			frames = 0;
			time = 0;
			avgMilliseconds = "Average Frame Time: " + std::to_string(milliseconds / frames) + " ms";
		}

		if (IsKeyPressed(KEY_DOWN) && instances >= 2000)
		{
			instances -= 1000;
			currentInstances = "Instances: " + std::to_string(instances);
			milliseconds = 0;
			frames = 0;
			time = 0;
			avgMilliseconds = "Average Frame Time: " + std::to_string(milliseconds / frames) + " ms";
		}

		if (IsKeyPressed(KEY_SPACE))
		{
			instancing = !instancing;
			milliseconds = 0;
			frames = 0;
			time = 0;
			avgMilliseconds = "Average Frame Time: " + std::to_string(milliseconds / frames) + " ms";
			isInstancing = (instancing) ? "true" : "false";
			instancingText = "Using Instancing: " + isInstancing;
		}

		// Update average frame time
		if (time > 1)
		{
			std::cout << instances << " " << std::to_string(milliseconds / frames) << std::endl;
			avgMilliseconds = "Average Frame Time: " + std::to_string(milliseconds / frames) + " ms";
			time = 0;
		}

		// Drawing
		BeginDrawing();
		{
			ClearBackground(GRAY);

			BeginMode3D(camera);
			{
				if (instancing)
					DrawMeshInstanced(cube, matInstances, transforms, instances);
				else
				{
					for (int i = 0; i < instances; i++)
						DrawMesh(cube, mat, transforms[i]);
				}
				
			}
			EndMode3D();

			DrawText(avgMilliseconds.c_str(), 10, 10, 16, GREEN);
			DrawText(currentInstances.c_str(), 10, 30, 16, GREEN);
			DrawText(maxInstances.c_str(), 10, 50, 16, GREEN);
			DrawText(instancingText.c_str(), 10, 70, 16, GREEN);
		}
		EndDrawing();
	}

	RL_FREE(transforms);

	CloseWindow();

	return 0;
}