#ifndef STATES_H
#define STATES_H

#include <vector>
#include <set>
#include <cassert>

#include "probability_distribution.h"

#include "utils/math_utils.h"

class ActionFluent;
class DeterministicEvaluatable;

/*****************************************************************
                             State
*****************************************************************/

class State {
public:
    State(int const& _remSteps = -1) :
        deterministicStateFluents(State::numberOfDeterministicStateFluents, 0.0),
        probabilisticStateFluents(State::numberOfProbabilisticStateFluents, 0.0),
        remSteps(_remSteps),
        stateFluentHashKeys(State::numberOfStateFluentHashKeys, 0),
        hashKey(-1) {}

    State(std::vector<double> _deterministicStateFluents, std::vector<double> _probabilisticStateFluents, int const& _remSteps) :
        deterministicStateFluents(_deterministicStateFluents),
        probabilisticStateFluents(_probabilisticStateFluents),
        remSteps(_remSteps),
        stateFluentHashKeys(State::numberOfStateFluentHashKeys, 0),
        hashKey(-1) {
        assert(deterministicStateFluents.size() == State::numberOfDeterministicStateFluents);
        assert(probabilisticStateFluents.size() == State::numberOfProbabilisticStateFluents);
    }

    State(State const& other) :
        deterministicStateFluents(other.deterministicStateFluents),
        probabilisticStateFluents(other.probabilisticStateFluents),
        remSteps(other.remSteps),
        stateFluentHashKeys(other.stateFluentHashKeys),
        hashKey(other.hashKey) {}

    virtual void setTo(State const& other) {
        for(unsigned int i = 0; i < State::numberOfDeterministicStateFluents; ++i) {
            deterministicStateFluents[i] = other.deterministicStateFluents[i];
        }
        for(unsigned int i = 0; i < State::numberOfProbabilisticStateFluents; ++i) {
            probabilisticStateFluents[i] = other.probabilisticStateFluents[i];
        }

        remSteps = other.remSteps;

        for(unsigned int i = 0; i < State::numberOfStateFluentHashKeys; ++i) {
            stateFluentHashKeys[i] = other.stateFluentHashKeys[i];
        }

        hashKey = other.hashKey;
    }

    virtual void reset(int _remSteps) {
        for(unsigned int i = 0; i < State::numberOfDeterministicStateFluents; ++i) {
            deterministicStateFluents[i] = 0.0;
        }
        for(unsigned int i = 0; i < State::numberOfProbabilisticStateFluents; ++i) {
            probabilisticStateFluents[i] = 0.0;
        }

        remSteps = _remSteps;

        for(unsigned int i = 0; i < State::numberOfStateFluentHashKeys; ++i) {
            stateFluentHashKeys[i] = 0;
        }

        hashKey = -1;
    }

    void swap(State& other) {
        deterministicStateFluents.swap(other.deterministicStateFluents);
        probabilisticStateFluents.swap(other.probabilisticStateFluents);

        std::swap(remSteps, other.remSteps);
        std::swap(hashKey, other.hashKey);
        stateFluentHashKeys.swap(other.stateFluentHashKeys);
    }

    // Calculate the hash key of a State
    static void calcStateHashKey(State& state) {
        if(State::stateHashingPossible) {
            state.hashKey = 0;
            for(unsigned int index = 0; index < State::numberOfDeterministicStateFluents; ++index) {
                state.hashKey += State::stateHashKeysOfDeterministicStateFluents[index][(int)state.deterministicStateFluents[index]];
            }
            for(unsigned int index = 0; index < State::numberOfProbabilisticStateFluents; ++index) {
                state.hashKey += State::stateHashKeysOfProbabilisticStateFluents[index][(int)state.probabilisticStateFluents[index]];
            }
        } else {
            assert(state.hashKey == -1);
        }
    }

    // Calculate the hash key for each state fluent in a State
    static void calcStateFluentHashKeys(State& state) {
        for(unsigned int i = 0; i < State::numberOfDeterministicStateFluents; ++i) {
            if(MathUtils::doubleIsGreater(state.deterministicStateFluents[i], 0.0)) {
                for(unsigned int j = 0; j < State::stateFluentHashKeysOfDeterministicStateFluents[i].size(); ++j) {
                    assert(state.stateFluentHashKeys.size() > State::stateFluentHashKeysOfDeterministicStateFluents[i][j].first);
                    state.stateFluentHashKeys[State::stateFluentHashKeysOfDeterministicStateFluents[i][j].first] +=
                        ((int)state.deterministicStateFluents[i]) * State::stateFluentHashKeysOfDeterministicStateFluents[i][j].second;
                }                
            }
        }

        for(unsigned int i = 0; i < State::numberOfProbabilisticStateFluents; ++i) {
            if(MathUtils::doubleIsGreater(state.probabilisticStateFluents[i], 0.0)) {
                for(unsigned int j = 0; j < State::stateFluentHashKeysOfProbabilisticStateFluents[i].size(); ++j) {
                    assert(state.stateFluentHashKeys.size() > State::stateFluentHashKeysOfProbabilisticStateFluents[i][j].first);
                    state.stateFluentHashKeys[State::stateFluentHashKeysOfProbabilisticStateFluents[i][j].first] +=
                        ((int)state.probabilisticStateFluents[i]) * State::stateFluentHashKeysOfProbabilisticStateFluents[i][j].second;
                }                
            }
        }
    }

