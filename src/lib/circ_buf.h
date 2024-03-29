/*******************************************************************************
 * circ_buf.h
 * The implementation of circular buffer.
 * 
 * 
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

class CircularBuffer {

	private:
		enum {dsize=4096};	//default size 4K bytes
		unsigned int m_uiSize;	//total length of the queue

		unsigned int m_uiHi;	//head of the queue
		unsigned int m_uiTi;	//tail of the queue. always points to the
					//location where data should be inserted
		unsigned int m_uiTotEl;	//total number of elements in the queue

		char *m_buf;		//queue buffer

		boost::mutex cb_mtx;	//mutex for the circular buffer 

	public:
		/*
			Number of bytes inserted. If n > size of the buffer, then only
			insert n-size elements.
		*/
		int insert(const char* buf, int n);
		int insert(const char* buf, int n, int& start, int& end);

		/*
			Remove n bytes from the start of the queue. If n > size, then only
			remove n-size elements
		*/
		int remove(int n);

		/*
			Get n bytes from the position start in the queue. If n > (size-start)%mod of
			the queue, then only the remaining bytes are returned
		*/
		int getAt(char* buf, int start, int n, int& end);

		/*
			0 if 0<=n, or empty queue
			Get n bytes from the start of the queue.
			returns the index of the queue where to fetch again
		*/
		int getAt(char* buf, int n, int& end);

		/*
			Get the size of the queue
		*/
		int getSize();

		/*
			Get the size which are available in the queue
		*/
		int getAvSize();

		/*
			set the size of the queue
		*/
		int setSize(int size);

		/*
			get the total number of elements in queue
		*/
		int getTotalElements();

		/*
			get the index of the head of the queue
		*/
		int getHead();

		/*
			print all the details
		*/
		void print_all();


		CircularBuffer();
		~CircularBuffer();

};//CircularBuffer
