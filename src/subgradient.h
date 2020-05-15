#ifndef __subgradient__h
#define __subgradient__h

#include "RSA.h"

using namespace lemon;
// For the moment only k=1 is implemented. This class 

/**********************************************************************************************
 * This class implements and solves a Langragian Relaxation of the Routing and Spectrum 
 * Allocation problem associated with a given set of k demands. The problem is decomposed into
 * k Shortest Path problems, and solved it by applying a subgradient method.  
 * Langrangian Relaxation is applied and hence the Shortest Path problems have their objective 
 * function modified to penalize max length infractions and overlapping occurence.
 * \note It uses the LEMON library to solve the Shortest Path problems.
 * *******************************************************************************************/
class Subgradient : public RSA {
    
private:
    const int MAX_NB_IT_WITHOUT_IMPROVEMENT;    /**< Maximum number of iterations without lower bound improvement. **/
    const int MAX_NB_IT;                        /**< Maximum number of performed iterations. **/
    const int MIN_STEPSIZE;

    int iteration;
    int itWithoutImprovement;

    double UB;
    double LB;
    double currentLagrCost;
    double currentRealCost;

    bool isFeasible;
    bool isUnfeasible;
    bool isOptimal;

    /* A vector storing the value of the Lagrangian multipliers associated with Length Constraints. */
    std::vector<double> lagrangianMultiplierLength;

    /* A 4-dimensional vector storing the value of the Lagrangian multipliers associated with each Non-Overlapping constraint. */
    std::vector< std::vector< std::vector< std::vector <double> > > > lagrangianMultiplierOverlap;

    /** Stores the value of the slack of lengths constraints (i.e., b - Dx). */
    std::vector<double> lengthSlack;

    /** Stores the value of the slack of Non-Overlapping constraints (i.e., b - Dx). */
    std::vector< std::vector< std::vector< std::vector <double> > > > overlapSlack;

    /** Stores the value of the step size used for updating the Lagrangian multipliers. **/
    double stepSize;

    /** Stores the value of lambda used for updating the step size. **/
    double lambda;

    /* Refers to the cost of an arc during iteration k of subgradient. cost = c_{ij} + u_k*length_{ij} */
    std::vector< std::shared_ptr<ArcCost> > cost; 
    
    std::vector< std::vector<bool> > assignmentMatrix;

public:
	/************************************************/
	/*				    Constructors 		   		*/
	/************************************************/
    Subgradient(const Instance &inst);

	/************************************************/
	/*					   Getters 		    		*/
	/************************************************/
    int getIteration() const { return iteration; }
    int getItWithoutImprovement() const { return itWithoutImprovement; }

    double getUB() const { return UB; }
    double getLB() const { return LB; }
    double getLagrCurrentCost() const { return currentLagrCost; }
    double getRealCurrentCost() const { return currentRealCost; }

    bool getIsFeasible() const { return isFeasible; }
    bool getIsUnfeasible() const { return isUnfeasible; }
    bool getIsOptimal() const { return isOptimal; }


    double getLengthMultiplier_k(int k) const { return lagrangianMultiplierLength[k]; }
    double getOverlapMultiplier_k(int d1, int d2, int e, int s) const { return lagrangianMultiplierOverlap[d1][d2][e][s]; }
    
    double getOverlapSlack_k(int d1, int d2, int e, int s) const { return overlapSlack[d1][d2][e][s]; }
    double getLengthSlack_k(int k) const { return lengthSlack[k]; }
    
    double getStepSize() const { return stepSize; }
    double getLambda() const { return lambda; }

    double getRealCostFromPath(int d, Dijkstra< ListDigraph, ListDigraph::ArcMap<double> > &path, const ListDigraph::Node &SOURCE, const ListDigraph::Node &TARGET);

	/************************************************/
	/*					   Setters 		    		*/
	/************************************************/
    void setIteration(int i) { iteration = i; }
    void setItWithoutImprovement(int i) { itWithoutImprovement = i; }
    void incIteration() { iteration++; }
    void incItWithoutImprovement() { itWithoutImprovement++; }

	/** Changes the cost of an arc in a graph. @param a The arc. @param d The graph index. @param val The new cost value. **/
	void setArcCost(const ListDigraph::Arc &a, int d, double val) { (*cost[d])[a] = val; }

	/** Increments the cost of an arc in a graph. @param a The arc. @param d The graph index. @param val The value to be added to cost. **/
	void incArcCost(const ListDigraph::Arc &a, int d, double val) { (*cost[d])[a] += val; }

