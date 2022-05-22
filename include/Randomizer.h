// Copyright 2022 vlados2003

#ifndef LAB10_RANDOMIZER_H
#define LAB10_RANDOMIZER_H
#include <random>

class Randomizer {
 public:
  Randomizer(unsigned int id);
  ~Randomizer();
  char* get();

 private:
  unsigned int id;
  const char* characters
      = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  const int cc=62;
  const int maxl=16;
  std::seed_seq* seed;
};

#endif  // LAB10_RANDOMIZER_H