    double& deterministicStateFluent(int const& index) {
        assert(index < deterministicStateFluents.size());
        return deterministicStateFluents[index];
    }

    double const& deterministicStateFluent(int const& index) const {
        assert(index < deterministicStateFluents.size());
        return deterministicStateFluents[index];
    }

    double& probabilisticStateFluent(int const& index) {
        assert(index < probabilisticStateFluents.size());
        return probabilisticStateFluents[index];
    }

    double const& probabilisticStateFluent(int const& index) const {
        assert(index < probabilisticStateFluents.size());
        return probabilisticStateFluents[index];
    }

    int const& remainingSteps() const {
        return remSteps;
    }

    int& remainingSteps() {
        return remSteps;
    }

    bool isTerminal() const {
        assert(remSteps >= 0);
        return (remSteps == 0);
    }

    long const& stateFluentHashKey(int const& index) const {
        assert(index < stateFluentHashKeys.size());
        return stateFluentHashKeys[index];
    }

    struct CompareIgnoringRemainingSteps {
        bool operator() (State const& lhs, State const& rhs) const {
            if((lhs.hashKey >= 0) && (rhs.hashKey >= 0)) {
                return (lhs.hashKey < rhs.hashKey);
            }

            for(unsigned int i = 0; i < State::numberOfDeterministicStateFluents; ++i) {
                if(MathUtils::doubleIsSmaller(rhs.deterministicStateFluents[i], lhs.deterministicStateFluents[i])) {
                    return false;
                } else if(MathUtils::doubleIsSmaller(lhs.deterministicStateFluents[i], rhs.deterministicStateFluents[i])) {
                    return true;
                }
            }

            for(unsigned int i = 0; i < State::numberOfProbabilisticStateFluents; ++i) {
                if(MathUtils::doubleIsSmaller(rhs.probabilisticStateFluents[i], lhs.probabilisticStateFluents[i])) {
                    return false;
                } else if(MathUtils::doubleIsSmaller(lhs.probabilisticStateFluents[i], rhs.probabilisticStateFluents[i])) {
                    return true;
                }
            }

            return false;
        }
    };

    struct CompareConsideringRemainingSteps {
        bool operator() (State const& lhs, State const& rhs) const {
            if(lhs.remSteps < rhs.remSteps) {
                return true;
            } else if(lhs.remSteps > rhs.remSteps) {
                return false;
            }

            if((lhs.hashKey >= 0) && (rhs.hashKey >= 0)) {
                return (lhs.hashKey < rhs.hashKey);
            }

            for(unsigned int i = 0; i < State::numberOfDeterministicStateFluents; ++i) {
                if(MathUtils::doubleIsSmaller(rhs.deterministicStateFluents[i], lhs.deterministicStateFluents[i])) {
                    return false;
                } else if(MathUtils::doubleIsSmaller(lhs.deterministicStateFluents[i], rhs.deterministicStateFluents[i])) {
                    return true;
                }
            }

            for(unsigned int i = 0; i < State::numberOfProbabilisticStateFluents; ++i) {
                if(MathUtils::doubleIsSmaller(rhs.probabilisticStateFluents[i], lhs.probabilisticStateFluents[i])) {
                    return false;
                } else if(MathUtils::doubleIsSmaller(lhs.probabilisticStateFluents[i], rhs.probabilisticStateFluents[i])) {
                    return true;
                }
            }

            return false;
        }
    };

    void printCompact(std::ostream& out) const;
    void print(std::ostream& out) const;

    // The number of state fluents (this is equal to CPFs.size())
    static int stateSize;

    // The number of deterministic and probabilistic state fluents
    static int numberOfDeterministicStateFluents;
    static int numberOfProbabilisticStateFluents;

    // The number of variables that have a state fluent hash key
    static int numberOfStateFluentHashKeys;

