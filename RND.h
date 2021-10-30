#pragma once

#include <stdint.h>

uint32_t
Noise1D(int position, uint32_t seed = 0)
{
  constexpr unsigned int BIT_NOISE1 = 0xB5297A4D;
  constexpr unsigned int BIT_NOISE2 = 0x68E31DA4;
  constexpr unsigned int BIT_NOISE3 = 0x1B56C4E9;

  unsigned int mangled = position;
  mangled *= BIT_NOISE1;
  mangled += seed;
  mangled ^= (mangled >> 8);
  mangled += BIT_NOISE2;
  mangled ^= (mangled << 8);
  mangled *= BIT_NOISE2;
  mangled ^= (mangled >> 8);

  return mangled;
}

inline uint32_t
Noise2D(int posX, int posY, uint32_t seed = 0)
{
  constexpr int PRIME_NUMBER = 198491317;
  return Noise1D(posX + (PRIME_NUMBER * posY), seed);
}

inline uint32_t
Noise3D(int posX, int posY, int posZ, uint32_t seed = 0)
{
  constexpr int PRIME_NUMBER1 = 198491317;
  constexpr int PRIME_NUMBER2 = 6542989;
  return Noise1D(posX + (PRIME_NUMBER1 * posY) 
    + (PRIME_NUMBER2 * posZ), seed);
}

uint32_t
Rand()
{
  constexpr int PRIME_NUMBER = 198491317;
  return Noise1D(PRIME_NUMBER);
}