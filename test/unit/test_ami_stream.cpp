// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

// This program tests sequential reads and writes of 8 MB of 64-bit int items,
// sequential read and write of 8 MB of 64-bit int arrays,
// random seeking in 8 MB followed by either a read or a write.

#include "common.h"

#include <iostream>
#include <boost/filesystem/operations.hpp>
#include <boost/random.hpp>
#include <tpie/tpie.h>

#include <tpie/array.h>
#include <tpie/stream.h>
#include <tpie/util.h>
#include <vector>
#include <tpie/progress_indicator_arrow.h>

static const std::string TEMPFILE = "tmp";
inline uint64_t ITEM(size_t i) {return i*98927 % 104639;}
static const size_t TESTSIZE = 8*1024*1024;
static const size_t ITEMS = TESTSIZE/sizeof(uint64_t);
static const size_t ARRAYSIZE = 512;
static const size_t ARRAYS = TESTSIZE/(ARRAYSIZE*sizeof(uint64_t));

bool basic() {
	boost::filesystem::remove(TEMPFILE);

	// Write ITEMS items sequentially to TEMPFILE
	{
		tpie::ami::stream<uint64_t> s(TEMPFILE, tpie::ami::WRITE_STREAM);
		for(size_t i=0; i < ITEMS; ++i) s.write_item(ITEM(i));
	}

	// Sequential verify
	{
		tpie::ami::stream<uint64_t> s(TEMPFILE, tpie::ami::READ_STREAM);
		uint64_t *x = 0;
		for(size_t i=0; i < ITEMS; ++i) {
			s.read_item(&x);
			if (*x != ITEM(i)) {
				std::cout << "Expected element " << i << " = " << ITEM(i) << ", got " << *x << std::endl;
				return EXIT_FAILURE;
			}
		}
	}

	// Write an ARRAYSIZE array ARRAYS times sequentially to TEMPFILE
	{
		tpie::ami::stream<uint64_t> s(TEMPFILE, tpie::ami::WRITE_STREAM);
		uint64_t x[ARRAYSIZE];
		for(size_t i=0; i < ARRAYSIZE; ++i) {
			x[i] = ITEM(i);
		}
		for(size_t i=0; i < ARRAYS; ++i) s.write_array(x, ARRAYSIZE);
	}

	// Sequentially verify the arrays
	{
		tpie::ami::stream<uint64_t> s(TEMPFILE, tpie::ami::READ_STREAM);
		uint64_t x[ARRAYSIZE];
		for(size_t i=0; i < ARRAYS; ++i) {
			TPIE_OS_SIZE_T len = ARRAYSIZE;
			s.read_array(x, len);
			if (len != ARRAYSIZE) {
				std::cout << "read_array only read " << len << " elements, expected " << ARRAYSIZE << std::endl;
				return false;
			}
			for (size_t i=0; i < ARRAYSIZE; ++i) {
				if (x[i] != ITEM(i)) {
					std::cout << "Expected element " << i << " = " << ITEM(i) << ", got " << x[i] << std::endl;
					return false;
				}
			}
		}
	}

	// Random read/write of items
	{
		tpie::ami::stream<uint64_t> s(TEMPFILE, tpie::ami::WRITE_STREAM);
		tpie::array<uint64_t> data(ITEMS);
		for (size_t i=0; i < ITEMS; ++i) {
			data[i] = ITEM(i);
			s.write_item(data[i]);
		}
		for (size_t i=0; i < 10; ++i) {
			// Seek to random index
			tpie::stream_offset_type idx = ITEM(i) % ITEMS;
			s.seek(idx);

			if (i%2 == 0) {
				uint64_t *read;
				s.read_item(&read);
				if (*read != data[idx]) {
					std::cout << "Expected element " << idx << " to be " << data[idx] << ", got " << *read << std::endl;
					return false;
				}
			} else {
				uint64_t write = ITEM(ITEMS+i);
				data[idx] = write;
				s.write_item(write);
			}

			tpie::stream_offset_type newoff = s.tell();
			if (newoff != idx+1) {
				std::cout << "Offset advanced to " << newoff << ", expected " << (idx+1) << std::endl;
				return false;
			}
		}
	}
	return true;
}


