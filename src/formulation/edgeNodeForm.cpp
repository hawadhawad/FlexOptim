#include "edgeNodeForm.h"


/* Constructor. Builds the Online RSA mixed-integer program and solves it using CPLEX. */
EdgeNodeForm::EdgeNodeForm(const Instance &inst) : AbstractFormulation(inst){
    std::cout << "--- Edge-Node formulation has been chosen ---" << std::endl;
    this->setVariables();
    this->setConstraints();
    this->setObjectives();
    std::cout << "--- Edge-Node formulation has been defined ---" << std::endl;
}



/****************************************************************************************/
/*										Variables    									*/
/****************************************************************************************/

/* Define variables x[d][a] for every arc a in the extedend graph #d. */
void EdgeNodeForm::setVariables(){
    int nbEdges = countEdges(compactGraph);
    x.resize(nbEdges);
    for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
        int edge = getCompactEdgeLabel(e);
        int uId = getCompactNodeLabel(compactGraph.u(e)) + 1;
        int vId = getCompactNodeLabel(compactGraph.v(e)) + 1;
        x[edge].resize(getNbDemandsToBeRouted());  
        for (int k = 0; k < getNbDemandsToBeRouted(); k++){
            std::ostringstream varName;
            varName << "x";
            varName << "(" + std::to_string(uId) + "," ;
            varName << std::to_string(vId) + "," ;
            varName << std::to_string(getToBeRouted_k(k).getId() + 1) + ")";
            int upperBound = 1;
            int varId = getNbVar();
            x[edge][k] = Variable(varId, 0, upperBound, Variable::TYPE_BOOLEAN, 0, varName.str());
            incNbVar();
        }
    }
    std::cout << "X variables were created." << std::endl;

    // Variables z[s][k]
    z.resize(instance.getPhysicalLinkFromIndex(0).getNbSlices());
    for (int s = 0; s < instance.getPhysicalLinkFromIndex(0).getNbSlices(); s++){
        z[s].resize(getNbDemandsToBeRouted());  
        for (int k = 0; k < getNbDemandsToBeRouted(); k++){
            std::ostringstream varName;
            varName << "z";
            varName << "(" + std::to_string(s + 1) + "," ;
            varName <<  std::to_string(getToBeRouted_k(k).getId() + 1) + ")";
            int upperBound = 1;
            int varId = getNbVar();
            z[s][k] = Variable(varId, 0, upperBound, Variable::TYPE_BOOLEAN, 0, varName.str());
            incNbVar();
        }
    }
    std::cout << "Z variables were created." << std::endl;

    // Variables t[e][s][k]
    t.resize(nbEdges);
    for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
        int edge = getCompactEdgeLabel(e);
        t[edge].resize(instance.getPhysicalLinkFromIndex(edge).getNbSlices());  
        for (int s = 0; s < instance.getPhysicalLinkFromIndex(edge).getNbSlices(); s++){
            t[edge][s].resize(getNbDemandsToBeRouted());  
            for (int k = 0; k < getNbDemandsToBeRouted(); k++){
                std::ostringstream varName;
                varName << "t";
                varName << "(" + std::to_string(edge+1) + "," + std::to_string(s+1) + "," ;
                varName <<  std::to_string(getToBeRouted_k(k).getId() + 1) + ")";
                int upperBound = 1;
                int varId = getNbVar();
                t[edge][s][k] = Variable(varId, 0, upperBound, Variable::TYPE_BOOLEAN, 0, varName.str());
                incNbVar();
            }
        }
    }
    std::cout << "T variables were created." << std::endl;

    maxSlicePerLink.resize(nbEdges);
    for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
        int edge = getCompactEdgeLabel(e);
        std::string varName = "maxSlice(" + std::to_string(edge) + ")";
        int lowerBound = instance.getPhysicalLinkFromIndex(edge).getMaxUsedSlicePosition();
        int upperBound = instance.getPhysicalLinkFromIndex(edge).getNbSlices();
        int varId = getNbVar();
        maxSlicePerLink[edge] = Variable(varId, lowerBound, upperBound, Variable::TYPE_INTEGER, 0, varName);
        incNbVar();
    }
    std::cout << "MaxSlicePerLink variables were created." << std::endl;

    std::string varName = "maxSlice";
    int lowerBound = instance.getMaxUsedSlicePosition();
    int upperBound = instance.getMaxSlice();
    int varId = getNbVar();
    maxSliceOverall = Variable(varId, lowerBound, upperBound, Variable::TYPE_INTEGER, 0, varName);
    incNbVar();
    std::cout << "MaxSliceOverall variable was created." << std::endl;
}

