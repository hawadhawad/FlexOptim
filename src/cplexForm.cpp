#include "cplexForm.h"



/* Constructor. Builds the Online RSA mixed-integer program and solves it using CPLEX. */
FlowForm::FlowForm(const Instance &inst) : Solver(inst), x(env, getNbDemandsToBeRouted()), maxSlicePerLink(env, instance.getNbEdges()), maxSliceOverall(env){
    std::cout << "--- Flow formulation has been chosen ---" << std::endl;
    /************************************************/
	/*				    SET VARIABLES				*/
	/************************************************/
    this->setVariables(x, maxSlicePerLink, maxSliceOverall, model);
    std::cout << "Variables have been defined..." << std::endl;

	/************************************************/
	/*			    SET OBJECTIVE FUNCTION			*/
	/************************************************/
    this->setObjective(x, maxSlicePerLink, maxSliceOverall, model);
    std::cout << "Objective function has been defined..." << std::endl;

	/************************************************/
	/*			      SET CONSTRAINTS				*/
	/************************************************/
    this->setSourceConstraints(x, model);
    std::cout << "Source constraints have been defined..." << std::endl;

    this->setFlowConservationConstraints(x, model);
    std::cout << "Flow conservation constraints have been defined..." << std::endl;

    this->setTargetConstraints(x, model);
    std::cout << "Target constraints have been defined..." << std::endl;

    this->setLengthConstraints(x, model);
    std::cout << "Length constraints have been defined..." << std::endl;

    //this->setNonOverlappingConstraints(x, model);    
    //std::cout << "Non-Overlapping constraints have been defined..." << std::endl;

    this->setImprovedNonOverlappingConstraints_1(x, model);    
    std::cout << "First set of Improved Non-Overlapping constraints has been defined..." << std::endl;

    this->setImprovedNonOverlappingConstraints_2(x, model);    
    std::cout << "Second set of Improved Non-Overlapping constraints has been defined..." << std::endl;

    
    if(instance.getInput().getChosenObj() == Input::OBJECTIVE_METRIC_1p){
        this->setMaxUsedSlicePerLinkConstraints(x, maxSlicePerLink, model);    
        std::cout << "Max Used Slice Per Link constraints have been defined..." << std::endl;
    }

    if(instance.getInput().getChosenObj() == Input::OBJECTIVE_METRIC_8){
        this->setMaxUsedSliceOverallConstraints(x, maxSliceOverall, model);    
        std::cout << "Max Used Slice Overall constraints have been defined..." << std::endl;
    }
    
    
	/************************************************/
	/*		    EXPORT LINEAR PROGRAM TO .LP		*/
	/************************************************/
    std::string file = getInstance().getInput().getOutputPath() + "LP/model" + std::to_string(count) + ".lp";
    //cplex.exportModel(file.c_str());
    std::cout << "LP model has been exported..." << std::endl;
    
	/************************************************/
	/*             DEFINE CPLEX PARAMETERS   		*/
	/************************************************/
    cplex.setParam(IloCplex::Param::MIP::Display, 3);
    cplex.setParam(IloCplex::Param::TimeLimit, getInstance().getInput().getTimeLimit());
    std::cout << "CPLEX parameters have been defined..." << std::endl;

	/************************************************/
	/*		         SOLVE LINEAR PROGRAM   		*/
	/************************************************/
    IloNum timeStart = cplex.getCplexTime();
    std::cout << "Solving..." << std::endl;
    cplex.solve();
    std::cout << "Solved!" << std::endl;
    IloNum timeFinish = cplex.getCplexTime();

	/************************************************/
	/*		    GET OPTIMAL SOLUTION FOUND        	*/
	/************************************************/
    if ((cplex.getStatus() == IloAlgorithm::Optimal) || (cplex.getStatus() == IloAlgorithm::Feasible)){
        std::cout << "Optimization done in " << timeFinish - timeStart << " secs." << std::endl;
        std::cout << "Objective Function Value: " << cplex.getObjValue() << std::endl;
        //displayVariableValues();
        updatePath();
        displayOnPath();
        
        std::cout << "Number of cplex cuts: " << getNbCutsFromCplex() << std::endl;
    }
    else{
        std::cout << "Could not find a feasible solution..." << std::endl;
    }
}




