#include "PQdeclare.h"
#include <iostream>

using namespace std;

int main() {
    PQ<int> queue;
    queue.insert(3,3);
    queue.insert(5,5);
    queue.insert(1,1);
    queue.insert(10,10);
    queue.insert(8, 8);
    queue.insert(2, 2);


    cout << "Min in the queue " << queue.findMin() << endl;

    queue.insert(0, 0);
    cout << "Min in the queue " << queue.findMin() << endl;

    queue.updatePriority(3, -1);
    cout << "Min in the queue " << queue.findMin() << endl;

    queue.updatePriority(10, -3);
    cout << "Min in the queue " << queue.findMin() << endl;

    while (!queue.isEmpty()) {
        cout << "Min is " << queue.findMin() << endl;
        queue.deleteMin();
    }
}