VarArray EdgeNodeForm::getVariables(){
    VarArray vec;
    vec.resize(getNbVar());
    for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
        int edge = getCompactEdgeLabel(e);
        for (int k = 0; k < getNbDemandsToBeRouted(); k++){
            int pos = x[edge][k].getId();
            vec[pos] = x[edge][k];
        }
    }
    for (int s = 0; s < instance.getPhysicalLinkFromIndex(0).getNbSlices(); s++){
        for (int k = 0; k < getNbDemandsToBeRouted(); k++){
            int pos = z[s][k].getId();
            vec[pos] = z[s][k];
        }
    }
    for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
        int edge = getCompactEdgeLabel(e);
        for (int s = 0; s < instance.getPhysicalLinkFromIndex(edge).getNbSlices(); s++){
            for (int k = 0; k < getNbDemandsToBeRouted(); k++){
                int pos = t[edge][s][k].getId();
                vec[pos] = t[edge][s][k];
            }
        }
    }
    for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
        int edge = getCompactEdgeLabel(e);
        int pos = maxSlicePerLink[edge].getId();
        vec[pos] = maxSlicePerLink[edge];
    }  
    int pos = maxSliceOverall.getId();
    vec[pos] = maxSliceOverall;
    return vec;
}


void EdgeNodeForm::setVariableValues(const std::vector<double> &vals){
    for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
        int edge = getCompactEdgeLabel(e);
        for (int k = 0; k < getNbDemandsToBeRouted(); k++){
            int pos = x[edge][k].getId();
            double newValue = vals[pos];
            x[edge][k].setVal(newValue);
        }
    }
    for (int s = 0; s < instance.getPhysicalLinkFromIndex(0).getNbSlices(); s++){
        for (int k = 0; k < getNbDemandsToBeRouted(); k++){
            int pos = z[s][k].getId();
            double newValue = vals[pos];
            z[s][k].setVal(newValue);
        }
    }
    for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
        int edge = getCompactEdgeLabel(e);
        for (int s = 0; s < instance.getPhysicalLinkFromIndex(edge).getNbSlices(); s++){
            for (int k = 0; k < getNbDemandsToBeRouted(); k++){
                int pos = t[edge][s][k].getId();
                double newValue = vals[pos];
                t[edge][s][k].setVal(newValue);
            }
        }
    }
    for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
        int edge = getCompactEdgeLabel(e);
        int pos = maxSlicePerLink[edge].getId();
        double newValue = vals[pos];
        maxSlicePerLink[edge].setVal(newValue);
    }
    int pos = maxSliceOverall.getId();
    double newValue = vals[pos];
    maxSliceOverall.setVal(newValue);
}
/****************************************************************************************/
/*									Objective Function    								*/
/****************************************************************************************/

/* Set the objective Function */
void EdgeNodeForm::setObjectives(){
    std::vector<Input::ObjectiveMetric> chosenObjectives = instance.getInput().getChosenObj();
    objectiveSet.resize(chosenObjectives.size());
    for (unsigned int i = 0; i < chosenObjectives.size(); i++){
        Expression myObjective = this->getObjFunctionFromMetric(chosenObjectives[i]);
        objectiveSet[i].setExpression(myObjective);
        objectiveSet[i].setDirection(ObjectiveFunction::DIRECTION_MIN);
        std::cout << "Objective " << chosenObjectives[i] << " has been defined." << std::endl;
    }
}

