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

/*
  This examples shows the application of LSTD to the Boyan chain.
*/

#include <rl.hpp>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <fstream>
#include <vector>
#include <gsl/gsl_blas.h>

// This is our simulator.
typedef rl::problem::boyan_chain::Simulator Simulator;

// MDP features
typedef Simulator::reward_type            Reward; 
typedef Simulator::observation_type       S;
typedef Simulator::action_type            A;

// Let us define transitions s,r,s'
struct Transition {
  S      s;
  Reward r;
  S      s_; // read s_ as s'
  bool   is_terminal;
};
std::ostream& operator<<(std::ostream& os, const Transition& t) {
  os << t.s << " -- " << t.r;
  if(t.is_terminal)
    os << " End";
  else
    os << " --> " << t.s_;
  return os;
}

typedef std::vector<Transition>           TransitionSet;

// The function that associates a feature vector to a State is the
// following.
typedef rl::problem::boyan_chain::Feature Feature;

#define paramREG   0
#define paramGAMMA 1
#define paramALPHA 0.05

#define NB_OF_EPISODES 100
int main(int argc, char* argv[]) {

  Simulator         simulator;
  TransitionSet     transitions;
  int               episode_length;
  Feature           phi;
  
  gsl_vector* theta = gsl_vector_alloc(phi.dimension());
  gsl_vector_set_zero(theta);
  gsl_vector* tmp = gsl_vector_alloc(phi.dimension());
  gsl_vector_set_zero(tmp);

  auto v_parametrized = [&phi,tmp](const gsl_vector* th,S s) -> Reward {double res;
									phi(tmp,s);                 // phi_s = phi(s)
									gsl_blas_ddot(th,tmp,&res); // res   = th^T  . phi_s
									return res;};
  auto grad_v_parametrized = [&phi,tmp](const gsl_vector* th,   
					gsl_vector* grad_th_s,
					S s) -> void {phi(tmp,s);                         // phi_s    = phi(s)
						      gsl_vector_memcpy(grad_th_s,tmp);}; // grad_th_s = phi_s
  
  try {
    
    // Let us fill a set of transitions from successive episodes.
    for(int episode = 0; episode < NB_OF_EPISODES; ++episode) {
      simulator.initPhase();
      rl::episode::run(simulator,
		       [](S s) -> A {return rl::problem::boyan_chain::actionNone;}, // This is the policy.
		       std::back_inserter(transitions),
		       [](S s, A a, Reward r, S s_) -> Transition {return {s,r,s_,false};}, 
		       [](S s, A a, Reward r)       -> Transition {return {s,r,s ,true};}, 
		       0);
    }


    // Now, we have to apply LSTD to the transition database.
    rl::lstd(theta,
	     paramGAMMA,paramREG,
	     transitions.begin(),transitions.end(),
	     grad_v_parametrized,
	     [](const Transition& t) -> S      {return t.s;},
	     [](const Transition& t) -> S      {return t.s_;},
	     [](const Transition& t) -> Reward {return t.r;},
	     [](const Transition& t) -> bool   {return t.is_terminal;});

      
    // Let us display the result
    std::cout << std::endl
	      << "LSTD estimation          : ("
	      << std::setw(15) << gsl_vector_get(theta,0) << ','
	      << std::setw(15) << gsl_vector_get(theta,1) << ','
	      << std::setw(15) << gsl_vector_get(theta,2) << ','
	      << std::setw(15) << gsl_vector_get(theta,3) << ')'
	      << std::endl;


    // We can learn the same by using TD
    
    auto td = rl::gsl::td<S>(theta,
			     paramGAMMA,paramALPHA,
			     v_parametrized,
			     grad_v_parametrized);

    // The learning can be done offline since we have collected
    // transitions.

    gsl_vector_set_zero(theta);
    for(auto& t : transitions)
      if(t.is_terminal)
	td.learn(t.s,t.r);
      else
	td.learn(t.s,t.r,t.s_);
      
    std::cout << "TD (offline) estimation  : ("
	      << std::setw(15) << gsl_vector_get(theta,0) << ','
	      << std::setw(15) << gsl_vector_get(theta,1) << ','
	      << std::setw(15) << gsl_vector_get(theta,2) << ','
	      << std::setw(15) << gsl_vector_get(theta,3) << ')'
	      << std::endl;

    // But this can be done on-line, directly from episodes.

    gsl_vector_set_zero(theta);
    for(int episode = 0; episode < NB_OF_EPISODES; ++episode) {
      simulator.initPhase();
      rl::episode::learn(simulator,
			 [](S s) -> A {return rl::problem::boyan_chain::actionNone;}, // This is the policy.
			 td,
			 0);
    }

    std::cout << "TD (online) estimation   : ("
	      << std::setw(15) << gsl_vector_get(theta,0) << ','
	      << std::setw(15) << gsl_vector_get(theta,1) << ','
	      << std::setw(15) << gsl_vector_get(theta,2) << ','
	      << std::setw(15) << gsl_vector_get(theta,3) << ')'
	      << std::endl;

    // With the boyan chain, the value function is known analytically.

    std::cout << "Optimal one should be    : ("
	      << std::setw(15) << -24  << ','
	      << std::setw(15) << -16  << ','
	      << std::setw(15) <<  -8  << ','
	      << std::setw(15) <<   0  << ')'
	      << std::endl;
  }
  catch(rl::exception::Any& e) {
    std::cerr << "Exception caught : " << e.what() << std::endl;
  }
  
  gsl_vector_free(theta);
  gsl_vector_free(tmp);
  return 0;
}