    // Is true if hashing of states (not state fluent hashing) is possible
    static bool stateHashingPossible;

    // These are used to calculate hash keys
    static std::vector<std::vector<long> > stateHashKeysOfDeterministicStateFluents;
    static std::vector<std::vector<long> > stateHashKeysOfProbabilisticStateFluents;

    // The Evaluatable with index
    // stateFluentHashKeysOfDeterministicStateFluents[i][j].first depends on the
    // deterministic state fluent with index i, and is updated by multiplication
    // with stateFluentHashKeysOfDeterministicStateFluents[i][j].second
    static std::vector<std::vector<std::pair<int, long> > > stateFluentHashKeysOfDeterministicStateFluents;
    static std::vector<std::vector<std::pair<int, long> > > stateFluentHashKeysOfProbabilisticStateFluents;

    friend class KleeneState;
    friend class PDState;

private:
    std::vector<double> deterministicStateFluents;
    std::vector<double> probabilisticStateFluents;

    int remSteps;
    std::vector<long> stateFluentHashKeys;
    long hashKey;
};

/*****************************************************************
                             ActionState
*****************************************************************/

struct ActionState {
    ActionState(int _index, std::vector<int> _state, std::vector<ActionFluent*> _scheduledActionFluents, std::vector<DeterministicEvaluatable*> _actionPreconditions) :
        index(_index), state(_state), scheduledActionFluents(_scheduledActionFluents), actionPreconditions(_actionPreconditions) {}

    int& operator[](int const& index) {
        return state[index];
    }

    const int& operator[](int const& index) const {
        return state[index];
    }

    void printCompact(std::ostream& out) const;
    void print(std::ostream& out) const;

    int index;
    std::vector<int> state;
    std::vector<ActionFluent*> scheduledActionFluents;
    std::vector<DeterministicEvaluatable*> actionPreconditions;
};

/*****************************************************************
                               PDState
*****************************************************************/

class PDState : public State {
public:
    PDState(int const& _remSteps = -1) :
        State(_remSteps),
        probabilisticStateFluentsAsPD(State::numberOfProbabilisticStateFluents, DiscretePD()) {}

    PDState(State const& origin) :
        State(origin),
        probabilisticStateFluentsAsPD(State::numberOfProbabilisticStateFluents, DiscretePD()) {}

    DiscretePD& probabilisticStateFluentAsPD(int const& index) {
        assert(index < probabilisticStateFluentsAsPD.size());
        return probabilisticStateFluentsAsPD[index];
    }

    DiscretePD const& probabilisticStateFluentAsPD(int const& index) const {
        assert(index < probabilisticStateFluentsAsPD.size());
        return probabilisticStateFluentsAsPD[index];
    }

    void reset(int _remSteps) {
        State::reset(_remSteps);

        for(unsigned int i = 0; i < State::numberOfProbabilisticStateFluents; ++i) {
            probabilisticStateFluentsAsPD[i].reset();
        }
    }

    void setTo(PDState const& other) {
        State::setTo(other);

        for(unsigned int i = 0; i < State::numberOfProbabilisticStateFluents; ++i) {
            probabilisticStateFluentsAsPD[i] = other.probabilisticStateFluentsAsPD[i];
        }
    }

    // Remaining steps are not considered here!
    struct PDStateCompare {
        bool operator() (PDState const& lhs, PDState const& rhs) const {
            for(unsigned int i = 0; i < State::numberOfDeterministicStateFluents; ++i) {
                if(MathUtils::doubleIsSmaller(rhs.deterministicStateFluents[i], lhs.deterministicStateFluents[i])) {
                    return false;
                } else if(MathUtils::doubleIsSmaller(lhs.deterministicStateFluents[i], rhs.deterministicStateFluents[i])) {
                    return true;
                }
            }

            for(unsigned int i = 0; i < State::numberOfProbabilisticStateFluents; ++i) {
                if(rhs.probabilisticStateFluentsAsPD[i] < lhs.probabilisticStateFluentsAsPD[i]) {
                    return false;
                } else if(lhs.probabilisticStateFluentsAsPD[i] < rhs.probabilisticStateFluentsAsPD[i]) {
                    return true;
                }
            }
            return false;
        }
    };

    // Calculate the hash key of a PDState
    // void calcPDStateHashKey(PDState& /*state*/) const {
    //     REPAIR AND MOVE TO PDSTATE
    // }

    // Calculate the hash key for each state fluent in a PDState
    // void calcPDStateFluentHashKeys(PDState& state) const {
    //     REPAIR AND MOVE TO PDSTATE
    // }

    void printPDState(std::ostream& out) const;

protected:
    std::vector<DiscretePD> probabilisticStateFluentsAsPD;
};

