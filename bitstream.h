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
#ifndef YURIKS_BITSTREAM_H
#define YURIKS_BITSTREAM_H

#include <vector>
#include <cassert>
#include <fstream>

class Bitstream
{
public:
	Bitstream();

	void push_back(bool bit);
	void push_back(const Bitstream& stream);
	void push_back(unsigned char byte);
	unsigned long length() const;
	//bool at(unsigned long i) const;

private:
	std::vector<unsigned char> store;
	unsigned long length_;

	friend class OFileBitstream;
};

inline Bitstream::Bitstream()
	: length_(0)
{
}

inline unsigned long Bitstream::length() const
{
	return length_;
}

class OFileBitstream
{
public:
	OFileBitstream(std::ostream& f);

	void push_back(bool bit);
	void push_back(const Bitstream& stream);
	void push_back(unsigned char byte);

	~OFileBitstream();

private:
	std::ostream& file;
	unsigned char cur_char;
	int cur_char_len;
};

#endif // YURIKS_BITSTREAM_H
