#ifndef _MERGEHEAP_H_
#define _MERGEHEAP_H_

using namespace std;

/////////////////////////////////////////////////////////
///
/// \class MergeHeap
/// \author Lars Hvam Petersen
///
/// MergeHeap
///
/////////////////////////////////////////////////////////
template<typename T, typename Comparator = std::less<T> >
class MergeHeap {
 public:
  /////////////////////////////////////////////////////////
  ///
  /// Constructor
  ///
  /// \param elements Maximum allowed size of the heap
  ///
  /////////////////////////////////////////////////////////
  MergeHeap(TPIE_OS_OFFSET elements);

  /////////////////////////////////////////////////////////
  ///
  /// Destructor
  ///
  /////////////////////////////////////////////////////////
  ~MergeHeap();

  /////////////////////////////////////////////////////////
  ///
  /// Insert an element into the priority queue
  ///
  /// \param x The item
  /// \param run Where it comes from
  ///
  /////////////////////////////////////////////////////////
  void push(const T& x, TPIE_OS_OFFSET run);

  /////////////////////////////////////////////////////////
  ///
  /// Remove the top element from the priority queue
  ///
  /////////////////////////////////////////////////////////
  void pop();

  /////////////////////////////////////////////////////////
  ///
  /// Remove the top element from the priority queue and 
  /// insert another
  ///
  /// \param x The item
  /// \param run Where it comes from
  ///
  /////////////////////////////////////////////////////////
  void pop_and_push(const T& x, TPIE_OS_OFFSET run);

  /////////////////////////////////////////////////////////
  ///
  /// See whats on the top of the priority queue
  ///
  /// \return Top element
  ///
  /////////////////////////////////////////////////////////
  const T& top();

  /////////////////////////////////////////////////////////
  ///
  /// Return top element run number
  ///
  /// \return Top element run number
  ///
  /////////////////////////////////////////////////////////
  const TPIE_OS_OFFSET top_run();

  /////////////////////////////////////////////////////////
  ///
  /// Returns the size of the queue
  ///
  /// \return Queue size
  ///
  /////////////////////////////////////////////////////////
  const TPIE_OS_OFFSET size();

  /////////////////////////////////////////////////////////
  ///
  /// Return true if queue is empty otherwise false
  ///
  /// \return Boolean - empty or not
  ///
  /////////////////////////////////////////////////////////
  const bool empty();

 private:
  void fixDown();
  void validate();
  void dump();

  TPIE_OS_OFFSET m_size;
  T min;
  Comparator comp_;

  T* heap;
  TPIE_OS_OFFSET* runs;
  TPIE_OS_OFFSET maxsize;
};

#ifndef CPPMERGEHEAP
#include "MergeHeap.cpp"
#endif

#endif