/* Returns the objective function expression. */
Expression EdgeNodeForm::getObjFunctionFromMetric(Input::ObjectiveMetric chosenObjective){
    Expression obj;
    switch (chosenObjective){
        case Input::OBJECTIVE_METRIC_0:{
            break;
        }
        case Input::OBJECTIVE_METRIC_1:{
            for (int s = 0; s < instance.getPhysicalLinkFromIndex(0).getNbSlices(); s++){
                for (int k = 0; k < getNbDemandsToBeRouted(); k++){
                    Term term(z[s][k], s+1);
                    obj.addTerm(term);
                }
            }
            break;
        }
        case Input::OBJECTIVE_METRIC_1p:{
            for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
                int edge = getCompactEdgeLabel(e);
                Term term(maxSlicePerLink[edge], 1);
                obj.addTerm(term);
            }
            break;
        }
        case Input::OBJECTIVE_METRIC_2:{
            for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
                int edge = getCompactEdgeLabel(e);
                for (int k = 0; k < getNbDemandsToBeRouted(); k++){
                    Term term(x[edge][k], 1);
                    obj.addTerm(term);
                }
            }
            break;
        }
        case Input::OBJECTIVE_METRIC_2p:{
            for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
                int edge = getCompactEdgeLabel(e);
                for (int k = 0; k < getNbDemandsToBeRouted(); k++){
                    Term term(x[edge][k], getToBeRouted_k(k).getLoad());
                    obj.addTerm(term);
                }
            }
            break;
        }
        case Input::OBJECTIVE_METRIC_4:{
            for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
                int edge = getCompactEdgeLabel(e);
                for (int k = 0; k < getNbDemandsToBeRouted(); k++){
                    Term term(x[edge][k], getCompactLength(e));
                    obj.addTerm(term);
                }
            }
            break;
        }
        case Input::OBJECTIVE_METRIC_8:{
            Term term(maxSliceOverall, 1);
            obj.addTerm(term);
            break;
        }
        default:{
            std::cout << "ERROR: Objective '" << chosenObjective << "' is not known." << std::endl;
            exit(0);
            break;
        }
    }
    return obj;
}

/****************************************************************************************/
/*									Base Constraints    								*/
/****************************************************************************************/

void EdgeNodeForm::setConstraints(){
    setOriginConstraints();
	setDestinationConstraints();
	setDegreeConstraints();
    setTransmissionReachConstraints();
    setChannelSelectionConstraints();
    setForbiddenSlotConstraints();
	setEdgeSlotConstraints();
	setDemandEdgeSlotConstraints();
    setNonOverlappingConstraints();

    setMaxUsedSlicePerLinkConstraints();
    setMaxUsedSliceOverallConstraints();

} 

/* Defines Origin constraints. At most one arc leaves each node and exactly one arc leaves the source. */
void EdgeNodeForm::setOriginConstraints(){
    for (int k = 0; k < getNbDemandsToBeRouted(); k++){  
        Constraint originConstraint = getOriginConstraint_k(k);
        constraintSet.push_back(originConstraint);
    }
    std::cout << "Origin constraints have been defined..." << std::endl;
}

/* Returns the source constraint associated with a demand and a node. */
Constraint EdgeNodeForm::getOriginConstraint_k(int k){
    Expression exp;
    int upperBound = 1;
    int lowerBound = 1;
    int origin = getToBeRouted_k(k).getSource();
    for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
        int labelU = getCompactNodeLabel(compactGraph.u(e));
        int labelV = getCompactNodeLabel(compactGraph.v(e));
        if (labelU == origin || labelV == origin){
            int edge = getCompactEdgeLabel(e);
            Term term(x[edge][k], 1);
            exp.addTerm(term);
        }
    }
    std::ostringstream constraintName;
    constraintName << "Origin(" << getToBeRouted_k(k).getId()+1 << ")";
    Constraint constraint(lowerBound, exp, upperBound, constraintName.str());
    return constraint;
}

/* Defines Destination constraints. Exactly one arc enters the target. */
void EdgeNodeForm::setDestinationConstraints(){
    for (int d = 0; d < getNbDemandsToBeRouted(); d++){   
        Constraint destinationConstraint = getDestinationConstraint_k(d);
        constraintSet.push_back(destinationConstraint);
    }
    std::cout << "Destination constraints have been defined..." << std::endl;
}