/****************************************************************************************/
/*										Variables    									*/
/****************************************************************************************/

/* Define variables x[d][a] for every arc a in the extedend graph #d. */
void FlowForm::setVariables(IloBoolVarMatrix &var, IloIntVarArray &maxSliceFromLink, IloIntVar &maxSlice, IloModel &mod){
    for (int d = 0; d < getNbDemandsToBeRouted(); d++){ 
        var[d] = IloBoolVarArray(mod.getEnv(), countArcs(*vecGraph[d]));  
        for (ListDigraph::ArcIt a(*vecGraph[d]); a != INVALID; ++a){
            int arc = getArcIndex(a, d); 
            int label = getArcLabel(a, d); 
            int labelSource = getNodeLabel((*vecGraph[d]).source(a), d);
            int labelTarget = getNodeLabel((*vecGraph[d]).target(a), d);
            int slice = getArcSlice(a, d);
            std::ostringstream varName;
            varName << "x";
            varName << "(" + std::to_string(getToBeRouted_k(d).getId() + 1) + "," ;
            varName <<  std::to_string(labelSource + 1) + "," + std::to_string(labelTarget + 1) + ",";
            varName <<  std::to_string(slice + 1) + ")";
            IloInt upperBound = 1;
            if (instance.hasEnoughSpace(label, slice, getToBeRouted_k(d)) == false){
                upperBound = 0;
                std::cout << "STILL REMOVING VARIABLES IN CPLEX. \n" ;
            }
            var[d][arc] = IloBoolVar(mod.getEnv(), 0, upperBound, varName.str().c_str());
            mod.add(var[d][arc]);
            // std::cout << "Created variable: " << var[d][arc].getName() << std::endl;
        }
    }

    if(instance.getInput().getChosenObj() == Input::OBJECTIVE_METRIC_1p){
        for (int i = 0; i < instance.getNbEdges(); i++){
            std::string varName = "maxSlice(" + std::to_string(instance.getPhysicalLinkFromIndex(i).getId() + 1) + ")";
            IloInt lowerBound = instance.getPhysicalLinkFromIndex(i).getMaxUsedSlicePosition();
            IloInt upperBound = instance.getPhysicalLinkFromIndex(i).getNbSlices();
            maxSliceFromLink[i] = IloIntVar(mod.getEnv(), lowerBound, upperBound, varName.c_str());
            mod.add(maxSliceFromLink[i]);
        }
    }

    if(instance.getInput().getChosenObj() == Input::OBJECTIVE_METRIC_8){
        std::string varName = "maxSlice";
        IloInt lowerBound = instance.getMaxUsedSlicePosition();
        IloInt upperBound = instance.getMaxSlice();
        maxSlice = IloIntVar(mod.getEnv(), lowerBound, upperBound, varName.c_str()); 
        mod.add(maxSlice);
    }
}

/****************************************************************************************/
/*									Objective Function    								*/
/****************************************************************************************/

/* Set the objective Function */
void FlowForm::setObjective(IloBoolVarMatrix &var, IloIntVarArray &maxSliceFromLink, IloIntVar &maxSlice, IloModel &mod){
    IloExpr objective = getObjFunction(var, maxSliceFromLink, maxSlice, mod);
    mod.add(IloMinimize(mod.getEnv(), objective));
    objective.end();
}

