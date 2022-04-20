#include "allocator.h"
#include "utils.h"

#define LFENCE asm volatile("lfence" : : : "memory")
#define SFENCE asm volatile("sfence" : : : "memory")

template<class T>
struct ptr_t {
    T* ptr;
    ptr_t() {}
    ptr_t(T* _ptr): ptr(_ptr) {}

    T* address() {
        // Get the address by getting the 48 least significant bits of ptr
        uintptr_t address_mask = 0x0000FFFFFFFFFFFF;
        return (T*) ((uintptr_t) ptr & address_mask);
    }
    uint count() {
        // Get the count from the 16 most significant bits of ptr
        uintptr_t count_mask = 0xFFFF000000000000;
        return (uintptr_t) ptr & count_mask;
    }
};

template<class T>
struct Node {
    T value;
    ptr_t<Node<T>> next;
};

template <class T>
class NonBlockingQueue {
    CustomAllocator my_allocator_;
    ptr_t<Node<T>> head;
    ptr_t<Node<T>> tail;
    
public:
    NonBlockingQueue() : my_allocator_() {
        std::cout << "Using NonBlockingQueue\n";
    }

    void initQueue(long t_my_allocator_size) {
        my_allocator_.initialize(t_my_allocator_size, sizeof(Node<T>));
        // Initialize the queue head / tail
        Node<T>* sentinelNode = (Node<T>*)my_allocator_.newNode();
        sentinelNode->next.ptr = nullptr;
        head.ptr = tail.ptr = sentinelNode;
    }

    void enqueue(T value) {
        Node<T>* node = (Node<T>*)my_allocator_.newNode();
        node->value = value;
        node->next.ptr = nullptr;
        SFENCE; // Store (write) fence

        ptr_t<Node<T>> currTail;
        while(true) {
            currTail = tail;
            LFENCE; // Load (read) fence
            ptr_t<Node<T>> next = tail.address()->next;
            LFENCE; // Load (read) fence
            if (currTail.address() == tail.address()) {
                if (next.address() == nullptr) {
                    ptr_t<Node<T>> newNode((Node<T>*) ((uintptr_t) node | ((uintptr_t) (next.count()+1)) << 48));
                    if(CAS(&tail.address()->next, next, newNode))
                        break;
                } else {
                    ptr_t<Node<T>> newTail((Node<T>*) ((uintptr_t) next.address() | ((uintptr_t) (currTail.count()+1)) << 48));
                    CAS(&tail, currTail, newTail);	// ELABEL
                }
            }
        }

        SFENCE; // Store (write) fence
        ptr_t<Node<T>> newTail((Node<T>*) ((uintptr_t) node | ((uintptr_t) (currTail.count()+1)) << 48));
        CAS(&tail, currTail, newTail);
    }

    bool dequeue(T *value) {
        ptr_t<Node<T>> currTail;
        ptr_t<Node<T>> currHead;
        while(true) {
            currHead = head;
            LFENCE;
            currTail = tail;
            LFENCE;
            ptr_t<Node<T>> next = currHead.address()->next;
            LFENCE;
            if (currHead.address() == head.address()) {
                if(currHead.address() == currTail.address()) {
                    if(next.address() == nullptr)
                        return false;
                    ptr_t<Node<T>> newTail((Node<T>*) ((uintptr_t) next.address() | ((uintptr_t) (currTail.count()+1)) << 48));
                    CAS(&tail, currTail, newTail);	//DLABEL
                } else {
                    *value = next.address()->value;
                    ptr_t<Node<T>> newHead((Node<T>*) ((uintptr_t) next.address() | ((uintptr_t) (currHead.count()+1)) << 48));
                    if(CAS(&head, currHead, newHead))
                        break;
                }
            }
        }
        my_allocator_.freeNode(currHead.address());
        return true;
    }

    // bool front(T *value) {
    //     Node<T> *first = head.address()->next.address();
    //     if (first) {
    //         *value = first->value;
    //         return true;
    //     }
    //     return false;
    // }

    void cleanup() {
        my_allocator_.cleanup();
    }
};

