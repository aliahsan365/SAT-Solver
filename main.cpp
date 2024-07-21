#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <vector>
using namespace std;

#define UNDEF -1
#define TRUE 1
#define FALSE 0


typedef unsigned int uint;

uint numVars;
uint numClauses;
vector<vector<int> > clauses;
vector<int> model;
vector<int> modelStack;
uint indexOfNextLitToPropagate;
uint decisionLevel;

int propagaciones;
int decisiones;
vector<vector<int> > positivas;
vector<vector<int> > negativas;
vector<int>  priority;



void readClauses() {
    // Skip comments
    char c = cin.get();
    while (c == 'c') {
        while (c != '\n') c = cin.get();
        c = cin.get();
    }
    // Read "cnf numVars numClauses"
    string aux;
    cin >> aux >> numVars >> numClauses;
    clauses.resize(numClauses);
    priority.resize(numVars + 1);
    negativas.resize(numVars + 1);
    positivas.resize(numVars + 1);
    // Read clauses
    for (uint i = 0; i < numClauses; ++i) {
        int lit;
        while (cin >> lit and lit != 0) {
            clauses[i].push_back(lit);
            ++priority[abs(lit)];
            if (lit < 0) negativas[-lit].push_back(i);
            else positivas[lit].push_back(i);
        }
    }
}



int currentValueInModel(int lit) {
    if (lit >= 0) return model[lit];
    else {
        if (model[-lit] == UNDEF) return UNDEF;
        else return 1 - model[-lit];
    }
}


void setLiteralToTrue(int lit) {
    modelStack.push_back(lit);
    if (lit > 0) model[lit] = TRUE;
    else model[-lit] = FALSE;
}



bool propagateGivesConflict() {
    while (indexOfNextLitToPropagate < modelStack.size()) {
        int lit = modelStack[indexOfNextLitToPropagate];
        ++indexOfNextLitToPropagate;
        vector<int> clausLit;
        if (lit < 0) clausLit = positivas[-lit];
        else clausLit = negativas[lit];
        for (uint i = 0; i < clausLit.size(); ++i) {
            int c = clausLit[i];
            bool someLitTrue = false;
            int numUndefs = 0;
            int lastLitUndef = 0;
            for (uint k = 0; not someLitTrue and k < clauses[c].size(); ++k) {
                int val = currentValueInModel(clauses[c][k]);
                if (val == TRUE) someLitTrue = true;
                else if (val == UNDEF) { ++numUndefs; lastLitUndef = clauses[c][k]; }
            }
            if (not someLitTrue and numUndefs == 0) {
                for (uint j = 0; j < clauses[c].size(); ++j) {
                    if (priority[abs(clauses[c][j])] + 2 > 1000) {
                        for (int i = 0; i < int(priority.size()); ++i)
                            priority[i] /= 10;
                        priority[abs(clauses[c][j])]++;
                    }
                    else priority[abs(clauses[c][j])] += 2;
                }
                return true;
            }
            else if (not someLitTrue and numUndefs == 1) { setLiteralToTrue(lastLitUndef); ++propagaciones; }
        }
    }
    return false;
}


void backtrack() {
    uint i = modelStack.size() - 1;
    int lit = 0;
    while (modelStack[i] != 0) { // 0 is the DL mark
        lit = modelStack[i];
        model[abs(lit)] = UNDEF;
        modelStack.pop_back();
        --i;
    }
    // at this point, lit is the last decision
    modelStack.pop_back(); // remove the DL mark
    --decisionLevel;
    indexOfNextLitToPropagate = modelStack.size();
    setLiteralToTrue(-lit);  // reverse last decision
}

// Heuristic for finding the next decision literal:
int getNextDecisionLiteral() {
    int next, max;
    next = max = 0;
    for (uint i = 1; i < priority.size(); ++i) {
        if (model[i] == UNDEF and priority[i] > max) {
            max = priority[i];
            next = i;
        }
    }
    decisiones++;
    return next;
}

void checkmodel() {
    for (uint i = 0; i < numClauses; ++i) {
        bool someTrue = false;
        for (uint j = 0; not someTrue and j < clauses[i].size(); ++j)
            someTrue = (currentValueInModel(clauses[i][j]) == TRUE);
        if (not someTrue) {
            cout << "Error in model, clause is not satisfied:";
            for (int j = 0; j < int(clauses[i].size()); ++j) cout << clauses[i][j] << " ";
            cout << endl;
            exit(1);
        }
    }
}



int main() {
    readClauses(); // reads numVars, numClauses and clauses
    model.resize(numVars + 1, UNDEF);
    indexOfNextLitToPropagate = 0;
    decisionLevel = 0;

    propagaciones = 0;
    decisiones = 0;

    // Take care of initial unit clauses, if any
    for (uint i = 0; i < numClauses; ++i)
        if (clauses[i].size() == 1) {
            int lit = clauses[i][0];
            int val = currentValueInModel(lit);
            if (val == FALSE) { cout << "UNSATISFIABLE" << endl; return 10; }
            else if (val == UNDEF) setLiteralToTrue(lit);
        }

    // DPLL algorithm
    while (true) {
        while (propagateGivesConflict()) {
            if (decisionLevel == 0) {
                cout << "UNSATISFIABLE" << endl;
                cout << "Numero de decisiones = " << decisiones << endl;
                cout << "Numero de propagaciones = " << propagaciones << endl;
                return 10;
            }
            backtrack();
        }
        int decisionLit = getNextDecisionLiteral();
        if (decisionLit == 0) {
            checkmodel();
            cout << "SATISFIABLE" << endl;
            cout << "Numero de decisiones = " << decisiones << endl;
            cout << "Numero de propagaciones = " << propagaciones << endl;
            return 20;
        }
        // start new decision level:
        modelStack.push_back(0);  // push mark indicating new DL
        ++indexOfNextLitToPropagate;
        ++decisionLevel;
        setLiteralToTrue(decisionLit);    // now push decisionLit on top of the mark
    }
}