/* Returns the objective function expression. */
IloExpr FlowForm::getObjFunction(IloBoolVarMatrix &var, IloIntVarArray &maxSliceFromLink, IloIntVar &maxSlice, IloModel &mod){
    IloExpr obj(mod.getEnv());
    
    if(instance.getInput().getChosenObj() == Input::OBJECTIVE_METRIC_0){
        IloInt CONSTANT = 0;
        obj += CONSTANT;
        return obj;
    }

    if(instance.getInput().getChosenObj() == Input::OBJECTIVE_METRIC_1p){
        for (int i = 0; i < instance.getNbEdges(); i++){
            obj += maxSliceFromLink[i];
        }
        return obj;
    }

    if(instance.getInput().getChosenObj() == Input::OBJECTIVE_METRIC_8){
        obj += maxSlice;
        return obj;
    }


    for (int d = 0; d < getNbDemandsToBeRouted(); d++){
        for (ListDigraph::ArcIt a(*vecGraph[d]); a != INVALID; ++a){
            int arc = getArcIndex(a, d);
            double coeff = getCoeff(a, d);
            //coeff += (instance.getInput().getInitialLagrangianMultiplier() * getArcLength(a, 0) );
            obj += coeff*var[d][arc];
        }
    }
    return obj;
}

/****************************************************************************************/
/*									Base Constraints    								*/
/****************************************************************************************/

/* Defines Source constraints. At most one arc leaves each node and exactly one arc leaves the source. */
void FlowForm::setSourceConstraints(IloBoolVarMatrix &var, IloModel &mod){
    for (int d = 0; d < getNbDemandsToBeRouted(); d++){  
        for (ListDigraph::NodeIt v(*vecGraph[d]); v != INVALID; ++v){
            int label = getNodeLabel(v, d);
            IloRange sourceConstraint = getSourceConstraint_d_n(var, mod, getToBeRouted_k(d), d, label);
            mod.add(sourceConstraint);
        } 
    }
}

/* Returns the source constraint associated with a demand and a node. */
IloRange FlowForm::getSourceConstraint_d_n(IloBoolVarMatrix &var, IloModel &mod, const Demand & demand, int d, int i){
    IloExpr exp(mod.getEnv());
    IloInt upperBound = 1;
    IloInt lowerBound = 0;
    for (ListDigraph::NodeIt v(*vecGraph[d]); v != INVALID; ++v){
        if (getNodeLabel(v, d) == i){
            for (ListDigraph::OutArcIt a((*vecGraph[d]), v); a != INVALID; ++a){
                int arc = getArcIndex(a, d); 
                exp += var[d][arc];
            }
        }
    }
    std::ostringstream constraintName;
    constraintName << "Source(" << i+1 << "," << demand.getId()+1 << ")";
    if (i == demand.getSource()){
        lowerBound = 1;
    }
    if (i == demand.getTarget()){
        upperBound = 0;
    }
    IloRange constraint(mod.getEnv(), lowerBound, exp, upperBound, constraintName.str().c_str());
    exp.end();
    return constraint;
}

/* Defines Flow Conservation constraints. If an arc enters a node, then an arc must leave it. */
void FlowForm::setFlowConservationConstraints(IloBoolVarMatrix &var, IloModel &mod){
    for (int d = 0; d < getNbDemandsToBeRouted(); d++){   
        for (ListDigraph::NodeIt v(*vecGraph[d]); v != INVALID; ++v){
            int label = getNodeLabel(v, d);
            if( (label != getToBeRouted_k(d).getSource()) && (label != getToBeRouted_k(d).getTarget()) ){
                IloRange st = getFlowConservationConstraint_i_d(var, mod, v, getToBeRouted_k(d), d);
                mod.add(st);
            }
        }
    }
}

/* Returns the flow conservation constraint associated with a demand and a node. */
IloRange FlowForm::getFlowConservationConstraint_i_d(IloBoolVarMatrix &var, IloModel &mod, ListDigraph::Node &v, const Demand & demand, int d){
    IloExpr exp(mod.getEnv());
    IloInt rhs = 0;
    for (ListDigraph::OutArcIt a((*vecGraph[d]), v); a != INVALID; ++a){
        int arc = getArcIndex(a, d); 
        exp += var[d][arc];
    }
    for (ListDigraph::InArcIt a((*vecGraph[d]), v); a != INVALID; ++a){
        int arc = getArcIndex(a, d); 
        exp += (-1)*var[d][arc];
    }
    std::ostringstream constraintName;
    int label = getNodeLabel(v, d);
    int slice = getNodeSlice(v, d);
    constraintName << "Flow(" << label+1 << "," << slice+1 << "," << demand.getId()+1 << ")";
    IloRange constraint(mod.getEnv(), rhs, exp, rhs, constraintName.str().c_str());
    exp.end();
    return constraint;
}