/* Returns the destination constraint associated with a demand. */
Constraint EdgeNodeForm::getDestinationConstraint_k(int k){
    Expression exp;
    int upperBound = 1;
    int lowerBound = 1;
    int destination = getToBeRouted_k(k).getTarget();
    for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
        int labelU = getCompactNodeLabel(compactGraph.u(e));
        int labelV = getCompactNodeLabel(compactGraph.v(e));
        if (labelU == destination || labelV == destination){
            int edge = getCompactEdgeLabel(e);
            Term term(x[edge][k], 1);
            exp.addTerm(term);
        }
    }
    std::ostringstream constraintName;
    constraintName << "Destination(" << getToBeRouted_k(k).getId()+1 << ")";
    Constraint constraint(lowerBound, exp, upperBound, constraintName.str());
    return constraint;
}

/* Defines Degree constraints. At most two edges are adjacent to any node. */
void EdgeNodeForm::setDegreeConstraints(){
    for (int d = 0; d < getNbDemandsToBeRouted(); d++){  
        for (ListGraph::NodeIt v(compactGraph); v != INVALID; ++v){
            int node = getCompactNodeLabel(v);
            if (node != getToBeRouted_k(d).getSource() && node != getToBeRouted_k(d).getTarget()){
                Constraint degreeConstraint = getDegreeConstraint_k(d, v);
                constraintSet.push_back(degreeConstraint);
            }
        }
    }
    std::cout << "Degree constraints have been defined..." << std::endl;
}

/* Returns the degree constraint associated with a demand k and a node v. */
Constraint EdgeNodeForm::getDegreeConstraint_k(int k, const ListGraph::Node &v){
    Expression exp;
    int upperBound = 2;
    int lowerBound = 0;
    for (ListGraph::IncEdgeIt e(compactGraph, v); e != INVALID; ++e){
        int edge = getCompactEdgeLabel(e);
        Term term(x[edge][k], 1);
        exp.addTerm(term);
    }
    std::ostringstream constraintName;
    constraintName << "Degree(" << getToBeRouted_k(k).getId()+1 << "," << getCompactNodeLabel(v)+1 << ")";
    Constraint constraint(lowerBound, exp, upperBound, constraintName.str());
    return constraint;
}

/* Defines Transmission-Reach constraints. Demands must be routed within a length limit. */
void EdgeNodeForm::setTransmissionReachConstraints(){
    for (int d = 0; d < getNbDemandsToBeRouted(); d++){   
        Constraint transmissionReach = getTransmissionReachConstraint_k(d);
        constraintSet.push_back(transmissionReach);
    }
    std::cout << "Transmission reach constraints have been defined..." << std::endl;
}

/* Returns the Transmission-Reach constraint associated with a demand. */
Constraint EdgeNodeForm::getTransmissionReachConstraint_k(int k){
    Expression exp;
    double upperBound = getToBeRouted_k(k).getMaxLength();
    int lowerBound = 0;
    for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
        int edge = getCompactEdgeLabel(e);
        double coeff = getCompactLength(e);
        Term term(x[edge][k], coeff);
        exp.addTerm(term);
    }
    std::ostringstream constraintName;
    constraintName << "TrasmissionReach(" << getToBeRouted_k(k).getId()+1 << ")";
    Constraint constraint(lowerBound, exp, upperBound, constraintName.str());
    return constraint;
}

/* Defines Channel-Selection constraints. Only one slot can be the last assigned to a demand. */
void EdgeNodeForm::setChannelSelectionConstraints(){
    for (int d = 0; d < getNbDemandsToBeRouted(); d++){   
        Constraint channelSelection = getChannelSelectionConstraint_k(d);
        constraintSet.push_back(channelSelection);
    }
    std::cout << "Channel selection constraints have been defined..." << std::endl;
}

/* Returns the channel-selection constraint associated with a demand. */
Constraint EdgeNodeForm::getChannelSelectionConstraint_k(int k){
    Expression exp;
    int upperBound = 1;
    int lowerBound = 1;
    int load_k = getToBeRouted_k(k).getLoad();
    for (int s = load_k-1; s < instance.getPhysicalLinkFromIndex(0).getNbSlices(); s++){
        Term term(z[s][k], 1);
        exp.addTerm(term);
    }
    std::ostringstream constraintName;
    constraintName << "ChannelSelection(" << getToBeRouted_k(k).getId()+1 << ")";
    Constraint constraint(lowerBound, exp, upperBound, constraintName.str());
    return constraint;
}

