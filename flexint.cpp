#include "flexint.h"
#include <stdexcept>
#include <charconv>


namespace calendar_converter {


/*static*/z3i::mpz_manager& flexint::m() {
  static z3i::mpz_manager m {} ;
  return m;
}


flexint::flexint(){}


flexint::flexint(int n) { m().set(m_num, n); }


flexint::flexint(std::string_view s) {
  if (s.empty()) {
    throw std::invalid_argument("invalid decimal integer: empty string");
  }
  bool negative = false;
  if (s.front() == '+' || s.front() == '-') {
    negative = (s.front() == '-');
    s.remove_prefix(1);
    if (s.empty()) {
      throw std::invalid_argument("invalid decimal integer: sign without digits");
    }
  }
  z3i::mpz_manager& mm = m();
  unsigned long long value = 0;
  char const* first = s.data();
  char const* last  = s.data() + s.size();
  auto [ptr, ec] = std::from_chars(first, last, value);
  if (ec == std::errc{} &&
      ptr == last &&
      value <= static_cast<unsigned long long>(std::numeric_limits<std::int64_t>::max())) {
    mm.set(m_num,
               negative ? -static_cast<std::int64_t>(value)
                        : static_cast<std::int64_t>(value));
    return;
  }
  for (unsigned char ch : s) {
    if (ch < '0' || ch > '9') {
      throw std::invalid_argument(
        "invalid decimal integer: '" +
        std::string(s) +
        "'"
      );
    }
  }
  flexint result;
  constexpr std::uint32_t BLOCK_DIGITS = 9;
  constexpr int BLOCK = 1'000'000'000;
  std::size_t head = s.size() % BLOCK_DIGITS;
  if (head == 0) head = BLOCK_DIGITS;
  std::uint32_t acc = 0;
  std::size_t i = 0;
  for (; i < head; ++i)
    acc = acc * 10 + static_cast<unsigned char>(s[i]) - '0';
  result = static_cast<int>(acc);
  for (; i < s.size();) {
    acc = 0;
    for (std::size_t k = 0; k < BLOCK_DIGITS; ++k, ++i)
      acc = acc * 10 + static_cast<unsigned char>(s[i]) - '0';
    result *= BLOCK;
    result += static_cast<int>(acc);
  }
  mm.set(m_num, result.get());
  if (negative) mm.neg(m_num);
}


flexint::flexint(const char* s) : flexint(std::string_view(s)) {}


flexint::flexint(const std::string& s) : flexint(std::string_view(s)) {}


flexint::flexint(flexint const& n) { m().set(m_num, n.m_num); }


flexint::flexint(flexint && n) noexcept { m().swap(m_num, n.m_num); }


flexint::~flexint() { m().del(m_num); }


flexint::operator int() const {
  if (!m().is_int(m_num))
      throw std::overflow_error("flexint: value does not fit into int");
  return m().get_int(m_num);
}


flexint::operator z3i::mpz const &() const { return m_num; }


z3i::mpz const & flexint::get() const { return m_num; }


z3i::mpz & flexint::get() { return m_num; }


flexint & flexint::operator=(flexint const & n) {
  if (this == &n) return *this;
  m().set(m_num, n.m_num);
  return *this;
}


flexint & flexint::operator=(flexint&& n) noexcept {
  if (this != &n) m().swap(m_num, n.m_num);
  return *this;
}


flexint & flexint::operator=(int n) {
  m().set(m_num, n);
  return *this;
}


flexint & flexint::operator=(const char* s) {
  *this = flexint(std::string_view(s)) ;
  return *this;
}


flexint & flexint::operator=(const std::string& s) {
  *this = flexint(std::string_view(s));
  return *this;
}


flexint & flexint::operator=(std::string_view s) {
  *this = flexint(s);
  return *this;
}


flexint & flexint::operator=(z3i::mpz const & n) {
  m().set(m_num, n);
  return *this;
}


void flexint::reset() {
  m().reset(m_num);
}


void flexint::swap(flexint & n) noexcept {
  m().swap(m_num, n.m_num);
}


void flexint::swap(z3i::mpz & n) noexcept {
  m().swap(m_num, n);
}


flexint & flexint::operator+=(z3i::mpz const & a) {
  m().add(m_num, a, m_num);
  return *this;
}


flexint & flexint::operator-=(z3i::mpz const & a) {
  m().sub(m_num, a, m_num);
  return *this;
}


flexint & flexint::operator*=(z3i::mpz const & a) {
  m().mul(m_num, a, m_num);
  return *this;
}


flexint & flexint::operator/=(z3i::mpz const & a) {
  m().machine_div(m_num, a, m_num);
  return *this;
}


flexint & flexint::operator%=(z3i::mpz const & a) {
  m().rem(m_num, a, m_num);
  return *this;
}


bool operator==(flexint const & a, z3i::mpz const & b) {
  return a.m().eq(a, b);
}


std::strong_ordering operator<=>(flexint const& a, z3i::mpz const& b) {
  return a.m().cmp(a, b) <=> 0;
}


bool flexint::is_zero() const {
  return m().is_zero(*this);
}


bool is_zero(flexint const & a) {
  return a.m().is_zero(a);
}


void swap(flexint& a, flexint& b) noexcept {
  a.swap(b);
}


std::string flexint::str() const {
  return m().to_string(m_num);
}


std::string to_string(flexint const& a) {
  return a.str();
}


flexint operator+(flexint const & r1, z3i::mpz const & r2) {
  flexint r;
  r.m().add(r1.get(), r2, r.get());
  return r;
}


flexint operator-(flexint const & r1, z3i::mpz const & r2) {
  flexint r;
  r.m().sub(r1.get(), r2, r.get());
  return r;
}


flexint operator*(flexint const & r1, z3i::mpz const & r2) {
  flexint r;
  r.m().mul(r1.get(), r2, r.get());
  return r;
}


flexint operator/(flexint const & r1, z3i::mpz const & r2) {
  flexint r;
  r.m().machine_div(r1.get(), r2, r.get());
  return r;
}


flexint operator%(flexint const & r1, z3i::mpz const & r2) {
  flexint r;
  r.m().rem(r1.get(), r2, r.get());
  return r;
}


std::ostream & operator<<(std::ostream & out, flexint const & s) {
  s.m().display(out, s);
  return out;
}


} // namespace calendar_converter