/* Defines Target constraints. Exactly one arc enters the target. */
void FlowForm::setTargetConstraints(IloBoolVarMatrix &var, IloModel &mod){
    for (int d = 0; d < getNbDemandsToBeRouted(); d++){   
        IloRange targetConstraint = getTargetConstraint_d(var, mod, getToBeRouted_k(d), d);
        mod.add(targetConstraint);
    }
}

/* Returns the target constraint associated with a demand. */
IloRange FlowForm::getTargetConstraint_d(IloBoolVarMatrix &var, IloModel &mod, const Demand & demand, int d){
    IloExpr exp(mod.getEnv());
    IloInt rhs = 1;
    for (ListDigraph::NodeIt v(*vecGraph[d]); v != INVALID; ++v){
        int label = getNodeLabel(v, d);
        if (label == demand.getTarget()){
            for (ListDigraph::InArcIt a((*vecGraph[d]), v); a != INVALID; ++a){
                int arc = getArcIndex(a, d); 
                exp += var[d][arc];
            }
        }
    }
    std::ostringstream constraintName;
    constraintName << "Target(" << demand.getId()+1 << ")";
    IloRange constraint(mod.getEnv(), rhs, exp, rhs, constraintName.str().c_str());
    exp.end();
    return constraint;
}

/* Defines Length constraints. Demands must be routed within a length limit. */
void FlowForm::setLengthConstraints(IloBoolVarMatrix &var, IloModel &mod){
    for (int d = 0; d < getNbDemandsToBeRouted(); d++){   
        IloRange lengthConstraint = getLengthConstraint(var, mod, getToBeRouted_k(d), d);
        mod.add(lengthConstraint);
    }
}

/* Returns the length constraint associated with a demand. */
IloRange FlowForm::getLengthConstraint(IloBoolVarMatrix &var, IloModel &mod, const Demand &demand, int d){
    IloExpr exp(mod.getEnv());
    double rhs = demand.getMaxLength();
    for (ListDigraph::ArcIt a(*vecGraph[d]); a != INVALID; ++a){
        int arc = getArcIndex(a, d); 
        double coeff = getArcLength(a, d);
        exp += coeff*var[d][arc];
    }
    std::ostringstream constraintName;
    constraintName << "Length(" << demand.getId()+1 << ")";
    IloRange constraint(mod.getEnv(), -IloInfinity, exp, rhs, constraintName.str().c_str());
    exp.end();
    return constraint;
}

/* Defines Non-Overlapping constraints. Demands must not overlap eachother's slices. */
void FlowForm::setNonOverlappingConstraints(IloBoolVarMatrix &var, IloModel &mod){
    for (int d1 = 0; d1 < getNbDemandsToBeRouted(); d1++){
        for (int d2 = 0; d2 < getNbDemandsToBeRouted(); d2++){
            if(d1 != d2){
                for (int i = 0; i < instance.getNbEdges(); i++){
                    for (int s = 0; s < instance.getPhysicalLinkFromIndex(i).getNbSlices(); s++){
                        IloRange nonOverlap = getNonOverlappingConstraint(var, mod, instance.getPhysicalLinkFromIndex(i).getId(), s, getToBeRouted_k(d1), d1, getToBeRouted_k(d2), d2);
                        mod.add(nonOverlap);
                    }
                }   
            }
        }
    }
}

