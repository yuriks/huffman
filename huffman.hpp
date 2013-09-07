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
#include "huffman.h"

#include <limits>
#include <vector>
#include <queue>
#include <algorithm>
#include <numeric>
#include <cassert>

namespace YURIKS_HUFFMAN_CPP
{

template <typename Iter>
std::vector<unsigned long> make_frequency(Iter begin, const Iter& end)
{
	const unsigned char max_val = std::numeric_limits<unsigned char>::max();

	std::vector<unsigned long> frequency(max_val+1, 0);

	for (; begin != end; ++begin)
	{
		++frequency[(unsigned char)(*begin)];
	}

	return frequency;
}

void populate_reverse_map(std::vector<Bitstream*>& map, Dictionary<unsigned char> *dict, Bitstream path)
{
	DictType type = dict->getType();

	switch (type)
	{
	case DICT_VALUE:
	//if (DictValue<unsigned char> *dict_val = dynamic_cast<DictValue<unsigned char>*>(dict))
	{
		DictValue<unsigned char> *dict_val = static_cast<DictValue<unsigned char>*>(dict);

		map[dict_val->val] = new Bitstream(path);
	} break;
	case DICT_NODE:
	//else if (DictNode<unsigned char> *dict_node = dynamic_cast<DictNode<unsigned char>*>(dict))
	{
		DictNode<unsigned char> *dict_node = static_cast<DictNode<unsigned char>*>(dict);
		Bitstream path_copy = path;

		path.push_back(false);
		populate_reverse_map(map, dict_node->l, path);
		path_copy.push_back(true);
		populate_reverse_map(map, dict_node->r, path_copy);
	} break;
	case DICT_NONE_EOF:
	//else // EOF
	{
		map[std::numeric_limits<unsigned char>::max() + 1] = new Bitstream(path);
	} break;
	}
}

void serialize_dictionary(OFileBitstream& s, Dictionary<unsigned char> *dict)
{
	DictType type = dict->getType();

	switch (type)
	{
	case DICT_VALUE:
	{
		DictValue<unsigned char> *dict_val = static_cast<DictValue<unsigned char>*>(dict);

		s.push_back(false);
		s.push_back(dict_val->val);
		// To differentiate from EOF
		if (dict_val->val == 0)
			s.push_back(false);
	} break;
	case DICT_NODE:
	{
		DictNode<unsigned char> *dict_node = static_cast<DictNode<unsigned char>*>(dict);

		s.push_back(true);
		serialize_dictionary(s, dict_node->l);
		serialize_dictionary(s, dict_node->r);
	} break;
	case DICT_NONE_EOF:
	{
		s.push_back(false);
		// EOF marker: 0 followed by bit 1
		s.push_back(static_cast<unsigned char>(0));
		s.push_back(true);
	} break;
	}
}

typedef std::pair<Dictionary<unsigned char>*, unsigned long> pairt;
struct pairt_gt
{
	inline bool operator()(const pairt& a, const pairt& b)
	{
		return a.second > b.second;
	}
};

} // namespace YURIKS_HUFFMAN_CPP

template <typename Iter>
Dictionary<unsigned char>* build_huffman_tree(Iter& begin, const Iter& end)
{
	using namespace YURIKS_HUFFMAN_CPP;

	static const unsigned char max_val = std::numeric_limits<unsigned char>::max();

	std::vector<unsigned long> frequency = make_frequency(begin, end);

	std::priority_queue<pairt, std::vector<pairt>, pairt_gt> dict_queue;

	for (unsigned int i = 0; i <= max_val; ++i)
	{
		if (frequency[i] > 0)
		{
			dict_queue.push(pairt(new DictValue<unsigned char>(i), frequency[i]));
		}
	}
	dict_queue.push(pairt(new Dictionary<unsigned char>, 1)); // Add EOF

	while (dict_queue.size() > 1)
	{
		pairt a = dict_queue.top();
		dict_queue.pop();
		pairt b = dict_queue.top();
		dict_queue.pop();

		dict_queue.push(pairt(new DictNode<unsigned char>(a.first, b.first), a.second + b.second));
	}
	assert(std::accumulate(frequency.begin(), frequency.end(), 0) == dict_queue.top().second-1);

	return dict_queue.top().first;
}

static const char spinner_chars[4] = {'|', '\\', '-', '/'};

template <typename Iter>
void huffman_compress(Dictionary<unsigned char>* tree, OFileBitstream& stream, Iter& begin, const Iter& end, unsigned long long size)
{
	using namespace YURIKS_HUFFMAN_CPP;

	static const unsigned char max_val = std::numeric_limits<unsigned char>::max();

	std::vector<Bitstream*> reverse_map(max_val+1+1, static_cast<Bitstream*>(0));
	populate_reverse_map(reverse_map, tree, Bitstream());

	// Write dictionary
	serialize_dictionary(stream, tree);

	int progress = 1;
	int spinner_pos = 0;
	unsigned long cur_pos = 0;

	// Write data
	for (; begin != end; ++begin)
	{
		if (--progress == 0)
		{
			std::cerr << '\r' << spinner_chars[spinner_pos] << ' ' << (int)((float)cur_pos / size * 100) << '%' << std::flush;
			if (++spinner_pos == 4)
				spinner_pos = 0;

			progress = 100000;
		}

		stream.push_back(*reverse_map[(unsigned char)*begin]);
		++cur_pos;
	}
	std::cerr << "\r  100%" << std::endl;

	// Write EOF
	stream.push_back(*reverse_map[max_val+1]);

	for (std::vector<Bitstream*>::iterator i = reverse_map.begin(); i != reverse_map.end(); ++i)
		delete *i;
}

void huffman_uncompress(IFileBitstream& stream, std::ostream& output, const Dictionary<unsigned char>* tree, unsigned long long size)
{
	const Dictionary<unsigned char> *const root = tree;
	const Dictionary<unsigned char>* cur = root;

	try
	{
		int progress = 1;
		int spinner_pos = 0;
		unsigned long cur_pos = 0;

		while (true)
		{
			// Isto eh uma otimizacao
			DictType t = cur->getType();

			switch (t)
			{
			case DICT_NODE:
			{
				const DictNode<unsigned char>* node = static_cast<const DictNode<unsigned char>*>(cur);

				if (!stream.nextBit())
					cur = node->l;
				else
					cur = node->r;

				++cur_pos;
			} break;
			case DICT_VALUE:
			{
				const DictValue<unsigned char>* val = static_cast<const DictValue<unsigned char>*>(cur);

				if (--progress == 0)
				{
					std::cerr << '\r' << spinner_chars[spinner_pos] << ' ' << (int)((float)cur_pos / 8 / size * 100) << '%' << std::flush;
					if (++spinner_pos == 4)
						spinner_pos = 0;

					progress = 100000;
				}

				output.put((char)val->val);
				cur = root;
			} break;
			case DICT_NONE_EOF:
			{
				// EOF
				goto out_while;
			} break;
			}
		}
out_while:;
		std::cerr << "\r  100%" << std::endl;
	}
	catch (std::ifstream::failure&)
	{
		std::cerr << "Erro durante a descompressao";
	}
}

Dictionary<unsigned char>* readNode(IFileBitstream& stream)
{
	if (stream.nextBit())
	{
		DictNode<unsigned char>* node = new DictNode<unsigned char>();
		node->l = readNode(stream);
		node->r = readNode(stream);
		return node;
	}
	else
	{
		unsigned char val = stream.nextChar();
		if (val == 0)
			if (stream.nextBit() == true)
				return new Dictionary<unsigned char>(); // EOF
		return new DictValue<unsigned char>(val);
	}
}
