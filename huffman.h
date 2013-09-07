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
#ifndef YURIKS_HUFFMAN_H
#define YURIKS_HUFFMAN_H

#include "bitstream.h"

enum DictType
{
	DICT_NONE_EOF,
	DICT_VALUE,
	DICT_NODE
};

template <typename T>
struct Dictionary
{
	DictType type;

	Dictionary() : type(DICT_NONE_EOF) {}
	virtual ~Dictionary() {}

	virtual DictType getType() const { return DICT_NONE_EOF; }
};

template <typename T>
struct DictValue : Dictionary<T>
{
	DictValue(T val);

	virtual DictType getType() const { return DICT_VALUE; }

	T val;
};

template <typename T>
struct DictNode : Dictionary<T>
{
	DictNode();
	DictNode(Dictionary<T> *l, Dictionary<T> *r);

	virtual DictType getType() const { return DICT_NODE; }

	Dictionary<T> *l, *r;
};

template <typename Iter>
Dictionary<unsigned char>* build_huffman_tree(Iter& begin, const Iter& end);
template <typename Iter>
void huffman_compress(Dictionary<unsigned char>* tree, OFileBitstream& stream, Iter& begin, const Iter& end);
Dictionary<unsigned char>* readNode(IFileBitstream& stream);

void huffman_uncompress(IFileBitstream& stream, std::ostream& output);

///////////////////////////////////////////////////////////////////////////////

template <typename T>
inline DictValue<T>::DictValue(T val)
	: val(val)
{
}

template <typename T>
inline DictNode<T>::DictNode()
	: l(0), r(0)
{
}

template <typename T>
inline DictNode<T>::DictNode(Dictionary<T> *l, Dictionary<T> *r)
	: l(l), r(r)
{
}

#include "huffman.hpp"

#endif // YURIKS_HUFFMAN_H
