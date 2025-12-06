#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

#include <Windows.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::vector<std::string> Names(const char* str, const int count) {
	std::vector<std::string> vector;
	std::string name{ "data\\" + std::string{ str } };
	for (unsigned index = 0; index < count; ++index) {
		std::string number{ std::to_string(index) };
		if (index < 100)
			number = "0" + number;
		if (index < 10)
			number = "0" + number;

		vector.push_back(name + number + ".png");
	}
	return vector;
}
std::tuple<unsigned, unsigned, unsigned, unsigned, unsigned, unsigned> Size(
	const std::vector<std::string>& names) {
	unsigned up = -1;
	unsigned down = 0;
	unsigned left = -1;
	unsigned right = 0;
	int width, height, nrChannels;

	for (unsigned index = 0, size = names.size(); index < size; ++index) {
		unsigned char* data = stbi_load(names[index].data(), &width, &height, &nrChannels, 0);;
		std::cout << names[index] << "\n";

		for (unsigned y = 0; y < height; ++y) {
			for (unsigned x = 0; x < width; ++x) {
				unsigned out = (y * width + x) * nrChannels;
				if (0 != data[out + 3]) {
					up = y < up ? y : up;
					down = y > down ? y : down;
					left = x < left ? x : left;
					right = x > right ? x : right;
				}
			}
		}
		stbi_image_free(data);
	}
	++down;
	++right;
	width = right - left;
	height = down - up;
	return { up, down, left, right, width, height };
}
unsigned char* BMP(const int x, const int y) {
	unsigned size = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + y * x * sizeof(RGBQUAD);
	unsigned char* bmp = new unsigned char[size];

	BITMAPFILEHEADER& header = *static_cast<BITMAPFILEHEADER*>(static_cast<void*>(bmp));
	header = BITMAPFILEHEADER{
		0x4d42,
		size,
		NULL,
		NULL,
		sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) };

	BITMAPINFOHEADER& info = *static_cast<BITMAPINFOHEADER*>(static_cast<void*>(bmp + sizeof(BITMAPFILEHEADER)));
	info = BITMAPINFOHEADER{
		sizeof(BITMAPINFOHEADER),
		x,
		y,
		1,
		sizeof(RGBQUAD) << 3,
		BI_RGB,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	};
	return bmp;
}

int main(int argc, char** argv)
{
	stbi_set_flip_vertically_on_load(true);
	unsigned count = std::stoul(argv[2]);
	const std::vector<std::string> names = Names(argv[1], count);
	auto [up, down, left, right, width, height] = Size(names);
	std::cout
		<< " up = " << up
		<< " down = " << down
		<< " left = " << left
		<< " right = " << right
		<< " height = " << height
		<< " width = " << width << "\n";

	unsigned char* bmp = BMP(800,600/*width, height * count*/);
	unsigned char* image = bmp + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	std::cout
		<< reinterpret_cast<uint64_t>(bmp) << "\n"
		<< reinterpret_cast<uint64_t>(image) << "\n";

	for (unsigned index = 0; index < count; ++index) {
		int image_X, image_Y, channels;
		unsigned char* data = stbi_load(names[index].data(), &image_X, &image_Y, &channels, STBI_rgb_alpha);
		std::cout
			<< names[index] << "\n"
			<< " height = " << image_Y
			<< " width = " << image_X
			<< " biBitCount = " << channels << "\n";

		for (int Y = 0/*up*/, in = 0; Y < 600/*down*/; ++Y) {
			for (int X = 0/*left*/; X < 800/*right*/; ++X, in += sizeof(RGBQUAD)) {
				int out = (Y * image_X + X) * channels;
				if (0 == data[out + 3]) {
					image[in + 0] = 0;
					image[in + 1] = 0;
					image[in + 2] = 0;
					image[in + 3] = 0;
				}
				else {
					image[in + 0] = data[out + 0];
					image[in + 1] = data[out + 1];
					image[in + 2] = data[out + 2];
					image[in + 3] = data[out + 3];
				}
			}
		}
		stbi_image_free(data);
	}
	std::ofstream file{ "bmp.bmp" };
	file.write(
		static_cast<char*>(static_cast<void*>(bmp)),
		static_cast<BITMAPFILEHEADER*>(static_cast<void*>(bmp))->bfSize
	);
	file.close();
}

