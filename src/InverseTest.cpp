// InverseTest.cpp
// -- simple test of the 'Inverse' function
// cs200 1/19


#include <iostream>
#include "Camera.h"
using namespace std;


ostream& operator<<(ostream& s, const glm::mat4 &A) {
  if (cs200::isAffine(A)) {
    s << "linear={{" << A[0][0] << ',' << A[1][0]
      << "},{" << A[0][1] << ',' << A[1][1] << "}}, "
      << "trans=<" << A[3][0] << ',' << A[3][1] << '>';
  }
  else {
    s << '{';
    for (int i=0; i < 4; ++i) {
      s << '{';
      for (int j=0; j < 4; ++j)
        s << A[j][i] << ((j < 3) ? ',' : '}');
      s << ((i < 3) ? ',' : '}');
    }
  }
  return s;
}


int main(void) {

  glm::mat4 A = cs200::affine(cs200::vector(1,3),
                              cs200::vector(2,4),
                              cs200::point(5,6)),
            iA = cs200::affineInverse(A);

  cout << "A = " << A << endl;
  cout << "A^-1 = " << iA << endl;

  return 0;
}

