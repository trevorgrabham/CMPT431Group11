#include "allocator.h"
#include <mutex>

template <class T>
class Node {
public:
    T value;
    Node<T>* next;
};

template <class T>
class TwoLockQueue {
    CustomAllocator my_allocator_;
    std::mutex mx_enq;
    std::mutex mx_deq;
    Node<T>* head;
    Node<T>* tail;
    
public:
    TwoLockQueue() : my_allocator_() {
        std::cout << "Using TwoLockQueue\n";
    }

    void initQueue(long t_my_allocator_size) {
        my_allocator_.initialize(t_my_allocator_size, sizeof(Node<T>));
        // Initialize the queue head / tail
        Node<T>* sentinelNode = (Node<T>*)my_allocator_.newNode();
        sentinelNode->next = nullptr;
        head = tail = sentinelNode;
    }

    void enqueue(T value) {
        Node<T>* newNode = (Node<T>*)my_allocator_.newNode();
        newNode->value = value;
        newNode->next = nullptr;
        std::lock_guard<std::mutex> lk(mx_enq);
        tail = tail->next = newNode;
    }

    bool dequeue(T *value) {
        // std::lock_guard<std::mutex> lk(mx_deq);
        mx_deq.lock();
        Node<T>* sentinelNode = head;
        Node<T>* firstNode = head->next;
        if(!firstNode){
            // Queue is empty
            mx_deq.unlock();
            return false;
        }
        *value = firstNode->value;
        // Update sentinel node
        head = firstNode;
        mx_deq.unlock();
        my_allocator_.freeNode(sentinelNode);
        return true;
    }

    bool empty() { return head == tail; }

    void cleanup() { my_allocator_.cleanup(); }
};