/* Defines Forbidden-Slot constraints. */
void EdgeNodeForm::setForbiddenSlotConstraints(){
    for (int d = 0; d < getNbDemandsToBeRouted(); d++){   
        Constraint forbiddenSlot = getForbiddenSlotConstraint_k(d);
        constraintSet.push_back(forbiddenSlot);
    }
    std::cout << "Forbidden slots constraints have been defined..." << std::endl;
}

/* Returns the Forbidden-Slot constraint associated with a demand. */
Constraint EdgeNodeForm::getForbiddenSlotConstraint_k(int k){
    Expression exp;
    int upperBound = 0;
    int lowerBound = 0;
    int load_k = getToBeRouted_k(k).getLoad();
    for (int s = 0; s < load_k-1; s++){
        Term term(z[s][k], 1);
        exp.addTerm(term);
    }
    std::ostringstream constraintName;
    constraintName << "ForbiddenSlot(" << getToBeRouted_k(k).getId()+1 << ")";
    Constraint constraint(lowerBound, exp, upperBound, constraintName.str());
    return constraint;
}

/* Defines Edge-Slot constraints. */
void EdgeNodeForm::setEdgeSlotConstraints(){
    for (int d = 0; d < getNbDemandsToBeRouted(); d++){
        for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
            int edge = getCompactEdgeLabel(e);
            Constraint edgeSlot = getEdgeSlotConstraint_k_e(d, edge);
            constraintSet.push_back(edgeSlot);
        }
    }
    std::cout << "Edge-slots constraints have been defined..." << std::endl;
}

/* Returns the Edge-Slot constraint associated with a demand and an edge. */
Constraint EdgeNodeForm::getEdgeSlotConstraint_k_e(int k, int e){
    Expression exp;
    int upperBound = 0;
    int lowerBound = 0;
    int load_k = getToBeRouted_k(k).getLoad();
    for (int s = 0; s < instance.getPhysicalLinkFromIndex(e).getNbSlices(); s++){
        Term term(t[e][s][k], 1);
        exp.addTerm(term);
    }
    Term term(x[e][k], -load_k);
    exp.addTerm(term);

    std::ostringstream constraintName;
    constraintName << "EdgeSlot(" << getToBeRouted_k(k).getId()+1 << "," << e+1 << ")";
    Constraint constraint(lowerBound, exp, upperBound, constraintName.str());
    return constraint;
}

/* Defines Demand-Edge-Slot constraints. */
void EdgeNodeForm::setDemandEdgeSlotConstraints(){
    for (int d = 0; d < getNbDemandsToBeRouted(); d++){
        for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
            int edge = getCompactEdgeLabel(e);
            for (int s = 0; s < instance.getPhysicalLinkFromIndex(edge).getNbSlices(); s++){
                Constraint demandEdgeSlot = getDemandEdgeSlotConstraint_k_e_s(d, edge, s);
                constraintSet.push_back(demandEdgeSlot); 
            }
        }
    }
    std::cout << "Demand-edge-slots constraints have been defined..." << std::endl;
}

/* Returns the Demand-Edge-Slot constraint associated with a demand, an edge and a slice. */
Constraint EdgeNodeForm::getDemandEdgeSlotConstraint_k_e_s(int k, int e, int s){
    Expression exp;
    int upperBound = 1;
    double lowerBound = -INFTY;
    int load_k = getToBeRouted_k(k).getLoad();
    int maxS = s + load_k - 1;
    if (instance.getPhysicalLinkFromIndex(e).getNbSlices() - 1 < s + load_k - 1){
        maxS = instance.getPhysicalLinkFromIndex(e).getNbSlices() - 1;
    }
    
    Term termX(x[e][k], 1);
    exp.addTerm(termX);
    for (int slice = s; slice <= maxS; slice++){
        Term termZ(z[slice][k], 1);
        exp.addTerm(termZ);
    }
    Term termT(t[e][s][k], -1);
    exp.addTerm(termT);
    
    std::ostringstream constraintName;
    constraintName << "DemandEdgeSlot(" << getToBeRouted_k(k).getId()+1 << "," << e+1 << "," << s+1 << ")";
    Constraint constraint(lowerBound, exp, upperBound, constraintName.str());
    return constraint;
}

