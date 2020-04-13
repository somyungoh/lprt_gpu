#ifndef SM_HITQUEUE_H_
#define SM_HITQUEUE_H_

/**********************************************************
*
*		hitrec.h
*
*		CUDA does not support std::queue,
*		so this is implementation of a priority queue
*       that takes hitrec as a type
*
**********************************************************/

#include "hit.h"
#include <thrust/device_vector.h>

class hitqueue{
public:

    __device__ hitqueue() : m_nSize(5), m_nTop(-1) {
        m_arr = new hitrec[m_nSize];
    }
    __device__ ~hitqueue() {
        delete m_arr;
    }

    __device__ int size() {
        return m_nSize;
    }

    __device__ hitrec top() const {
        return m_arr[m_nTop];
    }

    __device__ bool isEmpty() const {
        return m_nTop < 0;
    }

    __device__ void push(const hitrec &h){
    
        // array full
        if(m_nTop + 1 >= m_nSize) return;
        m_arr[++m_nTop] = h;
    
        sort();
    }

    __device__ hitrec pop() {
        return m_arr[m_nTop--];
    }

private:
    int m_nSize;
    int m_nTop;
	hitrec* m_arr;

    __device__ void sort() {
        int nCursor = m_nTop;

        while (nCursor > 0) {
            // swap if t is larger than the back
            if (m_arr[nCursor].t > m_arr[nCursor-1].t) {
                hitrec temp      = m_arr[nCursor];
                m_arr[nCursor]   = m_arr[nCursor - 1];
                m_arr[nCursor-1] = temp;
            
                nCursor--;
            }
            else
                break;
        }
    }
};

#endif