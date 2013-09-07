/*
 * The MIT License
 *
 * Copyright (c) 2010 Yuri K. Schlesner
 *               2010 Hugo S. K. Puhlmann
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

#if defined(_WIN32) // && defined(_DEBUG)
#	define _CRTDBG_MAP_ALLOC
#	include <stdlib.h>
#	include <crtdbg.h>
#endif

#include "huffman.h"
#include "bitstream.h"
#include "dump_tree.h"

#include <iostream>
#include <iterator>
#include <string>

int main(int argc, char *argv[])
{
#if defined(_WIN32) // && defined(_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif


	if (argc != 4)
	{
		std::cerr << "Invalid number of arguments." << std::endl;
		std::cerr << "Usage: Huffman.exe -c/-u <infile> <outfile>" << std::endl;

		return 1;
	}

	std::ifstream in_file(argv[2], std::ios::in | std::ios::binary);
	if (!in_file)
	{
		std::cerr << "Couldn't open " << argv[2] << std::endl;
		return 2;
	}

	std::ofstream out_file(argv[3], std::ios::out | std::ios::binary | std::ios::trunc);
	if (!out_file)
	{
		std::cerr << "Couldn't open " << argv[3] << std::endl;
		return 3;
	}

	std::string arg(argv[1]);

	if (arg == "-c")
	{
		OFileBitstream stream(out_file);
		std::istreambuf_iterator<char> in_iter(in_file);

		std::cerr << "Construindo arvore..." << std::endl;
		Dictionary<unsigned char>* tree = build_huffman_tree(in_iter, std::istreambuf_iterator<char>());
		std::ifstream::pos_type size = in_file.tellg();
		in_file.seekg(0);
		std::cerr << "Comprimindo arquivo..." << std::endl;
		huffman_compress(tree, stream, in_iter, std::istreambuf_iterator<char>(), size);
		delete tree;
	} 
	else if (arg == "-u")
	{
		in_file.seekg(0, std::ios::end);
		unsigned long long size = in_file.tellg();
		in_file.seekg(0);

		IFileBitstream istream(in_file);
		std::cerr << "Lendo arvore..." << std::endl;
		Dictionary<unsigned char>* tree = readNode(istream);
		size -= in_file.tellg();
		std::cerr << "Descomprimindo arquivo..." << std::endl;
		huffman_uncompress(istream, out_file, tree, size);
		delete tree;
	}
	else if (arg == "--make-tree")
	{
		std::istreambuf_iterator<char> in_iter(in_file);

		Dictionary<unsigned char>* tree = build_huffman_tree(in_iter, std::istreambuf_iterator<char>());
		print_huffman_tree(tree, out_file);
		delete tree;
	}
	else if (arg == "--read-tree")
	{
		IFileBitstream istream(in_file);
		Dictionary<unsigned char>* tree = readNode(istream);
		print_huffman_tree(tree, out_file);
		delete tree;
	}

	out_file.close();
	in_file.close();


	return 0;
}