/* Returns the non-overlapping constraint associated with an arc and a pair of demands. */
IloRange FlowForm::getNonOverlappingConstraint(IloBoolVarMatrix &var, IloModel &mod, int linkLabel, int slice, const Demand & demand1, int d1, const Demand & demand2, int d2){
    IloExpr exp(mod.getEnv());
    IloNum rhs = 1;
    for (ListDigraph::ArcIt a(*vecGraph[d1]); a != INVALID; ++a){
        if( (getArcLabel(a, d1) == linkLabel) && (getArcSlice(a, d1) == slice) ){
            int id = getArcIndex(a, d1);
            exp += var[d1][id];
        }
    }
    for (ListDigraph::ArcIt a(*vecGraph[d2]); a != INVALID; ++a){
        if( (getArcLabel(a, d2) == linkLabel) && (getArcSlice(a, d2) >= slice - demand1.getLoad() + 1) && (getArcSlice(a, d2) <= slice + demand2.getLoad() - 1) ){
            int id = getArcIndex(a, d2);
            exp += var[d2][id];
        }
    }
    std::ostringstream constraintName;
    constraintName << "NonOverlap(" << linkLabel+1 << "," << slice+1 << "," << demand1.getId()+1 << "," << demand2.getId()+1 << ")";
    IloRange constraint(mod.getEnv(), -IloInfinity, exp, rhs, constraintName.str().c_str());
    exp.end();
    return constraint;
}

/****************************************************************************************/
/*						Objective function related constraints    						*/
/****************************************************************************************/

/* Defines the Link's Max Used Slice Position constraints. The max used slice position on each link must be greater than every slice position used in the link. */
void FlowForm::setMaxUsedSlicePerLinkConstraints(IloBoolVarMatrix &var, IloIntVarArray &maxSlicePerLink, IloModel &mod){
    for (int i = 0; i < instance.getNbEdges(); i++){
        for (int d = 0; d < getNbDemandsToBeRouted(); d++){
            IloRange maxUsedSlicePerLinkConst = getMaxUsedSlicePerLinkConstraints(var, maxSlicePerLink, i, d, mod);
            mod.add(maxUsedSlicePerLinkConst);
        }
    }
}

IloRange FlowForm::getMaxUsedSlicePerLinkConstraints(IloBoolVarMatrix &var, IloIntVarArray &maxSlicePerLink, int linkIndex, int d, IloModel &mod){
    IloExpr exp(mod.getEnv());
    IloInt rhs = 0;
    int linkLabel = instance.getPhysicalLinkFromIndex(linkIndex).getId();
    for (ListDigraph::ArcIt a(*vecGraph[d]); a != INVALID; ++a){
        if (getArcLabel(a, d) == linkLabel){
            int index = getArcIndex(a, d);
            int slice = getArcSlice(a, d);
            exp += slice*var[d][index];
        }
    }
    exp += -maxSlicePerLink[linkIndex];
    
    std::ostringstream constraintName;
    constraintName << "MaxUsedSlicePerLink(" << linkLabel+1 << "," << getToBeRouted_k(d).getId()+1 << ")";
    IloRange constraint(mod.getEnv(), -IloInfinity, exp, rhs, constraintName.str().c_str());
    exp.end();
    return constraint;
}

/* Defines the Overall Max Used Slice Position constraints. */
void FlowForm::setMaxUsedSliceOverallConstraints(IloBoolVarMatrix &var, IloIntVar maxSliceOverall, IloModel &mod){
    for (int i = 0; i < instance.getNbEdges(); i++){
        for (int d = 0; d < getNbDemandsToBeRouted(); d++){
            IloRange maxUsedSliceOverallConst = getMaxUsedSliceOverallConstraints(var, maxSliceOverall, i, d, mod);
            mod.add(maxUsedSliceOverallConst);
        }
    }
}


