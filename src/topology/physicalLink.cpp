#include "physicalLink.h"
#include <iostream>

/****************************************************************************************/
/*										Constructor										*/
/****************************************************************************************/

/** Constructor. **/
Fiber::Fiber(int i, int ind, int s, int t, double l, int nb, double c) {
	this->setId(i);
	this->setIndex(ind);
	this->setSource(s);
	this->setTarget(t);
	this->setLength(l);
	this->setNbSlices(nb);
	for (int i = 0; i < nb; i++){
		this->spectrum.push_back(Slice());
	}
	this->setCost(c);
}

/****************************************************************************************/
/*										Destructor										*/
/****************************************************************************************/

/* Destructor. Clears the vector of slices. */
Fiber::~Fiber() {
	spectrum.clear();
}

/****************************************************************************************/
/*										Methods											*/
/****************************************************************************************/

/* Copies all information from a given fiber. */
void Fiber::copyFiber(Fiber & edge){
	this->setSource(edge.getSource());
	this->setTarget(edge.getTarget());
	this->setNbSlices(edge.getNbSlices());
	this->spectrum.resize(this->getNbSlices());
	for (int i = 0; i < this->getNbSlices(); i++){
		this->spectrum[i].setAssignment(edge.getSlice_i(i).getAssignment());
	}	
	this->setLength(edge.getLength());
	this->setCost(edge.getCost());
}

/* Verifies if the fiber is routing a demand. */
bool Fiber::contains(const Demand &d) const{
	for (int i = 0; i < this->getNbSlices(); i++){
		if (getSlice_i(i).getAssignment() == d.getId()){
			return true;
		}
	}
	return false;
}

/* Assigns a demand to a given position in the spectrum. */
void Fiber::assignSlices(const Demand &d, int p){
	int demandLoad = d.getLoad();
	// assign demand d to this edge from position p - demandLoad + 1 to position p
	int first = p - demandLoad + 1;
	for (int i = first; i <= p; i++) {
		this->spectrum[i].setAssignment(d.getId());
	}
}

/* Returns the maximal slice position used in the frequency spectrum. */
int Fiber::getMaxUsedSlicePosition() const {
	int max = 0;
	for (int i = 0; i < this->getNbSlices(); i++){
		if (getSlice_i(i).isUsed()){
			if (i >= max){
				max = i;
			}
		}
	}
	return max;
}

/* Returns the number of slices ocupied in the fiber's frequency spectrum. */
int Fiber::getNbUsedSlices() const {
	int value = 0;
	for (int i = 0; i < this->getNbSlices(); i++){
		if (getSlice_i(i).isUsed()){
			value++;
		}
	}
	return value;
}

/****************************************************************************************/
/*										Display											*/
/****************************************************************************************/

/* Displays summarized information about the fiber. */
void Fiber::displayFiber(){
	std::cout << "#" << this->getId()+1 << ". " << this->getSource()+1 << " -- " << this->getTarget()+1;
	std::cout << ". lenght: " << this->getLength() << ", cost: " << this->getCost() << std::endl;
}

/* Displays detailed information about state of the fiber. */
void Fiber::displayDetailedFiber(){
	std::cout << "#" << this->getId()+1 << ". " << this->getSource()+1 << " -- " << this->getTarget()+1;
	std::cout << ". lenght: " << this->getLength() << ", cost: " << this->getCost() << std::endl;
	for (int i = 0; i < this->getNbSlices(); i++){
		std::cout << "\tSlice #" << i+1 << ". ";
		if (this->spectrum[i].isUsed()) {
			std::cout << this->spectrum[i].getAssignment()+1 << std::endl;
		}
		else{
			std::cout << "--" << std::endl;
		}
	}
}

/* Displays summarized information about slice occupation. */
void Fiber::displaySlices(){
	for (int i = 0; i < this->getNbSlices(); i++){
		if (this->spectrum[i].isUsed()) {
			std::cout << "*";
		}
		else {
			std::cout << " ";
		}
	}
	std::cout << std::endl;
}
