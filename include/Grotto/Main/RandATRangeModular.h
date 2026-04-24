#pragma once

// Includes the endpoints.
// Gets a random number from AT via the usual rand() % range + min trick.
// Doesn't perform any range checks: AT always advances.
// Looks like a more generic function, but this version is only used
// by grotto functions so we put it here.
unsigned int RandATRangeModular(unsigned int minimum, unsigned int maximum);