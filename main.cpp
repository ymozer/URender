#include <vector>
#include <cmath>
#include <queue>
#include <iostream>
#include <cstdlib> // for rand()
#include <ctime> // for time()
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model* model = NULL;
const int width = 800;
const int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color);
void drawTriangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color);


int main(int argc, char** argv)
{

	std::srand(std::time(nullptr));

	if (2 == argc)
		model = new Model(argv[1]);
	else 
		model = new Model("media/african_head.obj");

	TGAImage image(width, height, TGAImage::RGB);
	Vec3f light_dir(1, 0.5f, -1); // define light_dir

	for (int i = 0; i < model->nfaces(); i++) 
	{
		std::vector<int> face = model->face(i);
		std::vector<Vec2i> lines;
		std::vector<Vec3f> world_coords;

		for (int j = 0; j < 3; j++) 
		{
			Vec3f v = model->vert(face[j]);
			lines.push_back(Vec2i((v.x + 1.0f) * width / 2., (v.y + 1.0f) * height / 2.0f));
			world_coords.push_back(v);
		}
		Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
		n.normalize();
		float intensity = n * light_dir;
		if (intensity>0 )
			drawTriangle(lines[0], lines[1], lines[2], image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
	}

	Vec2i t0{ 10,10 };
	Vec2i t1{ 11,40 };
	Vec2i t2{ 60,10 };
	drawTriangle(t0,t1, t2, image, red);
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("media/output4.tga");
	delete model;
	return 0;
}

void drawTriangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color) 
{
	if (t0.y == t1.y && t1.y == t2.y) return;
	// sort vertices by y-coordinate
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t1.y > t2.y) std::swap(t1, t2);
	if (t0.y > t1.y) std::swap(t0, t1);

	// draw bottom to top
	// todo(Metin): Use barycentric coordinates and bounding box in the future 
	const int total_height = t2.y - t0.y;
	for (int i = 0; i < total_height; i++) 
	{
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;
		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
		if (A.x > B.x) std::swap(A, B);
		for (int j = A.x; j <= B.x; j++) 
		{
			image.set(j, t0.y + i, color);
		}
	}
}


// error * dx * 2 assumed error 
void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) 
{
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) 
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}

	if (x0 > x1) 
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	int dx = x1 - x0;
	int dy = y1 - y0;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = y0;

	for (int x = x0; x <= x1; x++) 
	{
		if (steep) image.set(y, x, color);
		else image.set(x, y, color);

		error2 += derror2;
		if (error2 > dx) 
		{
			y += (y1 > y0 ? 1 : -1);
			error2 -= dx * 2;
		}
	}
}