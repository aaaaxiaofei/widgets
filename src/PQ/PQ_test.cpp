#include "PQdeclare.h"
#include <iostream>

using namespace std;

void testCreateAndDeletePQ() {
    cout << "\n==== Test Create and Delete PQ ======= " << endl;
    PQ<int> queue;
    for (int i = 0; i < 10; ++i) {
        queue.insert(i, i);
    }

    cout << "  Min in the queue is " << queue.findMin() << endl;
    cout << "  Start to delete min from queue until empty." << endl;
    while (!queue.isEmpty()) {
        cout << "    Min value " << queue.findMin() << " is deleted." << endl;
        queue.deleteMin();
    }
    cout << "===== Test Create and Delete PQ Finished ======= " << endl;
}

void testUpdatePriority() {
    cout << "\n===== Test Random Insertion and Update Priority ======= " << endl;
    PQ<int> queue;
    queue.insert(3,3);
    queue.insert(5,5);
    queue.insert(1,1);
    queue.insert(10,10);
    queue.insert(8,8);
    queue.insert(2,2);

    cout << "  Min in the queue " << queue.findMin() << endl;

    queue.insert(0, 0);
    cout << "  inserted object 0 with priority 0, Min in the queue is " << queue.findMin() << endl;

    queue.updatePriority(3, -1);
    cout << "  change 3's priority to -1, Min in the queue is " << queue.findMin() << endl;

    queue.updatePriority(10, -3);
    cout << "  change 10's priority to -3, Min in the queue is " << queue.findMin() << endl;

    queue.updatePriority(10, 7);
    cout << "  change 10's priority to 7, Min in the queue is " << queue.findMin() << endl;

    cout << "  Start to delete min from queue until empty." << endl;
    while (!queue.isEmpty()) {
        cout << "    Min value " << queue.findMin() << " is deleted." << endl;
        queue.deleteMin();
    }
    cout << "===== Test Random Insertion and Update Priority Finished ======= " << endl;
}

int main() {
    testCreateAndDeletePQ();
    testUpdatePriority();
}