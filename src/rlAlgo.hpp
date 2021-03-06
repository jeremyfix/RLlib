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

#include <utility>
#include <cstdlib>
#include <vector>
#include <type_traits>
#include <iterator>
#include <functional>

#include <gsl/gsl_vector.h>

namespace rl {

  template<typename ITERATOR,
	   typename fctEVAL>
  auto min(const fctEVAL& f,
	   const ITERATOR& begin, 
	   const ITERATOR& end)
    -> decltype(f(*begin))
  { 
    ITERATOR iter = begin;
    auto m     = f(*iter);
    for(++iter;iter!=end;++iter) {
      auto  v = f(*iter);
      if(v < m)
	m = v;
    }
    return m;
  }

  template<typename ITERATOR,
	   typename fctEVAL>
  auto max(const fctEVAL& f,
	   const ITERATOR& begin, 
	   const ITERATOR& end)
    -> decltype(f(*begin))
  { 
    ITERATOR iter = begin;
    auto m     = f(*iter);
    for(++iter;iter!=end;++iter) {
      auto  v = f(*iter);
      if(v > m)
	m = v;
    }
    return m;
  }

  template<typename ITERATOR,
	   typename fctEVAL>
  auto range(const fctEVAL& f,
	   const ITERATOR& begin, 
	   const ITERATOR& end)
    -> std::pair<decltype(f(*begin)),
		 decltype(f(*begin))>
  { 
    ITERATOR iter = begin;
    auto min   = f(*iter);
    auto max   = min;
    for(++iter;iter!=end;++iter) {
      auto  v = f(*iter);
      if(v > max)
	max = v;
      else if(v < min)
	min = v;
    }
    return {min,max};
  }

  template<typename ITERATOR,
	   typename fctEVAL>
  auto argmax(const fctEVAL& f,
	      const ITERATOR& begin, 
	      const ITERATOR& end)
    -> std::pair<decltype(*begin),
		 decltype(f(*begin))> 
{ 
    ITERATOR iter = begin;
    auto arg_max = *iter;
    auto max     = f(*iter);
    for(++iter;iter!=end;++iter) {
      auto a = *iter;
      auto v = f(a);
      if(v > max) {
	max = v;
	arg_max = a;
      }
    }
    return {arg_max,max};
  }

  template<typename ITERATOR,
	   typename fctEVAL>
  auto argmin(const fctEVAL& f,
	      const ITERATOR& begin, 
	      const ITERATOR& end)
    -> std::pair<decltype(*begin),
		 decltype(f(*begin))> 
{ 
    ITERATOR iter = begin;
    auto arg_min = *iter;
    auto min     = f(*iter);
    for(++iter;iter!=end;++iter) {
      auto  a = *iter;
      auto  v = f(a);
      if(v < min) {
	min = v;
	arg_min = a;
      }
    }
    return {arg_min,min};
  }
  
  template<typename T>
  class enumerator : public std::iterator<std::random_access_iterator_tag,T> {
  private:
    int j;
  public:
    enumerator() : j(0) {}
    enumerator(const enumerator& cp) : j(cp.j) {}
    enumerator(int i) : j(i) {}
    enumerator<T>& operator=(T i) {j=i; return *this;}
    enumerator<T>& operator=(const enumerator<T>& cp) {j=cp.j; return *this;}
    enumerator<T>& operator++() {++j; return *this;}
    enumerator<T>& operator--() {--j; return *this;}
    enumerator<T>& operator+=(int diff) {j+=diff; return *this;}
    enumerator<T>& operator-=(int diff) {j-=diff; return *this;}
    enumerator<T> operator++(int) {enumerator<T> res = *this; ++*this; return res;}
    enumerator<T> operator--(int) {enumerator<T> res = *this; --*this; return res;}
    int operator-(const enumerator<T>& i) const {return j - i.j;}
    enumerator<T> operator+(int i) const {return enumerator<T>(j+i);}
    enumerator<T> operator-(int i) const {return enumerator<T>(j-i);}
    T operator*() const {return (T)j;}
    bool operator==(const enumerator<T>& i) const {return j == i.j;}
    bool operator!=(const enumerator<T>& i) const {return j != i.j;}
  };

  namespace random {

    
    /**
     * Initialization of random seed.
     */
    inline void seed(unsigned int s) {srand(s);}
    
    /**
     * @return A random value in [0..1[
     */
    inline double uniform(void) {return rand()/(1.0+RAND_MAX);}

    /**
     * @return A random value in [min..max[
     */
    inline double uniform(double min,double max) {return min + (max-min)*uniform();}

    /**
     * @return A random value according to the histogram represented
     * by f(x), for x in [begin,end[.
     */
    template<typename ITERATOR,
	     typename fctEVAL>
    auto density(const fctEVAL& f,
		 const ITERATOR& begin, const ITERATOR& end) 
      -> decltype(*begin) {
      auto size = end-begin;
      std::vector<double> cum(size);
      auto iter = begin;
      auto citer = cum.begin();
      *citer =  f(*iter);
      auto prev = citer++;
      for(++iter; 
      	  iter != end; 
      	  prev = citer++, ++iter)
      	*citer = *prev + f(*iter);

      double val = rl::random::uniform(0,cum[size-1]);
      for(citer = cum.begin();
      	  val >= *citer;
      	  ++citer);
      return *(begin + (citer - cum.begin()));
    }
 
    /**
     * @return true with a probability proba
     */
    inline bool toss(double proba) {
      return uniform() < proba;
    }

    template<typename ITERATOR>
    auto select(const ITERATOR& begin, const ITERATOR& end) -> decltype(*begin) {
      return *(begin + (decltype(end-begin))(uniform()*(end-begin)));
    }

    template<typename ITERATOR,
	     typename fctEVAL>
    auto softmax(const fctEVAL& f,
		 double temperature,
		 const ITERATOR& begin, const ITERATOR& end) 
      -> decltype(*begin) {
      return rl::random::density([&temperature,&f](const decltype(*begin)& a) -> double {return exp(f(a)/temperature);},
				 begin,end);
    }
  }


  namespace sa {
    
    template <typename S, typename A>
    struct Pair {
      S s;
      A a;
    };
    
    template <typename S, typename A>
    Pair<S,A> pair(const S& s, const A& a) {return {s,a};}
    

    namespace gsl {
      // This rewrites q(theta,s,a) as v(theta,(s,a)).
      template<typename S, typename A, typename REWARD, typename Q>
      auto vparam_of_qparam(const Q& q) -> std::function<REWARD (const gsl_vector*,const Pair<S,A>&)> {
	return [&q](const gsl_vector* theta, const Pair<S,A>& sa) -> REWARD {return q(theta,sa.s,sa.a);};
      }
      // This rewrites grad_q(theta,grad,s,a) as v(theta,grad,(s,a)).
      template<typename S, typename A, typename REWARD, typename Q>
      auto gradvparam_of_gradqparam(const Q& gq) -> std::function<void (const gsl_vector*,gsl_vector*,Pair<S,A>)> {
	return [&gq](const gsl_vector* theta, gsl_vector* grad, const Pair<S,A>& sa) -> void {gq(theta,grad,sa.s,sa.a);};
      }
    }
  }
}
