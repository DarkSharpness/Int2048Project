#pragma once
#include "int2048.h"


/* Implementation of all operators of int2048. */
namespace dark {

bool int2048::operator !(void) const noexcept { return this->is_non_zero(); }

int2048_view int2048::operator + (void) & noexcept { return  int2048_view(*this); }
int2048_view int2048::operator - (void) & noexcept { return -int2048_view(*this); }

int2048 int2048::operator + (void) && noexcept { return  std::move(*this); }
int2048 int2048::operator - (void) && noexcept { return std::move(this->reverse()); }

int2048 operator + (int2048_view lhs, int2048 &&rhs) { return std::move(rhs += lhs); }
int2048 operator + (int2048 &&lhs, int2048_view rhs) { return std::move(lhs += rhs); }
int2048 operator + (int2048 &&lhs, int2048 &&rhs) { return std::move(lhs += std::move(rhs)); }

int2048 operator - (int2048_view lhs, int2048_view rhs) { return lhs + (-rhs); }
int2048 operator - (int2048_view lhs, int2048 &&rhs) { return -std::move(rhs -= lhs); }
int2048 operator - (int2048 &&lhs, int2048_view rhs) { return std::move(lhs -= rhs);  }
int2048 operator - (int2048 &&lhs, int2048 &&rhs) { return std::move(lhs -= std::move(rhs)); }

int2048 operator * (int2048_view lhs, int2048 &&rhs) { return std::move(rhs *= lhs); }
int2048 operator * (int2048 &&lhs, int2048_view rhs) { return std::move(lhs *= rhs); }
int2048 operator * (int2048 &&lhs, int2048 &&rhs) { return std::move(lhs *= std::move(rhs)); }

} // namespace dark
