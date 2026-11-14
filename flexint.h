#pragma once
#include <string>
#include <compare>
#include <ostream>
#include "ez3i/mpz.h"

namespace calendar_converter {

namespace z3i = extracted_z3_integer ;

class flexint {
  z3i::mpz m_num;

public:
  static z3i::mpz_manager& m() ;
  flexint();
  flexint(int n) ;
  flexint(const char* s) ;
  flexint(const std::string& s) ;
  flexint(std::string_view s) ;
  flexint(flexint const& n) ;
  flexint(flexint && n) noexcept ;
  ~flexint() ;
  explicit operator int() const ;
  operator z3i::mpz const &() const ;
  z3i::mpz const & get() const ;
  z3i::mpz & get() ;
  flexint & operator=(flexint const & n) ;
  flexint & operator=(flexint&&) noexcept;
  flexint & operator=(z3i::mpz const & n) ;
  flexint & operator=(int n) ;
  flexint & operator=(const char* s) ;
  flexint & operator=(const std::string& s) ;
  flexint & operator=(std::string_view s) ;
  void reset() ;
  void swap(flexint & n) noexcept ;
  void swap(z3i::mpz & n) noexcept ;
  bool is_zero() const ;
  std::string str() const ;
  flexint & operator+=(z3i::mpz const & a) ;
  flexint & operator-=(z3i::mpz const & a) ;
  flexint & operator*=(z3i::mpz const & a) ;
  flexint & operator/=(z3i::mpz const & a) ;
  flexint & operator%=(z3i::mpz const & a) ;
  friend bool is_zero(flexint const & a) ;
  friend void swap(flexint & a, flexint & b) noexcept ;
  friend std::string to_string(flexint const& a) ;
  friend bool operator==(flexint const & a, z3i::mpz const & b) ;
  friend std::strong_ordering operator<=>(flexint const & a, z3i::mpz const & b) ;
  friend flexint operator+(flexint const & r1, z3i::mpz const & r2) ;
  friend flexint operator-(flexint const & r1, z3i::mpz const & r2) ;
  friend flexint operator*(flexint const & r1, z3i::mpz const & r2) ;
  friend flexint operator/(flexint const & r1, z3i::mpz const & r2) ;
  friend flexint operator%(flexint const & r1, z3i::mpz const & r2) ;
  friend std::ostream & operator<<(std::ostream & out, flexint const & s) ;
}; // class flexint

} // namespace calendar_converter