/*****************************************************************
                          KleeneState
*****************************************************************/

class KleeneState {
public:
    KleeneState() :
        state(KleeneState::stateSize), stateFluentHashKeys(KleeneState::numberOfStateFluentHashKeys, 0), hashKey(-1) {}

    KleeneState(State const& origin) :
        state(KleeneState::stateSize), stateFluentHashKeys(KleeneState::numberOfStateFluentHashKeys, 0), hashKey(-1) {

        for(unsigned int index = 0; index < State::numberOfDeterministicStateFluents; ++index) {
            state[index].insert(origin.deterministicStateFluents[index]);
        }

        for(unsigned int index = 0; index < State::numberOfProbabilisticStateFluents; ++index) {
            state[State::numberOfDeterministicStateFluents+index].insert(origin.probabilisticStateFluents[index]);
        }
    }

    // Calculate the hash key of a KleeneState
    static void calcStateHashKey(KleeneState& state) {
        if(KleeneState::stateHashingPossible) {
            state.hashKey = 0;
            for(unsigned int index = 0; index < KleeneState::stateSize; ++index) {
                int multiplier = 0;
                for(std::set<double>::iterator it = state[index].begin(); it != state[index].end(); ++it) {
                    multiplier |= 1<<((int)*it);
                }
                --multiplier;

                state.hashKey += (multiplier * KleeneState::hashKeyBases[index]);
            }
        } else {
            assert(state.hashKey == -1);
        }
    }

    // Calculate the hash key for each state fluent in a KleeneState
    static void calcStateFluentHashKeys(KleeneState& state) {
        for(unsigned int i = 0; i < KleeneState::stateSize; ++i) {
            int multiplier = 0;
            for(std::set<double>::iterator it = state[i].begin(); it != state[i].end(); ++it) {
                multiplier |= 1<<((int)*it);
            }
           --multiplier;
           if(multiplier > 0) {
               for(unsigned int j = 0; j < KleeneState::indexToStateFluentHashKeyMap[i].size(); ++j) {
                   assert(state.stateFluentHashKeys.size() > KleeneState::indexToStateFluentHashKeyMap[i][j].first);
                   state.stateFluentHashKeys[KleeneState::indexToStateFluentHashKeyMap[i][j].first] +=
                       (multiplier * KleeneState::indexToStateFluentHashKeyMap[i][j].second);
               }
           }
        }
    }

    std::set<double>& operator[](int const& index) {
        assert(index < state.size());
        return state[index];
    }

    std::set<double> const& operator[](int const& index) const {
        assert(index < state.size());
        return state[index];
    }

    long const& stateFluentHashKey(int const& index) const {
        assert(index < stateFluentHashKeys.size());
        return stateFluentHashKeys[index];
    }

    bool operator==(KleeneState const& other) const {
        assert(state.size() == other.state.size());
        if((hashKey >= 0) && other.hashKey >= 0) {
            return (hashKey == other.hashKey);
        }

        for(unsigned int index = 0; index < state.size(); ++index) {
            if(!std::equal(state[index].begin(), state[index].end(), other.state[index].begin())) {
                return false;
            }
        }
        return true;
    }

    // This is used to merge two KleeneStates
    KleeneState operator|=(KleeneState const& other) {
        assert(state.size() == other.state.size());

        for(unsigned int i = 0; i < state.size(); ++i) {
            state[i].insert(other.state[i].begin(), other.state[i].end());
        }

        hashKey = -1;
        return *this;
    }

    KleeneState const operator||(KleeneState const& other) {
        assert(state.size() == other.state.size());
        return KleeneState(*this) |= other;
    }

    void print(std::ostream& out) const;

    // The number of state fluents (this is equal to CPFs.size())
    static int stateSize;

    // The number of variables that have a state fluent hash key
    static int numberOfStateFluentHashKeys;

    // Is true if hashing of States (not state fluent hashing) is possible
    static bool stateHashingPossible;

    // These are used to calculate hash keys
    static std::vector<long> hashKeyBases;

    // The Evaluatable with index indexToStateFluentHashKeyMap[i][j].first
    // depends on the CPF with index i, and is updated by multiplication with
    // indexToStateFluentHashKeyMap[i][j].second
    static std::vector<std::vector<std::pair<int, long> > > indexToStateFluentHashKeyMap;

protected:
    std::vector<std::set<double> > state;
    std::vector<long> stateFluentHashKeys;
    long hashKey;

private:
    KleeneState(KleeneState const& other) :
        state(other.state), stateFluentHashKeys(other.stateFluentHashKeys), hashKey(other.hashKey) {}
};


#endif