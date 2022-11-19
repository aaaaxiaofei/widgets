#ifndef PQ_H
#define PQ_H
#include "dsexceptions.h"
#include <vector>

using namespace std;

// PQ class
//
// Constructors:
// PQ --> constructs a new empty queue
// PQ( tasks, priorities ) --> constructs a new queue with a given set of task IDs and priorities 
// ******************PUBLIC OPERATIONS*********************
// void insert( x, p )       --> Insert task ID x with priority p 
// ID findMin( )  --> Return a task ID with smallest priority, without removing it 
// ID deleteMin( )   --> Remove and return a task ID with smallest priority 
// void updatePriority( x, p )   --> Changes priority of ID x to p (if x not in PQ, inserts x);
// bool isEmpty( )   --> Return true if empty; else false
// int size() --> return the number of task IDs in the queue 
// void makeEmpty( )  --> Remove all task IDs (and their priorities)
// ******************ERRORS********************************
// Throws UnderflowException as warranted

template <typename ID>
// ID is the type of task IDs to be used; the type must be Comparable (i.e., have < defined), so IDs can be AVL Tree keys.
class PQ
{
public:
    // Constructor
    // Initializes a new empty PQ
    PQ() {};

    // Constructor
    // Initializes a new PQ with a given set of tasks IDs and priorities  
    //      priority[i] is the priority for ID task[i] 
    PQ( const vector<ID> & tasks, const vector<int> & priorities ) {
		for (int i = 0; i < tasks.size(); ++i) {
			values.push_back(make_pair(priorities[i], tasks[i]));
			updatePriority(values.size() - 1);
		}
    }
						     
    // Emptiness check 
    bool isEmpty( ) const {
		return values.size() == 0;
	}

    // Deletes and Returns a task ID with minimum priority
    //    Throws exception if queue is empty
    const ID deleteMin() {
		if (isEmpty()) {
			throw UnderflowException{ };
		}
		ID result = values[0].second;
		int cur = 0;
		while (cur < values.size() - 1) {
			int p1 = cur * 2 + 1, p2 = cur * 2 + 2;
			if (p2 < values.size()) {
				int p = values[p1].first > values[p2].first ? p2 : p1;
				swap(values[cur], values[p]);
				cur = p;
			} else if (p1 < values.size()) {
				swap(values[cur], values[p1]);
				cur = p1;				
			} else {
				swap(values[cur], values.back());
				updatePriority(cur);
				break;
			}
		}
		values.pop_back();
		return result;
	}

    // Returns an ID with minimum priority without removing it
    //     Throws exception if queue is empty
    const ID & findMin() const {
		if (isEmpty()) {
			throw UnderflowException{ };
		}
		return values[0].second;
	}

    // Insert ID x with priority p.
    void insert( const ID & x, int p ) {
		values.push_back(make_pair(p, x));
		updatePriority(values.size() - 1);
	}

    // Update the priority of ID x to p
    //    Inserts x with p if s not already in the queue
    void updatePriority( const ID & x, int p ) {
		bool updated = false;
		for (int i = 0; i < values.size(); ++i) {
			if (values[i].second == x) {
				values[i].first = p;
				updatePriority(i);
				updated = true;
				break;
			}
		}
		if (!updated) {
			values.push_back(make_pair(p, x));
			updatePriority(values.size() - 1);
		}
	}

    // Return the number of task IDs in the queue
    int size() const {
		return values.size();
	}

    // Delete all IDs from the PQ
    void makeEmpty( ) {
		values = vector<pair<int, ID>>();
	}

private:
    vector<pair<int, ID>> values;

	// Update the priority queue with new element inserted at cur
	void updatePriority(int cur) {
		int parent = (cur - 1) / 2;
		while (cur > 0 && values[cur].first < values[parent].first) {
			swap(values[cur], values[parent]);
			cur = parent;
			parent = (cur - 1) / 2;
		}
	}
};
#endif
