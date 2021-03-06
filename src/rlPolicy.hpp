/*   This file is part of rl-lib
 *
 *   Copyright (C) 2010,  Supelec
 *
 *   Author : Herve Frezza-Buet and Matthieu Geist
 *
 *   Contributor :
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License (GPL) as published by the Free Software Foundation; either
 *   version 3 of the License, or any later version.
 *   
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *   Contact : Herve.Frezza-Buet@supelec.fr Matthieu.Geist@supelec.fr
 *
 */

#pragma once

#include <functional>
#include <type_traits>
#include <rlAlgo.hpp>

namespace rl {

  namespace policy {

    /**
     * @short This builds a greedy policy from an existing Q(S,A) function.
     */
    template<typename Q,
	     typename ACTION_ITERATOR>
    class Greedy {
    private:
      
      Q q;
      ACTION_ITERATOR begin,end;

    public:
	
      Greedy(const Q& q_function,
	     const ACTION_ITERATOR& action_begin,
	     const ACTION_ITERATOR& action_end) 
	: q(q_function),
	  begin(action_begin),
	  end(action_end){}
      Greedy(const Greedy<Q,ACTION_ITERATOR>& cp) 
	: q(cp.q), begin(cp.begin), end(cp.end) {}

      Greedy<Q,ACTION_ITERATOR>& operator=(const Greedy<Q,ACTION_ITERATOR>& cp) {
	if(&cp != this) {
	  q     = cp.q;
	  begin = cp.begin;
	  end   = cp.end;
	}
	return *this;
      }


      template<typename STATE>
      typename std::remove_reference<decltype(*begin)>::type operator()(const STATE& s) const {
	return rl::argmax(std::bind(q,s,std::placeholders::_1),begin,end).first;
      }
    };

    template<typename Q,
	     typename ACTION_ITERATOR>
    Greedy<Q,ACTION_ITERATOR> greedy(const Q& q_function,
				     const ACTION_ITERATOR& action_begin,
				     const ACTION_ITERATOR& action_end) {
      return Greedy<Q,ACTION_ITERATOR>(q_function,action_begin,action_end);
    }

    

    /**
     * @short This builds a epsilon-greedy policy from an existing Q(S,A) function.
     */
    template<typename Q,
	     typename ACTION_ITERATOR>
    class EpsilonGreedy {
    private:
      
      Q q;
      ACTION_ITERATOR begin,end;

    public:
	
      double epsilon;

      EpsilonGreedy(const Q& q_function,
		    double eps,
		    const ACTION_ITERATOR& action_begin,
		    const ACTION_ITERATOR& action_end) 
	: q(q_function),
	  begin(action_begin),
	  end(action_end),
	  epsilon(eps) {}
      EpsilonGreedy(const EpsilonGreedy<Q,ACTION_ITERATOR>& cp) 
	: q(cp.q), begin(cp.begin), end(cp.end), epsilon(cp.epsilon) {}

      EpsilonGreedy<Q,ACTION_ITERATOR>& operator=(const EpsilonGreedy<Q,ACTION_ITERATOR>& cp) {
	if(&cp != this) {
	  q       = cp.q;
	  begin   = cp.begin;
	  end     = cp.end;
	  epsilon = cp.epsilon;
	}
	return *this;
      }

      template<typename STATE>
      typename std::remove_reference<decltype(*begin)>::type operator()(const STATE& s) const {
	if(rl::random::toss(epsilon))
	  return rl::random::select(begin,end);
	return rl::argmax(std::bind(q,s,std::placeholders::_1),begin,end).first;
      }
    };

    template<typename Q,
	     typename ACTION_ITERATOR>
    EpsilonGreedy<Q,ACTION_ITERATOR> epsilon_greedy(const Q& q_function,
						    double epsilon,
						    const ACTION_ITERATOR& action_begin,
						    const ACTION_ITERATOR& action_end) {
      return EpsilonGreedy<Q,ACTION_ITERATOR>(q_function,epsilon,action_begin,action_end);
    }
      
    /**
     * @short This builds a random policy.
     */
    template<typename ACTION_ITERATOR>
    class Random {
    private:
      
      ACTION_ITERATOR begin,end;

    public:

      Random(const ACTION_ITERATOR& action_begin,
	     const ACTION_ITERATOR& action_end) 
	: begin(action_begin),
	  end(action_end) {}
      Random(const Random<ACTION_ITERATOR>& cp) 
	: begin(cp.begin), end(cp.end) {}

      Random<ACTION_ITERATOR>& operator=(const Random<ACTION_ITERATOR>& cp) {
	if(&cp != this) {
	  begin   = cp.begin;
	  end     = cp.end;
	}
	return *this;
      }

      template<typename STATE>
      typename std::remove_reference<decltype(*begin)>::type operator()(const STATE& s) const {
	return rl::random::select(begin,end);
      }
    };

    template<typename ACTION_ITERATOR>
    Random<ACTION_ITERATOR> random(const ACTION_ITERATOR& action_begin,
				   const ACTION_ITERATOR& action_end) {
      return Random<ACTION_ITERATOR>(action_begin,action_end);
    }
      
 

    /**
     * @short This builds a softmax policy from an existing Q(S,A) function.
     */
    template<typename Q,
	     typename ACTION_ITERATOR>
    class SoftMax {
    private:
      
      Q q;
      ACTION_ITERATOR begin,end;

    public:
	
      double temperature;

      SoftMax(const Q& q_function,
	      double temp,
	      const ACTION_ITERATOR& action_begin,
	      const ACTION_ITERATOR& action_end) 
	: q(q_function),
	  temperature(temp),
	  begin(action_begin),
	  end(action_end){
      }
      SoftMax(const SoftMax<Q,ACTION_ITERATOR>& cp) 
	: q(cp.q), temperature(cp.temperature), begin(cp.begin), end(cp.end) {}

      SoftMax<Q,ACTION_ITERATOR>& operator=(const SoftMax<Q,ACTION_ITERATOR>& cp) {
	if(&cp != this) {
	  q           = cp.q;
	  temperature = cp.temperature;
	  begin       = cp.begin;
	  end         = cp.end;
	}
	return *this;
      }


      template<typename STATE>
      typename std::remove_reference<decltype(*begin)>::type operator()(const STATE& s) const {
	return rl::random::softmax(std::bind(q,s,std::placeholders::_1),temperature,begin,end);
      }
    };

    template<typename Q,
	     typename ACTION_ITERATOR>
    SoftMax<Q,ACTION_ITERATOR> softmax(const Q& q_function,
				       double temperature,
				       const ACTION_ITERATOR& action_begin,
				       const ACTION_ITERATOR& action_end) {
      return SoftMax<Q,ACTION_ITERATOR>(q_function,temperature,action_begin,action_end);
    }    
  }
}
