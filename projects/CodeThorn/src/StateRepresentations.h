#ifndef STATE_REPRESENTATIONS_H
#define STATE_REPRESENTATIONS_H

/*************************************************************
 * Copyright: (C) 2012 by Markus Schordan                    *
 * Author   : Markus Schordan                                *
 * License  : see file LICENSE in the CodeThorn distribution *
 *************************************************************/

#define USER_DEFINED_PSTATE_COMP

#include <string>
#include <set>
#include <map>
#include <utility>
#include "Labeler.h"
#include "CFAnalysis.h"
#include "AType.h"
#include "VariableIdMapping.h"
#include "ConstraintRepresentation.h"

using namespace std;

typedef int PStateId;
typedef int EStateId;

#include "HashFun.h"
#include "HSetMaintainer.h"

using CodeThorn::CppCapsuleAValue;
using CodeThorn::ConstraintSet;
using CodeThorn::ConstraintSetMaintainer;
using SPRAY::Edge;

using namespace SPRAY;

namespace CodeThorn {

/*! 
  * \author Markus Schordan
  * \date 2012.
 */
class PState : public map<VariableId,CodeThorn::CppCapsuleAValue> {
 public:
    PState() {
    }
  friend ostream& operator<<(ostream& os, const PState& value);
  friend istream& operator>>(istream& os, PState& value);
  bool varExists(VariableId varId) const;
  bool varIsConst(VariableId varId) const;
  bool varIsTop(VariableId varId) const;
  AValue varValue(VariableId varId) const;
  string varValueToString(VariableId varId) const;
  void deleteVar(VariableId varname);
  long memorySize() const;
  void fromStream(istream& is);
  void toStream(ostream& os) const;
  string toString() const;
  string toString(VariableIdMapping* variableIdMapping) const;
  void setAllVariablesToTop();
  void setAllVariablesToValue(CodeThorn::CppCapsuleAValue val);
  void setVariableToTop(VariableId varId);
  void setVariableToValue(VariableId varId, CodeThorn::CppCapsuleAValue val);
  VariableIdSet getVariableIds() const;
};

  ostream& operator<<(ostream& os, const PState& value);
  istream& operator>>(istream& os, PState& value);

  typedef set<const PState*> PStatePtrSet;

#ifdef USE_CUSTOM_HSET
class PStateHashFun {
   public:
    PStateHashFun(long prime=9999991) : tabSize(prime) {}
    long operator()(PState s) const {
      unsigned int hash=1;
      for(PState::iterator i=s.begin();i!=s.end();++i) {
        hash=((hash<<8)+((long)(*i).second.getValue().hash()))^hash;
      }
      return long(hash) % tabSize;
    }
      long tableSize() const { return tabSize;}
   private:
    long tabSize;
};
#else
class PStateHashFun {
   public:
    PStateHashFun() {}
    long operator()(PState* s) const {
      unsigned int hash=1;
      for(PState::iterator i=s->begin();i!=s->end();++i) {
        hash=((hash<<8)+((long)(*i).second.getValue().hash()))^hash;
      }
      return long(hash);
    }
   private:
};
#endif
class PStateEqualToPred {
   public:
    PStateEqualToPred() {}
    bool operator()(PState* s1, PState* s2) const {
      if(s1->size()!=s2->size()) {
        return false;
      } else {
        for(PState::iterator i1=s1->begin(), i2=s2->begin();i1!=s1->end();(++i1,++i2)) {
          if(*i1!=*i2)
            return false;
        }
      }
      return true;
    }
   private:
};

/*! 
  * \author Markus Schordan
  * \date 2012.
 */
 class PStateSet : public HSetMaintainer<PState,PStateHashFun,PStateEqualToPred> {
 public:
  typedef HSetMaintainer<PState,PStateHashFun,PStateEqualToPred>::ProcessingResult ProcessingResult;
  string toString();
  PStateId pstateId(const PState* pstate);
  PStateId pstateId(const PState pstate);
  string pstateIdString(const PState* pstate);
 private:
};

/*! 
  * \author Markus Schordan
  * \date 2012.
 * \brief Input: a value val is read into a variable var
 * Output: either a variable or a value is written
 */
class InputOutput {
 public:
 InputOutput():op(NONE),var(VariableId()){ val=CodeThorn::AType::Bot();}
  enum OpType {NONE,STDIN_VAR,STDOUT_VAR,STDOUT_CONST,STDERR_VAR,STDERR_CONST, FAILED_ASSERT};
  OpType op;
  VariableId var;
  CodeThorn::AType::ConstIntLattice val;
  string toString() const;
  string toString(VariableIdMapping* variableIdMapping) const;
  void recordVariable(OpType op, VariableId varId);
  void recordConst(OpType op, CodeThorn::AType::ConstIntLattice val);
  void recordConst(OpType op, int val);
  void recordFailedAssert();
  bool isStdInIO() const { return op==STDIN_VAR; }
  bool isStdOutIO() const { return op==STDOUT_VAR || op==STDOUT_CONST; }
  bool isStdErrIO() const { return op==STDERR_VAR || op==STDERR_CONST; }
  bool isFailedAssertIO() const { return op==FAILED_ASSERT; }
  bool isNonIO() const { return op==NONE; }
};

bool operator<(const InputOutput& c1, const InputOutput& c2);
bool operator==(const InputOutput& c1, const InputOutput& c2);
bool operator!=(const InputOutput& c1, const InputOutput& c2);

/*! 
  * \author Markus Schordan
  * \date 2012.
 */

class EState {
 public:
 EState():_label(Label()),_pstate(0),_constraints(0){}
 EState(Label label, const PState* pstate):_label(label),_pstate(pstate),_constraints(0){}
 EState(Label label, const PState* pstate, const ConstraintSet* cset):_label(label),_pstate(pstate),_constraints(cset){}
 EState(Label label, const PState* pstate, const ConstraintSet* cset, InputOutput io):_label(label),_pstate(pstate),_constraints(cset),io(io){}