    void setUB(double i){ UB = i; }
    void setLB(double i){ LB = i; }
    void setCurrentLagrCost(double i){ currentLagrCost = i; }
    void incCurrentLagrCost(double i){ currentLagrCost += i; }
    void setCurrentRealCost(double i){ currentRealCost = i; }
    void incCurrentRealCost(double i){ currentRealCost += i; }
    

    void setIsUnfeasible(bool i) { isUnfeasible = i;}
    void setIsFeasible(bool i) { isFeasible = i;}
    void setIsOptimal(bool i) { isOptimal = i; }

    void setLengthMultiplier_k (int k, double i) { lagrangianMultiplierLength[k] = i; }
    void setOverlapMultiplier_k (int d1, int d2, int e, int s, double i) { lagrangianMultiplierOverlap[d1][d2][e][s] = i; }
    void setStepSize(double val){ stepSize = val; }
    void setLengthSlack_k(int k, double val){ lengthSlack[k] = val; }
    void setOverlapSlack_k(int d1, int d2, int e, int s, double val){ overlapSlack[d1][d2][e][s] = val; }

    /** Changes the value of lambda. @param val The new value of lambda. **/
    void setLambda(double val){ lambda = val; }

	/************************************************/
	/*					   Methods 		    		*/
	/************************************************/
    
    /** Sets the initial lagrangian multipliers values for the subgradient to run. **/
    void initMultipliers();

    /** Initializes the assignement matrix. **/
    void initAssignmentMatrix();

    /** Initializes the slack of relaxed constraints. **/
    void initSlacks();

    /** Initializes the costs in the objective function. **/
    void initCosts();

    /** Sets all the initial parameters for the subgradient to run. **/
    void initialization();

    /** Checks if all slacks are non-negative. **/
    bool checkFeasibility();

    /** Updates the arc costs. @note cost = coeff + u_d*length **/
    void updateCosts();

    /** Updates the assignment of a demand based on the a given path. @param d The d-th demand. @param path The path on which the demand is routed. @param SOURCE The path's source. @param TARGET The path's target. **/
    void updateAssignment_k(int d, Dijkstra< ListDigraph, ListDigraph::ArcMap<double> > &path, const ListDigraph::Node &SOURCE, const ListDigraph::Node &TARGET);

    /** Solves the RSA using the Subgradient Method. **/
    void run();

    /** Solves an iteration of the Subgradient Method. **/
    void runIteration();

    /* Updates the known lower bound. */
    void updateLB(double bound);

    /* Updates the known upper bound. */
    void updateUB(double bound);
    
    /* Updates lagrangian multiplier with the rule: u[k+1] = u[k] + t[k]*violation */
    void updateMultiplier();

    /* Updates the step size with the rule: lambda*(UB - Z[u])/|slack| */
    void updateStepSize();
    
    /* Updates the lambda used in the update of step size. Lambda is halved if LB has failed to increade in some fixed number of iterations */
    void updateLambda();
    
    /** Updates the slack of a length constraint based on a given path length. @param d The d-th length constraint. @param pathLength The path length. **/
    void updateLengthSlack(int d, double pathLength);
    
    void updateOverlapSlack();

    /* Verifies if optimality condition has been achieved and update STOP flag. */
    void updateStop(bool &STOP);

    /* Tests if CSP is feasible by searching for a shortest path with arc costs based on their physical length. */
    bool testFeasibility(const ListDigraph::Node &s, const ListDigraph::Node &t);
    
    /* Assigns length as the main cost of the arcs. */
    void setLengthCost();
    
    /* Stores the solution found in the arcMap onPath. */
    void updateOnPath();
    
    /* Returns the physical length of the path. */
    double getPathLength(int d, Dijkstra< ListDigraph, ListDigraph::ArcMap<double> > &path, const ListDigraph::Node &s, const ListDigraph::Node &t);
    
    /* Returns the actual cost of the path according to the metric used. */
    double getPathCost(int d, Dijkstra< ListDigraph, ListDigraph::ArcMap<double> > &path, const ListDigraph::Node &s, const ListDigraph::Node &t);
    
    bool isGradientMoving();
    /************************************************/
	/*					   Display 		    		*/
	/************************************************/
    std::string getPathString(int d, Dijkstra< ListDigraph, ListDigraph::ArcMap<double> > &path, const ListDigraph::Node &s, const ListDigraph::Node &t);
    void displayPath(Dijkstra< ListDigraph, ListDigraph::ArcMap<double> > &path, const ListDigraph::Node &s, const ListDigraph::Node &t);
    void displayMainParameters();
    void displayStepSize();
    void displayMultiplier();
    void displaySlack();
    void displayLambda();
};    
#endif