/* Defines Non-Overlapping constraints. Demands must not overlap eachother's slices. */ 
void EdgeNodeForm::setNonOverlappingConstraints(){
    for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
        int edge = getCompactEdgeLabel(e);
        for (int s = 0; s < instance.getPhysicalLinkFromIndex(edge).getNbSlices(); s++){
            Constraint nonOverlap = getNonOverlappingConstraint_e_s(edge, s);
            constraintSet.push_back(nonOverlap);
        }
    }
    std::cout << "Non-overlapping constraints have been defined..." << std::endl;
}

/* Returns the non-overlapping constraint associated with an edge and a slice. */
Constraint EdgeNodeForm::getNonOverlappingConstraint_e_s(int e, int s){
    Expression exp;
    int upperBound = 1;
    int lowerBound = 0;
    for (int k = 0; k < getNbDemandsToBeRouted(); k++){
        Term term(t[e][s][k], 1);
        exp.addTerm(term);
    }
    std::ostringstream constraintName;
    constraintName << "NonOverlap(" << e+1 << "," << s+1 << ")";
    Constraint constraint(lowerBound, exp, upperBound, constraintName.str());
    return constraint;
}

/****************************************************************************************/
/*						Objective function related constraints    						*/
/****************************************************************************************/

/* Defines the Link's Max Used Slice Position constraints. The max used slice position on each link must be greater than every slice position used in the link. */
void EdgeNodeForm::setMaxUsedSlicePerLinkConstraints(){
    for (int k = 0; k < getNbDemandsToBeRouted(); k++){
        for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
            int edge = getCompactEdgeLabel(e);
            for (int s = 0; s < instance.getPhysicalLinkFromIndex(edge).getNbSlices(); s++){
                Constraint maxUsedSlicePerLinkConstraint = getMaxUsedSlicePerLinkConstraints(k, edge, s);
                constraintSet.push_back(maxUsedSlicePerLinkConstraint);
            }
        }
    }
    std::cout << "Max Used Slice Per Link constraints have been defined..." << std::endl;
}

Constraint EdgeNodeForm::getMaxUsedSlicePerLinkConstraints(int k, int e, int s){
    Expression exp;
    int upperBound = 0;
    Term termT(t[e][s][k], s+1);
    exp.addTerm(termT);
    
    Term term(maxSlicePerLink[e], -1);
    exp.addTerm(term);
    
    std::ostringstream constraintName;
    constraintName << "MaxUsedSlicePerLink(" << getToBeRouted_k(k).getId()+1 << "," << e+1 << "," << s+1 << ")";
    Constraint constraint(-INFTY, exp, upperBound, constraintName.str());
    return constraint;
}

/* Defines the Overall Max Used Slice Position constraints. */
void EdgeNodeForm::setMaxUsedSliceOverallConstraints(){
    for (int d = 0; d < getNbDemandsToBeRouted(); d++){
        Constraint maxUsedSliceOverallConst = getMaxUsedSliceOverallConstraints(d);
        constraintSet.push_back(maxUsedSliceOverallConst);
    }
    std::cout << "Max Used Slice Overall constraints have been defined..." << std::endl;
}

Constraint EdgeNodeForm::getMaxUsedSliceOverallConstraints(int k){
    Expression exp;
    int rhs = 0;
    for (int s = 0; s < instance.getPhysicalLinkFromIndex(0).getNbSlices(); s++){
        Term term(z[s][k], s+1);
        exp.addTerm(term);
    }
    Term term(maxSliceOverall, -1);
    exp.addTerm(term);

    std::ostringstream constraintName;
    constraintName << "MaxUsedSliceOverall(" << getToBeRouted_k(k).getId()+1 << ")";
    Constraint constraint(-INFTY, exp, rhs, constraintName.str());
    return constraint;
}


