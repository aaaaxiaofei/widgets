#ifndef PQ_H
#define PQ_H
#include "dsexceptions.h"
#include <vector>
#include <climits>


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
			increasePriority(values.size() - 1);
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
		values[0].first = INT_MAX;
		int cur = 0;
		cur = decreasePriority(cur);
		if (cur != values.size() - 1) {
			swap(values[cur], values.back());
			increasePriority(cur);
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
		increasePriority(values.size() - 1);
	}

    // Update the priority of ID x to p
    //    Inserts x with p if s not already in the queue
    void updatePriority( const ID & x, int p ) {
		bool updated = false;
		for (int i = 0; i < values.size(); ++i) {
			if (values[i].second == x) {
				values[i].first = p;
				int cur = increasePriority(i);
				if (cur == i) {
					decreasePriority(i);
				}
				updated = true;
				break;
			}
		}
		if (!updated) {
			values.push_back(make_pair(p, x));
			increasePriority(values.size() - 1);
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

	// Insrease the priority queue with element change at cur
	int increasePriority(int cur) {
		int parent = (cur - 1) / 2;
		while (cur > 0 && values[cur].first < values[parent].first) {
			swap(values[cur], values[parent]);
			cur = parent;
			parent = (cur - 1) / 2;
		}
		return cur;
	}

	// Decrease the priority queue with element change at cur
	int decreasePriority(int cur) {
		while (cur < values.size()) {
			int c1 = cur * 2 + 1, c2 = cur * 2 + 2;
			if (c2 < values.size()) {
				int c = values[c1].first > values[c2].first ? c2 : c1;
				swap(values[cur], values[c]);
				cur = c;
			} else if (c1 < values.size()) {
				swap(values[cur], values[c1]);
				cur = c1;				
			} else {
				break;
			}
		}
		return cur;
	}
};
#endif