  string toString() const;
  string toString(VariableIdMapping* variableIdMapping) const;
  string toHTML() const; /// multi-line version for dot output
  long memorySize() const;

  void setLabel(Label lab) { _label=lab; }
  Label label() const { return _label; }
  void setPState(const PState* pstate) { _pstate=pstate; }
  //void setIO(InputOutput io) { io=io;} TODO: investigate
  const PState* pstate() const { return _pstate; }
  const ConstraintSet* constraints() const { return _constraints; }
  ConstraintSet allInfoAsConstraints() const;
  InputOutput::OpType ioOp() const;
  // isBot():no value, isTop(): any value (not unique), isConstInt():one concrete integer (int getIntValue())
  AType::ConstIntLattice determineUniqueIOValue() const;
  /* Predicate that determines whether all variables can be determined to be bound to a constant value.
     This function uses the entire PState and all available constraints to determine constness.
   */
  bool isConst(VariableIdMapping* vid) const;
  bool isRersTopified(VariableIdMapping* vid) const;
  string predicateToString(VariableIdMapping* vid) const;
 private:
  Label _label;
  const PState* _pstate;
  const ConstraintSet* _constraints;
 public:
  InputOutput io;
};

// define order for PState elements (necessary for PStateSet)
#ifdef  USER_DEFINED_PSTATE_COMP
bool operator<(const PState& c1, const PState& c2);
#if 0
bool operator==(const PState& c1, const PState& c2);
bool operator!=(const PState& c1, const PState& c2);
#endif
#endif

// define order for EState elements (necessary for EStateSet)
bool operator<(const EState& c1, const EState& c2);
bool operator==(const EState& c1, const EState& c2);
bool operator!=(const EState& c1, const EState& c2);

struct EStateLessComp {
  bool operator()(const EState& c1, const EState& c2) {
    return c1<c2;
  }
};

/*! 
  * \author Markus Schordan
  * \date 2012.
 */
#ifdef USE_CUSTOM_HSET
class EStateHashFun {
   public:
    EStateHashFun(long prime=9999991) : tabSize(prime) {}
    long operator()(EState s) const {
      unsigned int hash=1;
      hash=(long)s.label()*(((long)s.pstate())+1)*(((long)s.constraints())+1);
      return long(hash) % tabSize;
    }
    long tableSize() const { return tabSize;}
   private:
    long tabSize;
};
#else
class EStateHashFun {
   public:
    EStateHashFun() {}
    long operator()(EState* s) const {
      unsigned int hash=1;
      hash=(long)s->label().getId()*(((long)s->pstate())+1)*(((long)s->constraints())+1);
      return long(hash);
    }
   private:
};
#endif
class EStateEqualToPred {
   public:
    EStateEqualToPred() {}
    long operator()(EState* s1, EState* s2) const {
      return *s1==*s2;
    }
   private:
};
/*! 
  * \author Markus Schordan
  * \date 2012.
 */
 class EStateSet : public HSetMaintainer<EState,EStateHashFun,EStateEqualToPred> {
 public:
 EStateSet():HSetMaintainer<EState,EStateHashFun,EStateEqualToPred>(),_constraintSetMaintainer(0){}
 public:
  typedef HSetMaintainer<EState,EStateHashFun,EStateEqualToPred>::ProcessingResult ProcessingResult;
  string toString(VariableIdMapping* variableIdMapping=0) const;
  EStateId estateId(const EState* estate) const;
  EStateId estateId(const EState estate) const;
  string estateIdString(const EState* estate) const;
  int numberOfIoTypeEStates(InputOutput::OpType) const;
  int numberOfConstEStates(VariableIdMapping* vid) const;
 private:
  ConstraintSetMaintainer* _constraintSetMaintainer; 
};

/*! 
  * \author Markus Schordan
  * \date 2012.
 */
class Transition {
 public:
  Transition() {}
 Transition(const EState* source,Edge edge, const EState* target):source(source),edge(edge),target(target){}
public:
  const EState* source; // source node
  Edge edge;
  const EState* target; // target node
  string toString() const;

};
/*! 
  * \author Markus Schordan
  * \date 2012.
 */
#ifdef USE_CUSTOM_HSET
class TransitionHashFun {
   public:
    TransitionHashFun(long prime=99991) : tabSize(prime) {}
    long operator()(Transition s) const {
      unsigned int hash=1;
      hash=((((long)s.source)+1)<<8)+(long)s.target*(long)s.edge.hash();
      return long(hash) % tabSize;
    }
      long tableSize() const { return tabSize;}
      private:
    long tabSize;
};
#else
class TransitionHashFun {
   public:
    TransitionHashFun() {}
    long operator()(Transition* s) const {
      unsigned int hash=1;
      hash=((((long)s->source)+1)<<8)+(long)s->target*(long)s->edge.hash();
      return long(hash);
    }
      private:
};
#endif

class TransitionEqualToPred {
   public:
    TransitionEqualToPred() {}
    bool operator()(Transition* t1, Transition* t2) const {
      return t1->source==t2->source && t1->target==t2->target && t1->edge==t2->edge;
    }
   private:
};

bool operator==(const Transition& t1, const Transition& t2);
bool operator!=(const Transition& t1, const Transition& t2);
bool operator<(const Transition& t1, const Transition& t2);

class EStateList : public list<EState> {
 public:
  string toString();
};

/*! 
  * \author Markus Schordan
  * \date 2012.
 */
 
