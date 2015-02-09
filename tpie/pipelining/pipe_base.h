// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, 2012, 2013, The TPIE development team
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

#ifndef __TPIE_PIPELINING_PIPE_BASE_H__
#define __TPIE_PIPELINING_PIPE_BASE_H__

#include <tpie/types.h>
#include <tpie/pipelining/priority_type.h>
#include <tpie/pipelining/pair_factory.h>
#include <tpie/pipelining/pipeline.h>
#include <tpie/pipelining/node_set.h>

#ifdef WIN32
// Silence warning C4521: multiple copy constructors specified.
// This warning is emitted since we declare both
// pipe_middle(const pipe_middle &) and
// pipe_middle(pipe_middle &).
// However, both of these are necessary to ensure that copying takes
// preference over the template <T> pipe_middle(T) constructor.
#pragma warning(push)
#pragma warning(disable: 4521)
#endif

namespace tpie {

namespace pipelining {

namespace bits {

template <typename child_t>
class pipe_base {
public:
	///////////////////////////////////////////////////////////////////////////
	/// \brief  Set memory fraction for this node in the pipeline phase.
	///
	/// In the absence of minimum and maximum memory requirements set by node
	/// implementations, the memory assigned to the node will be proportional
	/// to the \c amount parameter which sets the memory priority of this node
	/// in relation to the rest of the phase.
	///
	/// \sa factory_base::memory(double)
	///////////////////////////////////////////////////////////////////////////
	inline child_t & memory(double amount) {
		self().factory.memory(amount);
		return self();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Get memory fraction for this node in the pipeline phase.
	///
	/// \sa memory(double)
	/// \sa factory_base::memory()
	///////////////////////////////////////////////////////////////////////////
	inline double memory() const {
		return self().factory.memory();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Set name for this node.
	///
	/// The name is used in the GraphViz plot generated by pipeline::plot.
	///
	/// The name priority given in the second argument should indicate how
	/// important this node is for the current phase, and is used in naming the
	/// progress indicator for this phase. The node with the highest priority
	/// name gets to name the entire phase.
	///
	/// \sa factory_base::name
	///////////////////////////////////////////////////////////////////////////
	inline child_t & name(const std::string & n, priority_type p = PRIORITY_USER) {
		self().factory.name(n, p);
		return self();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get a refenerce to this node.
	///
	/// This reference can be used to call add_dependency on another node
	///
	/// \sa factory_base::ref
	///////////////////////////////////////////////////////////////////////////
	inline child_t & add_to_set(node_set s) {
		self().factory.add_to_set(s);
		return self();
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Add a depencency to a referenced node.
	///
	/// \sa factory_base::ref
	///////////////////////////////////////////////////////////////////////////
	inline child_t & add_dependencies(node_set s) {
		self().factory.add_dependencies(s);
		return self();
	}		
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief  Set a prefix for the name of this node.
	///
	/// The name is used in the GraphViz plot generated by pipeline::plot.
	///
	/// \sa factory_base::push_breadcrumb
	///////////////////////////////////////////////////////////////////////////
	inline child_t & breadcrumb(const std::string & n) {
		self().factory.push_breadcrumb(n);
		return self();
	}

protected:
	inline child_t & self() {return *static_cast<child_t*>(this);}
	inline const child_t & self() const {return *static_cast<const child_t*>(this);}
};

// The derived class has to pass its factory type to us as a template argument.
// See the following Stack Overflow question, dated Nov 13, 2011, for a discussion.
// http://stackoverflow.com/q/8113878
// At the time this class is instantiated, child_t is not yet complete.
// This means we cannot use child_t::factory_type as an existing type name.
template <typename child_t, typename fact_t>
class pipe_term_base : public pipe_base<child_t> {
public:
	typedef typename fact_t::constructed_type constructed_type;

	constructed_type construct() const {
		return this->self().factory.construct();
	}
};

// For this class, we only use child_t::factory_type inside
// a templated member type, so at the time of its instantiation,
// child_t is complete and child_t::factory_type is valid.
template <typename child_t>
class pipe_nonterm_base : public pipe_base<child_t> {
public:
	template <typename dest_t>
	struct constructed {
		typedef typename child_t::factory_type::template constructed<dest_t>::type type;
	};

	template <typename dest_t>
	typename constructed<dest_t>::type construct(const dest_t & dest) const {
		return this->self().factory.construct(dest);
	}
};

} // namespace bits

template <typename fact_t>
class pipe_end : public bits::pipe_term_base<pipe_end<fact_t>, fact_t> {
public:
	typedef fact_t factory_type;

	pipe_end(const pipe_end & other) : factory(other.factory) {}
	pipe_end(pipe_end & other) : factory(other.factory) {}
#ifdef TPIE_CPP_RVALUE_REFERENCE
	pipe_end(pipe_end && other) : factory(std::move(other.factory)) {}
#endif

	#ifdef DOXYGEN
	///////////////////////////////////////////////////////////////////////////////
	/// \brief Forwards the arguments given to the constructor of the factory.
	///
	/// The implementation either usesvariadic template(if supported by the
	/// compiler) or a bunch of overloads to support a variable number of
	/// constructor parameters.
	///
	/// \tparam Args the variadic number of types of constructor parameters.
	/// \param args the variadic number of arguments to pass to the constructor of
	/// the factory
	///////////////////////////////////////////////////////////////////////////////
	template <typename Args>
	inline pipe_end(Args args);
	#elif defined(TPIE_CPP_VARIADIC_TEMPLATES) && defined(TPIE_CPP_RVALUE_REFERENCE)
	template <typename ... T_ARGS>
	inline pipe_end(T_ARGS && ... t) : factory(std::forward<T_ARGS>(t)...) {}
	#else
	#define TPIE_CLASS_NAME pipe_end
	#include <tpie/pipe_constructors.inl>
	#undef TPIE_CLASS_NAME
	#endif

	fact_t factory;
};

///////////////////////////////////////////////////////////////////////////////
/// \class pipe_middle
///
/// A pipe_middle class pushes input down the pipeline.
///
/// \tparam fact_t A factory with a construct() method like the factory_0,
///                factory_1, etc. helpers.
///////////////////////////////////////////////////////////////////////////////
template <typename fact_t>
class pipe_middle : public bits::pipe_nonterm_base<pipe_middle<fact_t> > {
public:
	typedef fact_t factory_type;

	pipe_middle(const pipe_middle & other) : factory(other.factory) {}
	pipe_middle(pipe_middle & other) : factory(other.factory) {}
#ifdef TPIE_CPP_RVALUE_REFERENCE
	pipe_middle(pipe_middle && other) : factory(std::move(other.factory)) {}
#endif

	#ifdef DOXYGEN
	///////////////////////////////////////////////////////////////////////////////
	/// \brief Forwards the arguments given to the constructor of the factory.
	///
	/// The implementation either usesvariadic template(if supported by the
	/// compiler) or a bunch of overloads to support a variable number of
	/// constructor parameters.
	///
	/// \tparam Args the variadic number of types of constructor parameters.
	/// \param args the variadic number of arguments to pass to the constructor of
	/// the factory
	///////////////////////////////////////////////////////////////////////////////
	template <typename Args>
	inline pipe_middle(Args args);
	#elif defined(TPIE_CPP_VARIADIC_TEMPLATES) && defined(TPIE_CPP_RVALUE_REFERENCE)
	template <typename ... T_ARGS>
	inline pipe_middle(T_ARGS && ... t) : factory(std::forward<T_ARGS>(t)...) {}
	#else
	#define TPIE_CLASS_NAME pipe_middle
	#include <tpie/pipe_constructors.inl>
	#undef TPIE_CLASS_NAME
	#endif

	///////////////////////////////////////////////////////////////////////////
	/// The pipe operator combines this generator/filter with another filter.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact2_t>
	inline pipe_middle<bits::pair_factory<fact_t, fact2_t> >
	operator|(const pipe_middle<fact2_t> & r) {
		factory.set_destination_kind_push();
		return bits::pair_factory<fact_t, fact2_t>(factory, r.factory);
	}

	///////////////////////////////////////////////////////////////////////////
	/// This pipe operator combines this generator/filter with a terminator to
	/// make a pipeline.
	///////////////////////////////////////////////////////////////////////////
	template <typename fact2_t>
	inline pipe_end<bits::termpair_factory<fact_t, fact2_t> >
	operator|(const pipe_end<fact2_t> & r) {
		factory.set_destination_kind_push();
		return bits::termpair_factory<fact_t, fact2_t>(factory, r.factory);
	}

	fact_t factory;
};

template <typename fact_t>
class pipe_begin : public bits::pipe_nonterm_base<pipe_begin<fact_t> > {
public:
	typedef fact_t factory_type;

	pipe_begin(const pipe_begin & other) : factory(other.factory) {}
	pipe_begin(pipe_begin & other) : factory(other.factory) {}
#ifdef TPIE_CPP_RVALUE_REFERENCE
	pipe_begin(pipe_begin && other) : factory(std::move(other.factory)) {}
#endif

	#ifdef DOXYGEN
	///////////////////////////////////////////////////////////////////////////////
	/// \brief Forwards the arguments given to the constructor of the factory.
	///
	/// The implementation either usesvariadic template(if supported by the
	/// compiler) or a bunch of overloads to support a variable number of
	/// constructor parameters.
	///
	/// \tparam Args the variadic number of types of constructor parameters.
	/// \param args the variadic number of arguments to pass to the constructor of
	/// the factory
	///////////////////////////////////////////////////////////////////////////////
	template <typename Args>
	inline pipe_begin(Args args);
	#elif defined(TPIE_CPP_VARIADIC_TEMPLATES) && defined(TPIE_CPP_RVALUE_REFERENCE)
	template <typename ... T_ARGS>
	inline pipe_begin(T_ARGS && ... t) : factory(std::forward<T_ARGS>(t)...) {}
	#else
	#define TPIE_CLASS_NAME pipe_begin
	#include <tpie/pipe_constructors.inl>
	#undef TPIE_CLASS_NAME
	#endif

	template <typename fact2_t>
	inline pipe_begin<bits::pair_factory<fact_t, fact2_t> >
	operator|(const pipe_middle<fact2_t> & r) {
		factory.set_destination_kind_push();
		return bits::pair_factory<fact_t, fact2_t>(factory, r.factory);
	}

	template <typename fact2_t>
	inline bits::pipeline_impl<bits::termpair_factory<fact_t, fact2_t> >
	operator|(const pipe_end<fact2_t> & r) {
		factory.set_destination_kind_push();
		return bits::termpair_factory<fact_t, fact2_t>(factory, r.factory).final();
	}

	fact_t factory;
};

template <typename fact_t>
class pullpipe_end : public bits::pipe_nonterm_base<pullpipe_end<fact_t> > {
public:
	typedef fact_t factory_type;

	pullpipe_end(const pullpipe_end & other) : factory(other.factory) {}
	pullpipe_end(pullpipe_end & other) : factory(other.factory) {}
#ifdef TPIE_CPP_RVALUE_REFERENCE
	pullpipe_end(pullpipe_end && other) : factory(std::move(other.factory)) {}
#endif

	#ifdef DOXYGEN
	///////////////////////////////////////////////////////////////////////////////
	/// \brief Forwards the arguments given to the constructor of the factory.
	///
	/// The implementation either usesvariadic template(if supported by the
	/// compiler) or a bunch of overloads to support a variable number of
	/// constructor parameters.
	///
	/// \tparam Args the variadic number of types of constructor parameters.
	/// \param args the variadic number of arguments to pass to the constructor of
	/// the factory
	///////////////////////////////////////////////////////////////////////////////
	template <typename Args>
	inline pullpipe_end(Args args);
	#elif defined(TPIE_CPP_VARIADIC_TEMPLATES) && defined(TPIE_CPP_RVALUE_REFERENCE)
	template <typename ... T_ARGS>
	inline pullpipe_end(T_ARGS && ... t) : factory(std::forward<T_ARGS>(t)...) {}
	#else
	#define TPIE_CLASS_NAME pullpipe_end
	#include <tpie/pipe_constructors.inl>
	#undef TPIE_CLASS_NAME
	#endif

	fact_t factory;
};

template <typename fact_t>
class pullpipe_middle : public bits::pipe_nonterm_base<pullpipe_middle<fact_t> > {
public:
	typedef fact_t factory_type;

	pullpipe_middle(const pullpipe_middle & other) : factory(other.factory) {}
	pullpipe_middle(pullpipe_middle & other) : factory(other.factory) {}
#ifdef TPIE_CPP_RVALUE_REFERENCE
	pullpipe_middle(pullpipe_middle && other) : factory(std::move(other.factory)) {}
#endif

	#ifdef DOXYGEN
	///////////////////////////////////////////////////////////////////////////////
	/// \brief Forwards the arguments given to the constructor of the factory.
	///
	/// The implementation either usesvariadic template(if supported by the
	/// compiler) or a bunch of overloads to support a variable number of
	/// constructor parameters.
	///
	/// \tparam Args the variadic number of types of constructor parameters.
	/// \param args the variadic number of arguments to pass to the constructor of
	/// the factory
	///////////////////////////////////////////////////////////////////////////////
	template <typename Args>
	inline pullpipe_middle(Args args);
	#elif defined(TPIE_CPP_VARIADIC_TEMPLATES) && defined(TPIE_CPP_RVALUE_REFERENCE)
	template <typename ... T_ARGS>
	inline pullpipe_middle(T_ARGS && ... t) : factory(std::forward<T_ARGS>(t)...) {}
	#else
	#define TPIE_CLASS_NAME pullpipe_middle
	#include <tpie/pipe_constructors.inl>
	#undef TPIE_CLASS_NAME
	#endif

	template <typename fact2_t>
	inline pullpipe_middle<bits::pair_factory<fact2_t, fact_t> >
	operator|(const pipe_middle<fact2_t> & r) {
		fact2_t f = r.factory;
		f.set_destination_kind_pull();
		return bits::pair_factory<fact2_t, fact_t>(f, factory);
	}

	template <typename fact2_t>
	inline pullpipe_end<bits::termpair_factory<fact2_t, fact_t> >
	operator|(const pipe_end<fact2_t> & r) {
		fact2_t f = r.factory;
		f.set_destination_kind_pull();
		return bits::termpair_factory<fact2_t, fact_t>(f, factory);
	}

	fact_t factory;
};

template <typename fact_t>
class pullpipe_begin : public bits::pipe_term_base<pullpipe_begin<fact_t>, fact_t> {
public:
	typedef fact_t factory_type;

	pullpipe_begin(const pullpipe_begin & other) : factory(other.factory) {}
	pullpipe_begin(pullpipe_begin & other) : factory(other.factory) {}
#ifdef TPIE_CPP_RVALUE_REFERENCE
	pullpipe_begin(pullpipe_begin && other) : factory(std::move(other.factory)) {}
#endif

	#ifdef DOXYGEN
	///////////////////////////////////////////////////////////////////////////////
	/// \brief Forwards the arguments given to the constructor of the factory.
	///
	/// The implementation either usesvariadic template(if supported by the
	/// compiler) or a bunch of overloads to support a variable number of
	/// constructor parameters.
	///
	/// \tparam Args the variadic number of types of constructor parameters.
	/// \param args the variadic number of arguments to pass to the constructor of
	/// the factory
	///////////////////////////////////////////////////////////////////////////////
	template <typename Args>
	inline pullpipe_begin(Args args);
	#elif defined(TPIE_CPP_VARIADIC_TEMPLATES) && defined(TPIE_CPP_RVALUE_REFERENCE)
	template <typename ... T_ARGS>
	inline pullpipe_begin(T_ARGS && ... t) : factory(std::forward<T_ARGS>(t)...) {}
	#else
	#define TPIE_CLASS_NAME pullpipe_begin
	#include <tpie/pipe_constructors.inl>
	#undef TPIE_CLASS_NAME
	#endif

	template <typename fact2_t>
	inline pullpipe_begin<bits::termpair_factory<fact2_t, fact_t> >
	operator|(const pullpipe_middle<fact2_t> & r) {
		fact2_t f = r.factory;
		f.set_destination_kind_pull();
		return bits::termpair_factory<fact2_t, fact_t>(f, factory);
	}

	template <typename fact2_t>
	inline bits::pipeline_impl<bits::termpair_factory<fact2_t, fact_t> >
	operator|(const pullpipe_end<fact2_t> & r) {
		fact2_t f = r.factory;
		f.set_destination_kind_pull();
		return bits::termpair_factory<fact2_t, fact_t>(f, factory).final();
	}

	fact_t factory;
};

} // namespace pipelining

} // namespace tpie

#ifdef WIN32
#pragma warning(pop)
#endif

#endif // __TPIE_PIPELINING_PIPE_BASE_H__
