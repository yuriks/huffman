/*
 * The MIT License
 *
 * Copyright (c) 2010 Yuri K. Schlesner
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
#ifndef YURIKS_DUMP_TREE_H
#define YURIKS_DUMP_TREE_H

#include "bitstream.h"

#include <iomanip>
#include <cctype>

template <typename T>
void print_huffman_tree(const Dictionary<T>* tree, std::ostream& s, int depth = 0)
{
	for (int i = 0; i < depth; ++i)
		s << '\t';

	if (const DictValue<unsigned char> *dict_val = dynamic_cast<const DictValue<unsigned char>*>(tree))
	{
		s << "| 0x" << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << (unsigned int)dict_val->val;
		if (std::isprint(dict_val->val))
			s << " (" << dict_val->val << ")";
		s << '\n';
	}
	else if (const DictNode<unsigned char> *dict_node = dynamic_cast<const DictNode<unsigned char>*>(tree))
	{
		s << "+ L\n";
		print_huffman_tree(dict_node->l, s, depth+1);

		for (int i = 0; i < depth; ++i)
			s << '\t';
		s << "+ R\n";
		print_huffman_tree(dict_node->r, s, depth+1);
	}
	else // EOF
	{
		s << "| EOF\n";
	}
}

#endif // YURIKS_DUMP_TREE_H
