#include "circularBuffer.h"
#include <string.h>
#include <stdio.h>

boost::mutex insertqmutex;
CircularBuffer::CircularBuffer() {
  m_uiHi=0;       //Head index
  m_uiTi=0;       //Tail index

  m_uiSize=dsize; //size of the queue

  m_uiTotEl=0;       //total number of elements in the queue
  m_buf=NULL;
  
}//CircularBuffer


CircularBuffer::~CircularBuffer() {

  //free the buffer if not NULL
  if(NULL != m_buf) {
    free(m_buf);
  }
  
}//CircularBuffer


/*
  -1 an error occured
  >=0 number of bytes that could be inserted
  start: start of the queue position where the data is being inserted
  end: end of the queue position

  //if inserting the first element, then the memory for queue has not
  //been allocated. Allocate it now.
 */

int CircularBuffer::insert(const char* buf, int n) {
  boost::mutex::scoped_lock lock(insertqmutex);
  int start, end, ret = 0;

	/* Check whether the size is available or not in circular buffer */
	if (0 < getAvSize()) 
		ret = insert(buf, n, start, end);

	return ret;
}//insert

int CircularBuffer::insert(const char* buf, int n, int& start, int& end) {
  int an=0; //actual number of elements that can be inserted in the queue

  //if number of elements to be inserted is less than or equal to
  //zero, return immediately
  if(n <= 0) {
    return 0;
  }

  //Allocate the buffer
  if(NULL==m_buf) {
    m_buf=(char*)malloc(m_uiSize);
    //if buffer could not be created
    if(NULL==m_buf) {
      return -1;
    }
  }

  an=m_uiSize-m_uiTotEl;  //empty space in the queue 
                       //(size of the queue-total number of elements
                       //in it)

  //if queue is full, then return
  if(0==an) {
    return 0;
  }

  //if number of elements to be inserted are more than the size of the
  //queue, we can only insert an elements.
  if(n > an ) {
    n=an;
  }

  //now insert the elements in the buffer
  //two steps

  //(1) first insert at the end of the queue
  //empty space from current position to the end of the queue
  unsigned int first = m_uiSize-m_uiTi; // first would be how mcuh left of queue
  unsigned int second = 0;
  start = m_uiTi;
  
  if(n > first) {
    second = n-first;
  } else {
    first = n;
  }
  
  memcpy(m_buf+m_uiTi, buf, first);  //copy the first
  m_uiTi=(m_uiTi+first)%m_uiSize;    //compute the tail index
  
  //(2) if still remaining, insert at the beginning of the queue
  if(second > 0) {
    memcpy(m_buf+m_uiTi, buf+first, second);
    m_uiTi=second;
  }//end if


  //end index
  end = m_uiTi;

  m_uiTotEl+=n;
  
  return n; //number of bytes that could be inserted
}//insert


/*
  Remove n bytes from the start of the queue
  0 if no elements to be removed
*/
int CircularBuffer::remove(int n) {
  boost::mutex::scoped_lock lock(insertqmutex);
  //if total number of elements are zero or n <= 0
  if(0==m_uiTotEl || n <= 0) {
    return 0;
  }

  if(n > m_uiTotEl) {
    m_uiHi = m_uiTi;  //head index = current index
    n=m_uiTotEl;      
    m_uiTotEl=0;      //total number of elements is zero
    return n;         //return total number of elements removed
  } 

  m_uiHi = (m_uiHi+n)%m_uiSize;  //set the head index
  
  m_uiTotEl = m_uiTotEl - n;     //update the total number of elements
                                 //in the queue

  return n;
  
}//remove


/*
  0 if n is zero or buffer is empty

  Get n bytes from the position start in the queue. 
  WARNING: does not check end of queue
*/
int CircularBuffer::getAt(char* buf, int start, int n, int& end) {
	boost::mutex::scoped_lock lock(insertqmutex);
  //if no elements in the buffer
  if(0 >= n || 0==m_uiTotEl) {
    return 0;
  }
 

  unsigned int first, second;
  first=m_uiSize-start;  //head of the queue (sizeofQ - start)
  second=0;
  
  //there is a wrap around
  if(n > first) {
    second = n-first;
  } else {
    first = n;
  }
  
  memcpy(buf, m_buf+start, first);
  end=(start+first)%m_uiSize;  
  
  //if there was a wrap around, copy the remaining buffer
  if(second > 0) {
    memcpy( buf+first, m_buf, second);
    end=second;
  } 
  
  return n;
  
}//getAt



/*
  Get n bytes from the start of the queue. Return the end+1 index in
  end, which can be used to read from this position in the queue.  
  kindof act as an iterator
*/
int CircularBuffer::getAt(char* buf, int n, int& end) {
  boost::mutex::scoped_lock lock(insertqmutex);
  //if no elements in the buffer
  if(0 >= n || 0==m_uiTotEl) {
    return 0;
  }

  //if we are requesting more elements than there are in the queue,
  //adjust the size of the requested elements
  if(n > m_uiTotEl) {
    n = m_uiTotEl;
  }
  
  unsigned int first, second;
  first=m_uiSize-m_uiHi;  //head of the queue
  second=0;
  
  //there is a wrap around
  if(n > first) {
    second = n-first;
  } else {
    first = n;
  }
  
  memcpy(buf, m_buf+m_uiHi, first);
  end=(m_uiHi+first)%m_uiSize;  
  
  //if there was a wrap around, copy the remaining buffer
  if(second > 0) {
    memcpy( buf+first, m_buf, second);
    end=second;
  } 
  
  return n;
}//getAt

/*
  Get queue head
*/
int CircularBuffer::getHead() {
	boost::mutex::scoped_lock lock(insertqmutex);
  return m_uiHi;
}//Head


/*
  Get the size of the queue
*/
int CircularBuffer::getSize() {
	boost::mutex::scoped_lock lock(insertqmutex);
  return m_uiSize;
}//getSize

/*
 * Get the size which are available in the queue
 */
int CircularBuffer::getAvSize() {
  return m_uiSize-m_uiTotEl;
}

/*
  set the size of the queue
  -1 error
  1 successful
*/
int CircularBuffer::setSize(int size) {
	boost::mutex::scoped_lock lock(insertqmutex);
  m_uiSize = size;

  if(m_buf!=NULL) {
    free(m_buf);
  }

  m_buf=(char*)malloc(m_uiSize);

  //if buffer could not be allocated, return -1
  if(NULL==m_buf) {
    return -1;
  }

  return 1;
}//setSize



/*
  get the total number of elements in queue
*/
int CircularBuffer::getTotalElements() {
	boost::mutex::scoped_lock lock(insertqmutex);
  return m_uiTotEl;
}//getTotalElements


void CircularBuffer::print() {
  
  unsigned int ind=m_uiHi;
  int size=m_uiTotEl; //total # of elements in the queue

  while(size > 0) {
    printf("%c index: %d\n", m_buf[ind], ind);
    ind=(ind+1)%m_uiSize;
    size--;
  }
  
  
}//print