IloRange FlowForm::getMaxUsedSliceOverallConstraints(IloBoolVarMatrix &var, IloIntVar &maxSlice, int linkIndex, int d, IloModel &mod){
    IloExpr exp(mod.getEnv());
    IloInt rhs = 0;
    int linkLabel = instance.getPhysicalLinkFromIndex(linkIndex).getId();
    for (ListDigraph::ArcIt a(*vecGraph[d]); a != INVALID; ++a){
        if (getArcLabel(a, d) == linkLabel){
            int index = getArcIndex(a, d);
            int slice = getArcSlice(a, d);
            exp += slice*var[d][index];
        }
    }
    exp += -maxSlice;
    
    std::ostringstream constraintName;
    constraintName << "MaxUsedSliceOverall(" << linkLabel+1 << "," << getToBeRouted_k(d).getId()+1 << ")";
    IloRange constraint(mod.getEnv(), -IloInfinity, exp, rhs, constraintName.str().c_str());
    exp.end();
    return constraint;
}



/****************************************************************************************/
/*						Improved Non-Overlapping constraints    						*/
/****************************************************************************************/

/* Defines the first set of Improved Non-Overlapping constraints. */
void FlowForm::setImprovedNonOverlappingConstraints_1(IloBoolVarMatrix &var, IloModel &mod){
    for (int k = 0; k < getNbLoadsToBeRouted(); k++){
        int load_k = getLoadsToBeRouted_k(k);
        for (int d2 = 0; d2 < getNbDemandsToBeRouted(); d2++){
            for (int i = 0; i < instance.getNbEdges(); i++){
                for (int s = 0; s < instance.getPhysicalLinkFromIndex(i).getNbSlices(); s++){
                    IloRange improvedNonOverlap1 = getImprovedNonOverlappingConstraint_1(var, mod, instance.getPhysicalLinkFromIndex(i).getId(), s, load_k, getToBeRouted_k(d2), d2);
                    mod.add(improvedNonOverlap1);
                }
            }
        }
    }
}

/* Returns the first improved non-overlapping constraint associated with an arc, a demand and a load. */
IloRange FlowForm::getImprovedNonOverlappingConstraint_1(IloBoolVarMatrix &var, IloModel &mod, int linkLabel, int slice, int min_load, const Demand & demand2, int d2){
	IloExpr exp(mod.getEnv());
    IloNum rhs = 1;
    
    for (int d1 = 0; d1 < getNbDemandsToBeRouted(); d1++){
        if ((d1 != d2) && (getToBeRouted_k(d1).getLoad() >= min_load)){
            for (ListDigraph::ArcIt a(*vecGraph[d1]); a != INVALID; ++a){
                if( (getArcLabel(a, d1) == linkLabel) && (getArcSlice(a, d1) == slice) ){
                    int index = getArcIndex(a, d1);
                    exp += var[d1][index];
                }
            }
        }
    }
    
    for (ListDigraph::ArcIt a(*vecGraph[d2]); a != INVALID; ++a){
        if( (getArcLabel(a, d2) == linkLabel) && (getArcSlice(a, d2) >= slice - min_load + 1) && (getArcSlice(a, d2) <= slice + demand2.getLoad() - 1) ){
            int index = getArcIndex(a, d2);
            exp += var[d2][index];
        }
    }
    
    std::ostringstream constraintName;
    constraintName << "ImprNonOverlap_1(" << linkLabel+1 << "," << slice+1 << "," << min_load << "," << demand2.getId()+1 << ")";
    IloRange constraint(mod.getEnv(), -IloInfinity, exp, rhs, constraintName.str().c_str());
    exp.end();
    return constraint;
}

/* Defines the second set of Improved Non-Overlapping constraints. */
void FlowForm::setImprovedNonOverlappingConstraints_2(IloBoolVarMatrix &var, IloModel &mod){
    for (int k1 = 0; k1 < getNbLoadsToBeRouted(); k1++){
        int load_k1 = getLoadsToBeRouted_k(k1);
        for (int k2 = 0; k2 < getNbLoadsToBeRouted(); k2++){
            int load_k2 = getLoadsToBeRouted_k(k2);
            for (int i = 0; i < instance.getNbEdges(); i++){
                for (int s = 0; s < instance.getPhysicalLinkFromIndex(i).getNbSlices(); s++){
                    IloRange improvedNonOverlap2 = getImprovedNonOverlappingConstraint_2(var, mod, instance.getPhysicalLinkFromIndex(i).getId(), s, load_k1, load_k2);
                    mod.add(improvedNonOverlap2);
                }
            }
        }
    }
}

