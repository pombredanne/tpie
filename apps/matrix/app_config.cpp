// Copyright 2008, The TPIE development team
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

// Copyright (c) 1994 Darren Erik Vengroff
//
// File: app_config.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/7/94
//




#include "app_config.h"

#ifdef NDEBUG
bool verbose = false;
#else
bool verbose = true;
#endif

TPIE_OS_SIZE_T test_mm_size = DEFAULT_TEST_MM_SIZE;
TPIE_OS_OFFSET test_size = DEFAULT_TEST_SIZE;
int random_seed = 17;
