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
	if (DictValue<unsigned char> *dict_val = dynamic_cast<DictValue<unsigned char>*>(dict))
	{
		map[dict_val->val] = new Bitstream(path);
	}
	else if (DictNode<unsigned char> *dict_node = dynamic_cast<DictNode<unsigned char>*>(dict))
	{
		Bitstream path_copy = path;

		path.push_back(false);
		populate_reverse_map(map, dict_node->l, path);
		path_copy.push_back(true);
		populate_reverse_map(map, dict_node->r, path_copy);
	}
	else // EOF
	{
		map[std::numeric_limits<unsigned char>::max() + 1] = new Bitstream(path);
	}
}

void serialize_dictionary(OFileBitstream& s, Dictionary<unsigned char> *dict)
{
	if (DictValue<unsigned char> *dict_val = dynamic_cast<DictValue<unsigned char>*>(dict))
	{
		s.push_back(false);
		s.push_back(dict_val->val);
		// To differentiate from EOF
		if (dict_val->val == 0)
			s.push_back(false);
	}
	else if (DictNode<unsigned char> *dict_node = dynamic_cast<DictNode<unsigned char>*>(dict))
	{
		s.push_back(true);
		serialize_dictionary(s, dict_node->l);
		serialize_dictionary(s, dict_node->r);
	}
	else // EOF
	{
		s.push_back(false);
		// EOF marker: 0 followed by bit 1
		s.push_back(static_cast<unsigned char>(0));
		s.push_back(true);
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

template <typename Iter>
void huffman_compress(Dictionary<unsigned char>* tree, OFileBitstream& stream, Iter& begin, const Iter& end)
{
	using namespace YURIKS_HUFFMAN_CPP;

	static const unsigned char max_val = std::numeric_limits<unsigned char>::max();

	std::vector<Bitstream*> reverse_map(max_val+1+1, static_cast<Bitstream*>(0));
	populate_reverse_map(reverse_map, tree, Bitstream());

	// Write dictionary
	serialize_dictionary(stream, tree);

	// Write data
	for (; begin != end; ++begin)
	{
		//(void)*begin;
		stream.push_back(*reverse_map[(unsigned char)*begin]);
	}
	// Write EOF
	stream.push_back(*reverse_map[max_val+1]);

	for (std::vector<Bitstream*>::iterator i = reverse_map.begin(); i != reverse_map.end(); ++i)
		delete *i;
}