/* Returns the first improved non-overlapping constraint associated with an arc, a demand and a load. */
IloRange FlowForm::getImprovedNonOverlappingConstraint_2(IloBoolVarMatrix &var, IloModel &mod, int linkLabel, int slice, int min_load1, int min_load2){
	IloExpr exp(mod.getEnv());
    IloNum rhs = 1;
    
    for (int d = 0; d < getNbDemandsToBeRouted(); d++){
        int demandLoad = getToBeRouted_k(d).getLoad();
        if ( (demandLoad >= min_load1) && (demandLoad <= min_load2 - 1) ){
            for (ListDigraph::ArcIt a(*vecGraph[d]); a != INVALID; ++a){
                if( (getArcLabel(a, d) == linkLabel) && (getArcSlice(a, d) >= slice)  && (getArcSlice(a, d) <= slice + min_load1 - 1) ){
                    int index = getArcIndex(a, d);
                    exp += var[d][index];
                }
            }
        }
    }

    
    for (int d = 0; d < getNbDemandsToBeRouted(); d++){
        int demandLoad = getToBeRouted_k(d).getLoad();
        if (demandLoad >= min_load2){
            for (ListDigraph::ArcIt a(*vecGraph[d]); a != INVALID; ++a){
                if( (getArcLabel(a, d) == linkLabel) && (getArcSlice(a, d) >= slice)  && (getArcSlice(a, d) <= slice + min_load2 - 1) ){
                    int index = getArcIndex(a, d);
                    exp += var[d][index];
                }
            }
        }
    }
    
    std::ostringstream constraintName;
    constraintName << "ImprNonOverlap_2(" << linkLabel+1 << "," << slice+1 << "," << min_load1 << "," << min_load2 << ")";
    IloRange constraint(mod.getEnv(), -IloInfinity, exp, rhs, constraintName.str().c_str());
    exp.end();
    return constraint;
}



/* Recovers the obtained MIP solution and builds a path for each demand on its associated graph from RSA. */
void FlowForm::updatePath(){
    for(int d = 0; d < getNbDemandsToBeRouted(); d++){
        for (ListDigraph::ArcIt a(*vecGraph[d]); a != INVALID; ++a){
            int arc = getArcIndex(a, d);
            if (cplex.getValue(x[d][arc]) >= 0.9){
                (*vecOnPath[d])[a] = getToBeRouted_k(d).getId();
            }
            else{
                (*vecOnPath[d])[a] = -1;
            }
        }
    }
}

/* Displays the value of each variable in the obtained solution. */
void FlowForm::displayVariableValues(){
    for(int d = 0; d < getNbDemandsToBeRouted(); d++){
        for (ListDigraph::ArcIt a(*vecGraph[d]); a != INVALID; ++a){
            int arc = getArcIndex(a, d);
            std::cout << x[d][arc].getName() << " = " << cplex.getValue(x[d][arc]) << "   ";
        }
        std::cout << std::endl;
    }
}

/** Displays the obtained paths. */
void FlowForm::displayOnPath(){
    for(int d = 0; d < getNbDemandsToBeRouted(); d++){ 
        std::cout << "For demand " << getToBeRouted_k(d).getId() + 1 << " : " << std::endl;
        for (ListDigraph::ArcIt a(*vecGraph[d]); a != INVALID; ++a){
            if ((*vecOnPath[d])[a] == getToBeRouted_k(d).getId()){
                displayArc(d, a);
            }
        }
    }
}

/****************************************************************************************/
/*										Destructor										*/
/****************************************************************************************/

/* Destructor. Clears the vectors of demands and links. */
FlowForm::~FlowForm(){
    x.end();
    maxSlicePerLink.end();
    maxSliceOverall.end();
}