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
#ifndef YURIKS_HUFFMAN_H
#define YURIKS_HUFFMAN_H

#include "bitstream.h"

template <typename T>
struct Dictionary
{
	virtual ~Dictionary() {};
};

template <typename T>
struct DictValue : Dictionary<T>
{
	DictValue(T val);
	~DictValue();

	T val;
};

template <typename T>
struct DictNode : Dictionary<T>
{
	DictNode();
	DictNode(Dictionary<T> *l, Dictionary<T> *r);
	~DictNode();

	Dictionary<T> *l, *r;
};

template <typename Iter>
Dictionary<unsigned char>* build_huffman_tree(Iter& begin, const Iter& end);
template <typename Iter>
void huffman_compress(Dictionary<unsigned char>* tree, OFileBitstream& stream, Iter& begin, const Iter& end);

///////////////////////////////////////////////////////////////////////////////

template <typename T>
inline DictValue<T>::DictValue(T val)
	: val(val)
{
}

template <typename T>
DictValue<T>::~DictValue()
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

template <typename T>
DictNode<T>::~DictNode()
{
	delete l;
	delete r;
}

#include "huffman.hpp"

#endif // YURIKS_HUFFMAN_H
