// Copyright 2022 vlados2003
#include <chrono>
#include <ctime>
#include <cstdint>
#include "Randomizer.h"
Randomizer::Randomizer(unsigned int _id)
{
  id = _id;
}
Randomizer::~Randomizer()
{
  delete(seed);
}
char* Randomizer::get()
{
  std::random_device dev;
  seed = new std::seed_seq
      {
          std::chrono::high_resolution_clock::now()
              .time_since_epoch().count(),
          time(0),
          (int64_t)dev(),
          (int64_t)dev(),
          (int64_t)dev(),
          (int64_t)id
      };
  std::mt19937 r(*seed);
  std::uniform_int_distribution<std::mt19937::result_type> dist(1, maxl);
  std::uniform_int_distribution<std::mt19937::result_type> rand(0, cc-1);
  int l = dist(r);
  char* word = new char[l+1];
  for (int i = 0; i != l; i++) {
    char c = Randomizer::characters[rand(r)];
    word[i] = c;
  }
  word[l] = '\0';
  return word;
}
