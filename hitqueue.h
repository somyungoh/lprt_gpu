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

    __device__ int size() {
        return m_queue.size();
    }

    __device__ hitrec top() const {
        return m_queue.back();
    }

    __device__ bool isEmpty() const {
        return m_queue.size() == 0;
    }

    __device__ void push(const hitrec &h){
       //m_queue.push_back(h);
    
        // TODO: sorting if not arranged
    }

    __device__ hitrec pop() {

        //hitrec h = m_queue.back();
        //m_queue.pop_back();

        return hitrec();
    }

private:
	thrust::device_vector<hitrec> m_queue;
};

#endif