/****************************************************************************************/
/*						                Methods                 						*/
/****************************************************************************************/
/* Recovers the obtained MIP solution and builds a path for each demand on its associated graph from RSA. */
void EdgeNodeForm::updatePath(const std::vector<double> &vals){
    setVariableValues(vals);
    // Reinitialize OnPath
    //std::cout << "Enter update." << std::endl;
    for(int d = 0; d < getNbDemandsToBeRouted(); d++){
        for (ListDigraph::ArcIt a(*vecGraph[d]); a != INVALID; ++a){
                (*vecOnPath[d])[a] = -1;
        }
    }
    for(int d = 0; d < getNbDemandsToBeRouted(); d++){
        for (ListDigraph::ArcIt a(*vecGraph[d]); a != INVALID; ++a){
            int edge = getArcLabel(a, d);
            int slice = getArcSlice(a, d);
            if ((x[edge][d].getVal() >= 0.9) && (z[slice][d].getVal() >= 0.9)){
                (*vecOnPath[d])[a] = getToBeRouted_k(d).getId();
            }
            else{
                (*vecOnPath[d])[a] = -1;
            }
        }
    }
    //std::cout << "Leave update." << std::endl;
}


/* Displays the value of each variable in the obtained solution. */
void EdgeNodeForm::displayVariableValues(){
    for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
        int edge = getCompactEdgeLabel(e);
        for (int k = 0; k < getNbDemandsToBeRouted(); k++){
            std::cout << x[edge][k].getName() << " = " << x[edge][k].getVal() << "   ";
        }
        std::cout << std::endl;
    }
    for (int s = 0; s < instance.getPhysicalLinkFromIndex(0).getNbSlices(); s++){
        for (int k = 0; k < getNbDemandsToBeRouted(); k++){
            std::cout << z[s][k].getName() << " = " << z[s][k].getVal() << "   ";
        }
        std::cout << std::endl;
    }
    for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
        int edge = getCompactEdgeLabel(e);
        for (int s = 0; s < instance.getPhysicalLinkFromIndex(edge).getNbSlices(); s++){
            for (int k = 0; k < getNbDemandsToBeRouted(); k++){
                std::cout << t[edge][s][k].getName() << " = " << t[edge][s][k].getVal() << "   ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
        int edge = getCompactEdgeLabel(e);
        std::cout << maxSlicePerLink[edge].getName() << " = " << maxSlicePerLink[edge].getVal() << "   ";
    }
    std::cout << std::endl;
    std::cout << maxSliceOverall.getName() << " = " << maxSliceOverall.getVal() << std::endl;
}

