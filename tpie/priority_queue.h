// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, 2012, The TPIE development team
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

///////////////////////////////////////////////////////////////////////////////
/// \file priority_queue.h
/// \brief External memory priority queue implementation.
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_PRIORITY_QUEUE_H_
#define _TPIE_PRIORITY_QUEUE_H_

#include <tpie/config.h>
#include "portability.h"
#include "tpie_log.h"
#include <cassert>
#include "pq_overflow_heap.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cmath>
#include <string>
#include <cstring> // for memcpy
#include <sstream>
#include "pq_merge_heap.h"
#include <tpie/err.h>
#include <tpie/stream.h>
#include <tpie/array.h>
#include <boost/filesystem.hpp>

namespace tpie {

	struct priority_queue_error : public std::logic_error {
		priority_queue_error(const std::string& what) : std::logic_error(what)
		{ }
	};
	
/////////////////////////////////////////////////////////
///
///  \class priority_queue
///  \author Lars Hvam Petersen
///
///  Inspiration: Sanders - Fast priority queues for cached memory (1999)
///  Refer to Section 2 and Figure 1 for an overview of the algorithm
///
/////////////////////////////////////////////////////////

template<typename T, typename Comparator = std::less<T>, typename OPQType = pq_overflow_heap<T, Comparator> >
class priority_queue {
	typedef memory_size_type group_type;
	typedef memory_size_type slot_type;
public:
    /////////////////////////////////////////////////////////
    ///
    /// Constructor
    ///
    /// \param f Factor of memory that the priority queue is 
    /// allowed to use.
	/// \param b Block factor
    ///
    /////////////////////////////////////////////////////////
    priority_queue(double f=1.0, float b=0.0625);

#ifndef DOXYGEN
    // \param mmavail Number of bytes the priority queue is
    // allowed to use.
	// \param b Block factor
    priority_queue(memory_size_type mm_avail, float b=0.0625);
#endif


    /////////////////////////////////////////////////////////
    ///
    /// Destructor
    ///
    /////////////////////////////////////////////////////////
    ~priority_queue();

    /////////////////////////////////////////////////////////
    ///
    /// Insert an element into the priority queue
    ///
    /// \param x The item
    ///
    /////////////////////////////////////////////////////////
    void push(const T& x);

    /////////////////////////////////////////////////////////
    ///
    /// Remove the top element from the priority queue
    ///
    /////////////////////////////////////////////////////////
    void pop();

    /////////////////////////////////////////////////////////
    ///
    /// See what's on the top of the priority queue
    ///
    /// \return Top element
    ///
    /////////////////////////////////////////////////////////
    const T& top();

    /////////////////////////////////////////////////////////
    ///
    /// Returns the size of the queue
    ///
    /// \return Queue size
    ///
    /////////////////////////////////////////////////////////
    stream_size_type size() const;

    /////////////////////////////////////////////////////////
    ///
    /// Return true if queue is empty otherwise false
    ///
    /// \return Boolean - empty or not
    ///
    /////////////////////////////////////////////////////////
    bool empty() const;

    /////////////////////////////////////////////////////////
    ///
    /// Pop all elements with priority equal to that of the
    /// top element, and process each by invoking f's call
    /// operator on the element.
    ///
    /// \param f - assumed to have a call operator with parameter of type T.
    ///
    /// \return The argument f
    ///
    /////////////////////////////////////////////////////////
    template <typename F> F pop_equals(F f);

private:
    Comparator comp_;
    T dummy;

    T min;
    bool min_in_buffer;

	tpie::auto_ptr<OPQType> opq; // insert heap
	tpie::array<T> buffer; // deletion buffer
	tpie::array<T> gbuffer0; // group buffer 0
	tpie::array<T> mergebuffer; // merge buffer for merging deletion buffer and group buffer 0
	tpie::array<stream_size_type> slot_state;
	tpie::array<stream_size_type> group_state;

    memory_size_type setting_k;
    memory_size_type current_r;
    memory_size_type setting_m;
    memory_size_type setting_mmark;

    memory_size_type slot_data_id;

    stream_size_type m_size;
    memory_size_type buffer_size;
    memory_size_type buffer_start;

	float block_factor;

	void init(memory_size_type mm_avail);

    void slot_start_set(slot_type slot, stream_size_type n);
    stream_size_type slot_start(slot_type slot) const;
    void slot_size_set(slot_type slot, stream_size_type n);
    stream_size_type slot_size(slot_type slot) const;
    void group_start_set(group_type group, stream_size_type n);
    memory_size_type group_start(group_type group) const;
    void group_size_set(group_type group, memory_size_type n);
    stream_size_type group_size(group_type group) const;
    array<temp_file> datafiles;
    array<temp_file> groupdatafiles;
    temp_file & slot_data(slot_type slotid);
    void slot_data_set(slot_type slotid, stream_size_type n);
    temp_file & group_data(group_type groupid);
    memory_size_type slot_max_size(slot_type slotid);
    void write_slot(slot_type slotid, T* arr, stream_size_type len);
    slot_type free_slot(group_type group);
    void empty_group(group_type group);
    void fill_buffer();
    void fill_group_buffer(group_type group);
    void compact(slot_type slot);
    void validate();
    void remove_group_buffer(group_type group);
    void dump();
};

#include "priority_queue.inl"

    namespace ami {
		using tpie::priority_queue;
    }  //  ami namespace

}  //  tpie namespace

#endif