 typedef set<const EState*> EStatePtrSet;
 typedef set<const Transition*> TransitionPtrSet;

 class TransitionGraph : public HSetMaintainer<Transition,TransitionHashFun,TransitionEqualToPred> {
 public:
   typedef set<const Transition*> TransitionPtrSet;
 TransitionGraph():_startLabel(Label()),_numberOfNodes(0){}
  EStatePtrSet transitionSourceEStateSetOfLabel(Label lab);
  EStatePtrSet estateSetOfLabel(Label lab);
  EStatePtrSet estateSet();
  long numberOfObservableStates(bool inlcudeIn=true, bool includeOut=true, bool includeErr=true);
  void add(Transition trans);
  string toString() const;
  LabelSet labelSetOfIoOperations(InputOutput::OpType op);
  // eliminates all duplicates of edges
  //long removeDuplicates();
  Label getStartLabel() { assert(_startLabel!=Label()); return _startLabel; }
  void setStartLabel(Label lab) { _startLabel=lab; }
  // this allows to deal with multiple start transitions (must share same start state)
  const EState* getStartEState();
  Transition getStartTransition();
#if 1
  void erase(TransitionGraph::iterator transiter);
  void erase(const Transition trans);
#endif

  //! deprecated
  void reduceEStates(set<const EState*> toReduce);
  void reduceEState(const EState* estate);
  //! reduces estates. Adds edge-annotation PATH. Structure preserving by remapping existing edges.
  void reduceEStates2(set<const EState*> toReduce);
  void reduceEState2(const EState* estate); // used for semantic folding
  TransitionPtrSet inEdges(const EState* estate);
  TransitionPtrSet outEdges(const EState* estate);
  EStatePtrSet pred(const EState* estate);
  EStatePtrSet succ(const EState* estate);
  bool checkConsistency();
  const Transition* hasSelfEdge(const EState* estate);
  // deletes EState and *deletes* all ingoing and outgoing transitions
  void eliminateEState(const EState* estate);
  int eliminateBackEdges();
  void determineBackEdges(const EState* state, set<const EState*>& visited, TransitionPtrSet& tpSet);
  void setIsPrecise(bool v);
  void setIsComplete(bool v);
  bool isPrecise();
  bool isComplete();
 private:
  Label _startLabel;
  int _numberOfNodes; // not used yet
  map<const EState*,TransitionPtrSet > _inEdges;
  map<const EState*,TransitionPtrSet > _outEdges;
  set<const EState*> _recomputedestateSet;
  bool _preciseSTG;
  bool _completeSTG;
};

} // namespace CodeThorn

#endif