Constraint EdgeNodeForm::solveSeparationProblemFract(const std::vector<double> &solution){
    setVariableValues(solution);
    
    for(unsigned int d = 0; d < getNbDemandsToBeRouted(); d++){
        int origin = getToBeRouted_k(d).getSource();
        int destination = getToBeRouted_k(d).getTarget();
        ListGraph::Node SOURCE = getCompactNodeFromLabel(origin);
        ListGraph::Node TARGET = getCompactNodeFromLabel(destination);
        
        //std::cout << "Checking path of demand " << d << ". From " << origin+1 << " to " << destination+1 << std::endl;
        //displaySolution_d(context, d);
        ListGraph::EdgeMap<double> capacityMap(compactGraph, 0.0);
        for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
            int edge = getCompactEdgeLabel(e);
            capacityMap[e] = x[edge][d].getVal();
        }
        Preflow< ListGraph, ListGraph::EdgeMap<double> > maxFlow(compactGraph, capacityMap, SOURCE, TARGET);
        maxFlow.runMinCut();

        Expression expr;
        double exprValue = 0.0;
        for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
            if (maxFlow.minCut(compactGraph.u(e)) != maxFlow.minCut(compactGraph.v(e))){
                int edge = getCompactEdgeLabel(e);
                expr.addTerm(Term(x[edge][d], 1));
                exprValue += x[edge][d].getVal();
            }
        }

        if (exprValue <= (1 - EPS)){
            return Constraint(1, expr, INFTY);
        }
    }
    
    Expression empty;
    Constraint noCut(0, empty, 0);
    return noCut;
}
Constraint EdgeNodeForm::solveSeparationProblemInt(const std::vector<double> &solution){
    setVariableValues(solution);
    for (unsigned int d = 0; d < getNbDemandsToBeRouted(); d++){
        int origin = getToBeRouted_k(d).getSource();
        int destination = getToBeRouted_k(d).getTarget();
        ListGraph::Node SOURCE = getCompactNodeFromLabel(origin);
        ListGraph::Node TARGET = getCompactNodeFromLabel(destination);
        //std::cout << "Checking path of demand " << d << ". From " << origin+1 << " to " << destination+1 << std::endl;
        
        //displaySolution_d(context, d);
        if (TARGET == INVALID || SOURCE == INVALID){
            std::cout << "ERROR: Could not find source or target from demand inside lazy constraints callback." << std::endl;
            exit(0);
        }
        
        ListGraph::Node currentNode = SOURCE;
        std::vector<int> setOfNodes;
        //setOfNodes.push_back(origin);
        bool cutFound = false;
        int previousNodeLabel = -1;
        while (currentNode != TARGET && !cutFound){
            //std::cout << "Add Node: " << (*nodeLabel)[currentNode]+1 << std::endl;
            setOfNodes.push_back(getCompactNodeLabel(currentNode));
            ListGraph::Edge nextEdge = INVALID;
            for (ListGraph::IncEdgeIt e(compactGraph, currentNode); e != INVALID; ++e){
                int edge = getCompactEdgeLabel(e);
                if (getCompactNodeLabel(compactGraph.u(e)) != previousNodeLabel && getCompactNodeLabel(compactGraph.v(e)) != previousNodeLabel){
                    if (x[edge][d].getVal() >= 1 - EPS){
                        nextEdge = e;
                    }
                }
            }
            
            if (nextEdge == INVALID){
                // create cut
                //displaySet(setOfNodes);
                Expression exp;
                for (ListGraph::EdgeIt e(compactGraph); e != INVALID; ++e){
                    int u = getCompactNodeLabel(compactGraph.u(e));
                    int v = getCompactNodeLabel(compactGraph.v(e));
                    if (std::find(setOfNodes.begin(), setOfNodes.end(), u) != setOfNodes.end()){
                        if (std::find(setOfNodes.begin(), setOfNodes.end(), v) == setOfNodes.end()){
                            // add edge to cut
                            int edge = getCompactEdgeLabel(e);
                            exp.addTerm(Term(x[edge][d], 1));
                        }
                    }
                    else{
                        if (std::find(setOfNodes.begin(), setOfNodes.end(), v) != setOfNodes.end()){
                            // add edge to cut
                            int edge = getCompactEdgeLabel(e);
                            exp.addTerm(Term(x[edge][d], 1));
                        }
                    }
                }
                
                //std::cout << "Adding lazy constraint: " << exp << " >= 1" << std::endl;
                //std::cout << "Candidate has: " << context.getCandidateValue(exp) << " > 1" << std::endl;
                return Constraint(1, exp, INFTY);
                cutFound = true;
            }
            else{
                //displayEdge(nextEdge);
                previousNodeLabel = getCompactNodeLabel(currentNode);
                if(getCompactNodeLabel(currentNode) == getCompactNodeLabel(compactGraph.u(nextEdge))){
                    currentNode = compactGraph.v(nextEdge);
                }
                else{
                    currentNode = compactGraph.u(nextEdge);
                }
            }
        }
    }
    Expression exp;
    Constraint cut(0, exp, 0);
    return cut;
}

Expression EdgeNodeForm::separationGNPY(const std::vector<double> &solution){
    setVariableValues(solution);
    Expression cut;
    
    if (cut.getNbTerms() > 0){
        std::cout << "A separating cut was found." << std::endl;
    }
    else{
        std::cout << "The solution is valid." << std::endl;
    }
    return cut;
}
/****************************************************************************************/
/*										Destructor										*/
/****************************************************************************************/

/* Destructor. Clears the vectors of demands and links. */
EdgeNodeForm::~EdgeNodeForm(){
    x.clear();
    z.clear();
    t.clear();
    maxSlicePerLink.clear();
}