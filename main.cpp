/*
 * The MIT License
 *
 * Copyright (c) 2010 yuriks.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "huffman.h"
#include "bitstream.h"

#include <iostream>
#include <iterator>

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cerr << "Invalid number of arguments." << std::endl;
		return 1;
	}

	std::ifstream in_file(argv[1], std::ios::in | std::ios::binary);
	if (!in_file)
	{
		std::cerr << "Couldn't open " << argv[1] << std::endl;
		return 2;
	}

	std::ofstream out_file(argv[2], std::ios::out | std::ios::binary | std::ios::trunc);
	if (!out_file)
	{
		std::cerr << "Couldn't open " << argv[2] << std::endl;
		return 3;
	}

	{
		OFileBitstream stream(out_file);
		std::istreambuf_iterator<char> in_iter(in_file);

		Dictionary<unsigned char>* tree = build_huffman_tree(in_iter, std::istreambuf_iterator<char>());
		in_file.seekg(0);
		huffman_compress(tree, stream, in_iter, std::istreambuf_iterator<char>());
		delete tree;
	}

	out_file.close();
	in_file.close();

	return 0;
}
