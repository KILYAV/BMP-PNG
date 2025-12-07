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
	down;
	right;
	width = right - left + 1;
	height = down - up + 1;
	return { up, down, left, right, width, height };
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

	BITMAPFILEHEADER header{
		0x4d42,
		sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + count * height * width * sizeof(RGBQUAD),
		NULL,
		NULL,
		sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) };
	BITMAPINFOHEADER info{
		sizeof(BITMAPINFOHEADER),
		width,
		height * count,
		1,
		sizeof(RGBQUAD) << 3,
		BI_RGB,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	};
	
	std::vector<unsigned char> image;
	image.reserve(count * height * width * sizeof(RGBQUAD));

	for (unsigned index = 0; index < count; ++index) {
		int image_X, image_Y, channels;
		unsigned char* data = stbi_load(names[index].data(), &image_X, &image_Y, &channels, 0);

		for (int Y = up; Y <= down; ++Y) {
			for (int X = left; X <= right; ++X) {
				int out = (Y * image_X + X) * channels;
				image.push_back(data[out + 2]);
				image.push_back(data[out + 1]);
				image.push_back(data[out + 0]);
				image.push_back(data[out + 3]);
			}
		}
		stbi_image_free(data);
	}

	std::ofstream file{ "bmp.bmp", std::ios::binary };
	file.write(static_cast<char*>(static_cast<void*>(&header)), sizeof(header));
	file.write(static_cast<char*>(static_cast<void*>(&info)), sizeof(info));
	file.write(static_cast<char*>(static_cast<void*>(image.data())), image.size());
	file.close();
}

