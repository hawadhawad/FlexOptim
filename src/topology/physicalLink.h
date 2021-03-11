#ifndef __PhysicalFiber__h
#define __PhysicalFiber__h

#include <vector>

#include "slice.h"
#include "demand.h"

/************************************************************************************
 * This class identifies a Fiber in the physical network. It corresponds to an 
 * edge in the topology graph and it is defined by its id, a source and a target node, 
 * its length in the physical network, and a cost. A edge also has a frequency 
 * spectrum that is splitted into a given number of slices. \note The id of each 
 * fiber is considered to be in the range [0, ..., n-1].
 ************************************************************************************/
class Fiber{
private:
	int id;							/**< The fiber identifier. **/
	int index;						/**< The index on which the fiber is positioned in the Instance's vector of edges. **/
	int idSource;					/**< Source node identifier. **/ 
	int idTarget;					/**< Target node identifier. **/ 
	int nbSlices;					/**< Number of slices the frequency spectrum is divided into. **/
	double length;					/**< Length of the fiber in the physical network. **/
	double cost;					/**< Cost of routing a demand through the fiber. **/
	std::vector<Slice> spectrum;	/**< Fiber's spectrum. **/

public:
	/****************************************************************************************/
	/*										Constructor										*/
	/****************************************************************************************/

	/** Constructor. @param i Id. @param ind Index. @param s Source node's id. @param t Target node's id. @param len Fiber's length n the physical network. @param nb Number of slices its frequency spectrum is divided into. @param c Cost of routing a demand through the fiber. **/
	Fiber(int i, int ind, int s, int t, double len = 0.0, int nb = 1, double c = 0.0);

	Fiber(const Fiber &);

	/****************************************************************************************/
	/*										Getters											*/
	/****************************************************************************************/

	/** Returns the fiber's id. **/
	int getId() const { return id; }

	/** Returns the fiber's index. **/
	int getIndex() const { return index; }
	
	/** Returns the fiber's source node id. **/
	int getSource() const { return idSource; }
	
	/** Returns the fiber's target node id. **/
	int getTarget() const { return idTarget; }
	
	/** Returns the number of slices its frequency spectrum is divided into. **/
	int getNbSlices() const { return nbSlices; }
	
	/** Returns the fiber's physical length. **/
	double getLength() const { return length; }

	/** Returns the cost of routing a demand through the fiber. \note This attribute is not really used for now but may be useful in the future. **/
	double getCost() const { return cost; }
	
	/** Returns the frequency spectrum as a vector of Slices. **/
	std::vector<Slice> getSlices() const { return spectrum; }
	
	/** Returns a slice of the frequency spectrum. @param i The slice in the i-th position. **/
	Slice getSlice_i(int i) const {return spectrum[i];}
	
	/** Returns a string summarizing the fiber's from/to information. **/
	std::string getString() const { return "[" + std::to_string(getSource()+1) + "," + std::to_string(getTarget()+1) + "]"; }

	/** Returns the maximal slice position used in the fiber's frequency spectrum. **/
	int getMaxUsedSlicePosition() const;
	
	/** Returns the number of slices ocupied in the fiber's frequency spectrum. **/
	int getNbUsedSlices() const;

	/****************************************************************************************/
	/*										Setters											*/
	/****************************************************************************************/

	/** Changes the fiber's id. @param i New id. **/
	void setId(int i) { this->id = i; }

	/** Changes the fiber's index. @param i New index. **/
	void setIndex(int i) { this->index = i; }
	
	/** Changes the fiber's source node id. @param s New source node's id. **/
	void setSource(int s) { this->idSource = s; }

	/** Changes the fiber's target node id. @param t New target node's id. **/
	void setTarget(int t) { this->idTarget = t; }

	/** Changes the number of slices its frequency spectrum is divided into. @param nb New number of slices.**/
	void setNbSlices(int nb) { this->nbSlices = nb; }

	/** Changes the fiber's length. @param l New length. **/
	void setLength(double l) { this->length = l; }
	
	/** Changes the cost of routing a demand through the fiber. @param c New cost. **/
	void setCost(double c) { this->cost = c; }

	/****************************************************************************************/
	/*										Methods											*/
	/****************************************************************************************/

	/** Copies all information from a given fiber. @param fiber The Fiber to be copied. \note It does not copy the fiber's id nor the fiber's index. **/
	void copyFiber(Fiber &fiber);

	/** Verifies if the fiber is routing a demand. @param dem The demand to be checked. **/
	bool contains(const Demand &dem) const;

	/** Assigns a demand to a given position in the spectrum. @param dem The demand to be assigned. @param p The position of the last slice to be assigned. **/
	void assignSlices(const Demand &dem, int p);

	/****************************************************************************************/
	/*										Display											*/
	/****************************************************************************************/

	/** Displays summarized information about the fiber. **/
	void displayFiber();

	/** Displays detailed information about the state of the fiber. **/
	void displayDetailedFiber();

	/** Displays summarized information about slice occupation. **/
	void displaySlices();
	
	/****************************************************************************************/
	/*										Destructor										*/
	/****************************************************************************************/

	/** Destructor. \note Clears the spectrum. **/
	~Fiber();
};

#endif