int stress(size_t actions=1024*1024*128, size_t max_size=1024*1024*128) {
	tpie::progress_indicator_arrow pi("Test", actions);
	const size_t chunk_size=1024*128;
	std::vector<int> elements(max_size, 0);
	std::vector<bool> defined(max_size, true);
	std::vector<int> arr(chunk_size);
	size_t location=0;
	size_t size=0;
	
	boost::mt19937 rng;
	boost::uniform_int<> todo(0, 6);
	boost::uniform_int<> ddist(0, 123456789);
	tpie::ami::stream<int> stream;
	pi.init(actions);
	for(size_t action=0; action < actions; ++action) {
		switch(todo(rng)) {
		case 0: //READ
		{
			size_t cnt=size-location;
			if (cnt > 0) {
				boost::uniform_int<> d(1,std::min<size_t>(cnt, chunk_size));
				cnt=d(rng);
				for (size_t i=0; i < cnt; ++i) {
					int * item;
					if (stream.read_item(&item) != tpie::ami::NO_ERROR) {
						std::cout << "Should be able to read" << std::endl;
						return false;
					}
					if (defined[location]) {
						if (elements[location] != *item) {
							std::cout << "Found " << *item << " expected " << elements[location] << std::endl;
							return false;
						}
					} else {
						defined[location] = true;
						elements[location] = *item;
					}
					++location;
				}
			} else {
				int * item;
				if (stream.read_item(&item) == tpie::ami::NO_ERROR) {
					std::cout << "Should not be able to read" << std::endl;
					return false;
				}
			}
			break;
		}
		case 1: //WRITE
		{
			boost::uniform_int<> d(1,chunk_size);
			size_t cnt=std::min<size_t>(d(rng), max_size-location);
			for (size_t i=0; i < cnt; ++i) {
				elements[location] = ddist(rng);
				defined[location] = true;
				stream.write_item(elements[location]);
				location++;
			}
			size = std::max(size, location);
			break;
		}
		case 2: //SEEK END
		{
			location = size;
			stream.seek(location);
			break;
		}
		case 3: //SEEK SOMEWHERE
		{
			boost::uniform_int<> d(0, size);
			location = d(rng);
			stream.seek(location);
			break;
		}
		case 4: //READ ARRAY
		{
			size_t cnt=size-location;
			if (cnt > 0) {
				boost::uniform_int<> d(1,std::min<size_t>(cnt, chunk_size));
				cnt=d(rng);
				stream.read_array(&arr[0], cnt);
				for (size_t i=0; i < cnt; ++i) {
					if (defined[location]) {
						if (elements[location] != arr[i]) {
							std::cout << "Found " << arr[i] << " expected " << elements[location] << std::endl;
							return false;
						}
					} else {
						defined[location] = true;
						elements[location] = arr[i];
					}
					++location;
				}
			}
		}
		case 5: //WRITE ARRAY
		{
			 boost::uniform_int<> d(1,chunk_size);
			 size_t cnt=std::min<size_t>(d(rng), max_size-location);
			 for (size_t i=0; i < cnt; ++i) {
				 arr[i] = elements[location] = ddist(rng);
				 defined[location] = true;
				 ++location;
			 }
			 stream.write_array(&arr[0], cnt);
			 size = std::max(size, location);
			 break;
		}
		case 6: //TRUNCATE 
		{
			boost::uniform_int<> d(std::max(0, (int)size-(int)chunk_size), std::min(size+chunk_size, max_size));
			size_t ns=d(rng);
			stream.truncate(ns);
			stream.seek(0);
			location=0;
			for (size_t i=size; i < ns; ++i)
				defined[i] = false;
			size=ns;
			break;
		}
		}
		//std::cout << location << " " << size << std::endl;
		if (stream.stream_len() != size) {
			std::cout << "Bad size" << std::endl;
			return false;
		}
		if (stream.tell() != location) {
			std::cout << "Bad offset" << std::endl;
			return false;
		}
		pi.step();
	}
	pi.done();
	return true;
}


int main(int argc, char **argv) {
	tpie_initer _;

	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " basic" << std::endl;
		return EXIT_FAILURE;
	}

	std::string testtype(argv[1]);

	// We only have one test
	if (testtype == "basic") 
		return basic()?EXIT_SUCCESS:EXIT_FAILURE;
	if (testtype == "stress") 
		return stress()?EXIT_SUCCESS:EXIT_FAILURE;
	std::cout << "Unknown test" << std::endl;
	return EXIT_FAILURE;
}
