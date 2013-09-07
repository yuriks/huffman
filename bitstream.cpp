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
#include "bitstream.h"

void Bitstream::push_back(bool bit)
{
	if ((length() & 0x7) == 0)
	{
		store.push_back(0);
	}

	unsigned char val = *(--store.end());

	val <<= 1;
	val |= (bit ? 1 : 0);

	*(--store.end()) = val;
	++length_;
}

void Bitstream::push_back(const Bitstream& stream)
{
	if (stream.length() >= 8)
	{
		for (unsigned long i = 0; i < (stream.length() >> 3)-1; ++i)
		{
			push_back(stream.store[i]);
		}
	}

	if ((stream.length() & 0x7) != 0)
	{
		unsigned char byte = stream.store[stream.length() >> 3];
		int i = stream.length() & 0x7;

		byte <<= (8-i);
		for (; i > 0; --i)
		{
			push_back((byte & 0x80) != 0);
			byte <<= 1;
		}
	}
}

void Bitstream::push_back(unsigned char byte)
{
	for (int i = 0; i < 8; ++i)
	{
		push_back((byte & 0x80) == 1);
		byte <<= 1;
	}
}

///////////////////////////////////////////////////////////////////////////////

OFileBitstream::OFileBitstream(std::ostream& f)
	: file(f), cur_char(0), cur_char_len(0)
{
}

void OFileBitstream::push_back(bool bit)
{
	cur_char <<= 1;
	cur_char |= (bit ? 1 : 0);
	
	if (++cur_char_len == 8)
	{
		file.put(cur_char);
		cur_char = 0;
		cur_char_len = 0;
	}
}

void OFileBitstream::push_back(const Bitstream& stream)
{
	if (stream.length() >= 8)
	{
		for (unsigned long i = 0; i < (stream.length() >> 3)-1; ++i)
		{
			push_back(stream.store[i]);
		}
	}

	if ((stream.length() & 0x7) != 0)
	{
		unsigned char byte = stream.store[stream.length() >> 3];
		int i = stream.length() & 0x7;

		byte <<= (8-i);
		for (; i > 0; --i)
		{
			push_back((byte & 0x80) != 0);
			byte <<= 1;
		}
	}
}

void OFileBitstream::push_back(unsigned char byte)
{
	for (int i = 0; i < 8; ++i)
	{
		push_back((byte & 0x80) != 0);
		byte <<= 1;
	}
}

OFileBitstream::~OFileBitstream()
{
	// flush remaining bits
	if (cur_char_len > 0)
	{
		// pad with 0
		while (cur_char_len != 0)
			push_back(false);
	}
}
