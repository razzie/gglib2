/**
 * Copyright (c) 2014-2015 G�bor G�rzs�ny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#pragma once

namespace gg
{
	inline unsigned char encode(unsigned char c)
	{
		static const unsigned char shuffle[256] =
		{
			125, 251, 120, 246, 115, 241, 110, 236, 105, 231, 100, 226, 95, 221, 90, 216,
			85, 211, 80, 206, 75, 201, 70, 196, 65, 191, 60, 186, 55, 181, 50, 176,
			45, 171, 40, 166, 35, 161, 30, 156, 25, 151, 20, 146, 15, 141, 10, 136,
			5, 131, 0, 126, 252, 121, 247, 116, 242, 111, 237, 106, 232, 101, 227, 96,
			222, 91, 217, 86, 212, 81, 207, 76, 202, 71, 197, 66, 192, 61, 187, 56,
			182, 51, 177, 46, 172, 41, 167, 36, 162, 31, 157, 26, 152, 21, 147, 16,
			142, 11, 137, 6, 132, 1, 127, 253, 122, 248, 117, 243, 112, 238, 107, 233,
			102, 228, 97, 223, 92, 218, 87, 213, 82, 208, 77, 203, 72, 198, 67, 193,
			62, 188, 57, 183, 52, 178, 47, 173, 42, 168, 37, 163, 32, 158, 27, 153,
			22, 148, 17, 143, 12, 138, 7, 133, 2, 128, 254, 123, 249, 118, 244, 113,
			239, 108, 234, 103, 229, 98, 224, 93, 219, 88, 214, 83, 209, 78, 204, 73,
			199, 68, 194, 63, 189, 58, 184, 53, 179, 48, 174, 43, 169, 38, 164, 33,
			159, 28, 154, 23, 149, 18, 144, 13, 139, 8, 134, 3, 129, 255, 124, 250,
			119, 245, 114, 240, 109, 235, 104, 230, 99, 225, 94, 220, 89, 215, 84, 210,
			79, 205, 74, 200, 69, 195, 64, 190, 59, 185, 54, 180, 49, 175, 44, 170,
			39, 165, 34, 160, 29, 155, 24, 150, 19, 145, 14, 140, 9, 135, 4, 130,
		};

		return shuffle[c];
	}

	inline unsigned char decode(unsigned char c)
	{
		static const unsigned char deshuffle[256] =
		{
			50, 101, 152, 203, 254, 48, 99, 150, 201, 252, 46, 97, 148, 199, 250, 44,
			95, 146, 197, 248, 42, 93, 144, 195, 246, 40, 91, 142, 193, 244, 38, 89,
			140, 191, 242, 36, 87, 138, 189, 240, 34, 85, 136, 187, 238, 32, 83, 134,
			185, 236, 30, 81, 132, 183, 234, 28, 79, 130, 181, 232, 26, 77, 128, 179,
			230, 24, 75, 126, 177, 228, 22, 73, 124, 175, 226, 20, 71, 122, 173, 224,
			18, 69, 120, 171, 222, 16, 67, 118, 169, 220, 14, 65, 116, 167, 218, 12,
			63, 114, 165, 216, 10, 61, 112, 163, 214, 8, 59, 110, 161, 212, 6, 57,
			108, 159, 210, 4, 55, 106, 157, 208, 2, 53, 104, 155, 206, 0, 51, 102,
			153, 204, 255, 49, 100, 151, 202, 253, 47, 98, 149, 200, 251, 45, 96, 147,
			198, 249, 43, 94, 145, 196, 247, 41, 92, 143, 194, 245, 39, 90, 141, 192,
			243, 37, 88, 139, 190, 241, 35, 86, 137, 188, 239, 33, 84, 135, 186, 237,
			31, 82, 133, 184, 235, 29, 80, 131, 182, 233, 27, 78, 129, 180, 231, 25,
			76, 127, 178, 229, 23, 74, 125, 176, 227, 21, 72, 123, 174, 225, 19, 70,
			121, 172, 223, 17, 68, 119, 170, 221, 15, 66, 117, 168, 219, 13, 64, 115,
			166, 217, 11, 62, 113, 164, 215, 9, 60, 111, 162, 213, 7, 58, 109, 160,
			211, 5, 56, 107, 158, 209, 3, 54, 105, 156, 207, 1, 52, 103, 154, 205,
		};

		return deshuffle[c];
	}

	inline void encode(unsigned char* buf, size_t len)
	{
		for (size_t i = 0; i < len; ++i)
			buf[i] = encode(buf[i]);
	}

	inline void decode(unsigned char* buf, size_t len)
	{
		for (size_t i = 0; i < len; ++i)
			buf[i] = decode(buf[i]);
	}
};

/*static void gen_shuffle_array()
{
	srand(time(NULL));

	int a = (rand() % 227) + 1;
	int b = (rand() % 223) + 1;
	int c = (rand() % 211) + 1;

	int skip = (a * 256 * 256) + (b * 256) + c;
	skip &= ~0xc0000000;

	std::cout << "{" << std::endl;

	int pos = 0;
	for (int i = 0; i < 256; ++i)
	{
		pos += skip;
		pos %= 257; // first prime greater than 256
		std::cout << pos - 1 << ", ";

		if (i % 16 == 15)
			std::cout << std::endl;
	}

	std::cout << "}" << std::endl;
}*/

/*static void gen_deshuffle_array(const unsigned char* shuffle)
{
	std::cout << "{" << std::endl;

	int pos;
	for (int i = 0; i < 256; ++i)
	{
		// find position of i in original array
		for (pos = 0; shuffle[pos] != i; ++pos);

		std::cout << pos << ", ";

		if (i % 16 == 15)
			std::cout << std::endl;
	}

	std::cout << "}" << std::endl;
}*/
