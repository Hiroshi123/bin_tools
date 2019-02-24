
class B {
  public:
  int f1();
};

// _Z2f1v

int f1() {
  return 3;
}

class C {
  int ca = 1;
};

class A : C {
  int a = 4;
  int b = 5;
  int c = 2;
  long cc = 11;
  B* bc;
  static int d;
  A();
  int f1();
  int f1(int);
  int f2();
  static int f3();
};

// _ZN1A1cE
A::A(){}

// _ZN1A2f1Ev
int A::f1() {
  //c = 2;
  return a+b+cc;
}

// _ZN1A2f1Ei
int A::f1(int a) {
  // B* bbb = new B;
  B b;
  b.f1();
  // bc->f1();
  return a+1;
}

// _ZN1A2f2Ev
int A::f2() {
  return a;
}

// _ZN1A2f3Ev
int A::f3() {
  return 1;
}

